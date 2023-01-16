#include "pngfile.h"

#include "defs.h"
#include "helperfunctions.h"

#include <png.h>
#include <cstring>

namespace libnfits
{

PNGFile::PNGFile():
    m_fileName(""), m_width(0), m_height(0), m_colorDepth(0), m_colorType(FITS_PNG_DEFAULT_COLOR_TYPE), m_title("")
{

}

PNGFile::PNGFile(const std::string& a_fileName, uint32_t a_width, uint32_t a_height, uint8_t a_colorDepth, const std::string& a_title):
    m_fileName(a_fileName), m_width(a_width), m_height(a_height), m_colorDepth(a_colorDepth), m_colorType(FITS_PNG_DEFAULT_COLOR_TYPE), m_title(a_title)
{

}

PNGFile::~PNGFile()
{
    m_fileName.clear();
}

void PNGFile::setParameters(const std::string& a_fileName, uint32_t a_width, uint32_t a_height, uint8_t a_colorDepth, uint8_t a_colorType,
                            const std::string& a_title)
{
    m_fileName = a_fileName;
    m_width = a_width;
    m_height = a_height;
    m_colorDepth =  a_colorDepth;
    m_colorType =  a_colorType; // the value FITS_PNG_DEFAULT_COLOR_TYPE is the default one for generic PNG files
    m_title = a_title;
}

int32_t PNGFile::createFromPixelData(const uint8_t* a_pixelBuffer)
{
    FILE*           pFile = nullptr;
    png_structp     pPng = nullptr;
    png_infop       pInfo = nullptr;
    png_bytepp      pngRows = nullptr;
    png_bytep       pngTmpRow = nullptr;

    int32_t retVal = FITS_GENERAL_SUCCESS;

    pFile = fopen(m_fileName.c_str(), "wb");
    if (pFile == nullptr)
       return FITS_PNG_FILE_CREATE_ERROR;

    pPng = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (pPng == nullptr)
    {
        fclose(pFile);

        return FITS_PNG_WRITE_STRUCT_CREATE_ERROR;
    }

    pInfo = png_create_info_struct(pPng);
    if (pInfo == nullptr)
    {
        fclose(pFile);

        if (pPng != nullptr)
            png_destroy_write_struct(&pPng, (png_infopp)nullptr);

        return FITS_PNG_INFO_STRUCT_CREATE_ERROR;
    }

    // libpng module init
    png_init_io(pPng, pFile);

    uint32_t colorType = PNG_COLOR_TYPE_RGB;
    uint8_t byteNumber = FITS_PNG_DEFAUL_PIXEL_BYTES_NUMBER;

    switch (m_colorType)
    {
        case FITS_PNG_COLOR_GRAYSCALE:
            colorType = PNG_COLOR_TYPE_GRAY;
            byteNumber = 1;
            break;
        case FITS_PNG_COLOR_RGB:
            colorType = PNG_COLOR_TYPE_RGB;
            byteNumber = 3;
            break;
        case FITS_PNG_COLOR_PALETTE:
            colorType = PNG_COLOR_TYPE_PALETTE;
            byteNumber = 1;
            break;
        case FITS_PNG_COLOR_GRAYSCALE_ALPHA:
            colorType = PNG_COLOR_TYPE_GRAY_ALPHA;
            byteNumber = 2;
            break;
        case FITS_PNG_COLOR_RGB_ALPHA:
            colorType = PNG_COLOR_TYPE_RGB_ALPHA;
            byteNumber = 4;
            break;
        default:
            break;
    }

    png_set_IHDR(pPng, pInfo, m_width, m_height, m_colorDepth, colorType, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    if (!m_title.empty())
    {
       png_text title_text;
       title_text.compression = PNG_TEXT_COMPRESSION_NONE;
       title_text.key = (char *)FITS_PNG_TITLE_KEY;
       title_text.text = (char *)m_title.c_str();
       png_set_text(pPng, pInfo, &title_text, 1);
    }

    png_write_info(pPng, pInfo);

    // calculating the PNG pixel buffer size. This way is more understandable in terms of logic
    size_t bufRowSize = m_width * byteNumber * (m_colorDepth / (sizeof(png_byte) * 8)) * sizeof(png_byte);

    size_t tmpBufRowSize = m_width * (byteNumber + ((m_colorType == FITS_PNG_COLOR_RGB) ? 1 : 0)) *
                            (m_colorDepth / (sizeof(png_byte) * 8)) * sizeof(png_byte);  // 32-bit element buffer for temp ussage

    pngRows = new png_bytep[m_height];

    pngTmpRow = new png_byte[tmpBufRowSize];

    //// Currently only RGB PNG format is supported
    //// TODO: write support for other PNG formats. Some of them may require endianess checking

    // Writing the buffer containing pixel data
    try
    {
        for (uint32_t i = 0; i < m_height; ++i)
            pngRows[i] = new png_byte[bufRowSize];

        for (uint32_t y = 0; y < m_height ; ++y)
        {
            if (m_colorType == FITS_PNG_COLOR_RGB_ALPHA)
            {
                std::memcpy(pngRows[y], a_pixelBuffer + (y * bufRowSize), bufRowSize);
                convertBufferFloat2RGBA(pngRows[y], bufRowSize);
            }
            else if (m_colorType == FITS_PNG_COLOR_RGB)
            {
                std::memcpy(pngTmpRow, a_pixelBuffer + (y * tmpBufRowSize), tmpBufRowSize);
                convertBufferFloat2RGB(pngTmpRow, tmpBufRowSize);

                for (uint32_t i = 0; i < tmpBufRowSize / 4; ++i)
                {
                    uint32_t baseIndex3 = i*3;
                    uint32_t baseIndex4 = i*4;

                    pngRows[y][baseIndex3]     = pngTmpRow[baseIndex4];
                    pngRows[y][baseIndex3 + 1] = pngTmpRow[baseIndex4 + 1];
                    pngRows[y][baseIndex3 + 2] = pngTmpRow[baseIndex4 + 2];
                }
            }
            else
                throw;
        }

        png_write_image(pPng, (png_bytepp)pngRows);
    }
    catch (...)
    {
        retVal = FITS_PNG_PIXEL_DATA_ERROR;
    }

    // finishing writing the PNG file
    try
    {
        png_write_end(pPng, nullptr);
    }
    catch (...)
    {
        retVal = FITS_PNG_WRITE_END_ERROR;
    }

    // final cleanup
    if (pFile != nullptr)
        fclose(pFile);

    if (pInfo != nullptr)
        png_free_data(pPng, pInfo, PNG_FREE_ALL, -1);

    if (pPng != nullptr)
        png_destroy_write_struct(&pPng, (png_infopp)nullptr);

    if (pngRows != nullptr)
    {
        for (uint32_t i = 0; i < m_height; ++i)
            delete [] pngRows[i];

        delete [] pngRows;
    }

    if (pngTmpRow != nullptr)
        delete [] pngTmpRow;

    return retVal;
}

void PNGFile::setWidth(uint32_t a_width)
{
    m_width = a_width;
}

void PNGFile::setHeight(uint32_t a_height)
{
    m_height = a_height;
}

void PNGFile::setGeometry(uint32_t a_width, uint32_t a_height)
{
    m_width = a_width;
    m_height = a_height;
}

void PNGFile::setColorDepth(uint8_t a_colorDepth)
{
    m_colorDepth = a_colorDepth;
}

void PNGFile::setColorType(uint8_t a_colorType)
{
    m_colorType = a_colorType;
}

void PNGFile::setTitle(const std::string& a_title)
{
    m_title = a_title;
}

uint32_t PNGFile::getWidth() const
{
    return m_width;
}

uint32_t PNGFile::getHeight() const
{
    return m_height;
}

uint8_t PNGFile::getColorDepth() const
{
    return m_colorDepth;
}

uint8_t PNGFile::getColorType() const
{
    return m_colorType;
}

std::string PNGFile::getTitle() const
{
    return m_title;
}

void PNGFile::reset()
{
    m_fileName.clear();
    m_width = 0;
    m_height = 0;
    m_colorDepth = 0;
    m_colorType = FITS_PNG_DEFAULT_COLOR_TYPE;
    m_title.clear();
}

int32_t PNGFile::createFromRGBData(const uint8_t** a_rgbBuffer)
{
    FILE*           pFile = nullptr;
    png_structp     pPng = nullptr;
    png_infop       pInfo = nullptr;
    png_bytepp      pngRows = nullptr;
    png_bytep       pngTmpRow = nullptr;

    int32_t retVal = FITS_GENERAL_SUCCESS;

    if (a_rgbBuffer == nullptr)
        return FITS_PNG_EXPORT_ERROR;

    pFile = fopen(m_fileName.c_str(), "wb");
    if (pFile == nullptr)
       return FITS_PNG_FILE_CREATE_ERROR;

    pPng = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (pPng == nullptr)
    {
        fclose(pFile);

        return FITS_PNG_WRITE_STRUCT_CREATE_ERROR;
    }

    pInfo = png_create_info_struct(pPng);
    if (pInfo == nullptr)
    {
        fclose(pFile);

        if (pPng != nullptr)
            png_destroy_write_struct(&pPng, (png_infopp)nullptr);

        return FITS_PNG_INFO_STRUCT_CREATE_ERROR;
    }

    // libpng module init
    png_init_io(pPng, pFile);

    png_set_IHDR(pPng, pInfo, m_width, m_height, m_colorDepth, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    if (!m_title.empty())
    {
       png_text title_text;
       title_text.compression = PNG_TEXT_COMPRESSION_NONE;
       title_text.key = (char *)FITS_PNG_TITLE_KEY;
       title_text.text = (char *)m_title.c_str();
       png_set_text(pPng, pInfo, &title_text, 1);
    }

    png_write_info(pPng, pInfo);

    // Writing the buffer containing pixel data
    try
    {
        png_write_image(pPng, (png_bytepp)a_rgbBuffer);
    }
    catch (...)
    {
        retVal = FITS_PNG_PIXEL_DATA_ERROR;
    }

    // finishing writing the PNG file
    try
    {
        png_write_end(pPng, nullptr);
    }
    catch (...)
    {
        retVal = FITS_PNG_WRITE_END_ERROR;
    }

    // final cleanup
    if (pFile != nullptr)
        fclose(pFile);

    if (pInfo != nullptr)
        png_free_data(pPng, pInfo, PNG_FREE_ALL, -1);

    if (pPng != nullptr)
        png_destroy_write_struct(&pPng, (png_infopp)nullptr);

    if (pngRows != nullptr)
    {
        for (uint32_t i = 0; i < m_height; ++i)
            delete [] pngRows[i];

        delete [] pngRows;
    }

    if (pngTmpRow != nullptr)
        delete [] pngTmpRow;

    return retVal;
}

}
