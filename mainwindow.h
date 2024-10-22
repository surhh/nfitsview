#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProgressBar>
#include <QSlider>
#include <QLabel>
#include <QTableWidgetItem>

#include "defsui.h"
#include "libnfits/fitsfile.h"
#include "updatemanager/filedownloader.h"

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
    qint32 exportFITSFileFromCmdLine(const QString& a_fileName, int32_t a_transform = FITS_FLOAT_DOUBLE_NO_TRANSFORM, bool a_gray = false);

private slots:
    void on_checkBoxGrayscale_stateChanged(int arg1);

    void on_actionOpen_triggered();

    void on_actionOpenToolBar_triggered();

    void om_m_sliderZoom_valueChanged(int a_value);

    void on_horizontalSliderR_valueChanged(int value);

    void on_horizontalSliderG_valueChanged(int value);

    void on_horizontalSliderB_valueChanged(int value);

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

    void on_actionAbout_triggered();

    void on_actionQuit_triggered();

    void on_tableWidgetHDUs_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous);

    void on_workspaceWidget_sendGammaCorrectionTabEnabled(bool a_flag);

    void on_checkBoxEyeComfort_stateChanged(int arg1);

    void on_resetRGBButton_clicked();

    void on_comboBoxMapping_currentIndexChanged(int index);

    void onSendMousewheelZoomChanged(int32_t a_scaleFactor);

    void on_actionCheckForUpdateToolBar_triggered();

    void on_actionCheckForUpdate_triggered();

    void on_horizontalSliderPercent_valueChanged(int value);

    int32_t updateChecked();

    void on_minRButton_clicked();

    void on_defaultRButton_clicked();

    void on_maxRButton_clicked();

    void on_minGButton_clicked();

    void on_defaultGButton_clicked();

    void on_maxGButton_clicked();

    void on_minBButton_clicked();

    void on_defaultBButton_clicked();

    void on_maxBButton_clicked();

    void on_comboBoxFormat_currentTextChanged(const QString &arg1);

private:
    Ui::MainWindow *ui;

    QString             m_fitsFileName;

    QProgressBar       *m_progressBar;

    QSlider            *m_sliderZoom;
    QLabel             *m_labelZoomLeft;
    QLabel             *m_labelZoomRight;
    QLabel             *m_labelZoomLevel;    

    int32_t             m_scaleFactor;
    int32_t             m_percentThreshold[4];

    libnfits::FitsFile  m_fitsFile;

    bool                m_bImageChanged;

    bool                m_bEnableGammaWidgets;
    bool                m_bEnableZoomWidget;
    bool                m_bEnableMappingWidgets;

    bool                m_bGrayscale;
    bool                m_bEyeComfort;

    QString             m_exportFormat;
    int32_t             m_exportQuality;

    FileDownloader     *m_fileDownloader;

private:
    void createStatusBarWidgets();
    void initGammaWidgetsValues();
    void initMappingWidgetsValues();
    void initImageExportSettingsWidgetValues();
    void initHDUInfoWidgetValues();
    void initHDUInfoWidgetMinMax();
    void enableGammaWidgets(bool a_flag = true);
    void enableRGBWidgets(bool a_flag = true);
    void enableFileOpenRelatedWidgets(bool a_flag = true);
    void enableZoomWidgets(bool a_flag = true);
    void enableRestoreWidgets(bool a_flag = true);
    void enableImageExportWidgets(bool a_flag = true);
    void enableImageExportSettigsWidgets(bool a_flag = true);
    void enableMappingWidgets(bool a_flag = true);
    void scrollToCenter();
    void scaleImage();
    void fitToWindow();
    void backupOriginalImage();
    void restoreOriginalImage();

    int32_t openFITSFile();
    int32_t openFITSFileByName(const QString& a_fileName, bool a_bShowMsg = true);
    int32_t closeFITSFile();
    int32_t exportAllImages(bool a_msgFlag = true, int32_t a_transform = FITS_FLOAT_DOUBLE_NO_TRANSFORM, bool a_gray = false);
    bool   exportImage();
    int32_t setAllWorkspaceImages();

    void populateHDUsWidget();
    void populateHeaderWidget(int32_t a_hduIndex);
    void populateRawDataWidget(int32_t a_hduIndex);
    void populateHDUInfoWidget(int32_t a_hduIndex);
    void updateHDUInfoWidgetMinMax();

    void zoomControlsCheck();
    void clearWidgets();

    void setStatus(const QString& a_statusStr);
    void setProgress(int32_t a_progress);

    QString getFileName() const;

    WidgetsStates getWidgetsStates() const;
    void setWidgetsStates(const WidgetsStates& a_widgetsStates);

    void changeRGBColorChannelLevel(uint8_t a_channel, int8_t a_value);
    void changeRGBColorChannelLevels(int8_t a_rValue, int8_t a_gValue, int8_t a_bValue);
    void restoreRGBColorChannelLevelsImage(int32_t a_hduIndex, uint32_t a_transformType = FITS_FLOAT_DOUBLE_NO_TRANSFORM);
    void grayScale();
    void eyeComfort();

    int32_t convertComboIndexToTransformType(int32_t a_index) const;

    void resizeEvent(QResizeEvent *event);

    void enableDisableMappingComboItem(uint32_t a_index, bool a_enable = true);

    void checkForUpdate();

signals:
    void sendProgressChanged(qint32 a_value);
};
#endif // MAINWINDOW_H
