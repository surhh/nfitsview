#include "fitsimagelabel.h"

#include <QWheelEvent>

FITSImageLabel::FITSImageLabel():
    m_isZoomable(true), m_scrollOffset(0,0), m_isDragging(false)
{

}

void FITSImageLabel::setZoomable(bool a_isZoomable)
{
    m_isZoomable = a_isZoomable;
}

bool FITSImageLabel::isZoomable() const
{
    return m_isZoomable;
}

void FITSImageLabel::wheelEvent(QWheelEvent* e)
{
    int32_t scaleFactor = 5;

    QPoint numDegrees = e->angleDelta();
    int32_t degreesY = e->angleDelta().y();

    if (!m_isZoomable)
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

void FITSImageLabel::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
    {
        m_mousePos = e->pos();
        m_isDragging = true;

        setCursor(Qt::ClosedHandCursor);
    }
}

void FITSImageLabel::mouseMoveEvent(QMouseEvent *e)
{
    if (m_isDragging)
    {
        QPoint deltaPos = e->pos() - m_mousePos;
        m_scrollOffset += deltaPos;
        m_mousePos = e->pos();

        setCursor(Qt::ClosedHandCursor);

        emit sendMousedragScrollChanged(deltaPos.x(), deltaPos.y());
    }
}

void FITSImageLabel::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
    {
        m_isDragging = false;

        setCursor(Qt::ArrowCursor);
    }
}
