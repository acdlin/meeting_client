#ifndef WIDGET_H
#define WIDGET_H

#include "mytcpsocket.h"
#include <QWidget>
#include <QMap>
#include <QListWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    Ui::Widget *ui;
    QPixmap m_avatar;
    MyTcpSocket * m_socket;
    QMap<quint64 , QListWidgetItem*> m_partnerMap;
    int roommember_count;

private slots:
    void connect_to_server();
    void send_msg();
    void create_meeting();
    void handleMessage(msgType type , quint32 ip , QByteArray data);
    void join_meeting();
};
#endif // WIDGET_H
