#include "widget.h"
#include "ui_widget.h"
#include "ChatMessage.h"
#include "partner.h"
#include "netheader.h"
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QScreen>
#include <QTcpSocket>
#include <QTimer>

quint64 makeKey(quint32 ip , quint16 port)
{
    return (static_cast<quint64>(ip)<<16)|port;
}


Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget),m_avatar(":/images/avator.jpg"),roommember_count(1)
{
    m_socket = new MyTcpSocket();
    if(m_avatar.isNull())
    {
        qDebug() << "头像加载失败，检查qrc路径和文件名";
    }


    ui->setupUi(this);
    ui->send_msg_text_edit->installEventFilter(this);
    QScreen* screen = QApplication::primaryScreen();
    QRect rect = screen->availableGeometry();
    int wid = static_cast<int>(rect.width() * 0.8);
    int hei = static_cast<int>(rect.height() * 0.8);
    resize(wid , hei);

    int min_wid = static_cast<int>(rect.width() * 0.7);
    int min_hei = static_cast<int>(rect.height() * 0.7);
    setMinimumSize(min_wid , min_hei);

    move((rect.width() - wid)/2 , (rect.height() - hei)/2 );


    QRegularExpression ipRe("^(25[0-5]|2[0-4]\\d|[01]?\\d\\d?)(\\.(25[0-5]|2[0-4]\\d|[01]?\\d\\d?)){3}$");
    ui->ip_edit->setValidator(new QRegularExpressionValidator(ipRe , this));

    QRegularExpression portRe("^\\d+$");
    ui->port_edit->setValidator(new QRegularExpressionValidator(portRe , this));

    QRegularExpression roomidRe("^[0-9]*$");
    ui->meeting_no_edit->setValidator(new QRegularExpressionValidator(roomidRe , this));

    ui->create_meeting_btn->setDisabled(true);
    ui->join_meeting_btn->setDisabled(true);
    ui->exit_meeting_btn->setDisabled(true);





    Partner* partner1 = new Partner(m_avatar , "you" , QHostAddress("127.0.0.1"), this);
    QListWidgetItem* item = new QListWidgetItem(ui->partner_list_widget);
    item->setSizeHint(partner1->sizeHint());
    ui->partner_list_widget->addItem(item);
    ui->partner_list_widget->setItemWidget(item , partner1);
    QHostAddress addr("127.0.0.1");
    quint32 ip = addr.toIPv4Address();
    quint16 port = 0;
    quint64 key = makeKey(ip , port);
    m_partnerMap.insert(key , item);








    connect(m_socket , &MyTcpSocket::connected ,this , [=](){
        QTimer::singleShot(0 , this , [=](){
            QMessageBox::information(this , "成功" , "已连接至指定位置");
        });
        ui->connect_btn->setDisabled(true);
        ui->create_meeting_btn->setDisabled(false);
        ui->join_meeting_btn->setDisabled(false);
    });
    connect(m_socket , &MyTcpSocket::errorOccurred , this , [=](QAbstractSocket::SocketError){
        QTimer::singleShot(0 , this , [=](){
            QMessageBox::warning(this , "失败" , m_socket->errorString());
        });
        ui->connect_btn->setDisabled(false);
        ui->create_meeting_btn->setDisabled(true);
        ui->join_meeting_btn->setDisabled(true);
    });
    connect(m_socket , &MyTcpSocket::disconnected , this , [=](){
        ui->connect_btn->setDisabled(false);
        ui->create_meeting_btn->setDisabled(true);
        ui->join_meeting_btn->setDisabled(true);
    });
    connect(partner1 , &Partner::sendip , ui->out_log , [=](quint32 ip){
        ui->out_log->setText(QHostAddress(ip).toString());
    });
    connect(ui->connect_btn ,&QPushButton::clicked , this , &Widget::connect_to_server);
    connect(ui->send_msg_btn , &QPushButton::clicked , this , &Widget::send_msg);
    connect(ui->create_meeting_btn , &QPushButton::clicked , this , &Widget::create_meeting);
    connect(m_socket , &MyTcpSocket::messageRecevied , this , &Widget::handleMessage);
    connect(ui->join_meeting_btn , &QPushButton::clicked , this , &Widget::join_meeting);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::connect_to_server()
{
    QString ip = ui->ip_edit->text();
    QString port = ui->port_edit->text();

    QRegularExpression ip_regex ("^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\."
                                "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\."
                                "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\."
                                "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    if(!ip_regex.match(ip).hasMatch())
    {
        QString msg = QString("错误的ip: %1").arg(ip);
        QTimer::singleShot(0 , this , [=](){
            QMessageBox::warning(this , "错误" , msg);
        });
        return;
    }

    bool ok;
    int port_num = port.toInt(&ok);
    if(!ok || port_num > 65535 || port_num < 0)
    {
        QString msg = QString("错误的port: %1").arg(port);
        QTimer::singleShot(0 , this , [=](){
            QMessageBox::warning(this , "错误" , msg);
        });
        return;
    }
    m_socket->connectToServer(ip , port_num);

}

void Widget::send_msg()
{
    QString text = ui->send_msg_text_edit->toPlainText();
    if(text.isEmpty())
    {
        return ;
    }
    ui->send_msg_text_edit->clear();



    ChatMessage* msg = new ChatMessage(text , true , m_avatar , ui->msg_list_widget);
    QListWidgetItem* item = new QListWidgetItem(ui->msg_list_widget);
    item->setSizeHint(msg->sizeHint());

    ui->msg_list_widget->addItem(item);
    ui->msg_list_widget->setItemWidget(item , msg);

    ui->msg_list_widget->scrollToBottom();

    MESG msgToSend;
    msgToSend.msg_type = msgType::TEXT_SEND;
    msgToSend.ip = m_socket->localAddress().toIPv4Address();
    msgToSend.data = qCompress(text.toUtf8());
    m_socket->write(packMessage(msgToSend));
}

void Widget::create_meeting()
{
    MESG msg;
    msg.msg_type = msgType::CREATE_MEETING;
    msg.ip = m_socket->localAddress().toIPv4Address();
    QByteArray byteArray = packMessage(msg);
    m_socket->write(byteArray);
    roommember_count = 1;
}

void Widget::handleMessage(msgType type , quint32 ip , QByteArray data)
{
    if(type == msgType::CREATE_MEETING_RESPONSE)
    {
        if(data.size() != sizeof(quint32))
        {
            return;
        }
        quint32 roomNo = qFromBigEndian<quint32>(data.constData());
        if(roomNo <= 100000)
        {
            QTimer::singleShot(0 , this , [=](){
                QMessageBox::information(this , "错误" , "创建房间失败");
            });
        }
        else
        {
            QTimer::singleShot(0 , this , [=](){
                QMessageBox::information(this , "成功" , "成功创建房间");
            });
            ui->out_log->setText(QString("已加入房间%1").arg(roomNo));
            ui->meeting_no_edit->setText(QString::number(roomNo));
            ui->create_meeting_btn->setDisabled(true);
            ui->join_meeting_btn->setDisabled(true);
            ui->exit_meeting_btn->setDisabled(false);
        }
    }
    else if(type == msgType::JOIN_MEETING_RESPONSE)
    {
        if(data.size() != sizeof(quint32))
        {
            return;
        }
        quint32 roomNo = qFromBigEndian<quint32>(data.constData());
        if(roomNo == 0)
        {
            QTimer::singleShot(0 , this , [=](){
                QMessageBox::information(this , "失败"  , "加入房间失败");
            });
        }
        else if(roomNo > 100000)
        {
            QTimer::singleShot(0 , this , [=](){
                    QMessageBox::information(this , "成功" , "成功加入房间");
            });
            ui->create_meeting_btn->setDisabled(true);
            ui->join_meeting_btn->setDisabled(true);
            ui->exit_meeting_btn->setDisabled(false);
        }
    }
    else if(type == msgType::TEXT_RECV)
    {
        QByteArray decompressed = qUncompress(data);
        QString text = QString::fromUtf8(decompressed);
        ChatMessage* msg = new ChatMessage(text , false , m_avatar , ui->msg_list_widget);
        QListWidgetItem* item = new QListWidgetItem(ui->msg_list_widget);
        item->setSizeHint(msg->sizeHint());
        ui->msg_list_widget->addItem(item);
        ui->msg_list_widget->setItemWidget(item , msg);
        ui->msg_list_widget->scrollToBottom();
    }
    else if(type == msgType::PARTNER_JOIN)
    {
        quint16 port = qFromBigEndian<quint16>(data.constData());
        quint64 key = makeKey(ip , port);
        QHostAddress addr = QHostAddress(ip);
        QString name = QString("user_%1").arg(roommember_count++);
        Partner* partner = new Partner(m_avatar , name , addr , this);
        QListWidgetItem* item = new QListWidgetItem(ui->partner_list_widget);
        item->setSizeHint(partner->sizeHint());
        ui->partner_list_widget->addItem(item);
        ui->partner_list_widget->setItemWidget(item , partner);
        m_partnerMap.insert(key , item);
    }
    else if(type == msgType::PARTNER_JOIN2)
    {
        QDataStream ds(data);
        while(true)
        {
            quint32 ip;
            quint16 port;
            ds >> ip >> port;
            if(ds.status() != QDataStream::Ok)
            {
                break;
            }
            QString name = QString("user_%1").arg(roommember_count++);
            QHostAddress addr = QHostAddress(ip);
            Partner* partner = new Partner(m_avatar , name , addr , this);
            QListWidgetItem* item = new QListWidgetItem(ui->partner_list_widget);
            item->setSizeHint(partner->sizeHint());
            ui->partner_list_widget->addItem(item);
            ui->partner_list_widget->setItemWidget(item , partner);
            quint64 key = makeKey(ip , port);
            m_partnerMap.insert(key , item);
            qDebug() << "Adding partner IP:" << QHostAddress(ip).toString()
                     << "port:" << port;
        }
    }
    else if(type == msgType::PARTNER_EXIT)
    {
        quint16 port = qFromBigEndian<quint16>(data.constData());
        quint64 key = makeKey(ip , port);
        QListWidgetItem* itemToRemove = m_partnerMap.value(key , nullptr);
        if(itemToRemove!= nullptr)
        {
            QWidget* itemWidget = ui->partner_list_widget->itemWidget(itemToRemove);
            if(itemWidget!= nullptr)
            {
                delete itemWidget;
            }
            int row = ui->partner_list_widget->row(itemToRemove);
            delete ui->partner_list_widget->takeItem(row);
            m_partnerMap.remove(key);
        }

    }
}

void Widget::join_meeting()
{
    bool ok;
    quint32 room_id = ui->meeting_no_edit->text().toUInt(&ok , 10);
    quint32 n_room_id = qToBigEndian(room_id);
    if(!ok || room_id < 100001)
    {
        QTimer::singleShot(0 , this , [=](){
            QMessageBox::information(this , "错误" , "检查房间号");
        });
        return;
    }
    MESG msg;
    msg.msg_type = msgType::JOIN_MEETING;
    msg.ip = m_socket->localAddress().toIPv4Address();
    msg.data.resize(sizeof(quint32));
    memcpy(msg.data.data() , &n_room_id , sizeof(quint32));
    QByteArray byteArray = packMessage(msg);
    m_socket->write(byteArray);
}

bool Widget::eventFilter(QObject *watched , QEvent* event)
{
    if(watched == ui->send_msg_text_edit && event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if(keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
        {
            if(keyEvent->modifiers() & Qt::ShiftModifier)
            {
                return false;
            }

            ui->send_msg_btn->click();

            return true;
        }
    }
    return QWidget::eventFilter(watched , event);
}


