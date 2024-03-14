#ifndef WORKSPACETABWIDGET_H
#define WORKSPACETABWIDGET_H

#include <QTabWidget>
#include <QLabel>
#include <QScrollBar>

#include "defsui.h"
#include "fitsimagelabel.h"

#include "libnfits/hdu.h"
#include "libnfits/image.h"

#define IMAGE_EXPORT_TYPE_PNG       "png"
#define IMAGE_EXPORT_TYPE_TIFF      "tiff"
#define IMAGE_EXPORT_TYPE_JPG       "jpg"
#define IMAGE_EXPORT_TYPE_BMP       "bmp"

struct FITSImageHDU
{
    libnfits::Image*    image;
    uint32_t            index;
    WidgetsStates       widgetsStates;
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
    void convertImage2EyeComfort();
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
                     size_t a_maxDataBufferSize, int8_t a_bitpix, uint32_t a_hduIndex, const WidgetsStates& a_widgetStates);
    //void insertImage(const uint8_t* a_image, ImageParams& a_imageParams, const WidgetsStates& a_widgetStates,
    //                 uint32_t a_transformType = FITS_FLOAT_DOUBLE_NO_TRANSFORM);
    void insertImage(const uint8_t* a_image, ImageParams& a_imageParams, const WidgetsStates& a_widgetStates,
                     uint32_t a_transformType, int32_t a_percent);

    void clearImages();
    //void setImage(uint32_t a_hduIndex, uint32_t a_transformType = FITS_FLOAT_DOUBLE_NO_TRANSFORM, bool a_bRecreate = false);
    void setImage(uint32_t a_hduIndex, uint32_t a_transformType, int32_t a_percent, bool a_bRecreate = false);
    libnfits::Image* getImage(uint32_t a_hduIndex) const;

    int32_t getScrollPosX() const;
    int32_t getScrollPosY() const;
    void setScrollPosX(int32_t a_x);
    void setScrollPosY(int32_t a_y);

    int32_t setImageHDUWidgetsStates(uint32_t a_hduIndex, const WidgetsStates& a_widgetStates);
    int32_t getImageHDUWidgetsStates(uint32_t a_hduIndex, WidgetsStates& a_widgetStates) const;
    void resetCurrentImageHDUIndex();
    int32_t getCurrentImageHDUIndex() const;

    int32_t findImageHDUIndexByTableIndex(int32_t a_index);

    void setNoImageDataImage();

    //void reloadImageWithTransformation(uint32_t a_transformType);
    void reloadImageWithTransformation(uint32_t a_transformType, float a_percent);

    uint32_t getTransformType() const;
    uint32_t getTransformType(uint32_t a_hduIndex) const;

    int8_t getBitPix() const;

    double getMinValue() const;
    double getMaxValue() const;
    int64_t getMinValueL() const;
    int64_t getMaxValueL() const;

    double getDistribMinValue() const;
    double getDistribMaxValue() const;
    int64_t getDistribMinValueL() const;
    int64_t getDistribMaxValueL() const;

    template<typename T> T getMinValue() const;
    template<typename T> T getMaxValue() const;
    template<typename T> T getDistribMinValue() const;
    template<typename T> T getDistribMaxValue() const;

    FITSImageLabel* getFITSImageLabel() const;

private slots:
    void on_WorkspaceTabWidget_currentChanged(int index);

signals:
    void sendGammaCorrectionTabEnabled(bool a_flag);

private:
    Ui::WorkspaceTabWidget *ui;

    FITSImageLabel                  *m_imageLabel;
    libnfits::Image                 *m_fitsImage;

    std::vector<FITSImageHDU>        m_vecFitsImages;
    int32_t                          m_fitsImageHDUIndex;
};

#endif // WORKSPACETABWIDGET_H
