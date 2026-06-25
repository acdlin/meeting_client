#include "logqueue.h"
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDir>
#include <QDebug>



void LogWriter::stop()
{
    m_quit = true;

    LogEntry dummy;
    dummy.line = -1;
    queue_log.push_msg(dummy);
    wait();
}

void LogWriter::run()
{

    QDir dir;
    if(!dir.exists("logs"))
    {
        dir.mkdir("logs");
    }
    QFile file("logs/app.log");
    if(!file.open(QIODevice::WriteOnly | QIODevice::Append))
    {
        qDebug() << "日志文件打开失败:" << file.errorString();
        return;
    }
    QTextStream stream(&file);
    while(!m_quit)
    {
        LogEntry e;
        if(!queue_log.pop_msg(e))
        {
            continue;
        }
        if(e.line == -1 )
        {
            break;
        }
        QString line = QString("[%1] [%2:%3 %4] %5\n")
                           .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"))
                           .arg(e.file.section('\\', -1))
                           .arg(e.line)
                           .arg(e.func)
                           .arg(e.msg);
        stream << line;
        stream.flush();
    }
    file.close();
}
