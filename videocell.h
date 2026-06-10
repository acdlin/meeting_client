#ifndef VIDEOCELL_H
#define VIDEOCELL_H
#include <QWidget>
#include <QLabel>

class VideoCell: public QWidget
{
    Q_OBJECT
public:
   explicit VideoCell(const QString &name , QWidget * parent = nullptr);

    void setImage(const QImage& image);
    void setName(const QString &name);
    void clearImage();
private:
    QLabel *m_videoLabel;
    QLabel *m_nameLabel;
};

#endif // VIDEOCELL_H
