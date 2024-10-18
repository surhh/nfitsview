#ifndef DEFSUI_H
#define DEFSUI_H

#include <cstdint>
#include "libnfits/defs.h"

#define UPDATE_CHECK_URL        "https://raw.githubusercontent.com/surhh/nfitsview/main/_lastversion.txt"
#define UPDATE_DOWNLOAD_URL     "https://surhh.github.io"
#define THRESHOLD_LABEL_TEXT    "Threshold: "

#define MIN_IMAGE_SCALE_FACTOR              (1)
#define MAX_IMAGE_SCALE_FACTOR              (400)

#define IMAGE_EXPORT_DEFAULT_QUALITY        (75)

#define IMAGE_EXPORT_MESSAGE_SUCCESS        "The current HDU has been successfully exported as an image."
#define IMAGE_EXPORT_MESSAGE_ERROR          "Error exporting the current HDU as an image."

#define IMAGE_EXPORT_HDUS_MESSAGE_SUCCESS   "All image HDUs have been successfully exported as images.\n"   \
                                            "The exported images are located in the same directory where\n" \
                                            "the original FITS file is located."

#define IMAGE_EXPORT_NO_HDUS_MESSAGE        "No image HDUs are available for exporting!"

#define IMAGE_EXPORT_HDUS_MESSAGE_ERROR     "Error exporting all image HDUs as image."

#define STATUS_MESSAGE_IMAGE_EXPORT         "Exporting the current image..."
#define STATUS_MESSAGE_IMAGE_EXPORT_HDUS    "Exporting all image HDUs..."
#define STATUS_MESSAGE_IMAGE_LOAD           "Loading FITS file..."
#define STATUS_MESSAGE_READY                "Ready"

#define NFITSVIEW_APP_NAME                  "nFITSview"

#define TOSTR(x)                            #x
#define TOSTRING(x)                         TOSTR(x)

#define NFITSVIEW_MAJOR_VERSION             LIBNFITS_MAJOR_VERSION
#define NFITSVIEW_MINOR_VERSION             LIBNFITS_MINOR_VERSION

#define NFITSVIEW_VERSION                   TOSTRING(NFITSVIEW_MAJOR_VERSION) "." TOSTRING(NFITSVIEW_MINOR_VERSION)
#define NFITSVIEW_VERSION_INT               TOSTRING(NFITSVIEW_MAJOR_VERSION)TOSTRING(NFITSVIEW_MINOR_VERSION)
#define NFITSVIEW_APP_FULL_NAME             NFITSVIEW_APP_NAME " " NFITSVIEW_VERSION

#define RGB_DEFAULT_VALUE                       (0)
#define RGB_MIN_VALUE                           (-100)
#define RGB_MAX_VALUE                           (100)

#define LABELS_RIGHT_JUSTIFICATION_VALUE        (8)

struct GammaWidgetsStates
{
    int8_t      rLevel;
    int8_t      gLevel;
    int8_t      bLevel;
    bool        gray;
    bool        eye;

    bool        rLevelEnabled;
    bool        gLevelEnabled;
    bool        bLevelEnabled;
    bool        grayEnabled;
    bool        eyeEnabled;

    bool        rgbResetEnabled;

    int8_t      mappingValue;
    bool        mappingEnabled;

    int8_t      mappingThreshold[FITS_NUMBER_OF_TRANSFORMS];
    bool        mappingThresholdEnabled;
};

struct ExportWidgetsStates
{
    int8_t      format;
    uint8_t     quality;

    bool        formatEnabled;
    bool        qualityEnabled;
};

struct ZoomWidgetsStates
{
    int32_t     factor;
    bool        zoomEnabled;
    bool        zoomInEnabled;
    bool        zoomOutEnabled;
    bool        fitWindowEnabled;
};

struct ScrollState
{
    int32_t x;
    int32_t y;
};

struct WidgetsStates
{
    bool                    stored;
    bool                    imageChanged;

    GammaWidgetsStates      gammaStates;
    ExportWidgetsStates     exportStates;
    ZoomWidgetsStates       zoomStates;
    ScrollState             scrollState;

    WidgetsStates()
    {
        stored = false;
        imageChanged = false;
    }
};

struct ImageParams
{
    uint32_t width;
    uint32_t height;
    double   bzero;
    double   bscale;
    size_t   HDUBaseOffset;
    size_t   maxDataBufferSize;
    uint32_t hduIndex;
    int8_t   bitpix;
    bool     grayscale;
};

#endif // DEFSUI_H
