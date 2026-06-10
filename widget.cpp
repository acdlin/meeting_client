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
#include <QBuffer>

quint64 makeKey(quint32 ip , quint16 port)
{
    return (static_cast<quint64>(ip)<<16)|port;
}


Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget),m_avatar(":/images/avator.jpg"),roommember_count(1),m_videoCellCount(1),m_cameraOn(false),m_audioInputOn(false),m_audioOutputOn(false)
{
    m_audioOutput = new AudioOutput(this);
    m_audioInput = new AudioInput(this);
    m_camera = new QCamera(this);
    m_videosurface = new MyVideoSurface;
    m_camera->setViewfinder(m_videosurface);
    m_socket = new MyTcpSocket();
    if(m_avatar.isNull())
    {
        qDebug() << "头像加载失败，检查qrc路径和文件名";
    }


    ui->setupUi(this);
    ui->scrollArea->setWidgetResizable(true);
    QWidget *container = ui->scrollArea->widget();
    m_gridLayout = new QGridLayout(container);
    m_gridLayout->setContentsMargins(4 , 4 , 4 , 4);
    m_gridLayout->setSpacing(4);


    m_selfCell = new VideoCell("you" , container);
    m_gridLayout->addWidget(m_selfCell , 0 , 0);
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
    ui->open_camera_btn->setDisabled(true);
    ui->open_audio_out_btn->setDisabled(true);
    ui->open_audio_btn->setDisabled(true);





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

    connect(m_audioInput , &AudioInput::audioError , this , [=](QAudio::Error err){
        QTimer::singleShot(0 , this , [=](){
            QMessageBox::warning(this , "错误" ,QString("声音输入设备错误：%1").arg(err));
        });
        if(m_audioInputOn)
        {
            m_audioInputOn = false;
            m_audioInput->stopCollect();
            ui->open_audio_btn->setText("打开麦克风");
        }
    });

    connect(m_audioOutput , &AudioOutput::audioError , this , [=](QAudio::Error err){
        QTimer::singleShot(0 , this , [=](){
            QMessageBox::warning(this , "错误" , QString("音频输出设备错误:%1").arg(err));
        });
        if(m_audioOutputOn)
        {
            m_audioOutputOn = false;
            m_audioOutput->stopPlay();
            ui->open_audio_out_btn->setText("打开听筒");
        }
    });

    connect(ui->open_audio_btn , &QPushButton::clicked , this , [=](){
        if(!m_audioInputOn)
        {
            m_audioInputOn = true;
            ui->open_audio_btn->setText("关闭麦克风");
            ui->audio_slider->setValue(70);
            m_audioInput->startCollect();
        }
        else
        {
            m_audioInputOn = false;
            ui->open_audio_btn->setText("打开麦克风");
            ui->audio_slider->setValue(0);
            m_audioInput->stopCollect();
        }
    });
    connect(ui->open_audio_out_btn , &QPushButton::clicked , this , [=](){
        if(!m_audioOutputOn)
        {
            m_audioOutputOn = true;
            ui->open_audio_out_btn->setText("关闭听筒");
            ui->audio_out_slider->setValue(70);
            m_audioOutput->startPlay();
        }
        else
        {
            m_audioOutputOn = false;
            ui->open_audio_out_btn->setText("打开听筒");
            ui->audio_out_slider->setValue(0);
            m_audioOutput->stopPlay();
        }
    });
    connect(ui->audio_slider , &QSlider::valueChanged , this , [=](){
        m_audioInput->setVolume(ui->audio_slider->value()/100.0);
    });
    connect(ui->audio_out_slider , &QSlider::valueChanged , this , [=](){
        m_audioOutput->setVolume(ui->audio_out_slider->value()/100.0);
    });
    connect(m_audioInput , &AudioInput::audioDataReady , this , & Widget::onAudioData);
    connect(ui->exit_meeting_btn , &QPushButton::clicked , this , [=](){
        MESG msg;
        msg.msg_type = msgType::EXIT_MEETING;
        msg.ip = m_socket->localAddress().toIPv4Address();
        m_socket->write(packMessage(msg));
        if(m_audioInputOn)
        {
            m_audioInput->stopCollect();
            m_audioInputOn = false;
            ui->open_audio_btn ->setText("打开麦克风");
            ui->audio_slider->setValue(0);
            ui->open_audio_btn->setDisabled(true);
        }
        if(m_audioOutputOn)
        {
            m_audioOutput->stopPlay();
            m_audioOutputOn = false;
            ui->open_audio_out_btn->setText("打开听筒");
            ui->audio_out_slider->setValue(0);
            ui->open_audio_out_btn->setDisabled(true);
        }

        if(m_cameraOn)
        {
            m_camera->stop();
            m_cameraOn = false;
            ui->open_camera_btn->setText("打开摄像头");
            ui->open_camera_btn->setDisabled(true);
        }

        ui->meeting_no_edit->clear();
        ui->exit_meeting_btn->setDisabled(true);
        ui->msg_list_widget->clear();

        for(auto it = m_partnerMap.begin(); it != m_partnerMap.end();)
        {
            quint64 key = it.key();
            if(key == makeKey(QHostAddress("127.0.0.1").toIPv4Address() , 0))
            {
                it++;
                continue;
            }
            QListWidgetItem *item = it.value();
            QWidget *w = ui->partner_list_widget->itemWidget(item);
            if(w) delete w;
            int row = ui->partner_list_widget->row(item);
            delete ui->partner_list_widget->takeItem(row);
            it = m_partnerMap.erase(it);
        }

        for(auto it = m_videoCellMap.begin() ; it != m_videoCellMap.end();)
        {
            VideoCell *cell = it.value();
            m_gridLayout->removeWidget(cell);
            delete cell;
            it = m_videoCellMap.erase(it);
        }
        roommember_count = 1;
    });

    connect(ui->open_camera_btn , &QPushButton::clicked , this , [=](){
        if(m_camera->status() == QCamera::ActiveStatus )
        {
            m_cameraOn = false;
            ui->open_camera_btn->setText("打开摄像头");
            ui->out_log->setText("camera stop");
            m_camera->stop();
            m_selfCell->clearImage();
            MESG msg;
            msg.ip = m_socket->localAddress().toIPv4Address();
            msg.msg_type = msgType::CLOSE_CAMERA;
            quint16 port = m_socket->localPort();
            quint16 n_port = qToBigEndian(port);
            msg.data.append(reinterpret_cast<const char*>(&n_port) , sizeof(n_port));
            m_socket->write(packMessage(msg));
        }
        else
        {
            m_cameraOn = true;
            ui->open_camera_btn->setText("关闭摄像头");
            ui->out_log->setText("camera start");
            m_camera->start();
        }
    });

    connect(m_videosurface , &MyVideoSurface::frameAvailable , this , &Widget::handle_frame);

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
        ui->open_camera_btn->setDisabled(true);
        ui->open_audio_out_btn->setDisabled(true);
        ui->open_audio_btn->setDisabled(true);

    });
    connect(m_socket , &MyTcpSocket::disconnected , this , [=](){
        ui->connect_btn->setDisabled(false);
        ui->create_meeting_btn->setDisabled(true);
        ui->join_meeting_btn->setDisabled(true);
        ui->open_camera_btn->setDisabled(true);
        ui->open_audio_out_btn->setDisabled(true);
        ui->open_audio_btn->setDisabled(true);
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
    quint16 port = m_socket->localPort();
    quint16 n_port = qToBigEndian(port);
    msgToSend.data.append(reinterpret_cast<const char*>(&n_port) , sizeof(n_port));
    msgToSend.data.append(qCompress(text.toUtf8()));
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
            ui->open_camera_btn->setDisabled(false);
            ui->open_audio_out_btn->setDisabled(false);
            ui->open_audio_btn->setDisabled(false);
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
            ui->open_camera_btn->setDisabled(false);
            ui->open_audio_out_btn->setDisabled(false);
            ui->open_audio_btn->setDisabled(false);
        }
    }
    else if(type == msgType::TEXT_RECV)
    {
        //quint16 port = qFromBigEndian<quint16>(data.constData());

        QByteArray decompressed = qUncompress(data.mid(2));
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
        qDebug() << QHostAddress(ip).toString() << ":" << port;
        quint64 key = makeKey(ip , port);
        QHostAddress addr = QHostAddress(ip);
        QString name = QString("user_%1").arg(roommember_count++);
        Partner* partner = new Partner(m_avatar , name , addr , this);
        QListWidgetItem* item = new QListWidgetItem(ui->partner_list_widget);
        item->setSizeHint(partner->sizeHint());
        ui->partner_list_widget->addItem(item);
        ui->partner_list_widget->setItemWidget(item , partner);
        m_partnerMap.insert(key , item);

        VideoCell *cell = new VideoCell(name , ui->show_scroll_area);
        int cols = 2;
        int row = (m_videoCellCount) / cols;
        int col = (m_videoCellCount) % cols;
        m_gridLayout->addWidget(cell , row , col);
        m_videoCellMap.insert(key , cell);
        m_videoCellCount++;

        qDebug() << "PARTNER_JOIN: ip =" << QHostAddress(ip).toString()
                 << "port =" << port << "key =" << key;
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
            qDebug() << QHostAddress(ip).toString() << ":" << port;
            quint64 key = makeKey(ip , port);
            m_partnerMap.insert(key , item);

            VideoCell *cell = new VideoCell(name , ui->show_scroll_area);
            int cols = 2;
            int row = (m_videoCellCount) / cols;
            int col = (m_videoCellCount) % cols;
            m_gridLayout->addWidget(cell , row , col);
            m_videoCellMap.insert(key , cell);
            m_videoCellCount++;
            qDebug() << "PARTNER_JOIN: ip =" << QHostAddress(ip).toString()
                     << "port =" << port << "key =" << key;
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
        VideoCell* cell = m_videoCellMap.value(key , nullptr);
        if(cell != nullptr)
        {
            m_gridLayout->removeWidget(cell);
            delete cell;
            m_videoCellMap.remove(key);
            m_videoCellCount--;
        }
    }
    else if(type == msgType::CLOSE_CAMERA)
    {
        quint16 port = qFromBigEndian<quint16>(data.constData());
        quint64 key = makeKey(ip , port);
        VideoCell* cell = m_videoCellMap.value(key , nullptr);
        if(cell != nullptr)
        {
            cell->clearImage();
        }
    }
    else if(type == msgType::IMG_RECV)
    {
        quint16 port = qFromBigEndian<quint16>(data.constData());
        qDebug() << "RECV ：" << QHostAddress(ip).toString()
                 << "port =" << port ;
        quint64 key = makeKey(ip, port);
        QByteArray compressed = data.mid(2);

        QByteArray jpg = qUncompress(compressed);

        QImage img = QImage::fromData(jpg);

        VideoCell* cell = m_videoCellMap.value(key, nullptr);
        if(cell != nullptr)
        {
            cell->setImage(img);
        }
    }
    else if(type == msgType::AUDIO_RECV)
    {
        if(!m_audioOutputOn)
        {
            return;
        }
        //quint16 port = qFromBigEndian<quint16>(data.constData());
        QByteArray pcm = qUncompress(data.mid(2));
        m_audioOutput->writePcm(pcm);
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

void Widget::handle_frame(QImage image)
{
    if(!m_cameraOn)
    {
        return ;
    }
    m_selfCell->setImage(image);
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer , "JPEG" , 50);

    QByteArray compressed = qCompress(buffer.data());

    MESG msg;
    msg.msg_type = msgType::IMG_SEND;
    msg.ip = m_socket->localAddress().toIPv4Address();
    quint16 port = m_socket->localPort();
    quint16 n_port = qToBigEndian(port);
    msg.data.append(reinterpret_cast<const char *>(& n_port), sizeof(n_port));
    msg.data.append(compressed);
    m_socket->write(packMessage(msg));
}

void Widget::onAudioData(const QByteArray pcm)
{
    MESG msg;
    msg.ip = m_socket->localAddress().toIPv4Address();
    msg.msg_type = msgType::AUDIO_SEND;
    quint16 port = m_socket->localPort();
    quint16 n_port = qToBigEndian(port);
    msg.data.append(reinterpret_cast<const char*>(&n_port) , sizeof(n_port));
    QByteArray compressed = qCompress(pcm);
    msg.data.append(compressed);
    m_socket->write(packMessage(msg));
}


