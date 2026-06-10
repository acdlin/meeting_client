#ifndef WIDGET_H
#define WIDGET_H

#include "mytcpsocket.h"
#include "myvideosurface.h"
#include "audioinput.h"
#include "videocell.h"
#include "audiooutput.h"
#include <QWidget>
#include <QMap>
#include <QListWidgetItem>
#include <QCamera>
#include <QGridLayout>

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

protected:
    bool eventFilter(QObject * watched , QEvent *event) override;

private:
    Ui::Widget *ui;
    QPixmap m_avatar;
    MyTcpSocket * m_socket;
    QMap<quint64 , QListWidgetItem*> m_partnerMap;
    int roommember_count;
    QCamera* m_camera;
    MyVideoSurface* m_videosurface;
    QGridLayout *m_gridLayout;
    VideoCell *m_selfCell;
    QMap<quint64 , VideoCell*> m_videoCellMap;
    int m_videoCellCount;
    bool m_cameraOn;
    AudioInput *m_audioInput;
    bool m_audioInputOn;
    AudioOutput *m_audioOutput;
    bool m_audioOutputOn;

private slots:
    void connect_to_server();
    void send_msg();
    void create_meeting();
    void handleMessage(msgType type , quint32 ip , QByteArray data);
    void join_meeting();
    void handle_frame(QImage image);
    void onAudioData(const QByteArray pcm);
};
#endif // WIDGET_H
