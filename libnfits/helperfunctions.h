#ifndef LIBNFITS_HELPERFUNCTIONS_H
#define LIBNFITS_HELPERFUNCTIONS_H

#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <any>
#include <ctime>
#include <cmath>

#include "defs.h"


namespace libnfits
{

struct RGBPixel
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

template<typename ... many> void LOG(const std::string& a_str, many ... a_args)
{
#if defined(DEBUG_MODE)
    std::time_t time_now = std::time(nullptr);

    std::vector<std::any> argList = { a_args ... };

    std::cout << std::put_time(std::localtime(&time_now), "%Y-%m-%d %OH:%OM:%OS") << " - [LOG]: ";

    std::string outputStr = "";

    int size = a_str.length();

    for (int i = 0; i < size; ++i)
    {
        if (a_str[i] == '%')
        {
            if (i + 1 < size && a_str[i + 1] == '%')
                ++i;
            else
            {
                if (argList.empty())
                    throw std::logic_error("ERROR: Not enough parameters!");

                if (argList[0].type() == typeid(char *))
                    outputStr += std::any_cast<char *>(argList[0]);

                if (argList[0].type() == typeid(const char *))
                    outputStr += std::any_cast<const char *>(argList[0]);

                if (argList[0].type() == typeid(std::string))
                    outputStr += std::any_cast<std::string>(argList[0]);

                if (argList[0].type() == typeid(int))
                    outputStr += std::to_string(std::any_cast<int>(argList[0]));

                if (argList[0].type() == typeid(long))
                    outputStr += std::to_string(std::any_cast<long>(argList[0]));

                if (argList[0].type() == typeid(int8_t))
                    outputStr += std::to_string(std::any_cast<int8_t>(argList[0]));

                if (argList[0].type() == typeid(int16_t))
                    outputStr += std::to_string(std::any_cast<int16_t>(argList[0]));

                if (argList[0].type() == typeid(int32_t))
                    outputStr += std::to_string(std::any_cast<int32_t>(argList[0]));

                if (argList[0].type() == typeid(int64_t))
                    outputStr += std::to_string(std::any_cast<int64_t>(argList[0]));

                if (argList[0].type() == typeid(uint8_t))
                    outputStr += std::to_string(std::any_cast<uint8_t>(argList[0]));

                if (argList[0].type() == typeid(uint16_t))
                    outputStr += std::to_string(std::any_cast<uint16_t>(argList[0]));

                if (argList[0].type() == typeid(uint32_t))
                    outputStr += std::to_string(std::any_cast<uint32_t>(argList[0]));

                if (argList[0].type() == typeid(uint64_t))
                    outputStr += std::to_string(std::any_cast<uint64_t>(argList[0]));

                if (argList[0].type() == typeid(size_t))
                    outputStr += std::to_string(std::any_cast<size_t>(argList[0]));

                if (argList[0].type() == typeid(float))
                    outputStr += std::to_string(std::any_cast<float>(argList[0]));

                if (argList[0].type() == typeid(double))
                    outputStr += std::to_string(std::any_cast<double>(argList[0]));

                if (argList[0].type() == typeid(bool))
                {
                    std::string flag = "FALSE";
                    bool b = std::any_cast<bool>(argList[0]);

                    if (b)
                        flag = "TRUE";

                    outputStr += flag;
                }

                argList.erase(argList.begin());

                ++i;
            }
        }

        outputStr += a_str[i];
    }

    std::cout << outputStr << std::endl;
#endif
}


#if defined(__unix__)
    #define swap16(x) (__builtin_bswap16(x))
    #define swap32(x) (__builtin_bswap32(x))
    #define swap64(x) (__builtin_bswap64(x))
#elif defined(__WIN64__) || defined(__WIN32__)
    #define swap16(x) (_byteswap_ushort(x))
    #define swap32(x) (_byteswap_ulong(x))
    #define swap64(x) (_byteswap_uint64(x))
#else
    inline uint16_t swap16(uint16_t x)
    {
        return ((x << 8) & 0xFF00) | ((x >> 8) & 0x00FF);
    }

