#ifndef LOGQUEUE_H
#define LOGQUEUE_H
#include "blockqueue.h"
#include <QString>
#include <QThread>
#include <QAtomicInt>


struct LogEntry{
    QString msg;
    QString file;
    QString func;
    int line;
};


extern QUEUE_DATA<LogEntry> queue_log;

class LogWriter : public QThread
{
    Q_OBJECT
public:
    void stop();

protected:
    void run() override;

private:
    QAtomicInt m_quit = false;
};

#define WRITE_LOG(message) do{\
LogEntry e;\
e.msg = message;\
e.file = __FILE__;\
e.func = __FUNCTION__;\
e.line = __LINE__;\
queue_log.push_msg(e);\
}while(0)

#define LOG_DEBUG(msg) WRITE_LOG(msg)
#define LOG_INFO(msg)  WRITE_LOG(msg)
#define LOG_WARN(msg)  WRITE_LOG(msg)
#define LOG_ERROR(msg) WRITE_LOG(msg)



#endif // LOGQUEUE_H
