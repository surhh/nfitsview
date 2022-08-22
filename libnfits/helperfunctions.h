#ifndef LIBNFITS_HELPERFUNCTIONS_H
#define LIBNFITS_HELPERFUNCTIONS_H

#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iomanip>

#include "defs.h"

namespace libnfits
{

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
    return a_value & 0x000000ff;
}

inline bool isEqual(double x, double y)
{
  const double epsilon = 1e-5;

  return std::abs(x - y) <= epsilon * std::abs(x);
}

inline uint8_t char2alphanum(const uint8_t a_char)
{
    return (a_char >= 0x20 && a_char < 0x7F) ? a_char : '.';
}

inline uint8_t convertRGB2Grayscale(uint8_t a_red, uint8_t a_green, uint8_t a_blue)
{
    //// Average method - faster method
    return (a_red +  a_green + a_blue) / 3;

    //// Luminosity method - this second method may differ, choose the one which best fits the needs (not used)
    //// return 0.299*(float)a_red + 0.587*(float)a_green + 0.114*(float)a_blue;

    //// Lightness method - for testing (straightforward code)
    /*
    uint8_t maxChannel = a_red;
    uint8_t minChannel = a_blue;

    if (a_green > maxChannel)
        maxChannel = a_green;

    if (a_blue > maxChannel)
        maxChannel = a_blue;

    if (a_red < minChannel)
        minChannel = a_red;

    if (a_green < minChannel)
        minChannel = a_green;

    return (maxChannel + minChannel) / 2;
    */
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
uint32_t convertFloat2RGBA(float a_value);

uint32_t convertDouble2RGBA(double a_value);

void convertFloat2RGB(float a_value, uint8_t& a_red, uint8_t& a_green, uint8_t& a_blue);

void convertDouble2RGB(double a_value, uint8_t& a_red, uint8_t& a_green, uint8_t& a_blue);

void convertFloat2Grayscale(float a_value, uint8_t& a_red, uint8_t& a_green, uint8_t& a_blue);

void convertDouble2Grayscale(double a_value, uint8_t& a_red, uint8_t& a_green, uint8_t& a_blue);


//// array of pixels conversion functions based on the single pixel conversion functions
void convertBufferFloat2RGBA(uint8_t* a_buffer, size_t a_size);

void convertBufferFloat2RGB(uint8_t* a_buffer, size_t a_size);

void convertBufferDouble2RGBA(uint8_t* a_buffer, size_t a_size);

void convertBufferDouble2RGB(uint8_t* a_buffer, size_t a_size);

void convertBufferShort2RGB(uint8_t* a_buffer, size_t a_size);

void convertBufferByte2RGB(uint8_t* a_buffer, size_t a_size);

void convertBufferInt2RGB(uint8_t* a_buffer, size_t a_size);

void convertBufferLong2RGB(uint8_t* a_buffer, size_t a_size);

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

#endif // LIBNFITS_HELPERFUNCTIONS_H
