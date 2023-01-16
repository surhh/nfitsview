#include "image.h"
#include "pngfile.h"

#include <cstring>
#include "helperfunctions.h"

namespace libnfits
{

Image::Image():
    m_dataBuffer(nullptr),
    m_rgbDataBuffer(nullptr), m_rgb32DataBuffer(nullptr), m_rgb32FlatDataBuffer(nullptr),
    m_rgbDataBackupBuffer(nullptr), m_rgb32DataBackupBuffer(nullptr), m_rgb32FlatDataBackupBuffer(nullptr), m_maxDataBufferSize(0), m_baseOffset(0),
    m_width(0), m_height(0), m_colorDepth(0), m_bitpix(0), m_isCompressed(false), m_bzero(FITS_BZERO_DEFAULT_VALUE),
    m_bscale(FITS_BSCALE_DEFAULT_VALUE), m_title(""), m_callbackFunc(nullptr), m_callbackFuncParam(nullptr),
    m_transformType(FITS_FLOAT_DOUBLE_NO_TRANSFORM)
{
    m_colorStats = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    m_minValue = std::numeric_limits<double>::min();
    m_maxValue = std::numeric_limits<double>::max();
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
    m_transformType = FITS_FLOAT_DOUBLE_NO_TRANSFORM;

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

int32_t Image::exportPNG(const std::string& a_fileName)
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

    if (bytesNum == 2 | bytesNum == 3 || bytesNum == 4 || bytesNum == 8)
        colorType = FITS_PNG_COLOR_RGB;  // the FITS_PNG_COLOR_RGB_ALPHA seems not to work as expected on the FITS pixel array
    else
        return FITS_PNG_EXPORT_ERROR;

    if (m_callbackFunc != nullptr)
        m_callbackFunc(50, m_callbackFuncParam); // sample callback to send info 50% of work is done

    pngFile.setParameters(a_fileName, m_width, m_height, m_colorDepth, colorType, m_title);

    // the generic working version, processing the float data, not used anymore
    //retVal = pngFile.createFromPixelData(pixelBuffer);

    createRGBData();
    //convertRGB2Grayscale();
    retVal = pngFile.createFromRGBData((const uint8_t**)m_rgbDataBuffer);
    deleteRGBData(); // 24-bit RGB buffer may not be needed anymore, preferrable to have 32-bit RGB buffer

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
    //deleteAllRGBData();
    //deleteAllBackupRGBData();
    deletAllData();

    m_width = 0;
    m_height = 0;
    m_colorDepth = 0;
    m_bitpix = 0;
    m_maxDataBufferSize = 0;
    m_baseOffset = 0;
    m_bscale = FITS_BSCALE_DEFAULT_VALUE;
    m_bzero = FITS_BZERO_DEFAULT_VALUE;
    m_isCompressed = false;
    m_callbackFunc = nullptr;
    m_title.clear();

    m_colorStats = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    m_minValue = std::numeric_limits<double>::min();
    m_maxValue = std::numeric_limits<double>::max();

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

void Image::deletAllData()
{
    deleteAllRGBData();
    deleteAllBackupRGBData();
}

int32_t Image::createRGBData(uint32_t a_transformType)
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

    size_t tmpBufRowSize = m_width * bytesNum * (m_colorDepth / (sizeof(uint8_t) * 8)) * sizeof(uint8_t);  // 32-bit element buffer for temp ussage

    m_rgbDataBuffer = new uint8_t*[m_height]();

    if (m_rgbDataBuffer == nullptr)
        return FITS_GENERAL_ERROR;

    uint8_t *tmpRow = new uint8_t[tmpBufRowSize];

    uint8_t *tmpDestRow = nullptr;

    uint8_t *tmpFinalRow = tmpRow;

    if (m_bitpix == 16 || m_bitpix == 8)
    {
        //tmpDestRow = new uint8_t[tmpBufRowSize * (bytesNum == 2 ? 2 : 1)];
        tmpDestRow = new uint8_t[tmpBufRowSize * (32/std::abs(m_bitpix))];
        tmpFinalRow = tmpDestRow;
    }

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

            if (m_bitpix == 8)
                convertBufferByte2RGB(tmpRow, tmpBufRowSize, tmpDestRow);
            else if (m_bitpix == 16)
                areEqual(m_bzero, FITS_BZERO_DEFAULT_VALUE) && areEqual(m_bscale, FITS_BSCALE_DEFAULT_VALUE) ?
                        convertBufferShort2RGB(tmpRow, tmpBufRowSize, tmpDestRow) :
                        convertBufferShortSZ2RGB(tmpRow, tmpBufRowSize, m_bzero, m_bscale, tmpDestRow);
            else if (m_bitpix == -32)
                convertBufferFloat2RGB(tmpRow, tmpBufRowSize, m_minValue, m_maxValue, a_transformType);
            else if (m_bitpix == 32)
                convertBufferInt2RGB(tmpRow, tmpBufRowSize);
            else if (m_bitpix == -64)
                convertBufferDouble2RGB(tmpRow, tmpBufRowSize, m_minValue, m_maxValue, a_transformType);
            else if (m_bitpix == 64)
                convertBufferLong2RGB(tmpRow, tmpBufRowSize);

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

    if (m_bitpix == 16 && tmpDestRow != nullptr)
        delete [] tmpDestRow;

    return retVal;
}

int32_t Image::createRGB32Data(uint32_t a_transformType)
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

