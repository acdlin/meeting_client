#include "chatmessage.h"

static QPixmap createCircleAvator(const QPixmap& avator , int size)
{
    QPixmap target(size , size);
    target.fill(Qt::transparent);
    QPainter painter(&target);
    painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addEllipse(0 , 0 , size , size);
    painter.setClipPath(path);
    painter.drawPixmap(0 , 0 , size , size , avator.scaled(size , size , Qt::KeepAspectRatioByExpanding , Qt::SmoothTransformation));
    return target;
}

ChatMessage::ChatMessage(const QString& text , bool isMe , const QPixmap& avator, QWidget *parent)
    : QWidget{parent}
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(5, 3 , 5, 3);
    layout->setSpacing(8);

    //头像部分
    QLabel *avatorLabel = new QLabel(this);
    avatorLabel->setPixmap(createCircleAvator(avator , 40));
    avatorLabel->setFixedSize(40 ,40);

    QLabel* bubble = new QLabel(this);
    QString time = QDateTime::currentDateTime().toString("hh:mm");


    bubble->setText(QString("<span style='font-size: 14px;'>%1 </span><br>"
                            "<span style='font-size: 10px; color:#666;'>%2</span>")
                        .arg(text.toHtmlEscaped(),time));
    bubble->setWordWrap(true);
    bubble->setMaximumWidth(300);
    bubble->setTextInteractionFlags(Qt::TextSelectableByMouse);

    if(isMe)
    {
        bubble->setStyleSheet(
            "QLabel { background: rgb(149, 236, 105);"
            "         border-radius: 8px;"
            "         padding: 8px 12px;"
            "         color: black; }"
            );
    }
    else
    {
        bubble->setStyleSheet(
            "QLabel { background: rgb(255, 255, 255);"
            "         border-radius: 8px;"
            "         padding: 8px 12px;"
            "         color: black; }"
            );
    }

    if(isMe)
    {
        layout->addStretch();
        layout->addWidget(bubble);
        layout->addWidget(avatorLabel);
    }
    else
    {
        layout->addWidget(avatorLabel);
        layout->addWidget(bubble);
        layout->addStretch();
    }
}
