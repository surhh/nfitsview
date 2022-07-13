#ifndef LIBNFITS_PNGFILE_H
#define LIBNFITS_PNGFILE_H

#include <cstdint>
#include <string>

#include "defs.h"

namespace libnfits
{

class PNGFile
{
private:
    uint32_t    m_width;
    uint32_t    m_height;
    uint8_t     m_colorDepth;
    uint8_t     m_colorType;
    std::string m_fileName;
    std::string m_title;

public:
    PNGFile();
    PNGFile(const std::string& a_fileName, uint32_t a_width, uint32_t a_height, uint8_t a_colorDepth, const std::string& a_title = FITS_PNG_TITLE);
    ~PNGFile();

    void setParameters(const std::string& a_fileName, uint32_t a_width, uint32_t a_height, uint8_t a_colorDepth = FITS_PNG_DEFAULT_PIXEL_DEPTH,
                       uint8_t a_colorType = FITS_PNG_DEFAULT_COLOR_TYPE, const std::string& a_title = FITS_PNG_TITLE);
    void setWidth(uint32_t a_width);
    void setHeight(uint32_t a_height);
    void setGeometry(uint32_t a_width, uint32_t a_height);
    void setColorDepth(uint8_t a_colorDepth = FITS_PNG_DEFAULT_PIXEL_DEPTH);
    void setColorType(uint8_t a_colorType = FITS_PNG_DEFAUL_PIXEL_BYTES_NUMBER);
    void setTitle(const std::string& a_title = FITS_PNG_TITLE);
    uint32_t getWidth() const;
    uint32_t getHeight() const;
    uint8_t getColorDepth() const;
    uint8_t getColorType() const;
    std::string getTitle() const;
    int32_t createFromPixelData(const uint8_t* a_pixelBuffer);
    int32_t createFromRGBData(const uint8_t** a_rgbBuffer);
    void reset();
};

}
#endif // LIBNFITS_PNGFILE_H
