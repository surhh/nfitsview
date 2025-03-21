#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>
#include <QTextEdit>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QDesktopServices>

#if defined(ENABLE_OPENMP)
#include <omp.h>
#endif

#include "libnfits/hdu.h"
#include "libnfits/fitsfile.h"
#include "libnfits/fits2png.h"
#include "libnfits/header.h"
#include "libnfits/headerrecord.h"

#include "aboutdialog.h"
#include "updatemanager/filedownloader.h"

#define FITS_FILE_EXTENSIONS    "FITS Files (*.fit *.fts *.fits *.fz)"

// This function acts as a wrapper over the callback method of signal/slot.
// "this" pointer of MainWindow is passed to callback function and then it
// is used in order to emit the signal from MainWindow class.
qint32 progressCallbackFunction(qint32 a_value, void* a_buffer = nullptr)
{
    MainWindow* mainWnd = static_cast<MainWindow*>(a_buffer);

    emit mainWnd->sendProgressChanged(a_value);

    return a_value;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_scaleFactor(100)
    //, m_percentThreshold(0)
    , m_bImageChanged(false)
    , m_bEnableGammaWidgets(false)
    , m_bEnableZoomWidget(false)
    , m_bEnableMappingWidgets(false)
    , m_bGrayscale(false)
    , m_bEyeComfort(false)
    , m_exportFormat(IMAGE_EXPORT_TYPE_PNG)
    , m_exportQuality(IMAGE_EXPORT_DEFAULT_QUALITY)
{
    ui->setupUi(this);

    createStatusBarWidgets();

    connect(m_sliderZoom, SIGNAL(valueChanged(int)), SLOT(om_m_sliderZoom_valueChanged(int)));
    connect(this, SIGNAL(sendProgressChanged(qint32)), SLOT(on_progressChanged(qint32)));
    connect(ui->workspaceWidget, SIGNAL(sendGammaCorrectionTabEnabled(bool)), this, SLOT(on_workspaceWidget_sendGammaCorrectionTabEnabled(bool)));
    connect(ui->workspaceWidget->getFITSImageLabel(), SIGNAL(sendMousewheelZoomChanged(int32_t)), this, SLOT(onSendMousewheelZoomChanged(int32_t)));
    connect(ui->workspaceWidget->getFITSImageLabel(), SIGNAL(sendMousedragScrollChanged(int32_t, int32_t)), this, SLOT(onSendMousedragScrollChanged(int32_t, int32_t)));

    //// currently the Undo/Redo logic is not implemented, not needed so far, so disabling the controls
    ui->actionUndo->setVisible(false);
    ui->actionUndoToolBar->setVisible(false);
    ui->actionRedo->setVisible(false);
    ui->actionRedoToolBar->setVisible(false);
    //// end of disabling Undo/Redo controls

    //// set gamma restore button to invisible, not implemented at the moment due to RGB processing restrictoins
    ui->actionRestoreOriginal->setVisible(false);
    ui->actionRestoreOriginalToolBar->setVisible(false);


    enableRestoreWidgets(false);
    enableGammaWidgets(m_bEnableGammaWidgets);
    enableFileOpenRelatedWidgets(false);
    enableZoomWidgets(m_bEnableZoomWidget);
    enableImageExportWidgets(false);
    enableImageExportSettigsWidgets(false);
    enableMappingWidgets(m_bEnableMappingWidgets);

    initGammaWidgetsValues();
    initMappingWidgetsValues();
    initImageExportSettingsWidgetValues();

    ui->toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    ui->tableWidgetHDUs->setColumnWidth(0, 50);
    ui->tableWidgetHDUs->setColumnWidth(1, 120);
    ui->tableWidgetHDUs->setColumnWidth(2, 50);

    ui->actionZoomIn->setShortcut(QKeySequence::ZoomIn);

    std::fill(m_percentThreshold, m_percentThreshold + 4, 0);

    ui->horizontalSliderPercent->setValue(m_percentThreshold[0]);
    QString valueStr = THRESHOLD_LABEL_TEXT + QString::number(100 - ui->horizontalSliderPercent->value()) + " %";
    valueStr = valueStr.rightJustified(16, ' ');
    ui->labelPercent->setText(valueStr);

    setWindowTitle(NFITSVIEW_APP_NAME);

    //// checking for the new version
    m_fileDownloader = new FileDownloader(QUrl(UPDATE_CHECK_URL));
    if (m_fileDownloader != nullptr)
        connect(m_fileDownloader, SIGNAL(downloaded()),this, SLOT(updateChecked()));

#if defined(ENABLE_OPENMP)
    int32_t numThreads = omp_get_max_threads();
    numThreads = numThreads > 2 ? numThreads - OPENMP_THREADS_DISABLE_NUMBER : numThreads;
    omp_set_num_threads(numThreads);
    //omp_set_num_threads(omp_get_max_threads() / 2);
#endif
}

MainWindow::~MainWindow()
{
    delete m_progressBar;
    delete m_labelZoomLeft;
    delete m_labelZoomRight;
    delete m_labelZoomLevel;
    delete m_sliderZoom;

    delete m_fileDownloader;

    delete ui;
}

void MainWindow::createStatusBarWidgets()
{
    setStatus(STATUS_MESSAGE_READY);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setVisible(true);
    statusBar()->addPermanentWidget(m_progressBar);

    m_labelZoomLeft = new QLabel(this);
    m_labelZoomLeft->setVisible(true);
    m_labelZoomLeft->setText("Zoom:   -");
    statusBar()->addPermanentWidget(m_labelZoomLeft);

    m_sliderZoom = new QSlider(this);
    m_sliderZoom->setVisible(true);
    m_sliderZoom->setOrientation(Qt::Horizontal);
    m_sliderZoom->setMinimum(MIN_IMAGE_SCALE_FACTOR);
    m_sliderZoom->setMaximum(MAX_IMAGE_SCALE_FACTOR);
    m_sliderZoom->setValue(100);
    statusBar()->addPermanentWidget(m_sliderZoom);

    m_labelZoomRight = new QLabel(this);
    m_labelZoomRight->setVisible(true);
    m_labelZoomRight->setText("+");
    statusBar()->addPermanentWidget(m_labelZoomRight);

    m_labelZoomLevel = new QLabel(this);
    m_labelZoomLevel->setVisible(true);

    int valueZoom = m_sliderZoom->value();
    QString valueZoomStr = "  | " + QString::number(valueZoom) + " %";
    valueZoomStr = valueZoomStr.rightJustified(16, ' ');
    m_labelZoomLevel->setText(valueZoomStr);
    statusBar()->addPermanentWidget(m_labelZoomLevel);
}

void MainWindow::enableRGBWidgets(bool a_flag)
{
    bool b = !(ui->checkBoxGrayscale->isChecked() | ui->checkBoxEyeComfort->isChecked());

    b = b & a_flag;

    ui->horizontalSliderR->setEnabled(b);
    ui->horizontalSliderG->setEnabled(b);
    ui->horizontalSliderB->setEnabled(b);

    ui->labelR->setEnabled(b);
    ui->labelG->setEnabled(b);
    ui->labelB->setEnabled(b);

    ui->labelRGBInfo->setEnabled(b);

    ui->labelValueR->setEnabled(b);
    ui->labelValueG->setEnabled(b);
    ui->labelValueB->setEnabled(b);

    ui->resetRGBButton->setEnabled(b);

    ui->minRButton->setEnabled(b);
    ui->defaultRButton->setEnabled(b);
    ui->maxRButton->setEnabled(b);

    ui->minGButton->setEnabled(b);
    ui->defaultGButton->setEnabled(b);
    ui->maxGButton->setEnabled(b);

    ui->minBButton->setEnabled(b);
    ui->defaultBButton->setEnabled(b);
    ui->maxBButton->setEnabled(b);
}

void MainWindow::enableGammaWidgets(bool a_flag)
{
    enableRGBWidgets(a_flag);

    bool bG = !(ui->checkBoxGrayscale->isChecked());
    bool bE = !(ui->checkBoxEyeComfort->isChecked());

    ui->checkBoxGrayscale->setEnabled(bE & a_flag);
    ui->checkBoxEyeComfort->setEnabled(bG & a_flag);
}

void MainWindow::enableFileOpenRelatedWidgets(bool a_flag)
{
    ui->actionClose->setEnabled(a_flag);
    ui->actionExport->setEnabled(a_flag);
    ui->actionUndo->setEnabled(a_flag);
    ui->actionRedo->setEnabled(a_flag);

    ui->actionCloseToolBar->setEnabled(a_flag);
    ui->actionExportToolBar->setEnabled(a_flag);
    ui->actionUndoToolBar->setEnabled(a_flag);
    ui->actionRedoToolBar->setEnabled(a_flag);

    ui->workspaceWidget->enableTabs(a_flag);
}

void MainWindow::on_checkBoxGrayscale_stateChanged(int arg1)
{
    enableRGBWidgets(!arg1);
    enableMappingWidgets(!arg1 & m_bEnableMappingWidgets);

    ui->checkBoxEyeComfort->setEnabled(!arg1);    

    int32_t scrollX = ui->workspaceWidget->getScrollPosX();
    int32_t scrollY = ui->workspaceWidget->getScrollPosY();

    if (arg1 && !m_bGrayscale)
    {
        grayScale();

        m_bGrayscale = true;
    }
    else
    {
        int32_t index = ui->workspaceWidget->getCurrentImageHDUIndex();
        if (index != - 1)
            restoreRGBColorChannelLevelsImage(index);

        m_bGrayscale = false;
    }

    //ui->workspaceWidget->reloadImage();
    ui->workspaceWidget->scaleImage(m_scaleFactor);
    ui->workspaceWidget->setScrollPosX(scrollX);
    ui->workspaceWidget->setScrollPosY(scrollY);
}

void MainWindow::om_m_sliderZoom_valueChanged(int a_value)
{
    int valueZoom = a_value;
    QString valueZoomStr = "  | " + QString::number(valueZoom) + " %";
    valueZoomStr = valueZoomStr.rightJustified(16, ' ');
    m_labelZoomLevel->setText(valueZoomStr);

    m_scaleFactor = a_value;

    scaleImage();
}

void MainWindow::populateHDUsWidget()
{
    uint32_t            numberOfHDUs = 0;
    int32_t             resTemp = FITS_GENERAL_ERROR;
    libnfits::HDU       hdu;

    m_fitsFile.setOffset(0);
    numberOfHDUs = m_fitsFile.getNumberOfHDUs();

    for (uint32_t i = 0; i < numberOfHDUs; ++i)
    {
        resTemp = m_fitsFile.getHDU(i, hdu);
        size_t sizeHDU = hdu.getSize();

        if (resTemp == FITS_GENERAL_SUCCESS)
        {
            ui->tableWidgetHDUs->insertRow(ui->tableWidgetHDUs->rowCount());

            bool bSuccess;
            int32_t bitpix = hdu.getKeywordValue<int32_t>(FITS_KEYWORD_BITPIX, bSuccess);
            QString bitpixStr = "N/A";

            if (bSuccess)
                bitpixStr = QString::number(bitpix);

            uint8_t HDUtype = hdu.getType();
            QString HDUtypeStr = "";

            if (hdu.isPrimary())
                HDUtypeStr = "P";

            if (HDUtype & FITS_HDU_TYPE_IMAGE_XTENSION)
                HDUtypeStr += "I";

            if (HDUtype & FITS_HDU_TYPE_ASCII_TABLE_XTENSION)
                HDUtypeStr += "T(A)";

            if (HDUtype & FITS_HDU_TYPE_BINARY_TABLE_XTENSION)
                HDUtypeStr += "T(B)";

            if (HDUtype & FITS_HDU_TYPE_COMPRESSED_IMAGE_XTENSION)
                HDUtypeStr += "I(C)";

            if (HDUtype & FITS_HDU_TYPE_COMPRESSED_TABLE_XTENSION)
                HDUtypeStr += "T(C)";

            if (HDUtype & FITS_HDU_TYPE_RANDOM_GROUP_RECORDS)
                HDUtypeStr += "R";

            QTableWidgetItem *itemType, *itemSize, *itemBitpix;

            itemType = new QTableWidgetItem(HDUtypeStr);
            itemSize = new QTableWidgetItem(QString::number(sizeHDU));
            itemBitpix = new QTableWidgetItem(bitpixStr);

            itemType->setFlags(itemType->flags() & ~Qt::ItemIsEditable);
            itemSize->setFlags(itemSize->flags() & ~Qt::ItemIsEditable);
            itemBitpix->setFlags(itemBitpix->flags() & ~Qt::ItemIsEditable);

            ui->tableWidgetHDUs->setItem(ui->tableWidgetHDUs->rowCount()-1, 0, itemType);
            ui->tableWidgetHDUs->setItem(ui->tableWidgetHDUs->rowCount()-1, 1, itemSize);
            ui->tableWidgetHDUs->setItem(ui->tableWidgetHDUs->rowCount()-1, 2, itemBitpix);

            uint32_t axisesNumber = hdu.getKeywordValue<uint32_t>(FITS_KEYWORD_NAXIS, bSuccess);
            std::vector<uint32_t> axises = hdu.getAxises();

            if ((HDUtype == FITS_HDU_TYPE_IMAGE_XTENSION || HDUtype == FITS_HDU_TYPE_PRIMARY) &&
                (axisesNumber >= 2 && bSuccess) && (axises.size() >= 2))
            {
                QBrush color = Qt::lightGray;

                ui->tableWidgetHDUs->item(ui->tableWidgetHDUs->rowCount()-1, 0)->setBackground(color);
                ui->tableWidgetHDUs->item(ui->tableWidgetHDUs->rowCount()-1, 1)->setBackground(color);
                ui->tableWidgetHDUs->item(ui->tableWidgetHDUs->rowCount()-1, 2)->setBackground(color);
            }
        }
    }

    int32_t rowCount = ui->tableWidgetHDUs->rowCount();

    if (rowCount > 0)
        ui->tableWidgetHDUs->selectRow(rowCount - 1);
}

void MainWindow::populateHeaderWidget(int32_t a_hduIndex)
{
    int32_t          resTemp = FITS_GENERAL_ERROR;
    libnfits::HDU    hdu;

    resTemp = m_fitsFile.getHDU(a_hduIndex, hdu);

    if (resTemp == FITS_GENERAL_SUCCESS)
        ui->workspaceWidget->populateHeaderWidget(hdu);
}

void MainWindow::populateRawDataWidget(int32_t a_hduIndex)
{
    int32_t          resTemp = FITS_GENERAL_ERROR;
    libnfits::HDU    hdu;

    resTemp = m_fitsFile.getHDU(a_hduIndex, hdu);

    if (resTemp == FITS_GENERAL_SUCCESS)
        ui->workspaceWidget->populateRawDataWidget(hdu);
}

int32_t MainWindow::openFITSFile()
{
    QString filter = FITS_FILE_EXTENSIONS;
    QString fileName = QFileDialog::getOpenFileName(this, "Open FITS file", "~/", filter);

    return openFITSFileByName(fileName);    
}

int32_t MainWindow::openFITSFileByNameFromCmdLine(const QString& a_fileName)
{
    int32_t resOpen = openFITSFileByName(a_fileName);

    return resOpen;
}

int32_t MainWindow::openFITSFileByName(const QString& a_fileName, bool a_bShowMsg)
{
    int32_t result = FITS_GENERAL_ERROR;

    if (!a_fileName.isEmpty())
    {
        m_fitsFileName = a_fileName;

        result = FITS_GENERAL_SUCCESS;
    }
    else
        return result;

    int32_t resTemp = FITS_GENERAL_ERROR;

    if (m_fitsFile.isOpen() && (closeFITSFile() == FITS_GENERAL_ERROR))
        return result;

    setStatus(STATUS_MESSAGE_IMAGE_LOAD);

    clearWidgets();

    setProgress(25);

    resTemp = m_fitsFile.loadFile(m_fitsFileName.toStdString());

    if (resTemp == FITS_GENERAL_SUCCESS)
    {
        m_fitsFile.setCallbackFunction(progressCallbackFunction, (void *)this);

        // this function loads all the image HDUs into the corresponding Image objects
        setAllWorkspaceImages();

        populateHDUsWidget();
        enableFileOpenRelatedWidgets();

        setWindowTitle(QString(NFITSVIEW_APP_NAME) + " - " + getFileName());

        setProgress(100);
    }
    else if (a_bShowMsg)
    {
        QMessageBox messageBox;
        messageBox.critical(this, FITS_MSG_ERROR_TYPE, FITS_MSG_ERROR_OPENING_FILE);
    }

    setStatus(STATUS_MESSAGE_READY);

    setProgress(0);

    m_bImageChanged = m_bEyeComfort = m_bGrayscale = false;

    return resTemp;
}

int32_t MainWindow::closeFITSFile()
{
    if (m_fitsFile.closeFile() != FITS_MEMORY_MAP_FILE_SUCCESS)
        return FITS_GENERAL_ERROR;

    clearWidgets();
    initGammaWidgetsValues();
    initMappingWidgetsValues();
    enableImageExportWidgets(false);
    enableImageExportSettigsWidgets(false);

    //ui->workspaceWidget->clearImage();
    ui->workspaceWidget->clearImages();

    m_scaleFactor = 100;

    m_bImageChanged = false;

    m_exportFormat = ui->comboBoxFormat->currentText().toLower();
    m_exportQuality = ui->horizontalSliderQuality->value();

    setWindowTitle(NFITSVIEW_APP_NAME);

    return FITS_GENERAL_SUCCESS;
}

int32_t MainWindow::exportFITSFileFromCmdLine(const QString& a_fileName, int32_t a_transform, bool a_gray)
{
    int32_t retVal;

    retVal = openFITSFileByName(a_fileName, false);

    if (retVal == FITS_GENERAL_SUCCESS)
        return exportAllImages(false, a_transform, a_gray);

    return retVal;
}

void MainWindow::on_actionOpen_triggered()
{
    openFITSFile();
}

void MainWindow::on_actionOpenToolBar_triggered()
{
    openFITSFile();
}

void MainWindow::on_horizontalSliderR_valueChanged(int value)
{
    changeRGBColorChannelLevel(0, value);
}

void MainWindow::on_horizontalSliderG_valueChanged(int value)
{
    changeRGBColorChannelLevel(1, value);
}

void MainWindow::on_horizontalSliderB_valueChanged(int value)
{
    changeRGBColorChannelLevel(2, value);
}

void MainWindow::initGammaWidgetsValues()
{
    ui->horizontalSliderR->setValue(RGB_DEFAULT_VALUE);
    QString valueRstr = QString::number(RGB_DEFAULT_VALUE) + " %";
    valueRstr = valueRstr.rightJustified(LABELS_RIGHT_JUSTIFICATION_VALUE, ' ');
    ui->labelValueR->setText(valueRstr);

    ui->horizontalSliderG->setValue(RGB_DEFAULT_VALUE);
    QString valueGstr = QString::number(RGB_DEFAULT_VALUE) + " %";
    valueGstr = valueGstr.rightJustified(LABELS_RIGHT_JUSTIFICATION_VALUE, ' ');
    ui->labelValueG->setText(valueGstr);

    ui->horizontalSliderB->setValue(RGB_DEFAULT_VALUE);
    QString valueBstr = QString::number(RGB_DEFAULT_VALUE) + " %";
    valueBstr = valueBstr.rightJustified(LABELS_RIGHT_JUSTIFICATION_VALUE, ' ');
    ui->labelValueB->setText(valueBstr);

    ui->checkBoxGrayscale->setChecked(false);
    ui->checkBoxEyeComfort->setChecked(false);
}

void MainWindow::initMappingWidgetsValues()
{
    ui->comboBoxMapping->setCurrentIndex(0);
}

void MainWindow::clearWidgets()
{
    ui->tableWidgetHDUs->clearContents();
    ui->tableWidgetHDUs->model()->removeRows(0, ui->tableWidgetHDUs->rowCount());

    enableRestoreWidgets(false);

    ui->workspaceWidget->clearWidgets();

    m_scaleFactor = 100;
    m_sliderZoom->setValue(m_scaleFactor);

    //m_percentThreshold = 0;
    std::fill(m_percentThreshold, m_percentThreshold + 4, 0);
    ui->horizontalSliderPercent->setValue(m_percentThreshold[0]);

    ui->comboBoxMapping->setCurrentIndex(0);

    m_bEnableGammaWidgets = false;
    m_bEnableZoomWidget = false;
    m_bEnableMappingWidgets = false;

    enableZoomWidgets(m_bEnableZoomWidget);
    enableGammaWidgets(m_bEnableGammaWidgets);
    enableMappingWidgets(m_bEnableMappingWidgets);
    enableFileOpenRelatedWidgets(false);

    initHDUInfoWidgetValues();
    initHDUInfoWidgetMinMax();
}

int32_t MainWindow::exportAllImages(bool a_msgFlag, int32_t a_transform, bool a_gray)
{
    setStatus(STATUS_MESSAGE_IMAGE_EXPORT_HDUS);

    setProgress(50);

    int32_t retVal = m_fitsFile.exportAllImageHDUs(a_transform, a_gray);

    setStatus(STATUS_MESSAGE_READY);

    if (a_msgFlag)
    {
        if (retVal > FITS_PNG_EXPORT_ERROR)
        {
            if (retVal > 0)
                QMessageBox::information(this, "Success", IMAGE_EXPORT_HDUS_MESSAGE_SUCCESS);
            else if (retVal == 0)
                QMessageBox::warning(this, "Warning", IMAGE_EXPORT_NO_HDUS_MESSAGE);
        }
        else
            QMessageBox::critical(this, "Error", IMAGE_EXPORT_HDUS_MESSAGE_ERROR);
    }

    setProgress(0);

    return retVal;
}

void MainWindow::on_actionExportToolBar_triggered()
{
    exportAllImages();
}


void MainWindow::on_actionExport_triggered()
{
    exportAllImages();
}


void MainWindow::on_actionClose_triggered()
{
    closeFITSFile();
}


void MainWindow::on_actionCloseToolBar_triggered()
{
    closeFITSFile();
}


void MainWindow::on_actionZoomOutToolBar_triggered()
{
    m_scaleFactor -= 5;

    scaleImage();
}


void MainWindow::on_actionZoomInToolBar_triggered()
{
    m_scaleFactor += 5;

    scaleImage();
}


void MainWindow::on_actionZoomOut_triggered()
{
    m_scaleFactor -= 5;

    scaleImage();
}


void MainWindow::on_actionZoomIn_triggered()
{
    m_scaleFactor += 5;

    scaleImage();
}

void MainWindow::scaleImage()
{
    zoomControlsCheck();

    ui->workspaceWidget->scaleImage(m_scaleFactor);

    //scrollToCenter(); // no need to center the image during zooming

    m_sliderZoom->setValue(m_scaleFactor);
}

void MainWindow::zoomControlsCheck()
{
    bool bZoomOut = true;
    bool bZoomIn = true;

    if (m_scaleFactor <= MIN_IMAGE_SCALE_FACTOR)
    {
        m_scaleFactor = MIN_IMAGE_SCALE_FACTOR;
        bZoomOut = false;
    }

    if (m_scaleFactor >= MAX_IMAGE_SCALE_FACTOR)
    {
        bZoomIn = false;
        m_scaleFactor = MAX_IMAGE_SCALE_FACTOR;
    }

    ui->actionZoomOut->setEnabled(bZoomOut);
    ui->actionZoomOutToolBar->setEnabled(bZoomOut);

    ui->actionZoomIn->setEnabled(bZoomIn);
    ui->actionZoomInToolBar->setEnabled(bZoomIn);
}

void MainWindow::enableZoomWidgets(bool a_flag)
{
    ui->actionZoomOut->setEnabled(a_flag);
    ui->actionZoomOutToolBar->setEnabled(a_flag);

    ui->actionZoomIn->setEnabled(a_flag);
    ui->actionZoomInToolBar->setEnabled(a_flag);

    ui->actionFitWindow->setEnabled(a_flag);
    ui->actionFitWindowToolBar->setEnabled(a_flag);

    m_labelZoomLeft->setEnabled(a_flag);
    m_labelZoomRight->setEnabled(a_flag);
    m_labelZoomLevel->setEnabled(a_flag);
    m_sliderZoom->setEnabled(a_flag);
}

void MainWindow::enableImageExportWidgets(bool a_flag)
{
    ui->actionExportImage->setEnabled(a_flag);
    ui->actionExportImageToolBar->setEnabled(a_flag);
}

void MainWindow::scrollToCenter()
{
    ui->workspaceWidget->scrollToCenter();
}

void MainWindow::on_actionFitWindow_triggered()
{
    fitToWindow();
}


void MainWindow::on_actionFitWindowToolBar_triggered()
{
    fitToWindow();
}

void MainWindow::fitToWindow()
{
    uint32_t width = ui->workspaceWidget->getImageWidth();
    uint32_t height = ui->workspaceWidget->getImageHeight();

    QSize scrollSize = ui->workspaceWidget->getScrollAreaSize();

    int32_t scaleX = 100 * ((double)scrollSize.width() / width);
    int32_t scaleY = 100 * ((double)scrollSize.height() / height);

    m_scaleFactor = scaleX < scaleY ? scaleX : scaleY;

    if (m_scaleFactor < MIN_IMAGE_SCALE_FACTOR || m_scaleFactor > MAX_IMAGE_SCALE_FACTOR)
        m_scaleFactor = 100;

    ui->workspaceWidget->scaleImage(m_scaleFactor);
    m_sliderZoom->setValue(m_scaleFactor);
}

void MainWindow::on_actionRestoreOriginalToolBar_triggered()
{
    //restoreOriginalImage();
}

void MainWindow::on_actionRestoreOriginal_triggered()
{
    //restoreOriginalImage();
}

void MainWindow::backupOriginalImage()
{
    if (!m_bImageChanged)
    {
        //libnfits::LOG("in backupOriginalImage()");
        m_bImageChanged = true;
        ui->workspaceWidget->backupImage();

        enableRestoreWidgets();
    }
}

void MainWindow::restoreOriginalImage()
{
    ui->workspaceWidget->restoreImage();
    ui->workspaceWidget->reloadImage();
    ui->workspaceWidget->scaleImage(m_scaleFactor);

    initGammaWidgetsValues();
    initMappingWidgetsValues();
    enableRestoreWidgets(false);

    m_bImageChanged = false;
}

void MainWindow::setStatus(const QString &a_statusStr)
{
    statusBar()->showMessage(a_statusStr);
    statusBar()->repaint();
}

void MainWindow::enableRestoreWidgets(bool a_flag)
{
    ui->actionRestoreOriginal->setEnabled(a_flag);
    ui->actionRestoreOriginalToolBar->setEnabled(a_flag);
}

void MainWindow::on_progressChanged(qint32 a_value)
{
   setProgress(a_value);
}

bool MainWindow::exportImage()
{
    QDir path = QFileInfo(m_fitsFileName).absoluteDir();
    QString dir = path.absolutePath();

    QString filter = m_exportFormat + QString(" Files (*")  + m_exportFormat.toLower() + QString(")");
    QString fileName = QFileDialog::getSaveFileName(this, QString("Export ") + m_exportFormat + QString(" file"), dir, filter);

    if (fileName.isEmpty())
        return false;

    setStatus(STATUS_MESSAGE_IMAGE_EXPORT);
    bool exportRes =  ui->workspaceWidget->exportImage(fileName, m_exportFormat, m_exportQuality);
    setStatus(STATUS_MESSAGE_READY);

    if (exportRes)
        QMessageBox::information(this, "Success", IMAGE_EXPORT_MESSAGE_SUCCESS);
    else
        QMessageBox::critical(this, "Error", IMAGE_EXPORT_MESSAGE_ERROR);

    return exportRes;
}

void MainWindow::on_actionExportImageToolBar_triggered()
{
    exportImage();
}

void MainWindow::on_actionExportImage_triggered()
{
    exportImage();
}


void MainWindow::on_horizontalSliderQuality_valueChanged(int value)
{
    int valueQ = value;
    QString valueQstr = QString::number(valueQ);
    valueQstr = valueQstr.rightJustified(LABELS_RIGHT_JUSTIFICATION_VALUE, ' ');

    ui->labelQualityValue->setText(valueQstr);

    m_exportQuality = ui->horizontalSliderQuality->value();
}

void MainWindow::initImageExportSettingsWidgetValues()
{
    QString valueQstr = QString::number(IMAGE_EXPORT_DEFAULT_QUALITY);
    valueQstr = valueQstr.rightJustified(LABELS_RIGHT_JUSTIFICATION_VALUE, ' ');

    ui->horizontalSliderQuality->setValue(IMAGE_EXPORT_DEFAULT_QUALITY);
    ui->labelQualityValue->setText(valueQstr);

    ui->horizontalSliderQuality->setEnabled(false);
    ui->labelQualityValue->setEnabled(false);
    ui->labelQuality->setEnabled(false);
}

void MainWindow::enableImageExportSettigsWidgets(bool a_flag)
{
    ui->labelFormat->setEnabled(a_flag);
    ui->labelQuality->setEnabled(a_flag);
    ui->comboBoxFormat->setEnabled(a_flag);

    QString currentItem = ui->comboBoxFormat->currentText();

    if (currentItem != QString(IMAGE_EXPORT_TYPE_JPG).toUpper())
    {
        ui->horizontalSliderQuality->setEnabled(false);
        ui->labelQualityValue->setEnabled(false);
        ui->labelQuality->setEnabled(false);
    }
    else
    {
        ui->horizontalSliderQuality->setEnabled(a_flag);
        ui->labelQualityValue->setEnabled(a_flag);
        ui->labelQuality->setEnabled(a_flag);
    }
}

void MainWindow::enableMappingWidgets(bool a_flag)
{
    ui->labelMappping->setEnabled(a_flag);
    ui->comboBoxMapping->setEnabled(a_flag);

    int index = ui->comboBoxMapping->currentIndex();

    ui->labelPercent->setEnabled(static_cast<bool>(index) & a_flag);
    ui->horizontalSliderPercent->setEnabled(static_cast<bool>(index) & a_flag);
}

void MainWindow::on_comboBoxFormat_currentTextChanged(const QString &arg1)
{
    m_exportFormat = QString(arg1.toLower());
    m_exportQuality = ui->horizontalSliderQuality->value();

    if (arg1 != QString(IMAGE_EXPORT_TYPE_JPG).toUpper())
    {
        ui->horizontalSliderQuality->setEnabled(false);
        ui->labelQualityValue->setEnabled(false);
        ui->labelQuality->setEnabled(false);
    }
    else
    {
        ui->horizontalSliderQuality->setEnabled(true);
        ui->labelQualityValue->setEnabled(true);
        ui->labelQuality->setEnabled(true);
    }
}

QString MainWindow::getFileName() const
{
    return QFileInfo(m_fitsFileName).fileName();
}

void MainWindow::populateHDUInfoWidget(int32_t a_hduIndex)
{
    int32_t          resTemp = FITS_GENERAL_ERROR;
    libnfits::HDU    hdu;

    resTemp = m_fitsFile.getHDU(a_hduIndex, hdu);

    if (resTemp == FITS_GENERAL_SUCCESS)
    {
        std::vector<uint32_t> axises = hdu.getAxises();

        int32_t numAxises = axises.size();

        ui->labelNAXIS->setText("NAXIS: " + QString::number(numAxises));

        bool bSuccess;

        if (numAxises == 0)
            return;

        if (numAxises >= 1)
            ui->labelNAXIS1->setText("NAXIS1: " + QString::number(hdu.getKeywordValue<int32_t>("NAXIS1", bSuccess)));

        if (numAxises >= 2)
            ui->labelNAXIS2->setText("NAXIS2: " + QString::number(hdu.getKeywordValue<int32_t>("NAXIS2", bSuccess)));

        axises.clear();

        //updateHDUInfoWidgetMinMax();
    }
}

void MainWindow::initHDUInfoWidgetValues()
{
    ui->labelNAXIS->setText("NAXIS:");
    ui->labelNAXIS1->setText("NAXIS1:");
    ui->labelNAXIS2->setText("NAXIS2:");
}

void MainWindow::on_actionAbout_triggered()
{
    AboutDialog dlg;

    dlg.setAppVersion(NFITSVIEW_APP_FULL_NAME);

    dlg.exec();
}

void MainWindow::on_actionQuit_triggered()
{
    closeFITSFile();

    qApp->exit();
}

void MainWindow::on_tableWidgetHDUs_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous)
{
    if (current == nullptr)
        return;

    int32_t             row = current->row();
    libnfits::HDU       hdu;
    bool                bSuccess;

    if (row != -1)
    {
        populateHeaderWidget(row);
        populateRawDataWidget(row);
    }
    else
        return;

    int32_t currentImageIndex = ui->workspaceWidget->getCurrentImageHDUIndex();
    int32_t prevImageHDUIndex = ui->workspaceWidget->findImageHDUIndexByTableIndex(currentImageIndex);
    int32_t currentTabIndex = ui->workspaceWidget->currentIndex();

    //m_bImageChanged = false;
    //m_bEyeComfort = m_bGrayscale = false;

    if (prevImageHDUIndex != -1 && currentTabIndex == 0)
    {
        WidgetsStates widgetsStates = getWidgetsStates();
        widgetsStates.stored = true;
        ui->workspaceWidget->setImageHDUWidgetsStates(prevImageHDUIndex, widgetsStates);
    }

    int32_t resTemp = m_fitsFile.getHDU(row, hdu);

    if (resTemp == FITS_GENERAL_SUCCESS)
    {
        uint32_t axisesNumber = hdu.getKeywordValue<uint32_t>(FITS_KEYWORD_NAXIS, bSuccess);
        std::vector<uint32_t> axises = hdu.getAxises();

        uint8_t HDUType = hdu.getType();

        if ((HDUType == FITS_HDU_TYPE_IMAGE_XTENSION || HDUType == FITS_HDU_TYPE_PRIMARY) &&
            (axisesNumber >= 2 && bSuccess) && (axises.size() >= 2))
        {
            int32_t bitpix = hdu.getKeywordValue<int32_t>(FITS_KEYWORD_BITPIX, bSuccess);

            if (bSuccess)
            {
                WidgetsStates widgetsStates;
                int32_t hduIndex = ui->workspaceWidget->findImageHDUIndexByTableIndex(row);
                ui->workspaceWidget->getImageHDUWidgetsStates(hduIndex, widgetsStates);

                for (int32_t i = 0; i < FITS_NUMBER_OF_TRANSFORMS; ++i)
                    m_percentThreshold[i] = widgetsStates.gammaStates.mappingThreshold[i];

                ui->workspaceWidget->imageSetVisible(true);
                //ui->workspaceWidget->setImage(hdu.getPayload(), axises[0], axises[1], hdu.getPayloadOffset(), m_fitsFile.getSize(), bitpix);
                //ui->workspaceWidget->setImage(row);
                ui->workspaceWidget->setImage(row, widgetsStates.gammaStates.mappingValue, m_percentThreshold[ui->comboBoxMapping->currentIndex()]);

                fitToWindow();

                if (std::abs(bitpix) > 8)
                    m_bEnableMappingWidgets = true;
                else
                    m_bEnableMappingWidgets = false;

                enableMappingWidgets(m_bEnableMappingWidgets);

                bool bPosBitpix = bitpix > 0 ? false : true;
                enableDisableMappingComboItem(2, bPosBitpix);
                enableDisableMappingComboItem(3, bPosBitpix);

                m_bEnableZoomWidget = true;
                enableZoomWidgets(m_bEnableZoomWidget);

                m_bEnableGammaWidgets = true;
                enableGammaWidgets(m_bEnableGammaWidgets);

                enableImageExportWidgets();
                enableImageExportSettigsWidgets();

                updateHDUInfoWidgetMinMax();

                if (hduIndex != -1)
                {
                    ui->workspaceWidget->getImageHDUWidgetsStates(hduIndex, widgetsStates);

                    //libnfits::LOG("HDU changed to hduIndex = %" , hduIndex);

                    if (widgetsStates.stored)
                    {
                        //libnfits::LOG("widgetStates.imageChanged = % , widgetStates.gammaStates.gray = % , widgetStates.gammaStates.eye = %",
                        //              widgetStates.imageChanged, widgetStates.gammaStates.gray, widgetStates.gammaStates.eye);
                        setWidgetsStates(widgetsStates);

                        int32_t index = ui->workspaceWidget->getCurrentImageHDUIndex();

                        if (widgetsStates.gammaStates.gray)
                        {

                            //libnfits::LOG("gray: index = % , hduIndex = %", index, hduIndex);
                            restoreRGBColorChannelLevelsImage(index);
                            grayScale();
                            ui->workspaceWidget->scaleImage(m_scaleFactor);
                        }

                        if (widgetsStates.gammaStates.eye)
                        {
                            //libnfits::LOG("eye: index = % , hduIndex = %", index, hduIndex);
                            restoreRGBColorChannelLevelsImage(index);
                            eyeComfort();
                            ui->workspaceWidget->scaleImage(m_scaleFactor);
                        }
                    }
                    else
                    {
                        m_bImageChanged = false;
                        m_bEyeComfort = false;
                        m_bGrayscale = false;

                        initGammaWidgetsValues();
                        initMappingWidgetsValues();
                    }
                }
            }
        }
        else
        {
            m_bEnableGammaWidgets = false;
            m_bEnableZoomWidget = false;
            m_bEnableMappingWidgets = false;
            enableGammaWidgets(m_bEnableGammaWidgets);
            enableZoomWidgets(m_bEnableZoomWidget);
            enableImageExportWidgets(false);
            enableImageExportSettigsWidgets(false);
            enableMappingWidgets(m_bEnableMappingWidgets);

            //ui->workspaceWidget->imageSetVisible(false);

            ui->workspaceWidget->resetCurrentImageHDUIndex();

            ui->workspaceWidget->setNoImageDataImage();

            initHDUInfoWidgetMinMax();
        }

        axises.clear();

        initHDUInfoWidgetValues();
        populateHDUInfoWidget(row);
    }

    ui->workspaceWidget->setCurrentIndex(0);

    //libnfits::LOG("last line of on_tableWidgetHDUs_currentItemChanged() % ", m_bImageChanged);
}

