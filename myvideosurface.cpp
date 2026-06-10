#include "myvideosurface.h"
#include <qDebug>

MyVideoSurface::MyVideoSurface() {}

QList<QVideoFrame::PixelFormat>MyVideoSurface::supportedPixelFormats(QAbstractVideoBuffer::HandleType type) const
{
    if(type == QAbstractVideoBuffer::NoHandle)
    {
        return {
                QVideoFrame::Format_RGB32,
                QVideoFrame::Format_ARGB32,
                QVideoFrame::Format_RGB24,
                QVideoFrame::Format_RGB565,
                QVideoFrame::Format_BGR32,
                QVideoFrame::Format_BGRA32
        };
    }
    else
    {
        return {};
    }
}

bool MyVideoSurface::isFormatSupported(const QVideoSurfaceFormat &format) const
{
    if(format.handleType() != QAbstractVideoBuffer::NoHandle)
    {
        return false;
    }
    auto fmt = QVideoFrame::imageFormatFromPixelFormat(format.pixelFormat());

    return (fmt != QImage::Format_Invalid);

}

bool MyVideoSurface::present(const QVideoFrame& frame)
{
    if(!frame.isValid())
    {
        return false;
    }
    QVideoFrame f(frame);
    if(!f.map(QAbstractVideoBuffer::ReadOnly))
    {
        qWarning() << "map failed";
        return true;
    }
    QImage::Format fmt = QVideoFrame::imageFormatFromPixelFormat(f.pixelFormat());
    if(fmt == QImage::Format_Invalid)
    {
        f.unmap();
        return true;
    }

    int stride = f.bytesPerLine();
    QImage img = QImage(f.bits() , f.width() , f.height(), stride , fmt).mirrored(false ,true).copy();
    f.unmap();
    emit frameAvailable(img);
    return true;
}
