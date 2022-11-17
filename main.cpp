#include "mainwindow.h"

#include <QApplication>

#define APP_EXIT_SUCCESS_CODE    (0)
#define APP_EXIT_ERROR_CODE      (1)

#define CMDLINE_SWITCH_HELP      "-h"
#define CMDLINE_SWITCH_EXPORT    "-e"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;

    int argCount = QCoreApplication::arguments().count();

    qint32 resOpen = FITS_GENERAL_ERROR, resConvert = 0;

    QString arg1, arg2;

    if (argCount == 1)
        resOpen = FITS_GENERAL_SUCCESS;
    else if (argCount == 2)
    {
        arg1 = QCoreApplication::arguments().at(1);

        if (arg1 == CMDLINE_SWITCH_HELP)
        {
            std::cout << std::endl << "nFITSview " << NFITSVIEW_VERSION << std::endl;
            std::cout << "________________________________________________________________________" << std::endl << std::endl;
            std::cout << "Command line usage:" << std::endl << std::endl;
            std::cout << "nfitsview [FITS file]" << std::endl << std::endl;
            std::cout << "  or" << std::endl << std::endl;
            std::cout << "nfitsview <command> [FITS file]" << std::endl << std::endl;
            std::cout << "Commands available:" << std::endl << std::endl;
            std::cout << "  " << CMDLINE_SWITCH_EXPORT<< "  Export all image HDU(s) of the FITS file to PNG file(s)" << std::endl;
            std::cout << "  " << CMDLINE_SWITCH_HELP << "  Show this help" << std::endl;

            return APP_EXIT_SUCCESS_CODE;
        }

        if (arg1 == CMDLINE_SWITCH_EXPORT)
        {
            std::cout << "[ERROR]: No input FITS file" << std::endl;

            return APP_EXIT_ERROR_CODE;
        }
        else
            resOpen = w.openFITSFileByNameFromCmdLine(arg1);
    }
    else if (argCount == 3)
    {
        arg1 = QCoreApplication::arguments().at(1);
        arg2 = QCoreApplication::arguments().at(2);

        if (arg1 == CMDLINE_SWITCH_EXPORT)
        {
            std::cout << "[INFO]: Exporting FITS file image HDUs to PNG images...: " << arg2.toStdString() << std::endl;
            resConvert = w.exportFITSFileFromCmdLine(arg2);
            std::cout << "[INFO]: Exported " << resConvert << " HDU(s)" << std::endl;
        }
        else
        {
            std::cout << "[ERROR]: Wrong command line switch!" << std::endl;

            return APP_EXIT_ERROR_CODE;
        }
    }
    else if (argCount > 3)
    {
        std::cout << "[ERROR]: Too many command line parameters!" << std::endl;

        return APP_EXIT_ERROR_CODE;
    }

    if (resConvert > 0)
        return APP_EXIT_SUCCESS_CODE;
    else if (resOpen == FITS_GENERAL_SUCCESS)
    {
        w.showMaximized();

        return a.exec();
    }

    return APP_EXIT_ERROR_CODE;
}
