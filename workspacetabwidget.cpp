#include "workspacetabwidget.h"
#include "ui_workspacetabwidget.h"

#include <cstring>

WorkspaceTabWidget::WorkspaceTabWidget(QWidget *parent) :
    QTabWidget(parent),
    ui(new Ui::WorkspaceTabWidget),
    m_fitsImage(nullptr),
    m_fitsImageHDUIndex(-1)
{
    ui->setupUi(this);

    // table tab so far is not needed and is hidden now
    ui->tableWidgetTable->setEnabled(false);
    ui->tableWidgetTable->setVisible(false);
    setTabVisible(1, false);

    //ui->scrollArea->setBackgroundRole(QPalette::Dark);

    m_imageLabel = new QLabel;
    m_imageLabel->adjustSize();
    m_imageLabel->setScaledContents(true);
    //m_imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    ui->scrollArea->setAlignment(Qt::AlignCenter);
    ui->scrollArea->setWidget(m_imageLabel);

    ui->scrollArea->setWidgetResizable(false);

    scaleImage(0);

    //// m_fitsImage = new libnfits::Image; // We don't need this anymore as we work with list of images for each HDU being created run-time

#if defined(__WIN32__) || defined(__WIN64__)
    QFont newFont("Courier New", 12, QFont::Normal, true);
    ui->tabHeader->setFont(newFont);
    ui->tabRawData->setFont(newFont);
    ui->tabTable->setFont(newFont);
#endif
}

WorkspaceTabWidget::~WorkspaceTabWidget()
{
    if (m_imageLabel != nullptr)
    {
        m_imageLabel->resize(0, 0);
        m_imageLabel->setScaledContents(true);
        m_imageLabel->clear();

        delete m_imageLabel;

        m_imageLabel = nullptr;
    }

    //// We don't need this anymore as we work with list of images for each HDU being created run-time
    //// if (m_fitsImage != nullptr)
    ////    delete m_fitsImage;

    //clearImage();
    clearImages();

    delete ui;
}

void WorkspaceTabWidget::populateHeaderWidget(const libnfits::HDU& a_hdu)
{
    libnfits::Header header;

    std::vector<libnfits::HeaderRecord> headerRecords;

    ui->textEditHeader->clear();

    header = a_hdu.getHeader();
    headerRecords = header.getHeaderRecords();

    for (std::vector<libnfits::HeaderRecord>::iterator it = headerRecords.begin(); it < headerRecords.end(); ++it)
    {
         QString keyword = QString((it->getKeyword().c_str()));
         QString keywordAligned = keyword.leftJustified(12, ' ');

         QString value = QString(it->getValueString().c_str());
         QString comment = QString(it->getComment().c_str());


         ui->textEditHeader->moveCursor(QTextCursor::End);
         ui->textEditHeader->setTextColor(Qt::blue);
         ui->textEditHeader->insertPlainText(keywordAligned);
         ui->textEditHeader->moveCursor(QTextCursor::End);
         ui->textEditHeader->setTextColor(Qt::darkGray);

         if (!keyword.isEmpty() && !value.isEmpty())
             ui->textEditHeader->insertPlainText("=  ");

         ui->textEditHeader->moveCursor(QTextCursor::End);
         ui->textEditHeader->setTextColor(Qt::black);
         ui->textEditHeader->insertPlainText(value);

         ui->textEditHeader->moveCursor(QTextCursor::End);
         ui->textEditHeader->setTextColor(Qt::darkGreen);

         if (!comment.isEmpty())
             ui->textEditHeader->insertPlainText("    /");

         ui->textEditHeader->moveCursor(QTextCursor::End);
         ui->textEditHeader->insertPlainText(comment + "\n");
     }

     headerRecords.clear();

     ui->textEditHeader->moveCursor(QTextCursor::Start);
}

void WorkspaceTabWidget::populateRawDataWidget(const libnfits::HDU& a_hdu)
{
    ui->textEditRawData->clear();

    uint8_t* pData = a_hdu.getData();
    uint32_t align = 4;
    const size_t maxMB = 1;         // load size is 1 MB
    const size_t maxSize = maxMB * 1048576; // raw data max.

    size_t outSize = a_hdu.getSize();
    std::string strInfoBig = "";

    if (outSize > maxSize)
    {
        outSize = maxSize;
        strInfoBig = "........\n";
        strInfoBig += "........\n";
        strInfoBig += "........\n";
        strInfoBig += "........\n";
        strInfoBig += "\n-----------------------------------------------------\n";
        strInfoBig += "NOTE: Only " + std::to_string(maxMB) + " MB of data is shown in raw data preview\n";
        strInfoBig += "-----------------------------------------------------\n";
    }

    std::string strRawData = "";
    strRawData = (libnfits::convertBuffer2HexString(pData, outSize, align));

    strRawData += strInfoBig;
    ui->textEditRawData->setText(QString((const char*)strRawData.c_str()));
}

