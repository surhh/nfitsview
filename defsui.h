#ifndef DEFSUI_H
#define DEFSUI_H

#include <cstdint>

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

#endif // DEFSUI_H
