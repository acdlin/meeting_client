#include "partner.h"

Partner::Partner(const QPixmap& img , const QString &userName, const QHostAddress &userAddress, QWidget *parent)
    : m_userName{userName} , m_userAddress{userAddress} ,  QWidget{parent}
{
    QPixmap avator = img;
    QPixmap target(40 , 40);
    QPainter painter(&target);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawPixmap(0 , 0 , 40 , 40 , avator.scaled(40 , 40 , Qt::KeepAspectRatioByExpanding , Qt::SmoothTransformation));
    QHBoxLayout* layout = new QHBoxLayout(this);
    QLabel* avatorlab = new QLabel(this);
    QLabel* name = new QLabel(this);
    QLabel* ip = new QLabel(this);
    name->setText(userName);
    QString ip_str = userAddress.toString();
    avatorlab->setPixmap(target);
    ip->setText(ip_str);
    layout->addWidget(avatorlab);
    layout->addWidget(name);
    layout->addWidget(ip);

}

void Partner::mousePressEvent(QMouseEvent* event)
{
    emit sendip(m_userAddress.toIPv4Address());
    QWidget::mousePressEvent(event);
}