    inline uint32_t swap32(uint32_t x)
    {
        uint32_t t = ((x << 8) & 0xFF00FF00) | ((x >> 8) & 0xFF00FF);

        return (t << 16) | (t >> 16);
    }

    inline uint64_t swap64(uint64_t x)
    {
        uint64_t t = x;

        t = ((t & 0x00000000FFFFFFFFull) << 32) | ((t & 0xFFFFFFFF00000000ull) >> 32);
        t = ((t & 0x0000FFFF0000FFFFull) << 16) | ((t & 0xFFFF0000FFFF0000ull) >> 16);
        t = ((t & 0x00FF00FF00FF00FFull) << 8)  | ((t & 0xFF00FF00FF00FF00ull) >> 8);

        return t;
    }
#endif

inline float swap32f(float f)
{
    int32_t val = *((int32_t*) &f);

    return swap32(val);
}

inline uint8_t max256(uint32_t a_value)
{
    return a_value > 0xff ? 0xff : a_value;
}

inline uint8_t max3Index(uint64_t a_value1, uint64_t a_value2, uint64_t a_value3)
{
    if (a_value1 >= a_value2 && a_value1 >= a_value2)
        return 1;
    else if (a_value2 >= a_value1 && a_value2 >= a_value3)
        return 2;
    else if (a_value3 >= a_value1 && a_value3 >= a_value2)
        return 3;

    return 1;
}

inline bool greater3(uint8_t a_value1, uint8_t a_value2, uint8_t a_value3, uint8_t a_threshold)
{
    return (a_threshold > a_value1 && a_threshold > a_value2 && a_threshold > a_value3) ? false : true;
}

inline bool areEqual(double a_x, double a_y)
{
    return std::fabs(a_x - a_y) < std::numeric_limits<double>::epsilon();
}

inline uint8_t char2alphanum(const uint8_t a_char)
{
    return (a_char >= 0x20 && a_char < 0x7F) ? a_char : '.';
}

inline uint8_t convertRGB2Grayscale(uint8_t a_red, uint8_t a_green, uint8_t a_blue)
{
    //// Average method - faster method
    return (a_red + a_green + a_blue) / 3;

    //// Luminosity method - this second method may differ, choose the one which best fits the needs (not used)
    //// return 0.299*(float)a_red + 0.587*(float)a_green + 0.114*(float)a_blue;

    //// Lightness method - for testing (straightforward code)
}

inline uint8_t convertRGB2Grayscale(RGBPixel& a_pixel)
{
    //// Average method - faster method
    return (a_pixel.red + a_pixel.green + a_pixel.blue) / 3;

    //// Luminosity method - this second method may differ, choose the one which best fits the needs (not used)
    //// return 0.299*(float)a_pixel.red + 0.587*(float)a_pixel.green + 0.114*(float)a_pixel.blue;
}

template<typename T> T convertStringMulti(const std::string& a_strData, bool& a_successFlag)
{
    T retVal;

    a_successFlag = false;

    std::istringstream strStream(a_strData);

    if (strStream.fail())
    {
        strStream.clear();

        return retVal;
    }

    strStream >> retVal;

    if (strStream.fail())
    {
        strStream.clear();

        return retVal;
    }

    a_successFlag = true;

    return retVal;
}

template<typename T> std::string int2hex(T a_value)
{
  std::stringstream strStream;

  strStream << std::setfill('0') << std::setw(sizeof(T) * 2) << std::hex << a_value;

  return strStream.str();
}


//// color normalization functions
template<typename T> T normalizeValue(T a_value, T a_min, T a_max, T a_minNew, T a_maxNew)
{
    return (a_value - a_min)*((a_maxNew - a_minNew)/(a_max - a_min)) + a_minNew;
}

void normalizeFloatBuffer(uint8_t* a_buffer, size_t a_size, float a_min, float a_max, float a_minNew, float a_maxNew);

void normalizeDoubleBuffer(uint8_t* a_buffer, size_t a_size, double a_min, double a_max, double a_minNew, double a_maxNew);

//// string manipulatoin functions
std::vector<std::string> splitString(const std::string& a_strData, const std::string& a_strDelim);

std::string removeSymbols(const std::string& a_strData, const int8_t a_sym);

int32_t countSymbol(const std::string& a_strData, const int8_t a_sym);

bool containsSymbol(const std::string& a_strData, const int8_t a_sym);

std::string getKeywordFromRecord(const std::string& a_strData);

std::string getAfterKeywordFromRecord(const std::string& a_strData);

void replaceSubstring(std::string& a_strData, const std::string& a_strOld, const std::string& a_strNew);

void trimStringLeft(std::string& a_strData, const int8_t a_sym);

void trimStringRight(std::string& a_strData, const int8_t a_sym);

void trimStringLeftRight(std::string& a_strData, const int8_t a_sym);

bool recordValueSyntaxCheck(const std::string& a_strData);

bool recordKeywordSyntaxCheck(const std::string& a_strData);

bool recordSyntaxCheck(const std::string& a_strData);

int32_t findFirstCharOutOfQuotes(const std::string& a_strData, const int8_t a_sym);

bool isValueContinued(const std::string& a_strData);

std::string getStringFromBuffer(const uint8_t* a_buffer, size_t a_offset, size_t a_length = FITS_HEADER_RECORD_SIZE);



//// compression-decompression functions declaration block
std::vector<int8_t> compressData(const std::vector<int8_t>& a_inputVector, const std::string& a_strMethod = FITS_COMPRESSIION_GZIP);

std::vector<int8_t> decompressData(const std::vector<int8_t>& a_inputVector, const std::string& a_strMethod = FITS_COMPRESSIION_GZIP);

std::vector<int8_t> compressDecompressData(const std::vector<int8_t>& a_inputVector, const std::string& a_strMethod = FITS_COMPRESSIION_GZIP,
                                           bool a_compressFlag = true);


std::string compressData(const std::string& a_strData, const std::string& a_strMethod = FITS_COMPRESSIION_GZIP);

std::string decompressData(const std::string& a_strData, const std::string& a_strMethod = FITS_COMPRESSIION_GZIP);

std::string compressDecompressData(const std::string& a_strData, const std::string& a_strMethod = FITS_COMPRESSIION_GZIP,
                                   bool a_compressFlag = true);


size_t compressData(const int8_t* a_inputBuffer, int8_t*& a_outputBuffer, size_t a_bufferSize, const std::string& a_strMethod = FITS_COMPRESSIION_GZIP);

size_t decompressData(const int8_t* a_inputBuffer, int8_t*& a_outputBuffer, size_t a_bufferSize, const std::string& a_strMethod = FITS_COMPRESSIION_GZIP);

size_t compressDecompressData(const int8_t* a_inputBuffer, int8_t*& a_outputBuffer, size_t a_bufferSize,
                              const std::string& a_strMethod = FITS_COMPRESSIION_GZIP, bool a_compressFlag = true);
//// end of compression-decompression functions declaration block


bool isLittleEndian();

size_t alignOffsetForward(size_t a_offset);

size_t alignOffsetBackward(size_t a_offset);

std::string formatNumberString(uint32_t a_number, uint8_t a_padding);

std::string formatFloatString(float a_number, uint8_t a_padding);

//// endiannes swap functions for buffers
void swapBuffer16(uint8_t* a_buffer, size_t a_size);

void swapBuffer32(uint8_t* a_buffer, size_t a_size);

void swapBuffer64(uint8_t* a_buffer, size_t a_size);

//// single pixel conversion functions
inline uint32_t convertFloat2RGBA(float a_value)
{
    const int32_t maxVal = 0x2ff; // default is 0x2ff
    const int32_t minVal = 0;     // default is 0

    int32_t val = (int32_t)(a_value * maxVal);

    if (val < minVal)
        val = minVal;

    if (val > maxVal)
        val = maxVal;

    int32_t offset = val % 0x100;

    uint8_t tmpBuf[3];

    if (val < 0x100)
    {
        tmpBuf[0] = 0;
        tmpBuf[1] = 0;
        tmpBuf[2] = offset;
    }
    else if (val < 0x200)
    {
        tmpBuf[0] = 0;
        tmpBuf[1] = offset;
        tmpBuf[2] = 0xff - offset;
    }
    else
    {
        tmpBuf[0] = offset;
        tmpBuf[1] = 0xff - offset;
        tmpBuf[2] = 0;
    }

    uint32_t retVal = 0, tmpVal = 0;

    tmpVal = tmpBuf[0]; //// 0
    tmpVal = tmpVal << 24;
    retVal = retVal | tmpVal;

    tmpVal = tmpBuf[1]; //// 1
    tmpVal = tmpVal << 16;
    retVal = retVal | tmpVal;

    tmpVal = tmpBuf[2]; //// 2
    tmpVal = tmpVal << 8;
    retVal = retVal | tmpVal;

    retVal = retVal | 0xff; //// the value for Alpha-channel, may be also 0xff

    return retVal;
}

inline uint32_t convertFloatRGBA(float a_value, float a_min, float a_max)
{
    uint32_t retVal = 0;

    if (areEqual(a_min, a_max))
        return retVal;

    float range = std::abs(a_max - a_min);

    float value = -1.0 + (2.0 * (a_value / range));

    return convertFloat2RGBA(value);
}

inline uint32_t convertDouble2RGBA(double a_value)
{
    return convertFloat2RGBA((float)a_value);
}

inline void convertFloat2RGB(float a_value, uint8_t& a_red, uint8_t& a_green, uint8_t& a_blue)
{
    uint32_t val = convertFloat2RGBA(a_value);

    a_red = (val & 0xff000000) >> 24;
    a_green = (val & 0x00ff0000) >> 16;
    a_blue = (val & 0x0000ff00) >> 8;
}

inline void convertFloat2RGB(float a_value, RGBPixel& a_pixel)
{
    uint32_t val = convertFloat2RGBA(a_value);

    a_pixel.red = (val & 0xff000000) >> 24;
    a_pixel.green = (val & 0x00ff0000) >> 16;
    a_pixel.blue = (val & 0x0000ff00) >> 8;
}

inline void convertDouble2RGB(double a_value, uint8_t& a_red, uint8_t& a_green, uint8_t& a_blue)
{
    convertFloat2RGB((float)a_value, a_red, a_green, a_blue);
}

inline void convertDouble2RGB(double a_value, RGBPixel& a_pixel)
{
    convertFloat2RGB((float)a_value, a_pixel);
}

inline void convertFloat2Grayscale(float a_value, uint8_t& a_red, uint8_t& a_green, uint8_t& a_blue)
{
    uint8_t red, green, blue;

    convertFloat2RGB(a_value, red, green, blue);

    uint8_t channelValue = convertRGB2Grayscale(red, green, blue);

    a_red = channelValue;
    a_green = channelValue;
    a_blue = channelValue;
}

inline void convertFloat2Grayscale(float a_value, RGBPixel& a_pixel)
{
    convertFloat2RGB(a_value, a_pixel);

    uint8_t channelValue = convertRGB2Grayscale(a_pixel);

    a_pixel.red = channelValue;
    a_pixel.green = channelValue;
    a_pixel.blue = channelValue;
}

inline void convertDouble2Grayscale(double a_value, uint8_t& a_red, uint8_t& a_green, uint8_t& a_blue)
{
    convertFloat2Grayscale((float)a_value, a_red, a_green, a_blue);
}

inline void convertInt2RGB(uint32_t a_value, uint8_t& a_red, uint8_t& a_green, uint8_t& a_blue)
{
    a_red = (a_value & 0xff000000) >> 24;
    a_green = (a_value & 0x00ff0000) >> 16;
    a_blue = (a_value & 0x0000ff00) >> 8;
}

inline void convertLong2RGB(uint32_t a_value, uint8_t& a_red, uint8_t& a_green, uint8_t& a_blue)
{
    a_red = (a_value & 0xff00000000000000) >> 56;
    a_green = (a_value & 0x00ff000000000000) >> 48;
    a_blue = (a_value & 0x0000ff0000000000) >> 40;
}

void convertShort2RGB(uint16_t a_value, uint8_t& a_red, uint8_t& a_green, uint8_t& a_blue);

void convertShort2RGB(uint16_t a_value, RGBPixel& a_pixel);

void convertShortSZ2RGB(uint16_t a_value, double a_bscale, double a_bzero, uint8_t& a_red, uint8_t& a_green, uint8_t& a_blue);

void convertShortSZ2RGB(uint16_t a_value, double a_bscale, double a_bzero, RGBPixel& a_pixel);

void convertShort2Grayscale(uint16_t a_value, uint8_t& a_red, uint8_t& a_green, uint8_t& a_blue);

void convertShort2Grayscale(uint16_t a_value, RGBPixel& a_pixel);

void convertShortSZ2Grayscale(uint16_t a_value, double a_bscale, double a_bzero, uint8_t& a_red, uint8_t& a_green, uint8_t& a_blue);

void convertShortSZ2Grayscale(uint16_t a_value, double a_bscale, double a_bzero, RGBPixel& a_pixel);


//// array of pixels (buffer) conversion functions based on the single pixel conversion functions
void convertBufferFloat2RGBA(uint8_t* a_buffer, size_t a_size);

void convertBufferFloat2RGB(uint8_t* a_buffer, size_t a_size);

void convertBufferDouble2RGBA(uint8_t* a_buffer, size_t a_size);

void convertBufferDouble2RGB(uint8_t* a_buffer, size_t a_size);

void convertBufferShort2RGB(uint8_t* a_buffer, size_t a_size, uint8_t* a_destBuffer, bool a_gray = true);

void convertBufferShortSZ2RGB(uint8_t* a_buffer, size_t a_size, double a_bzero, double a_bscale, uint8_t* a_destBuffer, bool a_gray = true);

void convertBufferByte2RGB(uint8_t* a_buffer, size_t a_size);

void convertBufferInt2RGB(uint8_t* a_buffer, size_t a_size);

void convertBufferLong2RGB(uint8_t* a_buffer, size_t a_size);


//// functions to convert buffers to grayscale
void convertBufferRGB2Grayscale(uint8_t* a_buffer, size_t a_size);

void convertBufferRGB2Grayscale(uint8_t** a_buffer, uint32_t a_width, uint32_t a_height);

void convertBufferRGB322Grayscale(uint8_t** a_buffer, uint32_t a_width, uint32_t a_height);

void convertBufferRGB32Flat2Grayscale(uint8_t* a_buffer, uint32_t a_width, uint32_t a_height);


//// hex manipulation functions
std::string char2hex(const uint8_t a_char);

void char2hex(const uint8_t a_char, uint8_t& a_hexCharHigh, uint8_t& a_hexCharLow);

uint8_t char2alphanum(const uint8_t a_char);

int32_t convertBuffer2HexString(uint8_t* a_buffer, uint8_t* a_output, size_t size, uint32_t a_align);

std::string convertBuffer2HexString(uint8_t* a_buffer, size_t size, uint32_t a_align);


//// dumpFloatDataBuffer() is slow and should be used for debug purposes only
int32_t dumpFloatDataBuffer(const uint8_t* a_buffer, size_t a_size, const std::string& a_filename, uint32_t a_rowSize);
}

typedef     void (*convertBufferShort)       (uint16_t, libnfits::RGBPixel&);
typedef     void (*convertBufferShortSZ)     (uint16_t, double, double, libnfits::RGBPixel&);

#endif // LIBNFITS_HELPERFUNCTIONS_H
