#include "widget.h"
#include "ui_widget.h"
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    QRegularExpression ipRe("^(25[0-5]|2[0-4]\\d|[01]?\\d\\d?)(\\.(25[0-5]|2[0-4]\\d|[01]?\\d\\d?)){3}$");
    ui->ip_edit->setValidator(new QRegularExpressionValidator(ipRe , this));

    QRegularExpression portRe("^\\d+$");
    ui->port_edit->setValidator(new QRegularExpressionValidator(portRe , this));

    connect(ui->connect_btn ,&QPushButton::clicked , this , &Widget::connect_to_server);
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
        QMessageBox::warning(this , "错误" , msg);
        return;
    }

    bool ok;
    int port_num = port.toInt(&ok);
    if(!ok || port_num > 65535 || port_num < 0)
    {
        QString msg = QString("错误的port: %1").arg(port);
        QMessageBox::warning(this , "错误" , msg);
        return;
    }


    QString msg = QString("连接到: %1 : %2").arg(ip , port);
    QMessageBox::information(this , "标题" , msg);
}
