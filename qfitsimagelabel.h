#ifndef QFITSIMAGELABEL_H
#define QFITSIMAGELABEL_H

#include <QLabel>

class QFITSImageLabel : public QLabel
{
    Q_OBJECT

private:
    bool    m_zoomable;

public:
    QFITSImageLabel();

    void wheelEvent(QWheelEvent* e);
    void setZoomable(bool a_zoomable = true);
    bool isZoomable() const;

signals:
    void sendMousewheelZoomChanged(int32_t a_scaleFactor);

};

#endif // QFITSIMAGELABEL_H
