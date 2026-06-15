#ifndef WRITEWORKER_H
#define WRITEWORKER_H

#include <QObject>
#include <QTimer>

class QTcpSocket;
class WriteWorker : public QObject
{
    Q_OBJECT
public:
    explicit WriteWorker(QTcpSocket* socket , QObject *parent = nullptr);

public slots:
    void start();
    void stop();

private slots:
    void drainOnce();

private:
    QTcpSocket* m_socket;
    QTimer* m_timer;

};

#endif // WRITEWORKER_H