    size_t tmpBufRowSize = m_width * bytesNum * (m_colorDepth / (sizeof(uint8_t) * 8)) * sizeof(uint8_t);  // 32-bit element buffer for temp ussage

    m_rgb32DataBuffer = new uint8_t*[m_height]();

    //// the image has been converted to RGB already
    if (m_rgb32DataBuffer == nullptr)
        return FITS_GENERAL_ERROR;

    uint8_t *tmpRow = new uint8_t[tmpBufRowSize];

    uint8_t *tmpDestRow = nullptr;

    uint8_t *tmpFinalRow = tmpRow;

    if (m_bitpix == 16 || m_bitpix == 8)
    {
        //tmpDestRow = new uint8_t[tmpBufRowSize * (bytesNum == 2 ? 2 : 1)];
        tmpDestRow = new uint8_t[tmpBufRowSize * (32/std::abs(m_bitpix))];
        tmpFinalRow = tmpDestRow;
    }

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

            if (m_bitpix == 8)
                convertBufferByte2RGB(tmpRow, tmpBufRowSize, tmpDestRow);
            else if (m_bitpix == 16)
                areEqual(m_bzero, FITS_BZERO_DEFAULT_VALUE) && areEqual(m_bscale, FITS_BSCALE_DEFAULT_VALUE) ?
                        convertBufferShort2RGB(tmpRow, tmpBufRowSize, tmpDestRow) :
                        convertBufferShortSZ2RGB(tmpRow, tmpBufRowSize, m_bzero, m_bscale, tmpDestRow);
            else if (m_bitpix == -32)
                convertBufferFloat2RGB(tmpRow, tmpBufRowSize, m_minValue, m_maxValue, a_transformType);
            else if (m_bitpix == 32)
                convertBufferInt2RGB(tmpRow, tmpBufRowSize);
            else if (m_bitpix == -64)
                convertBufferDouble2RGB(tmpRow, tmpBufRowSize, m_minValue, m_maxValue, a_transformType);
            else if (m_bitpix == 64)
                convertBufferLong2RGB(tmpRow, tmpBufRowSize);

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

    if (m_bitpix == 16 && tmpDestRow != nullptr)
        delete [] tmpDestRow;

    return retVal;
}

