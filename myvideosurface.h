#ifndef MYVIDEOSURFACE_H
#define MYVIDEOSURFACE_H

#include <QAbstractVideoSurface>
#include <QVideoSurfaceFormat>

class MyVideoSurface : public QAbstractVideoSurface
{
    Q_OBJECT

public:
    MyVideoSurface();
    QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType type = QAbstractVideoBuffer::NoHandle) const override;
    bool isFormatSupported(const QVideoSurfaceFormat &format) const override;
    bool present(const QVideoFrame& frame) override;

signals:
    void frameAvailable(QImage frame);
};

#endif // MYVIDEOSURFACE_H
