#include "fitsimagelabel.h"

#include <QWheelEvent>

FITSImageLabel::FITSImageLabel():
    m_zoomable(true)
{

}

void FITSImageLabel::setZoomable(bool a_zoomable)
{
    m_zoomable = a_zoomable;
}

bool FITSImageLabel::isZoomable() const
{
    return m_zoomable;
}

void FITSImageLabel::wheelEvent(QWheelEvent* e)
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
            emit sendMousewheelZoomChanged(scaleFactor);
        else if (degreesY < 0)
            emit sendMousewheelZoomChanged(-scaleFactor);
        else
            e->ignore();
    }

    e->accept();
}
