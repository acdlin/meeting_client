#include "sendimg.h"
#include "netheader.h"
#include "blockqueue.h"
#include "logqueue.h"
#include <QBuffer>
#include <QString>
#include <QtEndian>


SendImg::SendImg(QObject *parent)
    : QThread{parent},m_ip(0),m_port(0), m_cameraOn(false) , m_hasLocalAddress(false)
{}

SendImg::~SendImg()
{
    stop();
}

void SendImg::setLocalAddress(quint32 ip  , quint16 port)
{
    QMutexLocker locker(&m_mutex);
    m_ip = ip;
    m_port = port;
    m_hasLocalAddress = true;
}

void SendImg::setCameraStatus(bool status)
{
    m_cameraOn = status;
}

void SendImg::pushLastestImage(const QImage& img)
{
    {
        QMutexLocker locker(&m_mutex);
        m_pendingImg = img;
    }
    m_condition.wakeAll();
}

void SendImg::clearImage()
{
    QMutexLocker locker(&m_mutex);
    m_pendingImg = QImage();
}



void SendImg::stop()
{
    m_quit = true;
    {
        QMutexLocker locker(&m_mutex);
        m_condition.wakeAll();
    }

    if(isRunning()){
        wait();
    }
}

void SendImg::run()
{
    while(!m_quit)
    {
        QImage imageToSend;
        {
            QMutexLocker locker(&m_mutex);
            while(m_pendingImg.isNull() && !m_quit)
            {
                m_condition.wait(&m_mutex);
            }
            if(m_quit)
            {
                break;
            }
            if(!m_cameraOn || !m_hasLocalAddress)
            {
                m_pendingImg = QImage();
                continue;
            }
            imageToSend = m_pendingImg;
            m_pendingImg = QImage();
        }
        QByteArray jpgData;
        QBuffer buffer(&jpgData);
        buffer.open(QIODevice::WriteOnly);
        imageToSend.save(&buffer , "JPEG" , 50);

        MESG msg ;
        msg.ip = m_ip;
        msg.msg_type = msgType::IMG_SEND;
        quint16 n_port = qToBigEndian(m_port);
        msg.data.append(reinterpret_cast<const char*>(&n_port) , sizeof(n_port));
        msg.data.append(jpgData);

        if(queue_send.size() > 50)
        {
            LOG_DEBUG(QString("queue_send too full , drop frame , queue size = %1").arg(queue_send.size()));
            continue;
        }
        queue_send.push_msg(msg);

    }
}
