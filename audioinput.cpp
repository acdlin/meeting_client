#include "audioinput.h"

AudioInput::AudioInput(QObject* parent): QObject(parent)
{
    m_format.setSampleRate(8000);
    m_format.setSampleSize(16);
    m_format.setChannelCount(1);
    m_format.setCodec("audio/pcm");
    m_format.setByteOrder(QAudioFormat::LittleEndian);
    m_format.setSampleType(QAudioFormat::SignedInt);

    m_timer = new QTimer(this);
    m_audioInput = new QAudioInput(m_format , this);
    m_audioInput->setVolume(0.7);
    connect(m_timer , &QTimer::timeout , this , [=](){
        if(m_buffer.isEmpty())
        {
            return;
        }
        QByteArray pcm = m_buffer;
        m_buffer.clear();
        emit audioDataReady(pcm);
    });

    connect(m_audioInput , &QAudioInput::notify , this , [=](){
        QAudio::Error err = m_audioInput->error();
        if(err != QAudio::NoError)
        {
            qWarning() << "AudioInput error:" << err;
            emit audioError(err);
        }
    });
}

void AudioInput::startCollect()
{
    m_device = m_audioInput->start();
    m_timer->start(40);
    connect(m_device , &QIODevice::readyRead , this , &AudioInput::onReadyRead , Qt::UniqueConnection);
}

void AudioInput::stopCollect()
{
    m_audioInput->stop();
    m_timer->stop();
    m_buffer.clear();
    m_device = nullptr;
}

AudioInput::~AudioInput()
{
    delete m_timer;
    delete m_audioInput;
}

void AudioInput::onReadyRead()
{
    m_buffer.append(m_device->readAll());
}

void AudioInput::setVolume(qreal v)
{
    m_audioInput->setVolume(v);
}
