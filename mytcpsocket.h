#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QTcpSocket>
#include <QtEndian>
#include "netheader.h"
#include "blockqueue.h"

class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    explicit MyTcpSocket(QObject * parent = nullptr);
    void close() override;
    void sendFrame(const QByteArray&);

private:
    QByteArray m_recv_buf;


public slots:
    void connectToServer(const QString& ip , quint16 port);

private slots:
    void handleReadyRead();

signals:
    void connectedInfo(quint32 ip , quint16 port);
    void errorInfo(QString errorInfo);
    void dataReady();
};

#endif // MYTCPSOCKET_H
