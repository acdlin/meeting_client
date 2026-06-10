#ifndef AUDIOINPUT_H
#define AUDIOINPUT_H

#include <QAudioInput>
#include <QTimer>
#include <QIODevice>
#include <QDebug>

class AudioInput : public QObject
{
    Q_OBJECT

public:
    explicit AudioInput(QObject* parent = nullptr);
    ~AudioInput();

    void startCollect();
    void stopCollect();
    void setVolume(qreal v);

signals:
    void audioDataReady(const QByteArray &pcmData);
    void audioError(QAudio::Error err);

private:
    QAudioInput     *m_audioInput;
    QIODevice       *m_device;
    QAudioFormat    m_format;
    QByteArray      m_buffer;
    QTimer          *m_timer;

private slots:
    void onReadyRead();
};

#endif // AUDIOINPUT_H
