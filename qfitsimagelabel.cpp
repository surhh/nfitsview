#include "qfitsimagelabel.h"

#include <QWheelEvent>

QFITSImageLabel::QFITSImageLabel():
    m_zoomable(true)
{

}

void QFITSImageLabel::setZoomable(bool a_zoomable)
{
    m_zoomable = a_zoomable;
}

bool QFITSImageLabel::isZoomable() const
{
    return m_zoomable;
}

void QFITSImageLabel::wheelEvent(QWheelEvent* e)
{
    int32_t scaleFactor = 5;

    QPoint numDegrees = e->angleDelta();
    int32_t degreesY = e->angleDelta().y();

    if (!m_zoomable)
    {
        e->ignore();
        return;
    }

    if (!numDegrees.isNull())
    {
        if (degreesY > 0)
        {
            emit sendMousewheelZoomChanged(scaleFactor);
        }
        else if (degreesY < 0)
        {
            emit sendMousewheelZoomChanged(-scaleFactor);
        }
        else
            e->ignore();
    }

    e->accept();
}
