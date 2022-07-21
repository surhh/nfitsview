#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>
#include <QTextEdit>
#include <QThread>
#include <QMessageBox>

#include "libnfits/hdu.h"
#include "libnfits/fitsfile.h"
#include "libnfits/header.h"
#include "libnfits/headerrecord.h"

#include "aboutdialog.h"

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
    , m_bImageChanged(false)
    , m_bEnableGammaWidgets(false)
    , m_bEnableZoomWidget(false)
    , m_exportFormat(IMAGE_EXPORT_TYPE_PNG)
    , m_exportQuality(IMAGE_EXPORT_DEFAULT_QUALITY)
{
    ui->setupUi(this);

    createStatusBarWidgets();

    connect(m_sliderZoom, SIGNAL(valueChanged(int)), SLOT(om_m_sliderZoom_valueChanged(int)));
    connect(this, SIGNAL(sendProgressChanged(qint32)), SLOT(on_progressChanged(qint32)));
    connect(ui->workspaceWidget, SIGNAL(sendGammaCorrectionTabEnabled(bool)), this, SLOT(on_workspaceWidget_sendGammaCorrectionTabEnabled(bool)));

    //// currently the Undo/Redo logic is not implemented, not needed so far, so disabling the controls
    ui->actionUndo->setVisible(false);
    ui->actionUndoToolBar->setVisible(false);
    ui->actionRedo->setVisible(false);
    ui->actionRedoToolBar->setVisible(false);
    //// end of disabling Undo/Redo controls

    enableRestoreWidgets(false);
    enableGammaWidgets(m_bEnableGammaWidgets);
    enableFileOpenRelatedWidgets(false);
    enableZoomWidgets(m_bEnableZoomWidget);
    enableImageExportWidgets(false);
    enableImageExportSettigsWidgets(false);

    initGammaWidgetsValues();
    initImageExportSettingsWidgetValues();

    ui->toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    ui->tableWidgetHDUs->setColumnWidth(0,50);
    ui->tableWidgetHDUs->setColumnWidth(1,120);
    ui->tableWidgetHDUs->setColumnWidth(2,50);

    ui->actionZoomIn->setShortcut(QKeySequence::ZoomIn);

    setWindowTitle(NFITSVIEW_WND_CAPTION);
}

