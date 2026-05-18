#ifndef CHATMESSAGE_H
#define CHATMESSAGE_H

#include <QWidget>

class ChatMessage : public QWidget
{
    Q_OBJECT
public:
    explicit ChatMessage(QWidget *parent = nullptr);

signals:
};

#endif // CHATMESSAGE_H
