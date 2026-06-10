#include "videocell.h"
#include <QVBoxLayout>

VideoCell::VideoCell(const QString &name , QWidget *parent): QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(4 , 4 , 4 , 4);
    layout->setSpacing(4);

    m_videoLabel = new QLabel(this);
     m_videoLabel->setMinimumSize(160, 120);
    m_videoLabel -> setAlignment(Qt::AlignCenter);
    m_videoLabel -> setStyleSheet("background-color: white;");
    layout->addWidget(m_videoLabel , 1 );

    m_nameLabel = new QLabel(name , this);
    m_nameLabel->setAlignment(Qt::AlignCenter);
    m_nameLabel->setStyleSheet("font-size: 12 px;");
    layout->addWidget(m_nameLabel , 0 );
}

void VideoCell::setImage(const QImage &image)
{
    QPixmap pix = QPixmap::fromImage(image).scaled(m_videoLabel->size(), Qt::KeepAspectRatio , Qt::SmoothTransformation);
    m_videoLabel->setPixmap(pix);
}

void VideoCell::setName(const QString &name)
{
    m_nameLabel->setText(name);
}

void VideoCell::clearImage()
{
    m_videoLabel->clear();
}
