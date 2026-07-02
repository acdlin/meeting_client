#include "writeworker.h"
#include "logqueue.h"
#include "blockqueue.h"
#include "netheader.h"
#include <QTcpSocket>
#include <QThread>

WriteWorker::WriteWorker(QTcpSocket *socket, QObject *parent)
    : QObject{parent} , m_socket(socket)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(30);
    connect(m_timer , &QTimer::timeout , this , &WriteWorker::drainOnce);
}

void WriteWorker::start()
{
    m_timer->start();
}

void WriteWorker::stop()
{
    m_timer->stop();
}

void WriteWorker::drainOnce()
{
    MESG msg;
    while(queue_send.try_pop(msg))
    {
        if(m_socket->state() != QAbstractSocket::ConnectedState)
        {
            break;
        }

        // socket buffer 积压检查
        if(m_socket->bytesToWrite() > 200 * 1024)
        {
            // 视频帧：可丢，直接丢弃腾出队列空间
            if(msg.msg_type == msgType::IMG_SEND)
            {
                LOG_DEBUG("socket buffer full, drop video frame");
                continue;
            }

            if(msg.msg_type == msgType::AUDIO_SEND)
            {
                queue_send.push_front(msg);
                continue;
            }

            queue_send.push_front(msg);
            break;
        }

        m_socket->write(packMessage(msg));
    }
}
