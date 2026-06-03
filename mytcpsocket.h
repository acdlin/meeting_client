#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QTcpSocket>
#include <QtEndian>
#include "netheader.h"

class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    explicit MyTcpSocket(QObject * parent = nullptr);
    void close() override;
    void connectToServer(const QString& ip , quint16 port);
    void sendFrame(const QByteArray&);

private:
    QByteArray m_recv_buf;

private slots:
    void handleReadyRead();

signals:
    void messageRecevied(msgType type , quint32 ip, QByteArray data);
};

#endif // MYTCPSOCKET_H
