#include "qfitsimagelabel.h"

#include <QWheelEvent>

QFITSImageLabel::QFITSImageLabel()
{

}

void QFITSImageLabel::wheelEvent(QWheelEvent* e)
{
    int32_t scaleFactor = 5;

    QPoint numDegrees = e->angleDelta();
    int32_t degreesY = e->angleDelta().y();

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
