#ifndef FITSIMAGELABEL_H
#define FITSIMAGELABEL_H

#include <QLabel>

class FITSImageLabel : public QLabel
{
    Q_OBJECT

private:
    bool    m_isZoomable;

    QPoint  m_mousePos;
    bool    m_isDragging;
    QPoint  m_scrollOffset;

public:
    FITSImageLabel();

    void wheelEvent(QWheelEvent* e);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);

    void setZoomable(bool a_isZoomable = true);
    bool isZoomable() const;

signals:
    void sendMousewheelZoomChanged(int32_t a_scaleFactor);
    void sendMousedragScrollChanged(int32_t a_scrollX, int32_t a_scrollY);

};

#endif // QFITSIMAGELABEL_H
