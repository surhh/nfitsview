#include <cstring>
#include <cmath>

#include "image.h"
#include "pngfile.h"
//#include "helperfunctions.h"

namespace libnfits
{

Image::Image():
    m_dataBuffer(nullptr),
    m_rgbDataBuffer(nullptr), m_rgb32DataBuffer(nullptr), m_rgb32FlatDataBuffer(nullptr),
    m_rgbDataBackupBuffer(nullptr), m_rgb32DataBackupBuffer(nullptr), m_rgb32FlatDataBackupBuffer(nullptr), m_maxDataBufferSize(0), m_baseOffset(0),
    m_width(0), m_height(0), m_colorDepth(0), m_bitpix(0), m_isCompressed(false), m_isDistribCounted(false),
    m_bzero(FITS_BZERO_DEFAULT_VALUE), m_isMinMaxCounted(false),
    m_bscale(FITS_BSCALE_DEFAULT_VALUE), m_title(""), m_callbackFunc(nullptr), m_callbackFuncParam(nullptr),
    m_transformType(FITS_FLOAT_DOUBLE_NO_TRANSFORM), m_percentThreshold(-1)
{
    m_colorStats = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    for (int32_t i = 0; i < FITS_VALUE_DISTRIBUTION_SEGMENTS_NUMBER; ++i)
        m_distribStats[i] = { 0, 0.0 };

    resetDistribValues();
}

Image::Image(const uint8_t* a_dataBuffer, uint32_t a_witdth, uint32_t a_height, uint8_t a_colorDepth, int8_t a_bitpix,
             const std::string& a_title, bool a_isCompressed, CallbackFunctionPtr a_callbackFunc)
{
    m_dataBuffer = (uint8_t*)a_dataBuffer;
    m_width = a_witdth;
    m_height = a_height;
    m_colorDepth = a_colorDepth;
    m_bitpix = a_bitpix;
    m_isCompressed = a_isCompressed;
    m_isDistribCounted = false;
    m_isMinMaxCounted = false;
    m_title = a_title;
    m_maxDataBufferSize = 0;
    m_baseOffset = 0;
    m_bscale = FITS_BSCALE_DEFAULT_VALUE;
    m_bzero = FITS_BZERO_DEFAULT_VALUE;
    m_callbackFunc = a_callbackFunc;
    m_callbackFuncParam = nullptr;
    m_rgbDataBuffer = nullptr;
    m_rgb32DataBuffer = nullptr;
    m_rgb32FlatDataBuffer = nullptr;
    m_rgbDataBackupBuffer = nullptr;
    m_rgb32DataBackupBuffer = nullptr;
    m_rgb32FlatDataBackupBuffer = nullptr;
    m_transformType = FITS_FLOAT_DOUBLE_NO_TRANSFORM;
    m_percentThreshold = -1;

    m_colorStats = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    for (int32_t i = 0; i < FITS_VALUE_DISTRIBUTION_SEGMENTS_NUMBER; ++i)
        m_distribStats[i] = { 0, 0.0 };

    resetDistribValues();
}

Image::~Image()
{
    reset();
}

uint32_t Image::getWidth() const
{
    return m_width;
}

uint32_t Image::getHeight() const
{
    return m_height;
}

void Image::setData(const uint8_t* a_dataBuffer)
{
    m_dataBuffer = (uint8_t*)a_dataBuffer;
}

uint8_t* Image::getData() const
{
    return m_dataBuffer;
}

void Image::setCallbackFunction(CallbackFunctionPtr a_callbackFunc, void* a_callbackFuncParam)
{
    m_callbackFunc = a_callbackFunc;
    m_callbackFuncParam = a_callbackFuncParam;
}

int32_t Image::exportPNG(const std::string& a_fileName, int32_t a_transform, bool a_gray)
{
    int32_t retVal = FITS_GENERAL_SUCCESS;
    //uint8_t *pixelBuffer = nullptr;
    //int8_t percent = 0;

    PNGFile pngFile;

    uint8_t bytesNum = std::abs(m_bitpix) / 8 * sizeof(uint8_t);
    uint8_t colorType = FITS_PNG_DEFAULT_COLOR_TYPE;

    if (bytesNum == 0)              // < 8 bits per pixel is not supported
        return FITS_PNG_EXPORT_ERROR;

    // This block recalculates the new PNG buffer size. The multipliers reflect the number of bytes per pixel.
    // RGB type is the default one
    // RGB = 3, RGBA = 4
    //// TODO: RGBA and grayscale should be supported as well
    //pixelBuffer = m_dataBuffer;

    if (bytesNum >= 1 && bytesNum <= 8)
        colorType = FITS_PNG_COLOR_RGB;  // the FITS_PNG_COLOR_RGB_ALPHA seems not to work as expected on the FITS pixel array
    else
        return FITS_PNG_EXPORT_ERROR;

    if (m_callbackFunc != nullptr)
        m_callbackFunc(50, m_callbackFuncParam); // sample callback to send info 50% of work is done

    pngFile.setParameters(a_fileName, m_width, m_height, m_colorDepth, colorType, m_title);

    // the generic working version, processing the float data, not used anymore
    //retVal = pngFile.createFromPixelData(pixelBuffer);


    //// we transform the color mapping only in case of bitpix > 2
    if (bytesNum > 2 && a_transform != FITS_FLOAT_DOUBLE_NO_TRANSFORM)
        createRGBData(a_transform);
    else
        createRGBData();

    if (a_gray)
        convertRGB2Grayscale();

    retVal = pngFile.createFromRGBData((const uint8_t**)m_rgbDataBuffer);
    deleteAllData(); //deleteRGBData(); // 24-bit RGB buffer may not be needed anymore, preferrable to have 32-bit RGB buffer

    if (m_callbackFunc != nullptr)
        m_callbackFunc(100, m_callbackFuncParam); // sample callback to send info 100% of work is done

    return retVal;
}

void Image::setParameters(uint32_t a_width, uint32_t a_height, uint8_t a_colorDepth, int8_t a_bitpix, bool a_isCompressed)
{
    m_width = a_width;
    m_height = a_height;
    m_colorDepth = a_colorDepth;
    m_bitpix = a_bitpix;
    m_isCompressed = a_isCompressed;
}

void Image::setWidth(uint32_t a_width)
{
    m_width = a_width;
}

void Image::setHeight(uint32_t a_height)
{
    m_height = a_height;
}

void Image::setGeometry(uint32_t a_width, uint32_t a_height)
{
    m_width = a_width;
    m_height = a_height;
}

void Image::setColorDepth(uint8_t a_colorDepth)
{
    m_colorDepth = a_colorDepth;
}

void Image::setBitPix(int8_t a_bitpix)
{
    m_bitpix = a_bitpix;
}

void Image::setCompressed(bool a_isCompressed)
{
    m_isCompressed = a_isCompressed;
}

uint8_t Image::getColorDepth() const
{
    return m_colorDepth;
}

int8_t Image::getBitPix() const
{
    return m_bitpix;
}

bool Image::isCompressed() const
{
    return m_isCompressed;
}

void Image::reset()
{
    deleteAllData();

    m_width = 0;
    m_height = 0;
    m_colorDepth = 0;
    m_bitpix = 0;
    m_maxDataBufferSize = 0;
    m_baseOffset = 0;
    m_bscale = FITS_BSCALE_DEFAULT_VALUE;
    m_bzero = FITS_BZERO_DEFAULT_VALUE;
    m_isCompressed = false;
    m_isDistribCounted = false;
    m_isMinMaxCounted = false;
    m_callbackFunc = nullptr;
    m_title.clear();

    m_colorStats = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    for (int32_t i = 0; i < FITS_VALUE_DISTRIBUTION_SEGMENTS_NUMBER; ++i)
        m_distribStats[i] = { 0, 0.0 };

    resetDistribValues();

    m_transformType = FITS_FLOAT_DOUBLE_NO_TRANSFORM;
}

void Image::backupRGBData()
{
    uint8_t bytesNum = 3;

    // calculating the PNG pixel buffer size. This way is more understandable in terms of logic
    size_t bufRowSize = m_width * bytesNum * (m_colorDepth / (sizeof(uint8_t) * 8)) * sizeof(uint8_t);

    // the backup image already exists
    if (m_rgbDataBackupBuffer == nullptr)
    {
        m_rgbDataBackupBuffer = new uint8_t*[m_height]();

        for (uint32_t y = 0; y < m_height; ++y)
            m_rgbDataBackupBuffer[y] = new uint8_t[bufRowSize];
    }

    copyRGBData(m_rgbDataBackupBuffer, m_rgbDataBuffer);
}

void Image::backupRGB32Data()
{
    uint8_t bytesNum = 4;

    // calculating the PNG pixel buffer size. This way is more understandable in terms of logic
    size_t bufRowSize = m_width * bytesNum * (m_colorDepth / (sizeof(uint8_t) * 8)) * sizeof(uint8_t);

    // the backup image already exists
    if (m_rgb32DataBackupBuffer == nullptr)
    {
        m_rgb32DataBackupBuffer = new uint8_t*[m_height]();

        for (uint32_t y = 0; y < m_height; ++y)
            m_rgb32DataBackupBuffer[y] = new uint8_t[bufRowSize];
    }

    copyRGB32Data(m_rgb32DataBackupBuffer, m_rgb32DataBuffer);
}

void Image::backupRGB32FlatData()
{
    uint8_t bytesNum = 4;

    // calculating the PNG pixel buffer size. This way is more understandable in terms of logic
    size_t bufSize = m_width * m_height * bytesNum * (m_colorDepth / (sizeof(uint8_t) * 8)) * sizeof(uint8_t);

    // the backup image already exists
    if (m_rgb32FlatDataBackupBuffer == nullptr)
        m_rgb32FlatDataBackupBuffer = new uint8_t[bufSize];

    copyRGB32FlatData(m_rgb32FlatDataBackupBuffer, m_rgb32FlatDataBuffer);
}

void Image::restoreRGBData()
{
    if (m_rgbDataBackupBuffer != nullptr)
        copyRGBData(m_rgbDataBuffer, m_rgbDataBackupBuffer);
}

void Image::restoreGB32Data()
{
    if (m_rgb32DataBackupBuffer != nullptr)
        copyRGB32Data(m_rgb32DataBuffer, m_rgb32DataBackupBuffer);
}

void Image::restoreRGB32FlatData()
{
    if (m_rgb32FlatDataBackupBuffer != nullptr)
        copyRGB32FlatData(m_rgb32FlatDataBuffer, m_rgb32FlatDataBackupBuffer);
}

void Image::deleteAllBackupRGBData()
{
    _deleteRGBData(m_rgbDataBackupBuffer);
    _deleteRGB32Data(m_rgb32DataBackupBuffer);
    _deleteRGB32FlatData(m_rgb32FlatDataBackupBuffer);
}

void Image::_deleteRGBData(uint8_t**& a_rgbDataBuffer)
{
    if (a_rgbDataBuffer != nullptr)
    {
        for (uint32_t y = 0; y < m_height; ++y)
            if (a_rgbDataBuffer[y] != nullptr)
                delete [] a_rgbDataBuffer[y];

        delete [] a_rgbDataBuffer;

        a_rgbDataBuffer = nullptr;
    }
}

void Image::_deleteRGB32Data(uint8_t**& a_rgb32DataBuffer)
{
    if (a_rgb32DataBuffer != nullptr)
    {
        for (uint32_t y = 0; y < m_height; ++y)
            if (a_rgb32DataBuffer[y] != nullptr)
                delete [] a_rgb32DataBuffer[y];

        delete [] a_rgb32DataBuffer;

        a_rgb32DataBuffer = nullptr;
    }
}

void Image::_deleteRGB32FlatData(uint8_t*& a_rgb32FlatDataBuffer)
{
    if (a_rgb32FlatDataBuffer != nullptr)
    {
        delete [] a_rgb32FlatDataBuffer;
        a_rgb32FlatDataBuffer = nullptr;
    }
}

void Image::deleteRGBData()
{
    _deleteRGBData(m_rgbDataBuffer);
}

void Image::deleteRGB32Data()
{
    _deleteRGB32Data(m_rgb32DataBuffer);
}

void Image::deleteRGB32FlatData()
{
    _deleteRGB32FlatData(m_rgb32FlatDataBuffer);
}

void Image::deleteAllData()
{
    deleteAllRGBData();
    deleteAllBackupRGBData();
}

int32_t Image::createRGBData(uint32_t a_transformType, int32_t a_percent)
{
    int32_t retVal = FITS_GENERAL_SUCCESS;

    uint8_t bytesNum = std::abs(m_bitpix) / 8;

    if (bytesNum != 1 && bytesNum != 2 && bytesNum != 4 && bytesNum != 8)
        return FITS_GENERAL_ERROR;

    //// the image has been converted to RGB already
    if (m_rgbDataBuffer != nullptr)
        return FITS_GENERAL_ERROR;

    // calculating the PNG pixel buffer size. This way is more understandable in terms of logic
    size_t bufRowSize = m_width * 3 * (m_colorDepth / (sizeof(uint8_t) * 8)) * sizeof(uint8_t);

    size_t tmpBufRowSize = m_width * bytesNum * (m_colorDepth / (sizeof(uint8_t) * 8)) * sizeof(uint8_t);  //// 32-bit element buffer for temp usage

    m_rgbDataBuffer = new uint8_t*[m_height]();

    if (m_rgbDataBuffer == nullptr)
        return FITS_GENERAL_ERROR;

    uint8_t* tmpRow = new uint8_t[tmpBufRowSize];

    uint8_t* tmpDestRow = nullptr;

    uint8_t* tmpFinalRow = tmpRow;

    if (m_bitpix == 16 || m_bitpix == 8)
    {
        //tmpDestRow = new uint8_t[tmpBufRowSize * (bytesNum == 2 ? 2 : 1)];
        tmpDestRow = new uint8_t[tmpBufRowSize * (32/std::abs(m_bitpix))];
        tmpFinalRow = tmpDestRow;
    }

    m_transformType = a_transformType;

    calcBufferDistribution(a_percent);

    // Writing the buffer containing pixel data
    try
    {
        //uint32_t indexBase = bytesNum * (bytesNum == 2 ? 2 : 1);
        uint32_t indexBase = bytesNum * (bytesNum <= 2 ? 32/std::abs(m_bitpix) : 1);

        for (int64_t y = m_height - 1; y >= 0; --y) //// this loop is for correcting Y-axis upside down showing
        {
            //libnfits::LOG(" in createRGBData() 1 , y = % , bufRowSize = % , tmpBufRowSize = %" , y, bufRowSize, tmpBufRowSize);
            m_rgbDataBuffer[y] = new uint8_t[bufRowSize];
            std::memset(m_rgbDataBuffer[y] , 0, bufRowSize);

            //// this memcpy is for correcting Y-axis upside down showing
            size_t offset = (m_height - 1 - y) * tmpBufRowSize;

            //// checking if the memory-mapped file is corrupted and not all data is available
            //// e.g. data required by the HDUs is bigger then the file itself
            if ((m_baseOffset + offset + tmpBufRowSize) > m_maxDataBufferSize)
                continue;
            ////

            //// This case should not occur as FITS doesn't officially support 24-bit data values.
            //// Moreover, only bytesNum  is 3 and 4 are checked in this function.
            //// The (bytesNum == 3) is kept just in case if 24-bit data value support will be added one day.
            //// Otherwise, ideally this part of code should be removed
            /*
            if (bytesNum == 3)
            {
                //// this memcpy is for correcting Y-axis upside down showing
                std::memcpy(m_rgbDataBuffer[m_height - 1 - y], m_dataBuffer + ((m_height - 1 - y) * bufRowSize), bufRowSize);

                for (uint32_t x = 0; x < m_width; ++x)
                {
                    uint8_t b = m_rgbDataBuffer[y][x*3];
                    uint8_t g = m_rgbDataBuffer[y][x*3 + 1];
                    uint8_t r = m_rgbDataBuffer[y][x*3 + 2];

                    m_rgbDataBuffer[y][x*3]     = r;
                    m_rgbDataBuffer[y][x*3 + 1] = g;
                    m_rgbDataBuffer[y][x*3 + 2] = b;
                }
            }
            else if (bytesNum == 4 || bytesNum == 8)
            */
            //{
                //std::memcpy(tmpRow, m_dataBuffer + (y * tmpBufRowSize), tmpBufRowSize);

            //// this memcpy is for correcting Y-axis upside down showing
            std::memcpy(tmpRow, m_dataBuffer + offset, tmpBufRowSize);

            convertBufferAllTypes2RGB(tmpRow, tmpBufRowSize, tmpDestRow);

            for (uint32_t x = 0; x < m_width; ++x)
            {
                uint64_t indexSource = x*indexBase;
                uint64_t indexDest = x*3;

                m_rgbDataBuffer[y][indexDest]     = tmpFinalRow[indexSource];
                m_rgbDataBuffer[y][indexDest + 1] = tmpFinalRow[indexSource + 1];
                m_rgbDataBuffer[y][indexDest + 2] = tmpFinalRow[indexSource + 2];
            }
            //}
        }
    }
    catch (...)
    {
        retVal = FITS_PNG_PIXEL_DATA_ERROR;
    }

    if (tmpRow != nullptr)
        delete [] tmpRow;

    if (tmpDestRow != nullptr)
        delete [] tmpDestRow;

    return retVal;
}

int32_t Image::createRGB32Data(uint32_t a_transformType, int32_t a_percent)
{
    int32_t retVal = FITS_GENERAL_SUCCESS;

    uint8_t bytesNum = std::abs(m_bitpix) / 8;

    if (bytesNum != 1 && bytesNum != 2 && bytesNum != 4 && bytesNum != 8)
        return FITS_GENERAL_ERROR;

    // the image has been converted to RGB already
    if (m_rgb32DataBuffer != nullptr)
        return FITS_GENERAL_ERROR;

    // calculating the PNG pixel buffer size. This way is more understandable in terms of logic
    // thow we need to have only RGB data, we actually need to have it 32-bit aligned for some future use cases,
    // that's why it's multipled by 4 instead of 3 (kind of tricky stuff, but works fine)
    size_t bufRowSize = m_width * 4 * (m_colorDepth / (sizeof(uint8_t) * 8)) * sizeof(uint8_t);

    size_t tmpBufRowSize = m_width * bytesNum * (m_colorDepth / (sizeof(uint8_t) * 8)) * sizeof(uint8_t);  //// 32-bit element buffer for temp usage

    m_rgb32DataBuffer = new uint8_t*[m_height]();

    //// the image has been converted to RGB already
    if (m_rgb32DataBuffer == nullptr)
        return FITS_GENERAL_ERROR;

    uint8_t* tmpRow = new uint8_t[tmpBufRowSize];

    uint8_t* tmpDestRow = nullptr;

    uint8_t* tmpFinalRow = tmpRow;

    if (m_bitpix == 16 || m_bitpix == 8)
    {
        //tmpDestRow = new uint8_t[tmpBufRowSize * (bytesNum == 2 ? 2 : 1)];
        tmpDestRow = new uint8_t[tmpBufRowSize * (32/std::abs(m_bitpix))];
        tmpFinalRow = tmpDestRow;
    }

    m_transformType = a_transformType;

    calcBufferDistribution(a_percent);

    // Writing the buffer containing pixel data
    try
    {
        //uint32_t indexBase = bytesNum * (bytesNum == 2 ? 2 : 1);
        uint32_t indexBase = bytesNum * (bytesNum <= 2 ? 32/std::abs(m_bitpix) : 1);

        for (int64_t y = m_height - 1; y >= 0; --y) //// this loop is for correcting Y-axis upside down showing
        {
            m_rgb32DataBuffer[y] = new uint8_t[bufRowSize];
            std::memset(m_rgb32DataBuffer[y] , 0, bufRowSize);

            //// this memcpy is for correcting Y-axis upside down showing
            size_t offset = (m_height - 1 - y) * tmpBufRowSize;

            //// checking if the memory-mapped file is corrupted and not all data is available
            //// e.g. data required by the HDUs is bigger then the file itself
            if ((m_baseOffset + offset + tmpBufRowSize) > m_maxDataBufferSize)
                continue;
            ////

            //// this memcpy is for correcting Y-axis upside down showing
            std::memcpy(tmpRow, m_dataBuffer + offset, tmpBufRowSize);

            convertBufferAllTypes2RGB(tmpRow, tmpBufRowSize, tmpDestRow);

            for (uint32_t x = 0; x < m_width; ++x)
            {
                uint64_t indexSource = x*indexBase;
                uint64_t indexDest = x*4;

                m_rgb32DataBuffer[y][indexDest]     = tmpFinalRow[indexSource + 2];
                m_rgb32DataBuffer[y][indexDest + 1] = tmpFinalRow[indexSource + 1];
                m_rgb32DataBuffer[y][indexDest + 2] = tmpFinalRow[indexSource];
                m_rgb32DataBuffer[y][indexDest + 3] = 0xff;
            }
        }
    }
    catch (...)
    {
        retVal = FITS_PNG_PIXEL_DATA_ERROR;
    }

    if (tmpRow != nullptr)
        delete [] tmpRow;

    if (tmpDestRow != nullptr)
        delete [] tmpDestRow;

    return retVal;
}

int32_t Image::createRGB32FlatData(uint32_t a_transformType, int32_t a_percent)
{
    int32_t retVal = FITS_GENERAL_SUCCESS;

    uint8_t bytesNum = std::abs(m_bitpix) / 8;

    if (bytesNum != 1 && bytesNum != 2 && bytesNum != 4 && bytesNum != 8)
        return FITS_GENERAL_ERROR;

    //// the image has been converted to RGB already
    if (m_rgb32FlatDataBuffer != nullptr)
        return FITS_GENERAL_ERROR;

    //// Calculating the pixel buffer size. This way is more understandable in terms of logic
    //// thow we need to have only RGB data, we actually need to have it 32-bit aligned for some future use cases,
    //// that's why it's multipled by 4 instead of 3 (kind of tricky stuff, but works fine)

    size_t tmpBufRowSize = m_width * bytesNum * (m_colorDepth / (sizeof(uint8_t) * 8)) * sizeof(uint8_t);  //// 32/64-bit element buffer for temp usage

    size_t flatBufSize = m_height * m_width * 4;

    if (m_rgb32FlatDataBuffer == nullptr)
    {
        m_rgb32FlatDataBuffer = new uint8_t[flatBufSize];
        std::memset(m_rgb32FlatDataBuffer, 0, flatBufSize);
    }

    if (m_rgb32FlatDataBuffer == nullptr)
        return FITS_GENERAL_ERROR;

    uint8_t* tmpRow = new uint8_t[tmpBufRowSize];

    uint8_t* tmpDestRow = nullptr;

    uint8_t* tmpFinalRow = tmpRow;

    if (m_bitpix == 16 || m_bitpix == 8)
    {
        //tmpDestRow = new uint8_t[tmpBufRowSize * (bytesNum == 2 ? 2 : 1)];
        tmpDestRow = new uint8_t[tmpBufRowSize * (32/std::abs(m_bitpix))];
        tmpFinalRow = tmpDestRow;
    }

    m_transformType = a_transformType;

    calcBufferDistribution(a_percent);

    // Writing the buffer containing pixel data
    try
    {
        //uint32_t indexBase = bytesNum * (bytesNum == 2 ? 2 : 1);
        uint32_t indexBase = bytesNum * (bytesNum <= 2 ? 32/std::abs(m_bitpix) : 1);

        for (int64_t y = m_height - 1; y >= 0; --y) //// this loop is for correcting Y-axis upside down showing
        {
            //// this memcpy is for correcting Y-axis upside down showing
            size_t offset = (m_height - 1 - y) * tmpBufRowSize;

            //// checking if the memory-mapped file is corrupted and not all data is available
            //// e.g. data required by the HDUs is bigger then the file itself
            if ((m_baseOffset + offset + tmpBufRowSize) > m_maxDataBufferSize)
                break;
            ////
            std::memcpy(tmpRow, m_dataBuffer + offset, tmpBufRowSize);

            convertBufferAllTypes2RGB(tmpRow, tmpBufRowSize, tmpDestRow);

            for (uint32_t x = 0; x < m_width; ++x)
            {
                uint64_t indexSource = x*indexBase;
                uint64_t indexDest = 4*(y*m_width + x);

                m_rgb32FlatDataBuffer[indexDest]     = tmpFinalRow[indexSource + 2];
                m_rgb32FlatDataBuffer[indexDest + 1] = tmpFinalRow[indexSource + 1];
                m_rgb32FlatDataBuffer[indexDest + 2] = tmpFinalRow[indexSource];
                m_rgb32FlatDataBuffer[indexDest + 3] = 0xff;
            }
        }
    }
    catch (...)
    {
        retVal = FITS_PNG_PIXEL_DATA_ERROR;
    }

    if (tmpRow != nullptr)
        delete [] tmpRow;

    if (tmpDestRow != nullptr)
        delete [] tmpDestRow;

    return retVal;
}

uint8_t** Image::getRGBData() const
{
    return m_rgbDataBuffer;
}

void Image::setRGBData(uint8_t** a_rgbDataBuffer)
{
    m_rgbDataBuffer = a_rgbDataBuffer;
}

void Image::copyRGBData(uint8_t** a_rgbDataBufferDest, uint8_t** a_rgbDataBufferSrc)
{
    uint8_t bytesNum = 3;

#if defined(ENABLE_OPENMP)
#pragma omp parallel for
#endif
    for (uint32_t y = 0; y < m_height; ++y)
        std::memcpy(a_rgbDataBufferDest[y], a_rgbDataBufferSrc[y], m_width * bytesNum);
}

uint8_t** Image::getRGB32Data() const
{
    return m_rgb32DataBuffer;
}

void Image::setRGB32Data(uint8_t** a_rgb32DataBuffer)
{
    m_rgb32DataBuffer = a_rgb32DataBuffer;
}

void Image::copyRGB32Data(uint8_t** a_rgb32DataBufferDest, uint8_t** a_rgb32DataBufferSrc)
{
    uint8_t bytesNum = 4;

#if defined(ENABLE_OPENMP)
#pragma omp parallel for
#endif
    for (uint32_t y = 0; y < m_height; ++y)
        std::memcpy(a_rgb32DataBufferDest[y], a_rgb32DataBufferSrc[y], m_width * bytesNum);
}

uint8_t* Image::getRGB32FlatData() const
{
    return m_rgb32FlatDataBuffer;
}

void Image::setRGB32FlatData(uint8_t* a_rgb32FlatDataBuffer)
{
    m_rgb32FlatDataBuffer = a_rgb32FlatDataBuffer;
}

void Image::copyRGB32FlatData(uint8_t* a_rgb32FlatDataBufferDest, uint8_t* a_rgb32FlatDataBufferSrc)
{
    uint8_t bytesNum = 4;

    std::memcpy(a_rgb32FlatDataBufferDest, a_rgb32FlatDataBufferSrc, m_width * m_height * bytesNum);
}

int32_t Image::_changeRGBColorChannelLevel(uint8_t a_channel, float a_quatient)
{
    int32_t retVal = FITS_GENERAL_SUCCESS;

    if (a_quatient < MIN_RGB_CHANNEL_CHANGE_FACTOR)
        a_quatient = MIN_RGB_CHANNEL_CHANGE_FACTOR;
    else if (a_quatient > MAX_RGB_CHANNEL_CHANGE_FACTOR)
        a_quatient = MAX_RGB_CHANNEL_CHANGE_FACTOR;

    // R-channel = 0, G-channel = 1, B-channel = 2
    if ((m_rgbDataBuffer == nullptr))
        return FITS_GENERAL_ERROR;

    for (uint32_t y = 0; y < m_height; ++y)
        for (uint32_t x = 0; x < m_width; ++x)
        {
            uint8_t channelVal = m_rgbDataBuffer[y][x*3 + a_channel];
            channelVal = (float)channelVal * a_quatient;
            m_rgbDataBuffer[y][x*3 + a_channel] = channelVal;
        }

    return retVal;
}

int32_t Image::_changeRGB32ColorChannelLevel(uint8_t a_channel, float a_quatient)
{
    int32_t retVal = FITS_GENERAL_SUCCESS;

    if (a_quatient < MIN_RGB_CHANNEL_CHANGE_FACTOR)
        a_quatient = MIN_RGB_CHANNEL_CHANGE_FACTOR;
    else if (a_quatient > MAX_RGB_CHANNEL_CHANGE_FACTOR)
        a_quatient = MAX_RGB_CHANNEL_CHANGE_FACTOR;

    // R-channel = 0, G-channel = 1, B-channel = 2
    if ((m_rgb32DataBuffer == nullptr))
        return FITS_GENERAL_ERROR;

    for (uint32_t y = 0; y < m_height; ++y)
        for (uint32_t x = 0; x < m_width; ++x)
        {
            uint8_t channelVal = m_rgb32DataBuffer[y][x*3 + a_channel];
            channelVal = (float)channelVal * a_quatient;
            m_rgb32DataBuffer[y][x*3 + a_channel] = channelVal;
        }

    return retVal;
}

int32_t Image::_changeRGB32FlatColorChannelLevel(uint8_t a_channel, float a_quatient)
{
    int32_t retVal = FITS_GENERAL_SUCCESS;

    if (a_quatient < MIN_RGB_CHANNEL_CHANGE_FACTOR)
        a_quatient = MIN_RGB_CHANNEL_CHANGE_FACTOR;
    else if (a_quatient > MAX_RGB_CHANNEL_CHANGE_FACTOR)
        a_quatient = MAX_RGB_CHANNEL_CHANGE_FACTOR;

    // R-channel = 0, G-channel = 1, B-channel = 2
    if ((m_rgb32FlatDataBuffer == nullptr))
        return FITS_GENERAL_ERROR;

    size_t indexY, y;  // is here due to OpenMP nested loop logic

#if defined(ENABLE_OPENMP)
#pragma omp parallel for collapse(2) private(indexY, y)
#endif
    for (y = 0; y < m_height; ++y)
    {
#if !defined(ENABLE_OPENMP)
        indexY = y*m_width;
#endif
        for (uint32_t x = 0; x < m_width; ++x)
        {
#if defined(ENABLE_OPENMP)
            indexY = y*m_width; // is here due to OpenMP nested loop logic
#endif
            size_t index = 4*(indexY + x) + a_channel;

            uint32_t channelVal = m_rgb32FlatDataBuffer[index];
            channelVal = (float)channelVal * a_quatient;
            //channelVal = (float)channelVal + (float)channelVal * a_quatient;
            m_rgb32FlatDataBuffer[index] = max256(channelVal);
        }
    }

    return retVal;
}

int32_t Image::changeRLevel(float a_quatient)
{
    return _changeRGBColorChannelLevel(0, a_quatient);
}

int32_t Image::changeGLevel(float a_quatient)
{
    return _changeRGBColorChannelLevel(1, a_quatient);
}

int32_t Image::changeBLevel(float a_quatient)
{
    return _changeRGBColorChannelLevel(2, a_quatient);
}

int32_t Image::change32RLevel(float a_quatient)
{
    return _changeRGB32ColorChannelLevel(0, a_quatient);
}

int32_t Image::change32GLevel(float a_quatient)
{
    return _changeRGB32ColorChannelLevel(1, a_quatient);
}

int32_t Image::change32BLevel(float a_quatient)
{
    return _changeRGB32ColorChannelLevel(2, a_quatient);
}

int32_t Image::change32FlatRLevel(float a_quatient)
{
    return _changeRGB32FlatColorChannelLevel(0, a_quatient);
}

int32_t Image::change32FlatGLevel(float a_quatient)
{
    return _changeRGB32FlatColorChannelLevel(1, a_quatient);
}

int32_t Image::change32FlatBLevel(float a_quatient)
{
    return _changeRGB32FlatColorChannelLevel(2, a_quatient);
}


int32_t Image::convertRGB2Grayscale()
{
    int32_t retVal = FITS_GENERAL_SUCCESS;

    // R-channel = 0, G-channel = 1, B-channel = 2
    if (m_rgbDataBuffer == nullptr)
        return FITS_GENERAL_ERROR;

    convertBufferRGB2Grayscale(m_rgbDataBuffer, m_width, m_height);

    return retVal;
}

int32_t Image::convertRGB322Grayscale()
{
    int32_t retVal = FITS_GENERAL_SUCCESS;

    // R-channel = 0, G-channel = 1, B-channel = 2
    if (m_rgb32DataBuffer == nullptr)
        return FITS_GENERAL_ERROR;

    convertBufferRGB322Grayscale(m_rgb32DataBuffer, m_width, m_height);

    return retVal;
}

int32_t Image::convertRGB32Flat2Grayscale()
{
    int32_t retVal = FITS_GENERAL_SUCCESS;

    // R-channel = 0, G-channel = 1, B-channel = 2
    if (m_rgb32FlatDataBuffer == nullptr)
        return FITS_GENERAL_ERROR;

    convertBufferRGB32Flat2Grayscale(m_rgb32FlatDataBuffer, m_width, m_height);

    return retVal;
}

void Image::deleteAllRGBData()
{
    deleteRGBData();
    deleteRGB32Data();
    deleteRGB32FlatData();
}

void Image::normalize(float a_min, float a_max, float a_minNew, float a_maxNew)
{
    //// this function will not work as m_dataBuffer is a memory mapped file buffer
    //// and is mapped as read-only
    /*
    if (std::abs(m_bitpix) == 32)
        libnfits::normalizeFloatBuffer(m_dataBuffer, m_width * m_height * sizeof(float), a_min, a_max, a_minNew, a_maxNew);
    */
}

void Image::calcRGBDataColorStats()
{
    uint64_t sumR = 0, sumG = 0, sumB = 0;
    uint64_t countR = 0, countG = 0, countB = 0;
    uint8_t avgR = 0, avgG = 0, avgB = 0;
    uint8_t minR = 0xff, minG = 0xff, minB = 0xff;
    uint8_t maxR = 0, maxG = 0, maxB = 0;

    if (m_rgbDataBuffer != nullptr)
    {
        for (uint32_t y = 0; y < m_height; ++y)
            for (uint32_t x = 0; x < m_width; ++x)
            {
                uint8_t val;

                val = m_rgbDataBuffer[y][x*3];
                if (val > 0) ++countR;
                if (val > maxR) maxR = val;
                if (val < minR) minR = val;
                sumR += val;

                val = m_rgbDataBuffer[y][x*3 + 1];
                if (val > 0) ++countG;
                if (val > maxG) maxG = val;
                if (val < minG) minG = val;
                sumG += val;

                val = m_rgbDataBuffer[y][x*3 + 2];
                if (val > 0) ++countB;
                if (val > maxB) maxB = val;
                if (val < minB) minB = val;
                sumB == val;
            }

        if (countR > 0) avgR = sumR / countR;
        if (countG > 0) avgG = sumG / countG;
        if (countB > 0) avgB = sumB / countB;
    }

    m_colorStats = { sumR, sumG, sumB, countR, countG, countB, avgR, avgG, avgB, minR, minG, minB, maxR, maxG, maxB };
}

void Image::calcRGB32DataColorStats()
{
    uint64_t sumR = 0, sumG = 0, sumB = 0;
    uint64_t countR = 0, countG = 0, countB = 0;
    uint8_t avgR = 0, avgG = 0, avgB = 0;
    uint8_t minR = 0xff, minG = 0xff, minB = 0xff;
    uint8_t maxR = 0, maxG = 0, maxB = 0;

    if (m_rgb32DataBuffer != nullptr)
    {
        for (uint32_t y = 0; y < m_height; ++y)
            for (uint32_t x = 0; x < m_width; ++x)
            {
                uint8_t val;

                val = m_rgb32DataBuffer[y][x*3];
                if (val > 0) ++countR;
                if (val > maxR) maxR = val;
                if (val < minR) minR = val;
                sumR += val;

                val = m_rgb32DataBuffer[y][x*3 + 1];
                if (val > 0) ++countG;
                if (val > maxG) maxG = val;
                if (val < minG) minG = val;
                sumG += val;

                val = m_rgb32DataBuffer[y][x*3 + 2];
                if (val > 0) ++countB;
                if (val > maxB) maxB = val;
                if (val < minB) minB = val;
                sumB == val;
            }

        if (countR > 0) avgR = sumR / countR;
        if (countG > 0) avgG = sumG / countG;
        if (countB > 0) avgB = sumB / countB;
    }

    m_colorStats = { sumR, sumG, sumB, countR, countG, countB, avgR, avgG, avgB, minR, minG, minB, maxR, maxG, maxB };
}

void Image::calcRGB32FlatDataColorStats()
{
    uint64_t sumR = 0, sumG = 0, sumB = 0;
    uint64_t countR = 0, countG = 0, countB = 0;
    uint8_t avgR = 0, avgG = 0, avgB = 0;
    uint8_t minR = 0xff, minG = 0xff, minB = 0xff;
    uint8_t maxR = 0, maxG = 0, maxB = 0;

    if (m_rgb32FlatDataBuffer != nullptr)
    {
        for (uint32_t y = 0; y < m_height; ++y)
            for (uint32_t x = 0; x < m_width; ++x)
            {
                uint8_t val;

                val = m_rgb32FlatDataBuffer[4*(y*m_width + x) + 2];
                if (val > 0) ++countR;
                if (val > maxR) maxR = val;
                if (val < minR) minR = val;
                sumR += val;

                val = m_rgb32FlatDataBuffer[4*(y*m_width + x) + 1];
                if (val > 0) ++countG;
                if (val > maxG) maxG = val;
                if (val < minG) minG = val;
                sumG += val;

                val = m_rgb32FlatDataBuffer[4*(y*m_width + x)];
                if (val > 0) ++countB;
                if (val > maxB) maxB = val;
                if (val < minB) minB = val;
                sumB += val;
            }

        if (countR > 0) avgR = sumR / countR;
        if (countG > 0) avgG = sumG / countG;
        if (countB > 0) avgB = sumB / countB;
    }

    m_colorStats = { sumR, sumG, sumB, countR, countG, countB, avgR, avgG, avgB, minR, minG, minB, maxR, maxG, maxB };
}

void Image::_convertRGB2AltColors(uint8_t a_red, uint8_t a_green, uint8_t a_blue,
                                  uint8_t& a_newRed, uint8_t& a_newGreen, uint8_t& a_newBlue)
{
    const uint8_t rgbThreshold = 0x7f;
    const uint8_t avgQuatient = 2;

    a_newRed = a_red;
    a_newGreen = a_green;
    a_newBlue = a_blue;

    // finding the brightest objects and converting their colors to human-eye pleasant ones
    if (greater3(a_red, a_green, a_blue, rgbThreshold))
    {
        if (a_blue > a_red && a_blue > a_green)
        {
            a_newRed = 0xff / avgQuatient;
            a_newGreen = (255 - ((float)a_blue / 0xff) * (255 - 102)) / avgQuatient;
            a_newBlue = (0xff - a_blue) / avgQuatient;
        }
        else if (a_green > a_blue && a_green > a_red)
        {
            a_newRed = 0xff;
            a_newGreen = 240 - ((float)a_blue / 0xff) * (240 - 102);
            a_newBlue = 0xff - a_blue;
        }
        else if (a_red > a_blue && a_red > a_green)
        {
            a_newGreen = 0xff;
            a_newRed = 0xff - a_blue;
            a_newBlue = 240 - ((float)a_green / 0xff) * (240 - 102);
        }
    }
}

void Image::_convertBufferRGB32Flat2EyeComfortColors()
{
    if (m_rgb32FlatDataBuffer == nullptr)
        return;

#if defined(ENABLE_OPENMP)
#pragma omp parallel for collapse(2)
#endif
    for (uint32_t y = 0; y < m_height; ++y)
        for (uint32_t x = 0; x < m_width; ++x)
        {
            uint8_t red, green, blue;

            size_t indexBase = 4*(y*m_width + x);

            _convertRGB2AltColors(m_rgb32FlatDataBuffer[indexBase + 2], m_rgb32FlatDataBuffer[indexBase + 1],
                                  m_rgb32FlatDataBuffer[indexBase], red, green, blue);

            m_rgb32FlatDataBuffer[indexBase] = blue;
            m_rgb32FlatDataBuffer[indexBase + 1] = green;
            m_rgb32FlatDataBuffer[indexBase + 2] = red;
            m_rgb32FlatDataBuffer[indexBase + 3] = 0xff;
        }
}

int32_t Image::convertRGB32Flat2EyeComfortColors()
{
    int32_t retVal = FITS_GENERAL_SUCCESS;

    if (m_rgb32FlatDataBuffer == nullptr)
        return FITS_GENERAL_ERROR;

    calcRGB32FlatDataColorStats();

    _convertBufferRGB32Flat2EyeComfortColors();

    return retVal;
}

ImageColorStats Image::getColorStats() const
{
    return m_colorStats;
}

void Image::setMaxDataBufferSize(size_t a_size)
{
    m_maxDataBufferSize = a_size;
}

size_t Image::getMaxDataBufferSize() const
{
    return m_maxDataBufferSize;
}

void Image::setBaseOffset(size_t a_baseOffset)
{
    m_baseOffset = a_baseOffset;
}

size_t Image::getBaseOffset() const
{
    return m_baseOffset;
}

void Image::setBZero(double a_bzero)
{
    m_bzero = a_bzero;
}

void Image::setBScale(double a_bscale)
{
    m_bscale = a_bscale;
}

double Image::getBZero() const
{
    return m_bzero;
}

double Image::getBScale() const
{
    return m_bscale;
}

void Image::processRGBBrightnessFilter(uint8_t a_threshold)
{
    if (m_rgbDataBuffer == nullptr)
        return;

    for (int32_t y = 0; y < m_height; ++y)
    {
        for (int32_t x = 0; x < m_width; ++x)
        {
            uint8_t brightness = calcPixelBrightness(m_rgbDataBuffer[y][x], m_rgbDataBuffer[y][x +1], m_rgbDataBuffer[y][x + 2]);

            if (brightness < a_threshold)
            {
                m_rgbDataBuffer[y][x] = 0;
                m_rgbDataBuffer[y][x + 1] = 0;
                m_rgbDataBuffer[y][x + 2] = 0;
            }
        }
    }
}

void Image::processRGB32BrightnessFilter(uint8_t a_threshold)
{
    if (m_rgb32DataBuffer == nullptr)
        return;

    for (int32_t y = 0; y < m_height; ++y)
    {
        for (int32_t x = 0; x < m_width; ++x)
        {
            uint8_t brightness = calcPixelBrightness(m_rgb32DataBuffer[y][x], m_rgb32DataBuffer[y][x +1], m_rgb32DataBuffer[y][x + 2]);

            if (brightness < a_threshold)
            {
                m_rgb32DataBuffer[y][x] = 0;
                m_rgb32DataBuffer[y][x + 1] = 0;
                m_rgb32DataBuffer[y][x + 2] = 0;
            }
        }
    }
}

void Image::processRGBB32FlatBrightnessFilter(uint8_t a_threshold)
{
    if (m_rgb32FlatDataBuffer == nullptr)
        return;

    for (int32_t y = 0; y < m_height; ++y)
    {
        for (int32_t x = 0; x < m_width; ++x)
        {
            uint64_t indexDest = 4*(y*m_width + x);

            uint8_t brightness = calcPixelBrightness(m_rgb32FlatDataBuffer[indexDest], m_rgb32FlatDataBuffer[indexDest+1],
                                                     m_rgb32FlatDataBuffer[indexDest + 2]);

            if (brightness < a_threshold)
            {
                m_rgb32FlatDataBuffer[indexDest]     = 0;
                m_rgb32FlatDataBuffer[indexDest + 1] = 0;
                m_rgb32FlatDataBuffer[indexDest + 2] = 0;
            }
        }
    }
}

void Image::setMinValue(double a_value)
{
    m_minValue = a_value;
}

double Image::getMinValue() const
{
    return m_minValue;
}

void Image::setMaxValue(double a_value)
{
    m_maxValue = a_value;
}

double Image::getMaxValue() const
{
    return m_maxValue;
}

void Image::setMinMaxValues(double a_minValue, double a_maxValue)
{
    m_minValue = a_minValue;
    m_maxValue = a_maxValue;
}

void Image::setMinValueL(int64_t a_value)
{
    m_minValueL = a_value;
}

int64_t Image::getMinValueL() const
{
    return m_minValueL;
}

void Image::setMaxValueL(int64_t a_value)
{
    m_maxValueL = a_value;
}

int64_t Image::getMaxValueL() const
{
    return m_maxValueL;
}

void Image::setMinMaxValuesL(int64_t a_minValue, int64_t a_maxValue)
{
    m_minValueL = a_minValue;
    m_maxValueL = a_maxValue;
}

template<typename T> void Image::calcBufferMinMax()
{
    if (m_isMinMaxCounted)
        return;

    float minValueF = 0.0, maxValueF = 0.0;
    double minValueD = 0.0, maxValueD = 0.0;

    uint8_t minValue8 = 0, maxValue8 = 0;
    int16_t minValue16 = 0, maxValue16 = 0;
    int32_t minValue32 = 0, maxValue32 = 0;
    int64_t minValue64 = 0, maxValue64 = 0;

    size_t size = m_width * m_height * sizeof(T);

    if (m_baseOffset + size <= m_maxDataBufferSize)
    {                
        if (std::is_same<T, float>::value)
        {
            libnfits::getFloatBufferMinMax(m_dataBuffer, size, minValueF, maxValueF);

            m_minValue = minValueF;
            m_maxValue = maxValueF;

            return;
        }

        if (std::is_same<T, double>::value)
        {
            libnfits::getDoubleBufferMinMax(m_dataBuffer, size, minValueD, maxValueD);

            m_minValue = minValueD;
            m_maxValue = maxValueD;

            return;
        }

        if (std::is_same<T, uint8_t>::value)
        {
            libnfits::getByteBufferMinMax(m_dataBuffer, size, minValue8, maxValue8);

            m_minValueL = minValue8;
            m_maxValueL = maxValue8;

            return;
        }

        if (std::is_same<T, int16_t>::value)
        {
            libnfits::getShortBufferMinMax(m_dataBuffer, size, minValue16, maxValue16);

            m_minValueL = minValue16;
            m_maxValueL = maxValue16;

            return;
        }

        if (std::is_same<T, int32_t>::value)
        {
            libnfits::getIntBufferMinMax(m_dataBuffer, size, minValue32, maxValue32);

            m_minValueL = minValue32;
            m_maxValueL = maxValue32;

            return;
        }

        if (std::is_same<T, int64_t>::value)
        {
            libnfits::getLongBufferMinMax(m_dataBuffer, size, minValue64, maxValue64);

            m_minValueL = minValue64;
            m_maxValueL = maxValue64;

            return;
        }

        m_isMinMaxCounted =true;
    }
}

template<typename T> T Image::getMinValue() const
{
    if (std::is_same<T, float>::value || std::is_same<T, double>::value)
        return m_minValue;
    else if (std::is_same<T, uint8_t>::value|| std::is_same<T, uint16_t>::value || std::is_same<T, uint32_t>::value
             || std::is_same<T, uint64_t>::value)
        return m_minValueL;
    else
        return std::numeric_limits<T>::min();
}

template<typename T> T Image::getMaxValue() const
{
    if (std::is_same<T, float>::value || std::is_same<T, double>::value)
        return m_maxValue;
    else if (std::is_same<T, uint8_t>::value || std::is_same<T, uint16_t>::value || std::is_same<T, uint32_t>::value
             || std::is_same<T, uint64_t>::value)
        return m_maxValueL;
    else
        return std::numeric_limits<T>::max();
}

template<typename T> T Image::getBufferMinMaxRange() const
{
    return getMaxValue<T>() - getMinValue<T>();
}

template<typename T> T Image::getDistribMinValue() const
{
    if (std::is_same<T, float>::value || std::is_same<T, double>::value)
        return m_minDistribValue;
    else if (std::is_same<T, uint16_t>::value || std::is_same<T, uint32_t>::value || std::is_same<T, uint64_t>::value)
        return m_minDistribValueL;
    else
        return std::numeric_limits<T>::min();
}

template<typename T> T Image::getDistribMaxValue() const
{
    if (std::is_same<T, float>::value || std::is_same<T, double>::value)
        return m_maxDistribValue;
    else if (std::is_same<T, uint16_t>::value || std::is_same<T, uint32_t>::value || std::is_same<T, uint64_t>::value)
        return m_maxDistribValueL;
    else
        return std::numeric_limits<T>::max();
}

double Image::getDistribMinValue() const
{
    return m_minDistribValue;
}

double Image::getDistribMaxValue() const
{
    return m_maxDistribValue;
}

int64_t Image::getDistribMinValueL() const
{
    return m_minDistribValueL;
}

int64_t Image::getDistribMaxValueL() const
{
    return m_maxDistribValueL;
}

template void Image::calcBufferMinMax<float>();
template void Image::calcBufferMinMax<double>();
template void Image::calcBufferMinMax<uint8_t>();
template void Image::calcBufferMinMax<int16_t>();
template void Image::calcBufferMinMax<int32_t>();
template void Image::calcBufferMinMax<int64_t>();

template float Image::getMinValue<float>() const;
template float Image::getMaxValue<float>() const;

template double Image::getMinValue<double>() const;
template double Image::getMaxValue<double>() const;

template uint8_t Image::getMinValue<uint8_t>() const;
template uint8_t Image::getMaxValue<uint8_t>() const;

template uint16_t Image::getMinValue<uint16_t>() const;
template uint16_t Image::getMaxValue<uint16_t>() const;

template uint32_t Image::getMinValue<uint32_t>() const;
template uint32_t Image::getMaxValue<uint32_t>() const;

template uint64_t Image::getMinValue<uint64_t>() const;
template uint64_t Image::getMaxValue<uint64_t>() const;

uint32_t Image::getTransformType() const
{
    return  m_transformType;
}

void Image::setDistribCountFlag(bool a_flag)
{
    m_isDistribCounted = a_flag;
}

void Image::calcBufferDistribution(int32_t a_percent)
{
    //// checking if the memory-mapped file is corrupted and not all data is available
    //// e.g. data required by the HDUs is bigger then the file itself
    size_t offset = m_baseOffset + m_width*m_height*(std::abs(m_bitpix)/8);
    if (offset > m_maxDataBufferSize)
        return;
    ////

    float percent = (float)(100 - a_percent) / 100;

    if (a_percent != m_percentThreshold)
    {
        int8_t bpx = std::abs(m_bitpix)/8;

        if (m_bitpix == -32)
        {
            float tmpMin,tmpMax;

            getBufferDistributionMinMax<float>(m_dataBuffer, m_width*m_height*bpx, percent,
                                               m_minValue, m_maxValue, tmpMin, tmpMax, m_distribStats, m_isDistribCounted);

            m_isDistribCounted = true;
            m_minDistribValue = tmpMin;
            m_maxDistribValue = tmpMax;
            m_percentThreshold = a_percent;

            ////std::cout << "[INFO]: (F) m_minValue = " << m_minValue << " , m_maxValue = " << m_maxValue <<std::endl;
            ////std::cout << "[INFO]: (F) m_minDistribValue = " << m_minDistribValue << " , m_maxDistribValue = " << m_maxDistribValue <<std::endl;
        }
        else if (m_bitpix == -64)
        {
            double tmpMin,tmpMax;

            getBufferDistributionMinMax<double>(m_dataBuffer, m_width*m_height*bpx, percent,
                                                m_minValue, m_maxValue, tmpMin, tmpMax, m_distribStats, m_isDistribCounted);
            m_isDistribCounted = true;
            m_minDistribValue = tmpMin;
            m_maxDistribValue = tmpMax;
            m_percentThreshold = a_percent;

            ////std::cout << "[INFO]: (D) m_minValue = " << m_minValue << " , m_maxValue = " << m_maxValue <<std::endl;
            ////std::cout << "[INFO]: (D) m_minDistribValue = " << m_minDistribValue << " , m_maxDistribValue = " << m_maxDistribValue <<std::endl;
        }
        else if (m_bitpix == 8)
        {
            int8_t tmpMin, tmpMax;

            getBufferDistributionMinMax<int8_t>(m_dataBuffer, m_width*m_height*bpx, percent,
                                                m_minValueL, m_maxValueL, tmpMin, tmpMax, m_distribStats, m_isDistribCounted);

            m_isDistribCounted = true;
            m_minDistribValueL = tmpMin;
            m_maxDistribValueL = tmpMax;
            m_percentThreshold = a_percent;

            ////std::cout << "[INFO]: (I8) m_minValue = " << m_minValueL << " , m_maxValueL = " << m_maxValueL <<std::endl;
            ////std::cout << "[INFO]: (I8) m_minDistribValueL = " << m_minDistribValueL << " , m_maxDistribValueL = " << m_maxDistribValueL <<std::endl;
        }
        else if (m_bitpix == 16)
        {
            int16_t tmpMin, tmpMax;

            getBufferDistributionMinMax<int16_t>(m_dataBuffer, m_width*m_height*bpx, percent,
                                                 m_minValueL, m_maxValueL, tmpMin, tmpMax, m_distribStats, m_isDistribCounted);

            m_isDistribCounted = true;
            m_minDistribValueL = tmpMin;
            m_maxDistribValueL = tmpMax;
            m_percentThreshold = a_percent;

            ////std::cout << "[INFO]: (I16) m_minValue = " << m_minValueL << " , m_maxValueL = " << m_maxValueL <<std::endl;
            ////std::cout << "[INFO]: (I16) m_minDistribValueL = " << m_minDistribValueL << " , m_maxDistribValueL = " << m_maxDistribValueL <<std::endl;
        }
        else if (m_bitpix == 32)
        {
            int32_t tmpMin, tmpMax;

            getBufferDistributionMinMax<int32_t>(m_dataBuffer, m_width*m_height*bpx, percent,
                                                 m_minValueL, m_maxValueL, tmpMin, tmpMax, m_distribStats, m_isDistribCounted);

            m_isDistribCounted = true;
            m_minDistribValueL = tmpMin;
            m_maxDistribValueL = tmpMax;
            m_percentThreshold = a_percent;

            ////std::cout << "[INFO]: (I32) m_minValue = " << m_minValueL << " , m_maxValueL = " << m_maxValueL <<std::endl;
            ////std::cout << "[INFO]: (I32) m_minDistribValueL = " << m_minDistribValueL << " , m_maxDistribValueL = " << m_maxDistribValueL <<std::endl;
        }
        else if (m_bitpix == 64)
        {
            int64_t tmpMin, tmpMax;

            getBufferDistributionMinMax<int64_t>(m_dataBuffer, m_width*m_height*bpx, percent,
                                                 m_minValueL, m_maxValueL, tmpMin, tmpMax, m_distribStats, m_isDistribCounted);

            m_isDistribCounted = true;
            m_minDistribValueL = tmpMin;
            m_maxDistribValueL = tmpMax;
            m_percentThreshold = a_percent;

            ////std::cout << "[INFO]: (I64) m_minValue = " << m_minValueL << " , m_maxValueL = " << m_maxValueL <<std::endl;
            ////std::cout << "[INFO]: (I64) m_minDistribValueL = " << m_minDistribValueL << " , m_maxDistribValueL = " << m_maxDistribValueL <<std::endl;
        }
    }

    if (m_percentThreshold == 0)
    {
        m_finalMinValue = m_minValue;
        m_finalMaxValue = m_maxValue;
        m_finalMinValueL = m_minValueL;
        m_finalMaxValueL = m_maxValueL;
    }
    else
    {
        m_finalMinValue = m_minDistribValue;
        m_finalMaxValue = m_maxDistribValue;
        m_finalMinValueL = m_minDistribValueL;
        m_finalMaxValueL = m_maxDistribValueL;
    }
}

void Image::resetDistribValues()
{
    m_finalMinValue = m_minDistribValue = m_minValue = std::numeric_limits<double>::min();
    m_finalMaxValue = m_maxDistribValue = m_maxValue = std::numeric_limits<double>::max();

    m_finalMinValueL = m_minDistribValueL = m_minValueL = std::numeric_limits<int64_t>::min();
    m_finalMaxValueL = m_maxDistribValueL = m_maxValueL = std::numeric_limits<int64_t>::max();
}

void Image::convertBufferAllTypes2RGB(uint8_t* tmpRow, size_t tmpBufRowSize, uint8_t* tmpDestRow)
{
    bool a_zeroScaleFlag = !(areEqual(m_bzero, FITS_BZERO_DEFAULT_VALUE) && areEqual(m_bscale, FITS_BSCALE_DEFAULT_VALUE));

    if (m_bitpix == 8)
        convertBufferByte2RGB(tmpRow, tmpBufRowSize, tmpDestRow);
    else if (m_bitpix == 16)
        //// ORIGINAL working block
        /*
        !a_zeroScaleFlag ?
            ////convertBufferShort2RGB(tmpRow, tmpBufRowSize, tmpDestRow, true, m_minValueL, m_maxValueL, a_transformType) : // original WORKING version
            convertBufferShort2RGB(tmpRow, tmpBufRowSize, tmpDestRow, true, m_finalMinValueL, m_finalMaxValueL, m_transformType) : //// final working version
            ////convertBufferShortSZ2RGB(tmpRow, tmpBufRowSize, m_bzero, m_bscale, tmpDestRow, true, // original WORKING version
            ////                         m_minValueL, m_maxValueL, a_transformType);
            convertBufferShortSZ2RGB(tmpRow, tmpBufRowSize, m_bzero, m_bscale, tmpDestRow, true,     //// final working version
                                     m_finalMinValueL, m_finalMaxValueL, m_transformType);
        */
        //// End of ORIGINAL working block
        convertBufferShortRGB(tmpRow, tmpBufRowSize, tmpDestRow, m_finalMinValueL, m_finalMaxValueL,
                              m_bzero, m_bscale, true, a_zeroScaleFlag, m_transformType);
    else if (m_bitpix == -32)
        ////convertBufferFloat2RGB(tmpRow, tmpBufRowSize, m_minValue, m_maxValue, a_transformType); // original WORKING version
        convertBufferFloat2RGB(tmpRow, tmpBufRowSize, m_finalMinValue, m_finalMaxValue, m_bzero, m_bscale, a_zeroScaleFlag, m_transformType); //// final working BZERO+BSCALE version
    else if (m_bitpix == 32)
        ////convertBufferInt2RGB(tmpRow, tmpBufRowSize, m_minValueL, m_maxValueL, a_transformType); // original WORKING version
        convertBufferInt2RGB(tmpRow, tmpBufRowSize, m_finalMinValueL, m_finalMaxValueL, m_bzero, m_bscale, a_zeroScaleFlag, m_transformType); //// final working BZERO+BSCALE version
    else if (m_bitpix == -64)
        ////convertBufferDouble2RGB(tmpRow, tmpBufRowSize, m_minValue, m_maxValue, a_transformType); // original WORKING version
        convertBufferDouble2RGB(tmpRow, tmpBufRowSize, m_finalMinValue, m_finalMaxValue, m_bzero, m_bscale, a_zeroScaleFlag, m_transformType); //// final working BZERO+BSCALE version
    else if (m_bitpix == 64)
        ////convertBufferLong2RGB(tmpRow, tmpBufRowSize, m_minValueL, m_maxValueL, a_transformType); // original WORKING version
        convertBufferLong2RGB(tmpRow, tmpBufRowSize, m_finalMinValueL, m_finalMaxValueL, m_bzero, m_bscale, a_zeroScaleFlag, m_transformType); //// final working BZERO+BSCALE version
}

bool Image::isDefaultBZeroBScale() const
{
    if (!areEqual(m_bzero, 0.0) || !areEqual(m_bscale, 1.0))
        return false;
    else
        return true;
}

//// these functions are for debugging purposes only, they are slow
int32_t Image::dumpFloatDataBuffer(const std::string& a_filename, uint32_t a_rowSize)
{
    return libnfits::dumpFloatDataBuffer(m_dataBuffer, m_width * m_height * sizeof(float), a_filename, a_rowSize);
}

int32_t Image::dumpDoubleDataBuffer(const std::string& a_filename, uint32_t a_rowSize)
{
    return libnfits::dumpDoubleDataBuffer(m_dataBuffer, m_width * m_height * sizeof(float), a_filename, a_rowSize);
}
}