void MainWindow::on_workspaceWidget_sendGammaCorrectionTabEnabled(bool a_flag)
{
    enableGammaWidgets(m_bEnableGammaWidgets & a_flag);
    enableZoomWidgets(m_bEnableZoomWidget & a_flag);
    enableMappingWidgets(m_bEnableMappingWidgets & a_flag);
}

void MainWindow::setProgress(int32_t a_progress)
{
    m_progressBar->setValue(a_progress);
    m_progressBar->repaint();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    //// TODO: may be required to change the size adjusting and zooming behavior during rezie

    QSize scrollSize = ui->workspaceWidget->getScrollAreaSize();
    QSize imageLabelSize = ui->workspaceWidget->getImageLabelSize();

    if (scrollSize.width() > imageLabelSize.width() && scrollSize.height() > imageLabelSize.height())
        fitToWindow();
}

int32_t MainWindow::setAllWorkspaceImages()
{
    libnfits::HDU       hdu;

    int32_t retVal = 0;

    bool bSuccess;

    int32_t countHDU = m_fitsFile.getNumberOfHDUs();

    for (uint32_t h = 0; h < countHDU; ++h)
    {
        int32_t resTemp = m_fitsFile.getHDU(h, hdu);

        if (resTemp == FITS_GENERAL_SUCCESS)
        {
            uint32_t axisesNumber = hdu.getKeywordValue<uint32_t>(FITS_KEYWORD_NAXIS, bSuccess);
            std::vector<uint32_t> axises = hdu.getAxises();

            uint8_t HDUType = hdu.getType();

            //WidgetsStates widgetStates = getWidgetsStates(); // (2) <-> (1)

            if ((HDUType == FITS_HDU_TYPE_IMAGE_XTENSION || HDUType == FITS_HDU_TYPE_PRIMARY) && (axisesNumber >= 2 && bSuccess) && (axises.size() >= 2))
            {
                int32_t bitpix = hdu.getKeywordValue<int32_t>(FITS_KEYWORD_BITPIX, bSuccess);

                if (bSuccess)
                {
                    WidgetsStates widgetStates = getWidgetsStates(); // (1) <-> (2)

                    //// The old function, in the new one packed params into the structure
                    //ui->workspaceWidget->insertImage(hdu.getPayload(), axises[0], axises[1], hdu.getPayloadOffset(), m_fitsFile.getSize(),
                    //                                 bitpix, h, widgetStates);

                    ImageParams imageParams;
                    imageParams.width = axises[0];
                    imageParams.height = axises[1];
                    imageParams.HDUBaseOffset = hdu.getPayloadOffset();
                    imageParams.maxDataBufferSize = m_fitsFile.getSize();
                    imageParams.bitpix = bitpix;
                    imageParams.hduIndex = h;

                    bool bZSuccess = false, bSSuccess = false;

                    double bzero = hdu.getKeywordValue<double>(FITS_KEYWORD_BZERO, bZSuccess);
                    double bscale = hdu.getKeywordValue<double>(FITS_KEYWORD_BSCALE, bSSuccess);

                    imageParams.bzero = FITS_BZERO_DEFAULT_VALUE;
                    if (bZSuccess)
                        imageParams.bzero = bzero;

                    imageParams.bscale = FITS_BSCALE_DEFAULT_VALUE;
                    if (bSSuccess)
                        imageParams.bscale = bscale;

                    ui->workspaceWidget->insertImage(hdu.getPayload(), imageParams, widgetStates,
                                                     FITS_FLOAT_DOUBLE_NO_TRANSFORM, FITS_VALUE_DISTRIBUTION_RANGE_MIN_THREASHOLD);

                    ++retVal;
                }
            }
        }
    }

    return retVal;
}

