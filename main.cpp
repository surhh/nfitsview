#include "mainwindow.h"

#include <QApplication>

#define APP_EXIT_SUCCESS_CODE    (0)
#define APP_EXIT_ERROR_CODE      (1)

#define CMDLINE_SWITCH_HELP                             "-h"
#define CMDLINE_SWITCH_HELP_FULL                        "--help"
#define CMDLINE_SWITCH_EXPORT                           "-e"
#define CMDLINE_SWITCH_EXPORT_FULL                      "--export"
#define CMDLINE_SWITCH_EXPORT_OPTION_COLOR_MODE_0       "m0"
#define CMDLINE_SWITCH_EXPORT_OPTION_COLOR_MODE_1       "m1"
#define CMDLINE_SWITCH_EXPORT_OPTION_COLOR_RGB          "c"
#define CMDLINE_SWITCH_EXPORT_OPTION_COLOR_GRAY         "g"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;

    QGuiApplication::setDesktopFileName("nFITSview"); /// fixing app icon for Wayland

    int argCount = QCoreApplication::arguments().count();

    qint32 resOpen = FITS_GENERAL_ERROR, resConvert = 0;

    QString arg1, arg2, arg3, arg4;

    if (argCount == 1)
    {
        resOpen = FITS_GENERAL_SUCCESS;
    }
    else if (argCount == 2)
    {
        arg1 = QCoreApplication::arguments().at(1);

        if (arg1 == CMDLINE_SWITCH_HELP || arg1 == CMDLINE_SWITCH_HELP_FULL)
        {
            std::cout << std::endl << "nFITSview " << NFITSVIEW_VERSION << std::endl;
            std::cout << "________________________________________________________________________" << std::endl << std::endl;
            std::cout << "Command line usage:" << std::endl << std::endl;
            std::cout << "nfitsview [FITS file]" << std::endl << std::endl;
            std::cout << "  or" << std::endl << std::endl;
            std::cout << "nfitsview <command> <color mapping mode> <color option> [FITS file]" << std::endl << std::endl;
            std::cout << "Commands available:" << std::endl << std::endl;
            std::cout << "  " << CMDLINE_SWITCH_EXPORT << ", " << CMDLINE_SWITCH_EXPORT_FULL << "  Export all image HDUs of the FITS file to PNG files" << std::endl;
            std::cout << "  " << CMDLINE_SWITCH_HELP << ", " << CMDLINE_SWITCH_HELP_FULL << "  Show this help" << std::endl << std::endl;
            std::cout << "Color mapping modes available:" << std::endl << std::endl;
            std::cout << "  " << CMDLINE_SWITCH_EXPORT_OPTION_COLOR_MODE_0 << "  Export image HDUs in original color mapping" << std::endl;
            std::cout << "  " << CMDLINE_SWITCH_EXPORT_OPTION_COLOR_MODE_1 << "  Export image HDUs in positive range float/integer color mapping"
                      << std::endl << std::endl;
            std::cout << "Color options available: " << std::endl << std::endl;
            std::cout << "  " << CMDLINE_SWITCH_EXPORT_OPTION_COLOR_RGB << "   Export image HDUs in original colors" << std::endl;
            std::cout << "  " << CMDLINE_SWITCH_EXPORT_OPTION_COLOR_GRAY << "   Export image HDUs in grayscale" << std::endl << std::endl;
            std::cout << "Examples: " << std::endl << std::endl;
            std::cout << "  nfitsview -e m0 g example.fits" << std::endl;
            std::cout << "  nfitsview -e m1 c example.fits" << std::endl << std::endl;

            return APP_EXIT_SUCCESS_CODE;
        }

        if (arg1 == CMDLINE_SWITCH_EXPORT || arg1 == CMDLINE_SWITCH_EXPORT_FULL)
        {
            std::cout << "[ERROR]: No input FITS file" << std::endl;

            return APP_EXIT_ERROR_CODE;
        }
        else
            resOpen = w.openFITSFileByNameFromCmdLine(arg1);
    }
    else if (argCount == 5)
    {
        arg1 = QCoreApplication::arguments().at(1);
        arg2 = QCoreApplication::arguments().at(2);
        arg3 = QCoreApplication::arguments().at(3);
        arg4 = QCoreApplication::arguments().at(4);

        if (arg1 == CMDLINE_SWITCH_EXPORT || arg1 == CMDLINE_SWITCH_EXPORT_FULL)
        {
            int32_t transform;

            if (arg2 == CMDLINE_SWITCH_EXPORT_OPTION_COLOR_MODE_0)
                transform = FITS_FLOAT_DOUBLE_NO_TRANSFORM;
            else if (arg2 == CMDLINE_SWITCH_EXPORT_OPTION_COLOR_MODE_1)
                transform = FITS_FLOAT_DOUBLE_LINEAR_TRANSFORM_POSITIVE;
            else
            {
                std::cout << "[ERROR]: Unknown color mapping option " << arg2.toStdString() << std::endl;

                return APP_EXIT_ERROR_CODE;
            }

            bool bGray;
            if (arg3 == CMDLINE_SWITCH_EXPORT_OPTION_COLOR_GRAY)
                bGray = true;
            else if (arg3 == CMDLINE_SWITCH_EXPORT_OPTION_COLOR_RGB)
                bGray = false;
            else
            {
                std::cout << "[ERROR]: Unknown color gamma option " << arg3.toStdString() << std::endl;

                return APP_EXIT_ERROR_CODE;
            }

            std::cout << "[INFO]: Exporting FITS file " << arg4.toStdString() << "image HDUs to PNG images..." << std::endl;

            resConvert = w.exportFITSFileFromCmdLine(arg4, transform, bGray);

            if (resConvert > 0)
                std::cout << "[INFO]: Exported " << resConvert << " HDU(s)" << std::endl;
            else if (resConvert == 0)
                std::cout << "[INFO]: No image HDU to export! " << std::endl;
            else
                std::cout << "[ERROR]: Error opening file!" << std::endl;
        }
        else
        {
            std::cout << "[ERROR]: Wrong command line switch!" << std::endl;

            return APP_EXIT_ERROR_CODE;
        }
    }
    else if (argCount > 5)
    {
        std::cout << "[ERROR]: Too many command line parameters!" << std::endl;

        return APP_EXIT_ERROR_CODE;
    }
    else
    {
        std::cout << "[ERROR]: Not enough command line parameters!" << std::endl;

        return APP_EXIT_ERROR_CODE;
    }

    if (resConvert > 0)
    {
        return APP_EXIT_SUCCESS_CODE;
    }
    else if (resOpen == FITS_GENERAL_SUCCESS)
    {
        w.showMaximized();

        return a.exec();
    }

    return APP_EXIT_ERROR_CODE;
}