void WorkspaceTabWidget::clearWidgets() const
{
    ui->tableWidgetTable->clear();
    ui->textEditRawData->clear();
    ui->textEditHeader->clear();

    m_imageLabel->clear();
}

void WorkspaceTabWidget::setImage(const uint8_t* a_image, uint32_t a_width, uint32_t a_height, size_t a_HDUBaseOffset,
                                  size_t a_maxDataBufferSize, int8_t a_bitpix)
{
    m_fitsImage->reset(); // this is for showing correctly images from different image HDUs
    m_fitsImage->setParameters(a_width, a_height, FITS_PNG_DEFAULT_PIXEL_DEPTH, a_bitpix);
    m_fitsImage->setMaxDataBufferSize(a_maxDataBufferSize);
    m_fitsImage->setBaseOffset(a_HDUBaseOffset);
    m_fitsImage->setData(a_image);
    m_fitsImage->createRGB32FlatData();

    //// image float data dumping for debugging purposes
    //// m_fitsImage->dumpFloatDataBuffer("./image_dump.txt", 8);
    //// end of debugging dump

    reloadImage();
}

void WorkspaceTabWidget::insertImage(const uint8_t* a_image, uint32_t a_width, uint32_t a_height, size_t a_HDUBaseOffset,
                                        size_t a_maxDataBufferSize, int8_t a_bitpix, uint32_t a_hduIndex, const WidgetsStates& a_widgetStates)
{
    FITSImageHDU imageHDU;

    libnfits::Image *image = new libnfits::Image;

    image->setParameters(a_width, a_height, FITS_PNG_DEFAULT_PIXEL_DEPTH, a_bitpix);
    image->setMaxDataBufferSize(a_maxDataBufferSize);
    image->setBaseOffset(a_HDUBaseOffset);
    image->setData(a_image);
    image->createRGB32FlatData();

    imageHDU.index = a_hduIndex;
    imageHDU.image = image;
    imageHDU.widgetsStates = a_widgetStates;

    m_vecFitsImages.push_back(imageHDU);
}

void WorkspaceTabWidget::reloadImage()
{
    QImage *image = nullptr;

    m_imageLabel->clear();

    QImage::Format format = QImage::Format_RGB32;

    image = new QImage(m_fitsImage->getRGB32FlatData(), m_fitsImage->getWidth(), m_fitsImage->getHeight(), format);

    if (image != nullptr)
        m_imageLabel->setPixmap(QPixmap::fromImage(*image));

    m_imageLabel->adjustSize();

    if (image != nullptr)
        delete image;
}

void WorkspaceTabWidget::clearImage() const
{
    m_fitsImage->reset();

    m_imageLabel->clear();
    m_imageLabel->resize(0, 0);
    m_imageLabel->setScaledContents(true);
}

void WorkspaceTabWidget::scaleImage(int32_t a_factor)
{
    double scaleFactor = (double)a_factor / 100;

    m_imageLabel->resize(scaleFactor * (m_imageLabel->pixmap(Qt::ReturnByValue).size()));

    //scrollToCenter(); // no need to center the image during zooming
}

QSize WorkspaceTabWidget::getScrollAreaSize() const
{
    return ui->scrollArea->size();
}

void WorkspaceTabWidget::scrollToCenter() const
{
    QScrollBar *hScrollBar = nullptr;
    QScrollBar *vScrollBar = nullptr;

    hScrollBar = ui->scrollArea->horizontalScrollBar();
    vScrollBar = ui->scrollArea->verticalScrollBar();

    if (hScrollBar != nullptr)
        hScrollBar->setValue(int(hScrollBar->maximum()/2));

    if (vScrollBar != nullptr)
        vScrollBar->setValue(int(vScrollBar->maximum()/2));
}

void WorkspaceTabWidget::convertImage2Grayscale()
{
    //backupImage();

    m_fitsImage->convertRGB32Flat2Grayscale();
}

void WorkspaceTabWidget::convertImage2EyeComfort()
{
    //backupImage();

    m_fitsImage->convertRGB32Flat2EyeComfortColors();
}
void WorkspaceTabWidget::backupImage()
{
    m_fitsImage->backupRGB32FlatData();
}

void WorkspaceTabWidget::restoreImage()
{
    m_fitsImage->restoreRGB32FlatData();
}

void WorkspaceTabWidget::changeChannelLevel(uint8_t a_channel, float a_quatient)
{
    if (a_channel == 2)
        m_fitsImage->change32FlatRLevel(a_quatient);
    else if (a_channel == 1)
        m_fitsImage->change32FlatGLevel(a_quatient);
    else if (a_channel == 0)
        m_fitsImage->change32FlatBLevel(a_quatient);
}

