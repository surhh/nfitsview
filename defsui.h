#ifndef DEFSUI_H
#define DEFSUI_H

#include <cstdint>

#define UPDATE_CHECK_URL        "https://raw.githubusercontent.com/surhh/nfitsview/main/_lastversion.txt"
#define UPDATE_DOWNLOAD_URL     "https://surhh.github.io"

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

    int32_t     mappingValue;
    bool        mappingEnabled;
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
