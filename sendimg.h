#ifndef SENDIMG_H
#define SENDIMG_H


#include <QThread>
#include <QMutex>
#include <QImage>
#include <QWaitCondition>
#include <QAtomicInt>

class SendImg : public QThread
{
    Q_OBJECT
public:
    explicit SendImg(QObject *parent = nullptr);
    ~SendImg();

    void setLocalAddress(quint32 ip , quint16 port);

    void pushLastestImage(const QImage &img);

    void setCameraStatus(bool status);

    void clearImage();

    void stop();

protected:
    void run() override;


private:
    quint32 m_ip = 0;
    quint16 m_port = 0;
    QMutex m_mutex;
    QWaitCondition m_condition;
    QImage m_pendingImg;
    QAtomicInt m_cameraOn;
    QAtomicInt m_quit;
    bool m_hasLocalAddress;

};


#endif // SENDIMG_H