uint32_t WorkspaceTabWidget::getImageWidth() const
{
    if (m_fitsImage != nullptr)
        return m_fitsImage->getWidth();
    else
        return 0;
}

uint32_t WorkspaceTabWidget::getImageHeight() const
{
    if (m_fitsImage != nullptr)
        return m_fitsImage->getHeight();
    else
        return 0;
}

void WorkspaceTabWidget::imageSetVisible(bool a_visible)
{
    m_imageLabel->setVisible(a_visible);
}

bool WorkspaceTabWidget::exportImage(const QString& a_fileName, const QString& a_strType, int32_t a_quality)
{
    QString fileName = a_fileName + "." + a_strType;

    QPixmap pixmap = m_imageLabel->pixmap(Qt::ReturnByValue);

    if (!pixmap.isNull())
        return pixmap.save(fileName, a_strType.toStdString().c_str(), a_quality);
    else
        return false;
}

void WorkspaceTabWidget::on_WorkspaceTabWidget_currentChanged(int index)
{
    if (index != 0)
        emit sendGammaCorrectionTabEnabled(false);
    else
        sendGammaCorrectionTabEnabled(true);
}

QSize WorkspaceTabWidget::getImageLabelSize() const
{
    return m_imageLabel->size();
}

void WorkspaceTabWidget::enableTabs(bool a_flag)
{
    setEnabled(a_flag);
}

void WorkspaceTabWidget::clearImages()
{
    if (m_imageLabel != nullptr)
    {
        m_imageLabel->resize(0, 0);
        m_imageLabel->setScaledContents(true);
        m_imageLabel->clear();
    }

    for (auto it = m_vecFitsImages.begin(); it < m_vecFitsImages.end(); ++it)
    {
        it->image->reset();

        delete it->image;
    }

    m_vecFitsImages.clear();

    m_fitsImage = nullptr;

    m_fitsImageHDUIndex = -1;
}

void WorkspaceTabWidget::setImage(uint32_t a_hduIndex)
{
    for (auto it = m_vecFitsImages.begin(); it < m_vecFitsImages.end(); ++it)
        if (it->index == a_hduIndex)
        {
            m_fitsImage = it->image;
            m_fitsImageHDUIndex = it->index;

            reloadImage();
        }
}

int32_t WorkspaceTabWidget::getScrollPosX() const
{
    return ui->scrollArea->horizontalScrollBar()->value();
}

int32_t WorkspaceTabWidget::getScrollPosY() const
{
    return ui->scrollArea->verticalScrollBar()->value();
}

void WorkspaceTabWidget::setScrollPosX(int32_t a_x)
{
    ui->scrollArea->horizontalScrollBar()->setValue(a_x);
}

void WorkspaceTabWidget::setScrollPosY(int32_t a_y)
{
    ui->scrollArea->verticalScrollBar()->setValue(a_y);
}

int32_t WorkspaceTabWidget::setImageHDUWidgetsStates(uint32_t a_hduIndex, const WidgetsStates& a_widgetStates)
{
    if (a_hduIndex > m_vecFitsImages.size() - 1)
        return FITS_GENERAL_ERROR;
    else
        m_vecFitsImages[a_hduIndex].widgetsStates = a_widgetStates;

    return FITS_GENERAL_SUCCESS;
}

int32_t WorkspaceTabWidget::getImageHDUWidgetsStates(uint32_t a_hduIndex, WidgetsStates& a_widgetStates) const
{
    if (a_hduIndex > m_vecFitsImages.size() - 1)
        return FITS_GENERAL_ERROR;
    else
        a_widgetStates = m_vecFitsImages[a_hduIndex].widgetsStates;

    return FITS_GENERAL_SUCCESS;
}

void WorkspaceTabWidget::resetCurrentImageHDUIndex()
{
    m_fitsImageHDUIndex = -1;
}

int32_t WorkspaceTabWidget::getCurrentImageHDUIndex() const
{
    return m_fitsImageHDUIndex;
}

int32_t WorkspaceTabWidget::findImageHDUIndexByTableIndex(int32_t a_index)
{
    int32_t foundIndex = -1;

    if (a_index < 0)
        return foundIndex;

    for (int32_t i = 0; i < m_vecFitsImages.size(); ++i)
        if (m_vecFitsImages[i].index == a_index)
        {
            foundIndex = i;
            break;
        }

    return foundIndex;
}

void WorkspaceTabWidget::setNoImageDataImage()
{
    m_imageLabel->clear();

    m_imageLabel->setPixmap(QPixmap("://icons/no_image_data.png"));

    m_imageLabel->adjustSize();
}
