#ifndef LIBNFITS_IMAGE_H
#define LIBNFITS_IMAGE_H

#include <cstdint>
#include <string>

#include "defs.h"

#define MIN_RGB_CHANNEL_CHANGE_FACTOR       (0.0)
#define MAX_RGB_CHANNEL_CHANGE_FACTOR       (2.0)

namespace libnfits
{

struct ImageColorStats
{
    uint64_t    sumR;
    uint64_t    sumG;
    uint64_t    sumB;

    uint64_t    countR;
    uint64_t    countG;
    uint64_t    countB;

    uint8_t     avgR;
    uint8_t     avgG;
    uint8_t     avgB;

    uint8_t     minR;
    uint8_t     minG;
    uint8_t     minB;

    uint8_t     maxR;
    uint8_t     maxG;
    uint8_t     maxB;
};

class Image
{
private:
    bool                m_isCompressed;

    uint8_t*            m_dataBuffer;

    uint8_t**           m_rgbDataBuffer;
    uint8_t**           m_rgbDataBackupBuffer;

    uint8_t**           m_rgb32DataBuffer;
    uint8_t**           m_rgb32DataBackupBuffer;

    uint8_t*            m_rgb32FlatDataBuffer;
    uint8_t*            m_rgb32FlatDataBackupBuffer;

    uint32_t            m_width;
    uint32_t            m_height;
    uint8_t             m_colorDepth;
    int8_t              m_bitpix;
    double              m_bzero;
    double              m_bscale;
    size_t              m_maxDataBufferSize;
    size_t              m_baseOffset;
    std::string         m_title;

    CallbackFunctionPtr m_callbackFunc;
    void*               m_callbackFuncParam;

    ImageColorStats     m_colorStats;

private:
    int32_t _changeRGBColorChannelLevel(uint8_t a_channel, float a_quatient);
    int32_t _changeRGB32ColorChannelLevel(uint8_t a_channel, float a_quatient);
    int32_t _changeRGB32FlatColorChannelLevel(uint8_t a_channel, float a_quatient);

    void _deleteRGBData(uint8_t**& a_rgbDataBuffer);
    void _deleteRGB32Data(uint8_t**& a_rgb32DataBuffer);
    void _deleteRGB32FlatData(uint8_t*& a_rgb32FlatDataBuffer);

    void _convertRGB2AltColors(uint8_t a_red, uint8_t a_green, uint8_t a_blue,
                               uint8_t& a_newRed, uint8_t& a_newGreen, uint8_t& a_newBlue);
    void _convertBufferRGB32Flat2EyeComfortColors();

public:
    Image();
    Image(const uint8_t* a_dataBuffer, uint32_t a_witdth, uint32_t a_height, uint8_t a_colorDepth, int8_t a_bitpix,
          const std::string& a_title = "", bool a_isCompressed = false, CallbackFunctionPtr a_callbackFunc = nullptr);
    ~Image();

    void setParameters(uint32_t a_width, uint32_t a_height, uint8_t a_colorDepth, int8_t a_bitpix, bool a_isCompressed = false);
    void setWidth(uint32_t a_width);
    void setHeight(uint32_t a_height);
    void setGeometry(uint32_t a_width, uint32_t a_height);
    void setColorDepth(uint8_t a_colorDepth);
    void setBitPix(int8_t a_bitpix);
    void setCompressed(bool a_isCompressed = false);
    void setBZero(double a_bzero);
    void setBScale(double a_bscale);
    double getBZero() const;
    double getBScale() const;
    uint32_t getWidth() const;
    uint32_t getHeight() const;
    uint8_t getColorDepth() const;
    int8_t getBitPix() const;
    bool isCompressed() const;

    void setData(const uint8_t* a_dataBuffer);
    uint8_t* getData() const;

    void setCallbackFunction(CallbackFunctionPtr a_callbackFunc, void* a_callbackFuncParam);
    int32_t exportPNG(const std::string& a_fileName);

    int32_t createRGBData();
    uint8_t** getRGBData() const;
    void setRGBData(uint8_t** a_rgbDataBuffer);
    void copyRGBData(uint8_t** a_rgbDataBufferDest, uint8_t** a_rgbDataBufferSrc);

    int32_t createRGB32Data();
    uint8_t** getRGB32Data() const;
    void setRGB32Data(uint8_t** a_rgbDataBuffer);
    void copyRGB32Data(uint8_t** a_rgbDataBufferDest, uint8_t** a_rgbDataBufferSrc);

    int32_t createRGB32FlatData();
    uint8_t* getRGB32FlatData() const;
    void setRGB32FlatData(uint8_t* a_rgbFlatDataBuffer);
    void copyRGB32FlatData(uint8_t* a_rgbFlatDataBufferDest, uint8_t* a_rgbFlatDataBufferSrc);


    int32_t changeRLevel(float a_quatient);
    int32_t changeGLevel(float a_quatient);
    int32_t changeBLevel(float a_quatient);
    int32_t convertRGB2Grayscale();

    int32_t change32RLevel(float a_quatient);
    int32_t change32GLevel(float a_quatient);
    int32_t change32BLevel(float a_quatient);
    int32_t convertRGB322Grayscale();

    int32_t change32FlatRLevel(float a_quatient);
    int32_t change32FlatGLevel(float a_quatient);
    int32_t change32FlatBLevel(float a_quatient);
    int32_t convertRGB32Flat2Grayscale();
    int32_t convertRGB32Flat2EyeComfortColors();

    void backupRGBData();
    void backupRGB32Data();
    void backupRGB32FlatData();

    void restoreRGBData();
    void restoreGB32Data();
    void restoreRGB32FlatData();

    void deleteRGBData();
    void deleteRGB32Data();
    void deleteRGB32FlatData();
    void deleteAllRGBData();
    void deleteAllBackupRGBData();

    void normalize(float a_min, float a_max, float a_minNew, float a_maxNew);

    void calcRGBDataColorStats();
    void calcRGB32DataColorStats();
    void calcRGB32FlatDataColorStats();

    ImageColorStats getColorStats() const;

    void reset();

    void setMaxDataBufferSize(size_t a_size);
    size_t getMaxDataBufferSize() const;

    void setBaseOffset(size_t a_baseOffset);
    size_t getBaseOffset() const;

    void processRGBBrightnessFilter(uint8_t a_threshold);
    void processRGB32BrightnessFilter(uint8_t a_threshold);
    void processRGBB32FlatBrightnessFilter(uint8_t a_threshold);

    //// this function is for debug purposes only, is slow
    int32_t dumpFloatDataBuffer(const std::string& a_filename, uint32_t a_rowSize);
};

}
#endif // LIBNFITS_IMAGE_H