int32_t Image::createRGB32FlatData(uint32_t a_transformType)
{
    int32_t retVal = FITS_GENERAL_SUCCESS;

    uint8_t bytesNum = std::abs(m_bitpix) / 8;

    if (bytesNum != 1 && bytesNum != 2 && bytesNum != 4 && bytesNum != 8)
        return FITS_GENERAL_ERROR;

    //// the image has been converted to RGB already
    if (m_rgb32FlatDataBuffer != nullptr)
        return FITS_GENERAL_ERROR;

    //// calculating the PNG pixel buffer size. This way is more understandable in terms of logic
    //// thow we need to have only RGB data, we actually need to have it 32-bit aligned for some future use cases,
    //// that's why it's multipled by 4 instead of 3 (kind of tricky stuff, but works fine)

    size_t tmpBufRowSize = m_width * bytesNum * (m_colorDepth / (sizeof(uint8_t) * 8)) * sizeof(uint8_t);  //// 32/64-bit element buffer for temp ussage

    size_t flatBufSize = m_height * m_width * 4;

    if (m_rgb32FlatDataBuffer == nullptr)
    {
        m_rgb32FlatDataBuffer = new uint8_t[flatBufSize];
        std::memset(m_rgb32FlatDataBuffer, 0, flatBufSize);
    }

    if (m_rgb32FlatDataBuffer == nullptr)
        return FITS_GENERAL_ERROR;

    uint8_t *tmpRow = new uint8_t[tmpBufRowSize];

    uint8_t *tmpDestRow = nullptr;

    uint8_t *tmpFinalRow = tmpRow;

    if (m_bitpix == 16 || m_bitpix == 8)
    {
        //tmpDestRow = new uint8_t[tmpBufRowSize * (bytesNum == 2 ? 2 : 1)];
        tmpDestRow = new uint8_t[tmpBufRowSize * (32/std::abs(m_bitpix))];
        tmpFinalRow = tmpDestRow;
    }

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

            if (m_bitpix == 8)
                convertBufferByte2RGB(tmpRow, tmpBufRowSize, tmpDestRow);
            else if (m_bitpix == 16)
                areEqual(m_bzero, FITS_BZERO_DEFAULT_VALUE) && areEqual(m_bscale, FITS_BSCALE_DEFAULT_VALUE) ?
                        convertBufferShort2RGB(tmpRow, tmpBufRowSize, tmpDestRow) :
                        convertBufferShortSZ2RGB(tmpRow, tmpBufRowSize, m_bzero, m_bscale, tmpDestRow);
            else if (m_bitpix == -32)
                convertBufferFloat2RGB(tmpRow, tmpBufRowSize, m_minValue, m_maxValue, a_transformType);
            else if (m_bitpix == 32)
                convertBufferInt2RGB(tmpRow, tmpBufRowSize);
            else if (m_bitpix == -64)
                convertBufferDouble2RGB(tmpRow, tmpBufRowSize, m_minValue, m_maxValue, a_transformType);
            else if (m_bitpix == 64)
                convertBufferLong2RGB(tmpRow, tmpBufRowSize);

            for (uint32_t x = 0; x < m_width; ++x)
            {
                uint64_t indexSource = x*indexBase;
                uint64_t indexDest = 4*(y*m_width + x);

                m_rgb32FlatDataBuffer[indexDest]     = tmpFinalRow[indexSource + 2];
                m_rgb32FlatDataBuffer[indexDest + 1] = tmpFinalRow[indexSource + 1];
                m_rgb32FlatDataBuffer[indexDest + 2] = tmpFinalRow[indexSource];
                m_rgb32FlatDataBuffer[indexDest + 3] = 0xff;
            }

            m_transformType = a_transformType;
        }
    }
    catch (...)
    {
        retVal = FITS_PNG_PIXEL_DATA_ERROR;
    }

    if (tmpRow != nullptr)
        delete [] tmpRow;

    if (m_bitpix == 16 && tmpDestRow != nullptr)
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

    for (uint32_t y = 0; y < m_height; ++y)
        for (uint32_t x = 0; x < m_width; ++x)
        {
            uint32_t channelVal = m_rgb32FlatDataBuffer[4*(y*m_width + x) + a_channel];
            channelVal = (float)channelVal * a_quatient;
            //channelVal = (float)channelVal + (float)channelVal * a_quatient;
            m_rgb32FlatDataBuffer[4*(y*m_width + x) + a_channel] = max256(channelVal);
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

    for (uint32_t y = 0; y < m_height; ++y)
        for (uint32_t x = 0; x < m_width; ++x)
        {
            uint8_t red, green, blue;
            _convertRGB2AltColors(m_rgb32FlatDataBuffer[4*(y*m_width + x) + 2], m_rgb32FlatDataBuffer[4*(y*m_width + x) + 1],
                                  m_rgb32FlatDataBuffer[4*(y*m_width + x)], red, green, blue);

            m_rgb32FlatDataBuffer[4*(y*m_width + x)] = blue;
            m_rgb32FlatDataBuffer[4*(y*m_width + x) + 1] = green;
            m_rgb32FlatDataBuffer[4*(y*m_width + x) + 2] = red;
            m_rgb32FlatDataBuffer[4*(y*m_width + x) + 3] = 0xff;
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

void Image::processRGB32BrightnessFilter(uint8_t a_threshold)
{
    if (m_rgb32DataBuffer == nullptr)
        return;

    for (int32_t y = 0; y < m_height; ++y)
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

void Image::processRGBB32FlatBrightnessFilter(uint8_t a_threshold)
{
    if (m_rgb32FlatDataBuffer == nullptr)
        return;

    for (int32_t y = 0; y < m_height; ++y)
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

template<typename T> void Image::getBufferMinMax()
{
    T param;
    float minValueF = 0.0, maxValueF = 0.0;
    double minValueD = 0.0, maxValueD = 0.0;

    size_t size = m_width * m_height * sizeof(T);

    if (m_baseOffset + size <= m_maxDataBufferSize)
    {
        if (sizeof(param) == sizeof(float))
        {
            libnfits::getFloatBufferMinMax(m_dataBuffer, size, minValueF, maxValueF);

            m_minValue = minValueF;
            m_maxValue = maxValueF;
        }

        if (sizeof(param) == sizeof(double))
        {
            libnfits::getDoubleBufferMinMax(m_dataBuffer, size, minValueD, maxValueD)
                    ;
            m_minValue = minValueD;
            m_maxValue = maxValueD;
        }
    }
}

template void Image::getBufferMinMax<float>();
template void Image::getBufferMinMax<double>();

uint32_t Image::getTransformType() const
{
    return  m_transformType;
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