WidgetsStates MainWindow::getWidgetsStates() const
{
    WidgetsStates       widgetStates;
    GammaWidgetsStates  gammaStates;
    ExportWidgetsStates exportStates;
    ZoomWidgetsStates   zoomStates;
    ScrollState         scrollStates;


    gammaStates.rLevel = ui->horizontalSliderR->value();
    gammaStates.gLevel = ui->horizontalSliderG->value();
    gammaStates.bLevel = ui->horizontalSliderB->value();

    gammaStates.gray = ui->checkBoxGrayscale->isChecked();
    gammaStates.eye = ui->checkBoxEyeComfort->isChecked();

    gammaStates.rLevelEnabled = ui->horizontalSliderR->isEnabled();
    gammaStates.gLevelEnabled = ui->horizontalSliderG->isEnabled();
    gammaStates.bLevelEnabled = ui->horizontalSliderB->isEnabled();

    gammaStates.grayEnabled = ui->checkBoxGrayscale->isEnabled();
    gammaStates.eyeEnabled = ui->checkBoxEyeComfort->isEnabled();    
    gammaStates.rgbResetEnabled = ui->resetRGBButton->isEnabled();

    gammaStates.mappingValue = ui->comboBoxMapping->currentIndex();
    gammaStates.mappingEnabled = ui->comboBoxMapping->isEnabled();



    exportStates.format = ui->comboBoxFormat->currentIndex();
    exportStates.formatEnabled = ui->comboBoxFormat->isEnabled();

    exportStates.quality = ui->horizontalSliderQuality->value();
    exportStates.qualityEnabled = ui->horizontalSliderQuality->isEnabled();



    zoomStates.factor = m_sliderZoom->value();
    zoomStates.zoomEnabled = m_sliderZoom->isEnabled();

    zoomStates.zoomInEnabled = ui->actionZoomIn->isEnabled();
    zoomStates.zoomOutEnabled = ui->actionZoomOut->isEnabled();

    zoomStates.fitWindowEnabled = ui->actionFitWindow->isEnabled();

    scrollStates.x = ui->workspaceWidget->getScrollPosX();
    scrollStates.y = ui->workspaceWidget->getScrollPosY();

    widgetStates.gammaStates = gammaStates;
    widgetStates.exportStates = exportStates;
    widgetStates.zoomStates = zoomStates;
    widgetStates.scrollState = scrollStates;

    widgetStates.imageChanged = m_bImageChanged;


    for (int32_t i = 0; i < FITS_NUMBER_OF_TRANSFORMS; ++i)
        widgetStates.gammaStates.mappingThreshold[i] = m_percentThreshold[i];

    widgetStates.gammaStates.mappingThresholdEnabled = ui->horizontalSliderPercent->isEnabled();

    return widgetStates;
}

