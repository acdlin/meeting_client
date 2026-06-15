#include "mytcpsocket.h"
#include <cstring>
#include <QHostAddress>
#include <QThread>

MyTcpSocket::MyTcpSocket(QObject *parent) : QTcpSocket(parent)
{
    connect(this , &QTcpSocket::readyRead , this , &MyTcpSocket::handleReadyRead);
    connect(this , &QTcpSocket::connected , this  , [=](){
        emit connectedInfo(this->localAddress().toIPv4Address() , this->localPort());
    });
    connect(this , &QTcpSocket::errorOccurred , this , [=](){
        emit errorInfo(errorString());
    });
}

void MyTcpSocket::close()
{
    QTcpSocket::close();
}

void MyTcpSocket::connectToServer(const QString& ip , quint16 port)
{
    if(state() != QTcpSocket::UnconnectedState)
    {
        QTcpSocket::disconnectFromHost();
    }
    connectToHost(ip , port);

}



void MyTcpSocket::handleReadyRead()
{
    m_recv_buf.append(this->readAll());
    while (true) {
        // 1. 找帧头 $
        int start = m_recv_buf.indexOf('$');

        if (start == -1) break;

        // 2. $ 后面至少要有 header(11字节) + 1字节的 # 尾部
        if (m_recv_buf.size() - start < 12) break;   // 头都不够，等

        // 3. 从 header 读 len
        const char* p = m_recv_buf.constData() + start + 1;
        quint16 n_type;
        memcpy(&n_type, p + 0, 2);
        quint32 n_ip;
        memcpy(&n_ip, p + 2, 4);
        quint32 n_len;
        memcpy(&n_len, p + 6, 4);
        quint32 dataLen = qFromBigEndian(n_len);

        int totalLen = 11 + dataLen + 1;   // header + data + #

        // 4. 整个帧到了没
        if (m_recv_buf.size() - start < totalLen) break;

        // 5. 检查尾部 #
        if (m_recv_buf.at(start + 11 + dataLen) != '#') {
            qDebug() << "尾部校验失败, 丢弃$位置:" << start;
            m_recv_buf.remove(0, start + 1);
            continue;
        }

        // 6. 取出完整帧，解析
        QByteArray frame = m_recv_buf.mid(start, totalLen);
        sendFrame(frame);
        m_recv_buf.remove(0, start + totalLen);

    }

}

void MyTcpSocket::sendFrame(const QByteArray& frameByte)
{
    quint16 n_type;
    memcpy(&n_type, frameByte.constData() + 1, 2);

    if(frameByte.size() < 12)
    {
        return ;
    }
    const char* pointer = frameByte.constData() + 1;

    quint16 type = qFromBigEndian(n_type);
    msgType msg_type = static_cast<msgType>(type);
    quint32 n_ip;
    memcpy(&n_ip , pointer + 2 , 4);
    quint32 ip = qFromBigEndian(n_ip);
    quint32 n_len;
    memcpy(&n_len , pointer + 2 + 4 , 4);
    quint32 len = qFromBigEndian(n_len);
    if(len + 11 != frameByte.size() - 1)
    {
        qDebug() << "sendFrame 丢弃: 长度不匹配, len=" << len
                 << " frame.size=" << frameByte.size();
        return;
    }
    QByteArray data = frameByte.mid(11 , len);
    emit messageRecevied(msg_type , ip , data);
}
