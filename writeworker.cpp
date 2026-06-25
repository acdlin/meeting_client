#include "writeworker.h"
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
        if(m_socket->state() == QAbstractSocket::ConnectedState)
        {
            m_socket->write(packMessage(msg));
        }
    }
}