void MainWindow::setWidgetsStates(const WidgetsStates& a_widgetsStates)
{
    m_bImageChanged = a_widgetsStates.imageChanged;
    m_bGrayscale = a_widgetsStates.gammaStates.gray;
    m_bEyeComfort = a_widgetsStates.gammaStates.eye;

    ui->horizontalSliderR->setValue(a_widgetsStates.gammaStates.rLevel);
    ui->horizontalSliderG->setValue(a_widgetsStates.gammaStates.gLevel);
    ui->horizontalSliderB->setValue(a_widgetsStates.gammaStates.bLevel);

    QString valueStr;

    valueStr = QString::number(a_widgetsStates.gammaStates.rLevel) + " %";
    valueStr = valueStr.rightJustified(LABELS_RIGHT_JUSTIFICATION_VALUE, ' ');
    ui->labelValueR->setText(valueStr);

    valueStr = QString::number(a_widgetsStates.gammaStates.gLevel) + " %";
    valueStr = valueStr.rightJustified(LABELS_RIGHT_JUSTIFICATION_VALUE, ' ');
    ui->labelValueB->setText(valueStr);

    valueStr = QString::number(a_widgetsStates.gammaStates.bLevel) + " %";
    valueStr = valueStr.rightJustified(LABELS_RIGHT_JUSTIFICATION_VALUE, ' ');
    ui->labelValueB->setText(valueStr);

    ui->checkBoxGrayscale->setChecked(a_widgetsStates.gammaStates.gray);
    ui->checkBoxEyeComfort->setChecked(a_widgetsStates.gammaStates.eye);

    ui->horizontalSliderR->setEnabled(a_widgetsStates.gammaStates.rLevelEnabled );
    ui->horizontalSliderG->setEnabled(a_widgetsStates.gammaStates.gLevelEnabled);
    ui->horizontalSliderB->setEnabled(a_widgetsStates.gammaStates.bLevelEnabled);

    ui->checkBoxGrayscale->setEnabled(a_widgetsStates.gammaStates.grayEnabled);
    ui->checkBoxEyeComfort->setEnabled(a_widgetsStates.gammaStates.eyeEnabled);
    ui->resetRGBButton->setEnabled(a_widgetsStates.gammaStates.rgbResetEnabled);

    ui->comboBoxMapping->setCurrentIndex(a_widgetsStates.gammaStates.mappingValue);
    ui->labelMappping->setEnabled(a_widgetsStates.gammaStates.mappingEnabled);
    ui->comboBoxMapping->setEnabled(a_widgetsStates.gammaStates.mappingEnabled);

    ui->comboBoxFormat->setCurrentIndex(a_widgetsStates.exportStates.format);
    ui->comboBoxFormat->setEnabled(a_widgetsStates.exportStates.formatEnabled);

    ui->horizontalSliderQuality->setValue(a_widgetsStates.exportStates.quality);
    ui->horizontalSliderQuality->setEnabled(a_widgetsStates.exportStates.qualityEnabled);


    m_sliderZoom->setValue(a_widgetsStates.zoomStates.factor);
    m_sliderZoom->setEnabled(a_widgetsStates.zoomStates.zoomEnabled);

    ui->actionZoomIn->setEnabled(a_widgetsStates.zoomStates.zoomInEnabled);
    ui->actionZoomInToolBar->setEnabled(a_widgetsStates.zoomStates.zoomInEnabled);

    ui->actionZoomOut->setEnabled(a_widgetsStates.zoomStates.zoomOutEnabled);
    ui->actionZoomOutToolBar->setEnabled(a_widgetsStates.zoomStates.zoomOutEnabled);

    ui->actionFitWindow->setEnabled(a_widgetsStates.zoomStates.fitWindowEnabled);

    m_scaleFactor = a_widgetsStates.zoomStates.factor;


    ui->workspaceWidget->setScrollPosX(a_widgetsStates.scrollState.x);
    ui->workspaceWidget->setScrollPosY(a_widgetsStates.scrollState.y);

    int32_t currentIndex = ui->comboBoxMapping->currentIndex();
    ui->horizontalSliderPercent->setValue(a_widgetsStates.gammaStates.mappingThreshold[currentIndex]);
    ui->horizontalSliderPercent->setEnabled(a_widgetsStates.gammaStates.mappingThresholdEnabled);
}

