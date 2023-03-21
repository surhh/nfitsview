#ifndef FITSIMAGELABEL_H
#define FITSIMAGELABEL_H

#include <QLabel>

class FITSImageLabel : public QLabel
{
    Q_OBJECT

private:
    bool    m_zoomable;

public:
    FITSImageLabel();

    void wheelEvent(QWheelEvent* e);
    void setZoomable(bool a_zoomable = true);
    bool isZoomable() const;

signals:
    void sendMousewheelZoomChanged(int32_t a_scaleFactor);

};

#endif // QFITSIMAGELABEL_H
