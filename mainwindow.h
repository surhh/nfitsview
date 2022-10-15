#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProgressBar>
#include <QSlider>
#include <QLabel>
#include <QTableWidgetItem>

#include "libnfits/fitsfile.h"

#define MIN_IMAGE_SCALE_FACTOR              (1)
#define MAX_IMAGE_SCALE_FACTOR              (200)

#define IMAGE_EXPORT_DEFAULT_QUALITY        (75)

#define IMAGE_EXPORT_MESSAGE_SUCCESS        "The current HDU has been successfully exported as an image."
#define IMAGE_EXPORT_MESSAGE_ERROR          "Error exporting the current HDU as an image."

#define IMAGE_EXPORT_HDUS_MESSAGE_SUCCESS   "All image HDUs have been successfully exported as images.\n"   \
                                            "The exported images are located in the same directory where\n" \
                                            "the original FITS file is located."

#define IMAGE_EXPORT_NO_HDUS_MESSAGE        "No image HDUs available for converting!"

#define IMAGE_EXPORT_HDUS_MESSAGE_ERROR     "Error exporting all image HDUs as image."

#define STATUS_MESSAGE_IMAGE_EXPORT         "Exporting the current image..."
#define STATUS_MESSAGE_IMAGE_EXPORT_HDUS    "Exporting all image HDUs..."
#define STATUS_MESSAGE_IMAGE_LOAD           "Loading FITS file..."
#define STATUS_MESSAGE_READY                "Ready"

#define NFITSVIEW_APP_NAME                  "nFITSview"
#define NFITSVIEW_VERSION                   "1.3"
#define NFITSVIEW_APP_FULL_NAME             NFITSVIEW_APP_NAME " " NFITSVIEW_VERSION

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    qint32 openFITSFileByNameFromCmdLine(const QString& a_fileName);

private slots:
    void on_checkBoxGrayscale_stateChanged(int arg1);

    void on_actionOpen_triggered();

    void on_actionOpenToolBar_triggered();

    void om_m_sliderZoom_valueChanged(int a_value);

    void on_horizontalSliderR_valueChanged(int value);

    void on_horizontalSliderG_valueChanged(int value);

    void on_horizontalSliderB_valueChanged(int value);

    void on_tableWidgetHDUs_itemActivated(QTableWidgetItem *item);

    void on_actionExportToolBar_triggered();

    void on_actionExport_triggered();

    void on_actionClose_triggered();

    void on_actionCloseToolBar_triggered();

    void on_actionZoomOutToolBar_triggered();

    void on_actionZoomInToolBar_triggered();

    void on_actionZoomOut_triggered();

    void on_actionZoomIn_triggered();

    void on_actionFitWindow_triggered();

    void on_actionFitWindowToolBar_triggered();

    void on_actionRestoreOriginalToolBar_triggered();

    void on_actionRestoreOriginal_triggered();

    void on_progressChanged(qint32 a_value);

    void on_actionExportImageToolBar_triggered();

    void on_actionExportImage_triggered();

    void on_horizontalSliderQuality_valueChanged(int value);

    void on_comboBoxFormat_currentIndexChanged(const QString &arg1);

    void on_actionAbout_triggered();

    void on_actionQuit_triggered();

    void on_tableWidgetHDUs_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous);

    void on_workspaceWidget_sendGammaCorrectionTabEnabled(bool a_flag);

private:
    Ui::MainWindow *ui;

    QString             m_fitsFileName;

    QProgressBar       *m_progressBar;

    QSlider            *m_sliderZoom;
    QLabel             *m_labelZoomLeft;
    QLabel             *m_labelZoomRight;
    QLabel             *m_labelZoomLevel;    

    int32_t             m_scaleFactor;

    libnfits::FitsFile  m_fitsFile;

    bool                m_bImageChanged;
    bool                m_bEnableGammaWidgets;
    bool                m_bEnableZoomWidget;

    QString             m_exportFormat;
    int32_t             m_exportQuality;

private:
    void createStatusBarWidgets();
    void initGammaWidgetsValues();
    void initImageExportSettingsWidgetValues();
    void initHDUInfoWidgetValues();
    void enableGammaWidgets(bool a_flag = true);
    void enableFileOpenRelatedWidgets(bool a_flag = true);
    void enableZoomWidgets(bool a_flag = true);
    void enableRestoreWidgets(bool a_flag = true);
    void enableImageExportWidgets(bool a_flag = true);
    void enableImageExportSettigsWidgets(bool a_flag = true);
    void scrollToCenter();
    void scaleImage();
    void fitToWindow();
    void backupOriginalImage();
    void restoreOriginalImage();

    qint32 openFITSFile();
    qint32 openFITSFileByName(const QString& a_fileName);
    qint32 closeFITSFile();
    qint32 exportAllImages();
    bool   exportImage();
    qint32 setAllWorkspaceImages();

    void populateHDUsWidget();
    void populateHeaderWidget(int32_t a_hduIndex);
    void populateRawDataWidget(int32_t a_hduIndex);
    void populateHDUInfoWidget(int32_t a_hduIndex);

    void zoomControlsCheck();
    void clearWidgets();

    void setStatus(const QString& a_statusStr);
    void setProgress(int32_t a_progress);

    QString getFileName() const;

    void resizeEvent(QResizeEvent *event);
signals:
    void sendProgressChanged(qint32 a_value);
};
#endif // MAINWINDOW_H