void MainWindow::on_checkBoxEyeComfort_stateChanged(int arg1)
{
    enableRGBWidgets(!arg1);
    enableMappingWidgets(!arg1 & m_bEnableMappingWidgets);

    ui->checkBoxGrayscale->setEnabled(!arg1);

    int32_t scrollX = ui->workspaceWidget->getScrollPosX();
    int32_t scrollY = ui->workspaceWidget->getScrollPosY();

    if (arg1 && !m_bEyeComfort)
    {
        eyeComfort();

        m_bEyeComfort = true;
    }
    else if (!arg1 && m_bEyeComfort)
    {
        int32_t index = ui->workspaceWidget->getCurrentImageHDUIndex();
        if (index != -1)
            restoreRGBColorChannelLevelsImage(index);

        m_bEyeComfort = false;
    }

    //ui->workspaceWidget->reloadImage();
    ui->workspaceWidget->scaleImage(m_scaleFactor);
    ui->workspaceWidget->setScrollPosX(scrollX);
    ui->workspaceWidget->setScrollPosY(scrollY);
}

void MainWindow::changeRGBColorChannelLevel(uint8_t a_channel, int8_t a_value)
{
    //libnfits::LOG("in changeRGBColorChannelLevel(), before backupOriginalImage() call, m_bImageChanged = %", m_bImageChanged);
    backupOriginalImage();

    QString valueStr = QString::number(a_value) + " %";
    valueStr = valueStr.rightJustified(LABELS_RIGHT_JUSTIFICATION_VALUE, ' ');

    ui->workspaceWidget->restoreImage();

    int8_t tmpVal;
    if (a_channel == 0)
    {
        ui->labelValueR->setText(valueStr);

        tmpVal = ui->horizontalSliderG->value();
        if (tmpVal != 0)
            ui->workspaceWidget->changeChannelLevel(1, 1 + (float)(tmpVal)/100);

        tmpVal = ui->horizontalSliderB->value();
        if (tmpVal != 0)
            ui->workspaceWidget->changeChannelLevel(2, 1 + (float)(tmpVal)/100);
    }
    else if (a_channel == 1)
    {
        ui->labelValueG->setText(valueStr);

        tmpVal = ui->horizontalSliderR->value();
        if (tmpVal != 0)
            ui->workspaceWidget->changeChannelLevel(0, 1 + (float)(tmpVal)/100);

        tmpVal = ui->horizontalSliderB->value();
        if (tmpVal != 0)
            ui->workspaceWidget->changeChannelLevel(2, 1 + (float)(tmpVal)/100);
    }
    else if (a_channel == 2)
    {
        ui->labelValueB->setText(valueStr);

        tmpVal = ui->horizontalSliderR->value();
        if (tmpVal != 0)
            ui->workspaceWidget->changeChannelLevel(0, 1 + (float)(tmpVal)/100);

        tmpVal = ui->horizontalSliderG->value();
        if (tmpVal != 0)
            ui->workspaceWidget->changeChannelLevel(1, 1 + (float)(tmpVal)/100);
    }

    //ui->workspaceWidget->restoreImage();
    ui->workspaceWidget->changeChannelLevel(a_channel, 1 + (float)(a_value)/100);
    ui->workspaceWidget->reloadImage();
    ui->workspaceWidget->scaleImage(m_scaleFactor);
}

