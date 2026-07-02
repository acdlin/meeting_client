#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H

#include "netheader.h"
#include <QQueue>
#include <QMutexLocker>
#include <QMutex>
#include <QWaitCondition>

template<typename T>
class QUEUE_DATA
{

public:
    explicit QUEUE_DATA(int setSize):maxSize(setSize){}
    ~QUEUE_DATA()=default;
    QUEUE_DATA(const QUEUE_DATA&) = delete;
    QUEUE_DATA& operator=(const QUEUE_DATA&) = delete;
    QUEUE_DATA(QUEUE_DATA&&) = delete;
    QUEUE_DATA&& operator=(QUEUE_DATA&&) = delete;

    void clear()
    {
        QMutexLocker locker(&m_mutex);
        m_queue.clear();
        m_notFull.wakeAll();
    }

    void push_msg(const T&t)
    {
        QMutexLocker locker(&m_mutex);
        while(m_queue.size() >= maxSize)
        {
            m_notFull.wait(&m_mutex);
        }
        m_queue.push_back(t);
        m_notEmpty.wakeOne();
    }

    void push_front(const T& t)
    {
        QMutexLocker locker(&m_mutex);
        m_queue.push_front(t);
        m_notEmpty.wakeOne();
    }


    bool pop_msg(T& out)
    {
        QMutexLocker locker(&m_mutex);
        while(m_queue.isEmpty())
        {
            if(!m_notEmpty.wait(&m_mutex,2000))
            {
                return false;
            }
        }
        out = m_queue.dequeue();
        m_notFull.wakeOne();
        return true;
    }

    bool try_pop(T& out)
    {
        QMutexLocker locker(&m_mutex);
        if(m_queue.isEmpty())
        {
            return false;
        }
        out = m_queue.dequeue();
        m_notFull.wakeOne();
        return true;
    }

    int size()
    {
        QMutexLocker locker(&m_mutex);
        return m_queue.size();
    }



private:
    int maxSize;
    QMutex m_mutex;
    QQueue<T> m_queue;
    QWaitCondition m_notFull;
    QWaitCondition m_notEmpty;

};



extern QUEUE_DATA<MESG> queue_send;
extern QUEUE_DATA<MESG> queue_recv;

#endif // BLOCKQUEUE_H
