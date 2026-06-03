#ifndef CHATMESSAGE_H
#define CHATMESSAGE_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QDatetime>
#include <qDebug>
#include <QPainter>
#include <QPainterPath>

class ChatMessage : public QWidget
{
    Q_OBJECT
public:
    explicit ChatMessage(const QString& text , bool isMe , const QPixmap& avator , QWidget *parent = nullptr);

signals:
};

#endif // CHATMESSAGE_H
