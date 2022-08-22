#include "image.h"
#include "pngfile.h"

#include <cstring>
#include "helperfunctions.h"

namespace libnfits
{

Image::Image():
    m_dataBuffer(nullptr),
    m_rgbDataBuffer(nullptr), m_rgb32DataBuffer(nullptr), m_rgb32FlatDataBuffer(nullptr),
    m_rgbDataBackupBuffer(nullptr), m_rgb32DataBackupBuffer(nullptr), m_rgb32FlatDataBackupBuffer(nullptr),
    m_width(0), m_height(0), m_colorDepth(0), m_bitpix(0), m_isCompressed(false), m_title(""), m_callbackFunc(nullptr), m_callbackFuncParam(nullptr)
{

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
    m_callbackFunc = a_callbackFunc;
    m_callbackFuncParam = nullptr;
    m_rgbDataBuffer = nullptr;
    m_rgb32DataBuffer = nullptr;
    m_rgb32FlatDataBuffer = nullptr;
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
        return FITS_GENERAL_ERROR;

    // This block recalculates the new PNG buffer size. The multipliers reflect the number of bytes per pixel.
    // RGB type is the default one
    // RGB = 3, RGBA = 4
    //// TODO: RGBA and grayscale should be supported as well
    //pixelBuffer = m_dataBuffer;

    if (bytesNum == 3 || bytesNum == 4)
        colorType = FITS_PNG_COLOR_RGB;  // the FITS_PNG_COLOR_RGB_ALPHA seems not to work as expected on the FITS pixel array
    else
        return FITS_GENERAL_ERROR;

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
    deleteAllRGBData();
    deleteAllBackupRGBData();

    m_width = 0;
    m_height = 0;
    m_colorDepth = 0;
    m_bitpix = 0;
    m_isCompressed = false;
    m_callbackFunc = nullptr;
    m_title.clear();
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

int32_t Image::createRGBData()
{
    int32_t retVal = FITS_GENERAL_SUCCESS;

    uint8_t bytesNum = std::abs(m_bitpix) / 8;

    if (bytesNum != 3 && bytesNum != 4)   //// TODO: in the future it's needed to support other 8/16/32/64/ bit images
        return FITS_GENERAL_ERROR;

    // the image has been converted to RGB already
    if (m_rgbDataBuffer != nullptr)
        return FITS_GENERAL_ERROR;

    // calculating the PNG pixel buffer size. This way is more understandable in terms of logic
    size_t bufRowSize = m_width * 3 * (m_colorDepth / (sizeof(uint8_t) * 8)) * sizeof(uint8_t);

    size_t tmpBufRowSize = m_width * bytesNum * (m_colorDepth / (sizeof(uint8_t) * 8)) * sizeof(uint8_t);  // 32-bit element buffer for temp ussage

    m_rgbDataBuffer = new uint8_t*[m_height]();

    if (m_rgbDataBuffer == nullptr)
        return FITS_GENERAL_ERROR;

    uint8_t *tmpRow = new uint8_t[tmpBufRowSize];

    // Writing the buffer containing pixel data
    try
    {
        //for (uint32_t y = 0; y < m_height; ++y)
        for (int64_t y = m_height - 1; y >= 0; --y) //// this loop is for correcting Y-axis upside down showing
        {
            m_rgbDataBuffer[y] = new uint8_t[bufRowSize];

            if (bytesNum == 3)
            {
                //std::memcpy(m_rgbDataBuffer[y], m_dataBuffer + (y * bufRowSize), bufRowSize);
                //// this memcpy is for correcting Y-axis upside down showing
                std::memcpy(m_rgbDataBuffer[m_height - 1 - y], m_dataBuffer + ((m_height - 1 - y) * bufRowSize), bufRowSize);

                //convertBufferFloat2RGB(m_rgbDataBuffer[y], bufRowSize);  // commented, is changed to the code below, the former code was wrong

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
            else if (bytesNum == 4)
            {
                std::memcpy(tmpRow, m_dataBuffer + (y * tmpBufRowSize), tmpBufRowSize);
                convertBufferFloat2RGB(tmpRow, tmpBufRowSize);

                for (uint32_t x = 0; x < m_width; ++x)
                {
                    m_rgbDataBuffer[y][x*3]     = tmpRow[x*4];
                    m_rgbDataBuffer[y][x*3 + 1] = tmpRow[x*4 + 1];
                    m_rgbDataBuffer[y][x*3 + 2] = tmpRow[x*4 + 2];
                }
            }
        }
    }
    catch (...)
    {
        retVal = FITS_PNG_PIXEL_DATA_ERROR;
    }

    if (tmpRow != nullptr)
        delete [] tmpRow;

    return retVal;
}

int32_t Image::createRGB32Data()
{
    int32_t retVal = FITS_GENERAL_SUCCESS;

    uint8_t bytesNum = std::abs(m_bitpix) / 8;

    if (bytesNum != 3 && bytesNum != 4)   //// TODO: in the future it's needed to support other 8/16/32/64/ bit images
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

    if (m_rgb32DataBuffer == nullptr)
        return FITS_GENERAL_ERROR;

    uint8_t *tmpRow = new uint8_t[tmpBufRowSize];

    // Writing the buffer containing pixel data
    try
    {
        //for (uint32_t y = 0; y < m_height; ++y)
        for (int64_t y = m_height - 1; y >= 0; --y) //// this loop is for correcting Y-axis upside down showing
        {
            m_rgb32DataBuffer[y] = new uint8_t[bufRowSize];

            //std::memcpy(tmpRow, m_dataBuffer + (y * tmpBufRowSize), tmpBufRowSize);
            //// this memcpy is for correcting Y-axis upside down showing
            std::memcpy(tmpRow, m_dataBuffer + ((m_height - 1 - y) * tmpBufRowSize), tmpBufRowSize);


            if (bytesNum == 4)
                convertBufferFloat2RGB(tmpRow, tmpBufRowSize);
            else if (bytesNum = 8)
                convertBufferDouble2RGB(tmpRow, tmpBufRowSize);

            if (bytesNum == 4 || bytesNum == 3)
                for (uint32_t x = 0; x < m_width; ++x)
                {
                    m_rgb32DataBuffer[y][x*4]     = tmpRow[x*4 + 2];
                    m_rgb32DataBuffer[y][x*4 + 1] = tmpRow[x*4 + 1];
                    m_rgb32DataBuffer[y][x*4 + 2] = tmpRow[x*4];
                    m_rgb32DataBuffer[y][x*4 + 3] = 0;
                }
        }
    }
    catch (...)
    {
        retVal = FITS_PNG_PIXEL_DATA_ERROR;
    }

    if (tmpRow != nullptr)
        delete [] tmpRow;

    return retVal;
}

int32_t Image::createRGB32FlatData()
{
    int32_t retVal = FITS_GENERAL_SUCCESS;

    uint8_t bytesNum = std::abs(m_bitpix) / 8;

    if (bytesNum != 3 && bytesNum != 4)   //// TODO: in the future it's needed to support other 8/16/32/64/ bit images
        return FITS_GENERAL_ERROR;

    // the image has been converted to RGB already
    if (m_rgb32FlatDataBuffer != nullptr)
        return FITS_GENERAL_ERROR;

    // calculating the PNG pixel buffer size. This way is more understandable in terms of logic
    // thow we need to have only RGB data, we actually need to have it 32-bit aligned for some future use cases,
    // that's why it's multipled by 4 instead of 3 (kind of tricky stuff, but works fine)

    size_t tmpBufRowSize = m_width * bytesNum * (m_colorDepth / (sizeof(uint8_t) * 8)) * sizeof(uint8_t);  // 32-bit element buffer for temp ussage

    size_t flatBufSize = m_height*m_width*4;

    m_rgb32FlatDataBuffer = new uint8_t[flatBufSize];

    if (m_rgb32FlatDataBuffer == nullptr)
        return FITS_GENERAL_ERROR;

    uint8_t *tmpRow = new uint8_t[tmpBufRowSize];

    // Writing the buffer containing pixel data
    try
    {
        //for (uint32_t y = 0; y < m_height; ++y)
        for (int64_t y = m_height - 1; y >= 0; --y) //// this loop is for correcting Y-axis upside down showing
        {
            //std::memcpy(tmpRow, m_dataBuffer + (y * tmpBufRowSize), tmpBufRowSize);
            //// this memcpy is for correcting Y-axis upside down showing
            std::memcpy(tmpRow, m_dataBuffer + ((m_height - 1 - y) * tmpBufRowSize), tmpBufRowSize);

            if (bytesNum == 4)
                convertBufferFloat2RGB(tmpRow, tmpBufRowSize);
            else if (bytesNum = 8)
                convertBufferDouble2RGB(tmpRow, tmpBufRowSize);

            if (bytesNum == 4 || bytesNum == 3)
                for (uint32_t x = 0; x < m_width; ++x)
                {
                    m_rgb32FlatDataBuffer[4*(y*m_width + x)]     = tmpRow[x*4 + 2];
                    m_rgb32FlatDataBuffer[4*(y*m_width + x) + 1] = tmpRow[x*4 + 1];
                    m_rgb32FlatDataBuffer[4*(y*m_width + x) + 2] = tmpRow[x*4];
                    m_rgb32FlatDataBuffer[4*(y*m_width + x) + 3] = 0;
                }
        }
    }
    catch (...)
    {
        retVal = FITS_PNG_PIXEL_DATA_ERROR;
    }

    if (tmpRow != nullptr)
        delete [] tmpRow;

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

int32_t Image::changeRGBColorChannelLevel(uint8_t a_channel, float a_quatient)
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

int32_t Image::changeRGB32ColorChannelLevel(uint8_t a_channel, float a_quatient)
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

int32_t Image::changeRGB32FlatColorChannelLevel(uint8_t a_channel, float a_quatient)
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
            uint8_t channelVal = m_rgb32FlatDataBuffer[4*(y*m_width + x) + a_channel];
            channelVal = (float)channelVal * a_quatient;
            m_rgb32FlatDataBuffer[4*(y*m_width + x) + a_channel] = channelVal;
        }

    return retVal;
}

int32_t Image::changeRLevel(float a_quatient)
{
    return changeRGBColorChannelLevel(0, a_quatient);
}

int32_t Image::changeGLevel(float a_quatient)
{
    return changeRGBColorChannelLevel(1, a_quatient);
}

int32_t Image::changeBLevel(float a_quatient)
{
    return changeRGBColorChannelLevel(2, a_quatient);
}

int32_t Image::change32RLevel(float a_quatient)
{
    return changeRGB32ColorChannelLevel(0, a_quatient);
}

int32_t Image::change32GLevel(float a_quatient)
{
    return changeRGB32ColorChannelLevel(1, a_quatient);
}

int32_t Image::change32BLevel(float a_quatient)
{
    return changeRGB32ColorChannelLevel(2, a_quatient);
}

int32_t Image::change32FlatRLevel(float a_quatient)
{
    return changeRGB32FlatColorChannelLevel(0, a_quatient);
}

int32_t Image::change32FlatGLevel(float a_quatient)
{
    return changeRGB32FlatColorChannelLevel(1, a_quatient);
}

int32_t Image::change32FlatBLevel(float a_quatient)
{
    return changeRGB32FlatColorChannelLevel(2, a_quatient);
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

//// this function is for debug purposes only, is slow
int32_t Image::dumpFloatDataBuffer(const std::string &a_filename, uint32_t a_rowSize)
{
    return libnfits::dumpFloatDataBuffer(m_dataBuffer, m_width * m_height * sizeof(float), a_filename, a_rowSize);
}

}
