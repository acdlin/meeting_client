#ifndef AUDIOOUTPUT_H
#define AUDIOOUTPUT_H

#include <QObject>
#include <QAudioOutput>
#include <QIODevice>
#include <qDebug>


class AudioOutput : public QObject
{
    Q_OBJECT
public:
    explicit AudioOutput(QObject * parent = nullptr);
    ~AudioOutput();
    void startPlay();
    void stopPlay();
    void writePcm(const QByteArray &pcm);
    void setVolume(qreal v);

private:
    QAudioOutput    *m_audioOutput;
    QIODevice       *m_device;
    QAudioFormat     m_format;
    QByteArray       m_buffer;

signals:
    void audioError(QAudio::Error err);
};

#endif // AUDIOOUTPUT_H