void MainWindow::changeRGBColorChannelLevels(int8_t a_rValue, int8_t a_gValue, int8_t a_bValue)
{
    changeRGBColorChannelLevel(0, a_rValue);
    changeRGBColorChannelLevel(1, a_gValue);
    changeRGBColorChannelLevel(2, a_bValue);
}

void MainWindow::restoreRGBColorChannelLevelsImage(int32_t a_hduIndex, uint32_t a_transformType)
//void MainWindow::restoreRGBColorChannelLevelsImage(int32_t a_hduIndex, uint32_t a_transformType,)
{
    int8_t rValue = ui->horizontalSliderR->value();
    int8_t gValue = ui->horizontalSliderG->value();
    int8_t bValue = ui->horizontalSliderB->value();

    if (rValue != 0 || gValue != 0 || bValue != 0)
    {
        //libnfits::LOG("in restoreRGBColorChannelLevelsImage(), if case, a_hduIndex = %", a_hduIndex);
        ui->workspaceWidget->setImage(a_hduIndex, a_transformType, m_percentThreshold[ui->comboBoxMapping->currentIndex()]);
        changeRGBColorChannelLevels(rValue, gValue, bValue);
    }
    else
    {
        //libnfits::LOG("in restoreRGBColorChannelLevelsImage(), else case");
        ui->workspaceWidget->restoreImage();
        ui->workspaceWidget->reloadImage();
    }
}

