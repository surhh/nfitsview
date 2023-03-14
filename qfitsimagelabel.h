#ifndef QFITSIMAGELABEL_H
#define QFITSIMAGELABEL_H

#include <QLabel>

class QFITSImageLabel : public QLabel
{
    Q_OBJECT

public:
    QFITSImageLabel();

    void wheelEvent(QWheelEvent* e);

signals:
    void sendMousewheelZoomChanged(int32_t a_scaleFactor);

};

#endif // QFITSIMAGELABEL_H
