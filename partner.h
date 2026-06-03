#ifndef PARTNER_H
#define PARTNER_H

#include <QWidget>
#include <QHostAddress>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QMouseEvent>

class Partner : public QWidget
{
    Q_OBJECT
public:
    explicit Partner(const QPixmap & img , const QString& userName = "user", const QHostAddress & userAddress = QHostAddress()  ,  QWidget *parent = nullptr);

signals:
    void sendip(quint32);

protected:
    void mousePressEvent(QMouseEvent* event)override;

private:
    QString m_userName;
    QHostAddress m_userAddress;

private slots:

};

#endif // PARTNER_H