void MainWindow::grayScale()
{
    //libnfits::LOG("in grayScale() , m_bImageChanged = ", m_bImageChanged);
    backupOriginalImage();
    ui->workspaceWidget->convertImage2Grayscale();
    enableRestoreWidgets(false);
    ui->workspaceWidget->reloadImage();

}

void MainWindow::eyeComfort()
{
    //libnfits::LOG("in eyeComfort() , m_bImageChanged = ", m_bImageChanged);
    backupOriginalImage();
    ui->workspaceWidget->convertImage2EyeComfort();
    enableRestoreWidgets(false);
    ui->workspaceWidget->reloadImage();
}

void MainWindow::on_resetRGBButton_clicked()
{
    int32_t r,g,b;

    r = ui->horizontalSliderR->value();
    g = ui->horizontalSliderG->value();
    b = ui->horizontalSliderB->value();

    if (r != 0)
    {
        changeRGBColorChannelLevel(0, 0);
        ui->horizontalSliderR->setValue(RGB_DEFAULT_VALUE);
    }

    if (g != 0)
    {
        changeRGBColorChannelLevel(1, 0);
        ui->horizontalSliderG->setValue(RGB_DEFAULT_VALUE);
    }

    if (b != 0)
    {
        changeRGBColorChannelLevel(2, 0);
        ui->horizontalSliderB->setValue(RGB_DEFAULT_VALUE);
    }
}


void MainWindow::on_comboBoxMapping_currentIndexChanged(int index)
{
    int32_t scrollX = ui->workspaceWidget->getScrollPosX();
    int32_t scrollY = ui->workspaceWidget->getScrollPosY();

    if (!m_fitsFile.isOpen())
        return;

    int32_t transformType = index; //convertComboIndexToTransformType(index);  // reordered values in defs.h

    if (transformType == FITS_UNDEFINED_VALUE)
        return;

    enableRestoreWidgets(false);  //// legacy code calling, those menu and button items are not used anymore

    //libnfits::LOG("Transform type changed to: % ", transformType);

    ui->workspaceWidget->reloadImageWithTransformation(transformType, m_percentThreshold[transformType]);
    m_bImageChanged = false;
    backupOriginalImage();
    restoreRGBColorChannelLevelsImage(ui->workspaceWidget->getCurrentImageHDUIndex(), transformType);

    ui->workspaceWidget->scaleImage(m_scaleFactor);
    ui->workspaceWidget->setScrollPosX(scrollX);
    ui->workspaceWidget->setScrollPosY(scrollY);

    ui->labelPercent->setEnabled(index);
    ui->horizontalSliderPercent->setEnabled(index);
    ui->horizontalSliderPercent->setValue(m_percentThreshold[index]);

    updateHDUInfoWidgetMinMax();
}