MainWindow::~MainWindow()
{
    delete m_progressBar;
    delete m_labelZoomLeft;
    delete m_labelZoomRight;
    delete m_labelZoomLevel;
    delete m_sliderZoom;

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

void MainWindow::enableGammaWidgets(bool a_flag)
{
    ui->horizontalSliderR->setEnabled(a_flag);
    ui->horizontalSliderG->setEnabled(a_flag);
    ui->horizontalSliderB->setEnabled(a_flag);

    ui->labelR->setEnabled(a_flag);
    ui->labelG->setEnabled(a_flag);
    ui->labelB->setEnabled(a_flag);

    ui->labelRGBInfo->setEnabled(a_flag);

    ui->checkBoxGrayscale->setEnabled(a_flag);

    ui->labelValueR->setEnabled(a_flag);
    ui->labelValueG->setEnabled(a_flag);
    ui->labelValueB->setEnabled(a_flag);
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
}

void MainWindow::on_checkBoxGrayscale_stateChanged(int arg1)
{
    ui->horizontalSliderR->setEnabled(!arg1);
    ui->horizontalSliderG->setEnabled(!arg1);
    ui->horizontalSliderB->setEnabled(!arg1);

    ui->labelR->setEnabled(!arg1);
    ui->labelG->setEnabled(!arg1);
    ui->labelB->setEnabled(!arg1);

    ui->labelRGBInfo->setEnabled(!arg1);

    ui->labelValueR->setEnabled(!arg1);
    ui->labelValueG->setEnabled(!arg1);
    ui->labelValueB->setEnabled(!arg1);

    if (arg1)
    {
        ui->workspaceWidget->convertImage2Grayscale();
        enableRestoreWidgets(false);
    }
    else
        ui->workspaceWidget->restoreImage();

    ui->workspaceWidget->reloadImage();
    ui->workspaceWidget->scaleImage(m_scaleFactor);
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

            ui->tableWidgetHDUs->setItem(ui->tableWidgetHDUs->rowCount()-1, 0, new QTableWidgetItem(HDUtypeStr));
            ui->tableWidgetHDUs->setItem(ui->tableWidgetHDUs->rowCount()-1, 1, new QTableWidgetItem(QString::number(sizeHDU)));
            ui->tableWidgetHDUs->setItem(ui->tableWidgetHDUs->rowCount()-1, 2, new QTableWidgetItem(bitpixStr));
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

qint32 MainWindow::openFITSFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open FITS file", "~/", "FITS Files (*.fits)");

    return openFITSFileByName(fileName);    
}

qint32 MainWindow::openFITSFileByNameFromCmdLine(const QString &a_fileName)
{
    qint32 resOpen = openFITSFileByName(a_fileName);

    return resOpen;
}

qint32 MainWindow::openFITSFileByName(const QString& a_fileName)
{
    qint32 result = FITS_GENERAL_ERROR;

    setStatus(STATUS_MESSAGE_IMAGE_LOAD);

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

    clearWidgets();

    resTemp = m_fitsFile.loadFile(m_fitsFileName.toStdString());

    m_fitsFile.setCallbackFunction(progressCallbackFunction, (void *)this);

    if (resTemp == FITS_GENERAL_SUCCESS)
    {
        populateHDUsWidget();
        enableFileOpenRelatedWidgets();

        setWindowTitle(QString(NFITSVIEW_WND_CAPTION) + " - " + getFileName());
    }
    else
    {
        QMessageBox messageBox;
        messageBox.critical(this, FITS_MSG_ERROR_TYPE, FITS_MSG_ERROR_OPENING_FILE);
    }

    setStatus(STATUS_MESSAGE_READY);

    return resTemp;
}

qint32 MainWindow::closeFITSFile()
{
    if (m_fitsFile.closeFile() != FITS_MEMORY_MAP_FILE_SUCCESS)
        return FITS_GENERAL_ERROR;

    clearWidgets();
    initGammaWidgetsValues();
    enableImageExportWidgets(false);
    enableImageExportSettigsWidgets(false);

    ui->workspaceWidget->clearImage();

    m_scaleFactor = 100;

    m_bImageChanged = false;

    m_exportFormat = ui->comboBoxFormat->currentText();
    m_exportQuality = ui->horizontalSliderQuality->value();

    setWindowTitle(NFITSVIEW_WND_CAPTION);

    return FITS_GENERAL_SUCCESS;
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
    backupOriginalImage();

    int valueR = value;
    QString valueRstr = QString::number(valueR) + " %";
    valueRstr = valueRstr.rightJustified(12, ' ');

    ui->labelValueR->setText(valueRstr);

    ui->workspaceWidget->restoreImage();
    ui->workspaceWidget->changeChannelLevel(0, 1 + (float)valueR/100);
    ui->workspaceWidget->reloadImage();
    ui->workspaceWidget->scaleImage(m_scaleFactor);
}

void MainWindow::on_horizontalSliderG_valueChanged(int value)
{
    backupOriginalImage();

    int valueG = value;
    QString valueGstr = QString::number(valueG) + " %";
    valueGstr = valueGstr.rightJustified(12, ' ');

    ui->labelValueG->setText(valueGstr);

    ui->workspaceWidget->restoreImage();
    ui->workspaceWidget->changeChannelLevel(1, 1 + (float)valueG/100);
    ui->workspaceWidget->reloadImage();
    ui->workspaceWidget->scaleImage(m_scaleFactor);
}

void MainWindow::on_horizontalSliderB_valueChanged(int value)
{
    backupOriginalImage();

    int valueB= value;
    QString valueBstr = QString::number(valueB) + " %";
    valueBstr = valueBstr.rightJustified(12, ' ');

    ui->labelValueB->setText(valueBstr);

    ui->workspaceWidget->restoreImage();
    ui->workspaceWidget->changeChannelLevel(2, 1 + (float)valueB/100);
    ui->workspaceWidget->reloadImage();
    ui->workspaceWidget->scaleImage(m_scaleFactor);
}

void MainWindow::initGammaWidgetsValues()
{
    int valueR = 0;
    ui->horizontalSliderR->setValue(valueR);
    QString valueRstr = QString::number(valueR) + " %";
    valueRstr = valueRstr.rightJustified(12, ' ');
    ui->labelValueR->setText(valueRstr);

    int valueG = 0;
    ui->horizontalSliderG->setValue(valueG);
    QString valueGstr = QString::number(valueG) + " %";
    valueGstr = valueGstr.rightJustified(12, ' ');
    ui->labelValueG->setText(valueGstr);

    int valueB = 0;
    ui->horizontalSliderB->setValue(valueB);
    QString valueBstr = QString::number(valueB) + " %";
    valueBstr = valueBstr.rightJustified(12, ' ');
    ui->labelValueB->setText(valueBstr);

    ui->checkBoxGrayscale->setChecked(false);
}

void MainWindow::on_tableWidgetHDUs_itemActivated(QTableWidgetItem *item)
{
    //// This slot is not used anymore. For better compatibility for all the supported platforms
    //// the signal is changed to currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous).
    //// This one should have the same behavior on every platform. "item" is replaced with "current"
}

void MainWindow::clearWidgets()
{
    ui->tableWidgetHDUs->clearContents();
    ui->tableWidgetHDUs->model()->removeRows(0, ui->tableWidgetHDUs->rowCount());

    enableRestoreWidgets(false);

    ui->workspaceWidget->clearWidgets();

    m_scaleFactor = 100;

    m_sliderZoom->setValue(m_scaleFactor);

    m_bEnableGammaWidgets = false;
    m_bEnableZoomWidget = false;

    enableZoomWidgets(m_bEnableZoomWidget);
    enableGammaWidgets(m_bEnableGammaWidgets);

    enableFileOpenRelatedWidgets(false);

    initHDUInfoWidgetValues();
}

qint32 MainWindow::exportAllImages()
{
    setStatus(STATUS_MESSAGE_IMAGE_EXPORT_HDUS);

    setProgress(50);

    qint32 retVal = m_fitsFile.exportAllImageHDUs();    

    setStatus(STATUS_MESSAGE_READY);

    if (retVal == FITS_GENERAL_SUCCESS)
        QMessageBox::information(this, "Success", IMAGE_EXPORT_HDUS_MESSAGE_SUCCESS);
    else
        QMessageBox::critical(this, "Error", IMAGE_EXPORT_HDUS_MESSAGE_ERROR);

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

    ui->workspaceWidget->scaleImage(m_scaleFactor);
    m_sliderZoom->setValue(m_scaleFactor);
}

void MainWindow::on_actionRestoreOriginalToolBar_triggered()
{
    restoreOriginalImage();
}

void MainWindow::on_actionRestoreOriginal_triggered()
{
    restoreOriginalImage();
}

void MainWindow::backupOriginalImage()
{
    if (!m_bImageChanged)
    {
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

    QString fileName = QFileDialog::getSaveFileName(this, QString("Save ") + m_exportFormat + QString(" file"), dir,
                                                    m_exportFormat + QString(" Files (*")  + m_exportFormat.toLower() + QString(")"));

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
    valueQstr = valueQstr.rightJustified(12, ' ');

    ui->labelQualityValue->setText(valueQstr);

    m_exportQuality = ui->horizontalSliderQuality->value();
}

void MainWindow::initImageExportSettingsWidgetValues()
{
    QString valueQstr = QString::number(IMAGE_EXPORT_DEFAULT_QUALITY);
    valueQstr = valueQstr.rightJustified(12, ' ');

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
    if (currentItem == IMAGE_EXPORT_TYPE_PNG || currentItem == IMAGE_EXPORT_TYPE_BMP)
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

void MainWindow::on_comboBoxFormat_currentIndexChanged(const QString &arg1)
{
    m_exportFormat = arg1;
    m_exportQuality = ui->horizontalSliderQuality->value();

    if (arg1 == IMAGE_EXPORT_TYPE_PNG || arg1 == IMAGE_EXPORT_TYPE_BMP)
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

    dlg.setAppVersion(NFITSVIEW_WND_CAPTION);

    dlg.exec();
}


void MainWindow::on_actionQuit_triggered()
{
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

    int32_t resTemp = m_fitsFile.getHDU(row, hdu);

    if (resTemp == FITS_GENERAL_SUCCESS)
    {
        uint32_t axisesNumber = hdu.getKeywordValue<uint32_t>(FITS_KEYWORD_NAXIS, bSuccess);
        std::vector<uint32_t> axises = hdu.getAxises();

        uint8_t HDUType = hdu.getType();

        if ((HDUType == FITS_HDU_TYPE_IMAGE_XTENSION || HDUType == FITS_HDU_TYPE_PRIMARY) && (axisesNumber >= 2 && bSuccess) && (axises.size() >= 2))
        {
            int32_t bitpix = hdu.getKeywordValue<int32_t>(FITS_KEYWORD_BITPIX, bSuccess);

            if (bSuccess)
            {
                ui->workspaceWidget->imageSetVisible(true);

                ui->workspaceWidget->setImage(hdu.getPayload(), axises[0], axises[1], bitpix);

                fitToWindow();

                if (std::abs(bitpix) > 8)
                {
                    m_bEnableGammaWidgets = true;
                    enableGammaWidgets(m_bEnableGammaWidgets);
                }

                m_bEnableZoomWidget = true;
                enableZoomWidgets(m_bEnableZoomWidget);
                enableImageExportWidgets();
                enableImageExportSettigsWidgets();
            }
        }
        else
        {
            m_bEnableGammaWidgets = false;
            m_bEnableZoomWidget = false;
            enableGammaWidgets(m_bEnableGammaWidgets);
            enableZoomWidgets(m_bEnableZoomWidget);
            enableImageExportWidgets(false);
            enableImageExportSettigsWidgets(false);

            ui->workspaceWidget->imageSetVisible(false);
        }

        axises.clear();

        initHDUInfoWidgetValues();
        populateHDUInfoWidget(row);
    }

    ui->workspaceWidget->setCurrentIndex(0);
}

void MainWindow::on_workspaceWidget_sendGammaCorrectionTabEnabled(bool a_flag)
{
    enableGammaWidgets(m_bEnableGammaWidgets & a_flag);
    enableZoomWidgets(m_bEnableZoomWidget & a_flag);
}

void MainWindow::setProgress(int32_t a_progress)
{
    m_progressBar->setValue(a_progress);
    m_progressBar->repaint();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    //// TODO: to decide if to have a different resize behavior or not

}
