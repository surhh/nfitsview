#ifndef WORKSPACETABWIDGET_H
#define WORKSPACETABWIDGET_H

#include <QTabWidget>
#include <QLabel>
#include <QScrollBar>

#include "libnfits/hdu.h"
#include "libnfits/image.h"

#define IMAGE_EXPORT_TYPE_PNG       "png"
#define IMAGE_EXPORT_TYPE_JPG       "jpg"
#define IMAGE_EXPORT_TYPE_BMP       "bmp"

struct FITSImageHDU
{
    libnfits::Image*    image;
    uint32_t            index;
};

namespace Ui {
class WorkspaceTabWidget;
}

class WorkspaceTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    explicit WorkspaceTabWidget(QWidget *parent = nullptr);
    ~WorkspaceTabWidget();

    void populateHeaderWidget(const libnfits::HDU& a_hdu);
    void populateRawDataWidget(const libnfits::HDU& a_hdu);
    void clearWidgets() const;
    void setImage(const uint8_t* a_image, uint32_t a_width, uint32_t a_height, size_t a_HDUBaseOffset,
                  size_t a_maxDataBufferSize, int8_t a_bitpix);
    void reloadImage();
    void clearImage() const;
    void scaleImage(int32_t a_factor);
    QSize getScrollAreaSize() const;
    void scrollToCenter() const;
    void convertImage2Grayscale();
    void restoreImage();
    void changeChannelLevel(uint8_t a_channel, float a_quatient);
    uint32_t getImageWidth() const;
    uint32_t getImageHeight() const;
    void imageSetVisible(bool a_visible);
    void backupImage();
    bool exportImage(const QString& a_fileName, const QString& a_strType = IMAGE_EXPORT_TYPE_PNG, int32_t a_quality = -1);
    QSize getImageLabelSize() const;
    void enableTabs(bool a_flag = true);

    void insertImage(const uint8_t* a_image, uint32_t a_width, uint32_t a_height, size_t a_HDUBaseOffset,
                        size_t a_maxDataBufferSize, int8_t a_bitpix, uint32_t a_hduIndex);
    void clearImages();
    void setImage(uint32_t a_hduIndex);

private slots:
    void on_WorkspaceTabWidget_currentChanged(int index);

signals:
    void sendGammaCorrectionTabEnabled(bool a_flag);

private:
    Ui::WorkspaceTabWidget *ui;

    QLabel                          *m_imageLabel;
    libnfits::Image                 *m_fitsImage;
    std::vector<FITSImageHDU>        m_vecFitsImages;

    int32_t              m_zoomFactor;

};

#endif // WORKSPACETABWIDGET_H