int32_t MainWindow::convertComboIndexToTransformType(int32_t a_index) const
{
    int32_t transformType = FITS_UNDEFINED_VALUE;

    switch (a_index)
    {
        case 0:
            transformType = FITS_FLOAT_DOUBLE_NO_TRANSFORM;
            break;
        case 1:
            transformType = FITS_FLOAT_DOUBLE_LINEAR_TRANSFORM_POSITIVE;
            break;
        case 2:
            transformType = FITS_FLOAT_DOUBLE_LINEAR_TRANSFORM_NEGATIVE_POSITIVE;
            break;
        case 3:
            transformType = FITS_FLOAT_DOUBLE_LINEAR_TRANSFORM_NEGATIVE;
            break;
        default:
            break;
    }

    return transformType;
}

void MainWindow::enableDisableMappingComboItem(uint32_t a_index, bool a_enable)
{
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->comboBoxMapping->model());

    if (model == nullptr)
        return;

    QStandardItem* item = model->item(a_index);

    if (item == nullptr)
        return;

    //item->setFlags(!a_enable ? item->flags() & ~Qt::ItemIsEnabled: item->flags() | Qt::ItemIsEnabled);
}

void MainWindow::onSendMousewheelZoomChanged(int32_t a_scaleFactor)
{
    m_scaleFactor += a_scaleFactor;

    scaleImage();
}

void MainWindow::checkForUpdate()
{
    ui->actionCheckForUpdate->setEnabled(false);
    ui->actionCheckForUpdateToolBar->setEnabled(false);

    m_fileDownloader->doRequest();
}

int32_t MainWindow::updateChecked()
{
    ui->actionCheckForUpdate->setEnabled(true);
    ui->actionCheckForUpdateToolBar->setEnabled(true);

    QByteArray data = m_fileDownloader->getDownloadedData();

    int32_t newVersion = data.toInt();
    int32_t curVersion = QString(NFITSVIEW_VERSION_INT).toInt();

    QString text = "";
    QString caption = "Update status";

    if (curVersion < newVersion)
    {
        text = "Your current version of nFITSview is " + QString(TOSTRING(NFITSVIEW_MAJOR_VERSION)) + "." +
               QString(TOSTRING(NFITSVIEW_MINOR_VERSION)) + ". The latest version is " +
               QString::number(newVersion / 10) + "." + QString::number(newVersion % 10) +
               " and is available for download. Do you want to upgrade to the latest version?";

        QMessageBox::StandardButton reply = QMessageBox::question(this, caption, text, QMessageBox::Yes|QMessageBox::No);

        if (reply == QMessageBox::Yes)
            QDesktopServices::openUrl(QUrl(UPDATE_DOWNLOAD_URL));
    }
    else
    {
        text = "You already have the latest version of nFITSview!";
        QMessageBox::information(this, caption, text, QMessageBox::Ok);
    }

    return newVersion;
}

void MainWindow::on_actionCheckForUpdateToolBar_triggered()
{
    checkForUpdate();
}

void MainWindow::on_actionCheckForUpdate_triggered()
{
    checkForUpdate();
}

void MainWindow::updateHDUInfoWidgetMinMax()
{
    double min, max;
    int64_t minL, maxL;
    int32_t bitpix = ui->workspaceWidget->getBitPix();

    if (bitpix == -64 || bitpix == -32)
    {
        if (ui->horizontalSliderPercent->value() == 0)
        {
            min = ui->workspaceWidget->getMinValue();
            max = ui->workspaceWidget->getMaxValue();
        }
        else
        {
            min = ui->workspaceWidget->getDistribMinValue();
            max = ui->workspaceWidget->getDistribMaxValue();
        }

        ui->labelMINValue->setText("Min: " + QString::number(min));
        ui->labelMAXValue->setText("Max: " + QString::number(max));
    }
    else
    {
        if (ui->horizontalSliderPercent->value() == 0)
        {
            minL = ui->workspaceWidget->getMinValueL();
            maxL = ui->workspaceWidget->getMaxValueL();
        }
        else
        {
            minL = ui->workspaceWidget->getDistribMinValueL();
            maxL = ui->workspaceWidget->getDistribMaxValueL();
        }

        ui->labelMINValue->setText("Min: " + QString::number(minL));
        ui->labelMAXValue->setText("Max: " + QString::number(maxL));
    }
}

void MainWindow::on_horizontalSliderPercent_valueChanged(int value)
{
    QString valueStr = THRESHOLD_LABEL_TEXT + QString::number(100 - value) + " %";
    valueStr = valueStr.rightJustified(16, ' ');
    ui->labelPercent->setText(valueStr);

    int32_t scrollX = ui->workspaceWidget->getScrollPosX();
    int32_t scrollY = ui->workspaceWidget->getScrollPosY();

    if (!m_fitsFile.isOpen())
        return;

    int32_t transformType = ui->comboBoxMapping->currentIndex(); //convertComboIndexToTransformType(index); // reordered values in defs.h
    m_percentThreshold[transformType] = value;

    if (transformType == FITS_UNDEFINED_VALUE || transformType == FITS_FLOAT_DOUBLE_NO_TRANSFORM)
        return;

    enableRestoreWidgets(false);  //// legacy code calling, those menu and button items are not used anymore

    //libnfits::LOG("Transform type changed to: % ", transformType);

    ui->workspaceWidget->reloadImageWithTransformation(transformType, m_percentThreshold[transformType]);
    m_bImageChanged = false;
    backupOriginalImage();
    restoreRGBColorChannelLevelsImage(ui->workspaceWidget->getCurrentImageHDUIndex(), transformType);

    ui->workspaceWidget->scaleImage(m_scaleFactor);
    ui->workspaceWidget->setScrollPosX(scrollX);
    ui->workspaceWidget->setScrollPosY(scrollY);

    updateHDUInfoWidgetMinMax();
}

void MainWindow::initHDUInfoWidgetMinMax()
{
    ui->labelMINValue->setText("Min:");
    ui->labelMAXValue->setText("Max:");
}

void MainWindow::on_minRButton_clicked()
{
    ui->horizontalSliderR->setValue(RGB_MIN_VALUE);
}


void MainWindow::on_defaultRButton_clicked()
{
    ui->horizontalSliderR->setValue(RGB_DEFAULT_VALUE);
}


void MainWindow::on_maxRButton_clicked()
{
    ui->horizontalSliderR->setValue(RGB_MAX_VALUE);
}


void MainWindow::on_minGButton_clicked()
{
    ui->horizontalSliderG->setValue(RGB_MIN_VALUE);
}


void MainWindow::on_defaultGButton_clicked()
{
    ui->horizontalSliderG->setValue(RGB_DEFAULT_VALUE);
}


void MainWindow::on_maxGButton_clicked()
{
    ui->horizontalSliderG->setValue(RGB_MAX_VALUE);
}


void MainWindow::on_minBButton_clicked()
{
    ui->horizontalSliderB->setValue(RGB_MIN_VALUE);
}


void MainWindow::on_defaultBButton_clicked()
{
    ui->horizontalSliderB->setValue(RGB_DEFAULT_VALUE);
}


void MainWindow::on_maxBButton_clicked()
{
    ui->horizontalSliderB->setValue(RGB_MAX_VALUE);
}


void MainWindow::on_horizontalSliderPercent_sliderMoved(int position)
{
    QString valueStr = THRESHOLD_LABEL_TEXT + QString::number(100 - position) + " %";
    valueStr = valueStr.rightJustified(16, ' ');
    ui->labelPercent->setText(valueStr);
}

void MainWindow::onSendMousedragScrollChanged(int32_t a_scrollX, int32_t a_scrollY)
{
    bool isScrollX = ui->workspaceWidget->isScrollVisibleX();
    bool isScrollY = ui->workspaceWidget->isScrollVisibleY();

    /// moving the image horizontally
    if (isScrollX)
    {
        int32_t scrollX = ui->workspaceWidget->getScrollPosX();

        ui->workspaceWidget->setScrollPosX(scrollX - a_scrollX);
    }

    /// moving the image vertically
    if (isScrollY)
    {
        int32_t scrollY = ui->workspaceWidget->getScrollPosY();

        ui->workspaceWidget->setScrollPosY(scrollY - a_scrollY);
    }
}

void MainWindow::on_actionAboutToolBar_triggered()
{
    AboutDialog dlg;

    dlg.setAppVersion(NFITSVIEW_APP_FULL_NAME);

    dlg.exec();
}

