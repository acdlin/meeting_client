#include "audiooutput.h"

AudioOutput::AudioOutput(QObject *parent):QObject(parent)
{
    m_format.setSampleRate(8000);
    m_format.setSampleSize(16);
    m_format.setChannelCount(1);
    m_format.setCodec("audio/pcm");
    m_format.setByteOrder(QAudioFormat::LittleEndian);
    m_format.setSampleType(QAudioFormat::SignedInt);

    m_audioOutput = new QAudioOutput(m_format , this);
    connect(m_audioOutput , &QAudioOutput::notify , this ,[=](){
        QAudio::Error err = m_audioOutput->error();
        if(err == QAudio::NoError && err != QAudio::UnderrunError)
        {
            return;
        }
        qWarning() << "AudioInput error:" << err;
        emit audioError(err);
    });
}

AudioOutput::~AudioOutput()
{
    delete m_audioOutput;
}

void AudioOutput::writePcm(const QByteArray &pcm)
{
    if(!m_device) return ;
    m_buffer.append(pcm);
    qint64 free = m_audioOutput->bytesFree();
    qint64 toWrite = qMin(free , (qint64)m_buffer.size());

    if(toWrite > 0)
    {
        m_device->write(m_buffer.constData() , toWrite);
        m_buffer.remove(0 , toWrite);
    }
}

void AudioOutput::startPlay()
{
    m_device = m_audioOutput->start();
}

void AudioOutput::stopPlay()
{
    m_audioOutput->stop();
}

void AudioOutput::setVolume(qreal v)
{
    m_audioOutput->setVolume(v);
}
