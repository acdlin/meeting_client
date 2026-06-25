#include "audiooutput.h"
#include "logqueue.h"
#include <QMessageBox>
#include <QTimer>

const int MAX_BUFFER_SIZE = 50 * 1024;
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
        if(err == QAudio::NoError || err == QAudio::UnderrunError)
        {
            return;
        }
        qWarning() << "AudioOutput error:" << err;
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
    if(m_buffer.size() > MAX_BUFFER_SIZE )
    {
        m_buffer.remove(0 , m_buffer.size()- MAX_BUFFER_SIZE);
        LOG_DEBUG("AudioOutput buffer overflow, dropped old data");
    }
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
    if(!m_device)
    {
        QTimer::singleShot(0 , this , [=](){
            QMessageBox::critical(nullptr , "错误" , "音频设备错误");
        });
        emit AudioOutput::audioError(QAudio::OpenError);
    }
}

void AudioOutput::stopPlay()
{
    m_audioOutput->stop();
    m_buffer.clear();
    m_device = nullptr;
}

void AudioOutput::setVolume(qreal v)
{
    m_audioOutput->setVolume(v);
}
