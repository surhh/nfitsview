#include "helperfunctions.h"

#include <sstream>
#include <fstream>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#define HEX_DELIM_SYMBOL        ' '

namespace libnfits
{

std::vector<std::string> splitString(const std::string& a_strData, const std::string& a_strDelim)
{
    std::vector<std::string> tokens;

    size_t start, end = 0;

    while ((start = a_strData.find_first_not_of(a_strDelim, end)) != std::string::npos)
    {
        end = a_strData.find(a_strDelim, start);
        tokens.push_back(a_strData.substr(start, end - start));
    }

    return tokens;
}

std::string removeSymbols(const std::string& a_strData, const int8_t a_sym)
{
    std::string retStr(a_strData);

    retStr.erase(std::remove(retStr.begin(), retStr.end(), a_sym), retStr.end());

    return retStr;
}

int32_t countSymbol(const std::string& a_strData, const int8_t a_sym)
{
    return std::count(a_strData.begin(), a_strData.end(), a_sym);
}

bool containsSymbol(const std::string& a_strData, const int8_t a_sym)
{
    // for C++23 (is so far in draft version) ,  contains() can be used
#if __cplusplus > 202002L  // > C++20
    return a_strData.conains(std::string(1, a_sym));
#else
    if (a_strData.find(a_sym) != std::string::npos)
        return true;
    else
        return false;
#endif
}

std::string getKeywordFromRecord(const std::string& a_strData)
{
    return a_strData.substr(0, FITS_KEYWORD_END_POS);
}

std::string getAfterKeywordFromRecord(const std::string& a_strData)
{
    return a_strData.substr(FITS_KEYWORD_END_POS + 1, a_strData.length());
}

void replaceSubstring(std::string& a_strData, const std::string& a_strOld, const std::string& a_strNew)
{
    if (a_strData.find(a_strOld) != std::string::npos)
        a_strData.replace(a_strData.find(a_strOld), a_strOld.length(), a_strNew);
}

void trimStringLeft(std::string& a_strData, const int8_t a_sym)
{
    a_strData.erase(0, a_strData.find_first_not_of(a_sym));
}

void trimStringRight(std::string& a_strData, const int8_t a_sym)
{
    a_strData.erase(a_strData.find_last_not_of(a_sym) + 1);
}

void trimStringLeftRight(std::string& a_strData, const int8_t a_sym)
{
    trimStringLeft(a_strData, a_sym);
    trimStringRight(a_strData, a_sym);
}

bool recordValueSyntaxCheck(const std::string& a_strData)
{
    bool retVal = true;

    bool bQuote = false;

    for (uint32_t i = 0; i < a_strData.length(); ++i)
    {
        int8_t c = static_cast<int8_t>(a_strData[i]);

        if ((c < FITS_MIN_RECORD_ASCII_CHAR) || (c > FITS_MAX_RECORD_ASCII_CHAR))
            return false;

        if (c != FITS_QUOTE_CHAR)
            bQuote = !bQuote;
    }

    retVal = !bQuote;

    return retVal;
}

bool recordKeywordSyntaxCheck(const std::string& a_strData)
{
    for (uint32_t i = 0; i < a_strData.length(); ++i)
    {
        int8_t c = static_cast<int8_t>(a_strData[i]);

        if (((c < FITS_MIN_KEYWORD_ASCII_CHAR1) || (c > FITS_MAX_KEYWORD_ASCII_CHAR1)) &&
           ((c != FITS_KEYWORD_ASCII_CHAR3) && (c != FITS_KEYWORD_ASCII_CHAR4)))
            return false;

        if (((c < FITS_MIN_KEYWORD_ASCII_CHAR2) || (c > FITS_MAX_KEYWORD_ASCII_CHAR2)) &&
           ((c != FITS_KEYWORD_ASCII_CHAR3) && (c != FITS_KEYWORD_ASCII_CHAR4)))
            return false;
    }

    return true;
}

bool recordSyntaxCheck(const std::string& a_strData)
{
    // This checking is wrong. False positive on the "/ someone's comment" case
    //uint32_t quoteNum = countSymbol(a_strData, FITS_QUOTE_CHAR);
    //if (quoteNum % 2 != 0)
    //    return false;

    for (uint32_t i = 0; i < a_strData.length(); ++i)
    {
        int8_t c = static_cast<int8_t>(a_strData[i]);

        if ((c < FITS_MIN_RECORD_ASCII_CHAR) || (c > FITS_MAX_RECORD_ASCII_CHAR))
            return false;
    }

    return true;
}

int32_t findFirstCharOutOfQuotes(const std::string& a_strData, const int8_t a_sym)
{
    size_t retPos = -1;

    bool bQuote = false;

    for (uint32_t i = 0; i < a_strData.length(); ++i)
    {
        int8_t c = static_cast<int8_t>(a_strData[i]);

        if (c == FITS_QUOTE_CHAR)
            bQuote = !bQuote;

        if (c == a_sym && !bQuote)
            return i;
    }

    return retPos;
}

bool isValueContinued(const std::string& a_strData)
{
    return a_strData[a_strData.length() - 1] == FITS_VALUE_CONTINUE_CHAR ? true : false;
}

std::string getStringFromBuffer(const uint8_t* a_buffer, size_t a_offset, size_t a_length)
{
    std::string retStr = "";

    retStr.assign((const char*)a_buffer + a_offset, a_length);

    return retStr;
}

std::vector<int8_t> compressData(const std::vector<int8_t>& a_inputVector, const std::string& a_strMethod)
{
    return compressDecompressData(a_inputVector, a_strMethod);
}

std::vector<int8_t> decompressData(const std::vector<int8_t>& a_inputVector, const std::string& a_strMethod)
{
    return compressDecompressData(a_inputVector, a_strMethod, false);
}

std::vector<int8_t> compressDecompressData(const std::vector<int8_t>& a_inputVector, const std::string& a_strMethod, bool a_compressFlag)
{
    std::vector<int8_t> outputVector;

    if (a_inputVector.empty())
        return outputVector;

    boost::iostreams::filtering_streambuf<boost::iostreams::input> inBuf;

    if (a_compressFlag)
    {
        if (a_strMethod == FITS_COMPRESSIION_GZIP)
            inBuf.push(boost::iostreams::gzip_compressor(boost::iostreams::gzip::best_compression));
        else if (a_strMethod == FITS_COMPRESSIION_ZLIB)
            inBuf.push(boost::iostreams::zlib_compressor(boost::iostreams::zlib::best_compression));
    }
    else
    {
        if (a_strMethod == FITS_COMPRESSIION_GZIP)
            inBuf.push(boost::iostreams::gzip_decompressor());
        else if (a_strMethod == FITS_COMPRESSIION_ZLIB)
            inBuf.push(boost::iostreams::zlib_decompressor());
    }

    inBuf.push(boost::iostreams::array_source((char *)a_inputVector.data(), a_inputVector.size()));

    for (size_t i = 0; inBuf.sgetc() != -1; ++i)
        outputVector.push_back((int8_t)inBuf.sbumpc());

    boost::iostreams::close(inBuf);

    return outputVector;
}

std::string compressData(const std::string& a_strData, const std::string& a_strMethod)
{
    return compressDecompressData(a_strData, a_strMethod);
}

std::string decompressData(const std::string& a_strData, const std::string& a_strMethod)
{
    return compressDecompressData(a_strData, a_strMethod, false);
}

std::string compressDecompressData(const std::string& a_strData, const std::string& a_strMethod, bool a_compressFlag)
{
    std::stringstream compressedStream;
    std::stringstream decompressedStream;

    if (a_strData.empty())
        return std::string("");

    compressedStream << a_strData;

    boost::iostreams::filtering_streambuf<boost::iostreams::input> inBuf;

    if (a_compressFlag)
    {
        if (a_strMethod == FITS_COMPRESSIION_GZIP)
            inBuf.push(boost::iostreams::gzip_compressor(boost::iostreams::gzip::best_compression));
        else if (a_strMethod == FITS_COMPRESSIION_ZLIB)
            inBuf.push(boost::iostreams::zlib_compressor(boost::iostreams::zlib::best_compression));
    }
    else
    {
        if (a_strMethod == FITS_COMPRESSIION_GZIP)
            inBuf.push(boost::iostreams::gzip_decompressor());
        else if (a_strMethod == FITS_COMPRESSIION_ZLIB)
            inBuf.push(boost::iostreams::zlib_decompressor());
    }

    inBuf.push(compressedStream);

    boost::iostreams::copy(inBuf, decompressedStream);

    boost::iostreams::close(inBuf);

    return decompressedStream.str();
}

size_t compressData(const int8_t* a_inputBuffer, int8_t*& a_outputBuffer, size_t a_bufferSize, const std::string& a_strMethod)
{
    return compressDecompressData(a_inputBuffer, a_outputBuffer, a_bufferSize, a_strMethod);
}

size_t decompressData(const int8_t* a_inputBuffer, int8_t*& a_outputBuffer, size_t a_bufferSize, const std::string& a_strMethod)
{
    return compressDecompressData(a_inputBuffer, a_outputBuffer, a_bufferSize, a_strMethod, false);
}

size_t compressDecompressData(const int8_t* a_inputBuffer, int8_t*& a_outputBuffer, size_t a_bufferSize, const std::string& a_strMethod,
                              bool a_compressFlag)
{
    size_t              bufSize = 0, i = 0, chunkIndex = 0, chunkNumber = -1;
    char*               chunkBuffer = nullptr;
    std::vector<char *> vectorChunkBuffers;

    boost::iostreams::filtering_streambuf<boost::iostreams::input> inBuf;

    if (a_compressFlag)
    {
        if (a_strMethod == FITS_COMPRESSIION_GZIP)
            inBuf.push(boost::iostreams::gzip_compressor(boost::iostreams::gzip::best_compression));
        else if (a_strMethod == FITS_COMPRESSIION_ZLIB)
            inBuf.push(boost::iostreams::zlib_compressor(boost::iostreams::zlib::best_compression));
    }
    else
    {
        if (a_strMethod == FITS_COMPRESSIION_GZIP)
            inBuf.push(boost::iostreams::gzip_decompressor());
        else if (a_strMethod == FITS_COMPRESSIION_ZLIB)
            inBuf.push(boost::iostreams::zlib_decompressor());
    }

    inBuf.push(boost::iostreams::array_source((char *)a_inputBuffer, a_bufferSize));

    for (i = 0; inBuf.sgetc() != -1; ++i)
    {
        if ((chunkBuffer == nullptr) || (chunkIndex >= FITS_COMPRESS_MEMORY_CHUNK_SIZE))
        {
            chunkBuffer = new char[FITS_COMPRESS_MEMORY_CHUNK_SIZE];
            vectorChunkBuffers.push_back(chunkBuffer);
            chunkIndex = 0;
            ++chunkNumber;
        }

        chunkBuffer[chunkIndex++] = (char) inBuf.sbumpc();
    }

    bufSize = chunkNumber*FITS_COMPRESS_MEMORY_CHUNK_SIZE + chunkIndex;

    a_outputBuffer = new int8_t[bufSize];
    i = 0;

    for (std::vector<char *>::iterator it = vectorChunkBuffers.begin(); it < vectorChunkBuffers.end(); ++it, ++i)
    {
        size_t offset = i*FITS_COMPRESS_MEMORY_CHUNK_SIZE;
        std::memcpy(a_outputBuffer + offset, (*it), (bufSize - offset) > FITS_COMPRESS_MEMORY_CHUNK_SIZE ? FITS_COMPRESS_MEMORY_CHUNK_SIZE : chunkIndex);
        delete [] (*it);
    }

    boost::iostreams::close(inBuf);

    vectorChunkBuffers.clear();

    return bufSize;
}

bool isLittleEndian()
{
    uint16_t i = 0x0001;

    return *((uint8_t*) &i) ? true : false;
}

size_t alignOffsetForward(size_t a_offset)
{
    return a_offset % FITS_BLOCK_SIZE ? (a_offset / FITS_BLOCK_SIZE + 1) * FITS_BLOCK_SIZE : a_offset ;
}

size_t alignOffsetBackward(size_t a_offset)
{
    return a_offset % FITS_BLOCK_SIZE ? (a_offset / FITS_BLOCK_SIZE) * FITS_BLOCK_SIZE : a_offset ;
}

std::string formatNumberString(uint32_t a_number, uint8_t a_padding)
{
    char tmpBuf[0x40];

    std::string fmtStr = "%0" + std::to_string(a_padding) + "d";

    std::snprintf(tmpBuf, sizeof(tmpBuf), fmtStr.c_str(), a_number);
    std::string retStr = tmpBuf;

    return retStr;
}

std::string formatFloatString(float a_number, uint8_t a_padding)
{
    char tmpBuf[0x40];

    std::string fmtStr = "%" + std::to_string(a_padding) + "." + std::to_string(a_padding) + "f";

    std::snprintf(tmpBuf, sizeof(tmpBuf), fmtStr.c_str(), a_number);
    std::string retStr = tmpBuf;

    return retStr;
}

void swapBuffer16(uint8_t* a_buffer, size_t a_size)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    // checking for buffer granularity
    if (a_size % sizeof(int16_t) != 0)
        return;

    int16_t *tmpBuf = (int16_t*)(a_buffer);
    for (size_t i = 0; i < a_size / sizeof(int16_t); ++i)
        tmpBuf[i] = swap16(tmpBuf[i]);
#else
    return;
#endif

}

void swapBuffer32(uint8_t* a_buffer, size_t a_size)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    // checking for buffer granularity
    if (a_size % sizeof(int32_t) != 0)
        return;

    int32_t *tmpBuf = (int32_t*)(a_buffer);
    for (size_t i = 0; i < a_size / sizeof(int32_t); ++i)
        tmpBuf[i] = swap32(tmpBuf[i]);
#else
    return;
#endif
}

void swapBuffer64(uint8_t* a_buffer, size_t a_size)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    // checking for buffer granularity
    if (a_size % sizeof(int64_t) != 0)
        return;

    int64_t *tmpBuf = (int64_t*)(a_buffer);
    for (size_t i = 0; i < a_size / sizeof(int64_t); ++i)
        tmpBuf[i] = swap64(tmpBuf[i]);
#else
    return;
#endif
}

void convertBufferFloat2RGBA(uint8_t* a_buffer, size_t a_size)
{
    // checking for buffer granularity
    if (a_size % sizeof(float) != 0)
        return;

    uint32_t *tmpBuf = (uint32_t*)(a_buffer);

    for (size_t i = 0; i < a_size / sizeof(float); ++i)
    {
#if __BYTE_ORDER == __LITTLE_ENDIAN
        int32_t s = swap32(tmpBuf[i]);
#else
        int32_t s = tmpBuf[i];
#endif
        float f = *((float *)&s);
        tmpBuf[i] = convertFloat2RGBA(f);
    }
}

void convertBufferFloat2RGB(uint8_t* a_buffer, size_t a_size)
{
    // checking for buffer granularity
    if (a_size % sizeof(float) != 0)
        return;

    uint32_t *tmpBuf = (uint32_t*)(a_buffer);

    for (size_t i = 0; i < a_size / sizeof(float); ++i)
    {
#if __BYTE_ORDER == __LITTLE_ENDIAN
        int32_t s = swap32(tmpBuf[i]);
#else
        int32_t s = tmpBuf[i];
#endif
        float f = *((float *)&s);

        uint8_t red, green, blue;

        convertFloat2RGB(f, red, green, blue);

        a_buffer[i*4]     = red;
        a_buffer[i*4 + 1] = green;
        a_buffer[i*4 + 2] = blue;
        a_buffer[i*4 + 3] = 0x00;
    }
}

void convertBufferRGB2Grayscale(uint8_t* a_buffer, size_t a_size)
{
    // checking for buffer granularity -> 3-bytes RGB
    if (a_size % sizeof(float) != 0)
        return;

    uint32_t *tmpBuf = (uint32_t*)(a_buffer);

    for (size_t i = 0; i < a_size / sizeof(float); ++i)
    {
#if __BYTE_ORDER == __LITTLE_ENDIAN
        int32_t s = swap32(tmpBuf[i]);
#else
        int32_t s = tmpBuf[i];
#endif
        float f = *((float *)&s);

        uint8_t red, green, blue;
        convertFloat2Grayscale(f, red, green, blue);

        a_buffer[i*3]     = red;
        a_buffer[i*3 + 1] = green;
        a_buffer[i*3 + 2] = blue;
    }
}

void convertBufferRGB2Grayscale(uint8_t** a_buffer, uint32_t a_width, uint32_t a_height)
{
    if (a_buffer == nullptr)
        return;

    for (uint32_t y = 0; y < a_height; ++y)
        for (uint32_t x = 0; x < a_width; ++x)
        {
            uint8_t average = convertRGB2Grayscale(a_buffer[y][x*3], a_buffer[y][x*3 + 1], a_buffer[y][x*3 + 2]);

            a_buffer[y][x*3] = average;
            a_buffer[y][x*3 + 1] = average;
            a_buffer[y][x*3 + 2] = average;
        }
}

void convertBufferRGB322Grayscale(uint8_t** a_buffer, uint32_t a_width, uint32_t a_height)
{
    if (a_buffer == nullptr)
        return;

    for (uint32_t y = 0; y < a_height; ++y)
        for (uint32_t x = 0; x < a_width; ++x)
        {
            uint8_t average = convertRGB2Grayscale(a_buffer[y][x*4], a_buffer[y][x*4 + 1], a_buffer[y][x*4 + 2]);

            a_buffer[y][x*4] = average;
            a_buffer[y][x*4 + 1] = average;
            a_buffer[y][x*4 + 2] = average;
        }
}

void convertBufferRGB32Flat2Grayscale(uint8_t* a_buffer, uint32_t a_width, uint32_t a_height)
{
    if (a_buffer == nullptr)
        return;

    for (uint32_t y = 0; y < a_height; ++y)
        for (uint32_t x = 0; x < a_width; ++x)
        {
            uint8_t average = convertRGB2Grayscale(a_buffer[4*(y*a_width + x)], a_buffer[4*(y*a_width + x) + 1], a_buffer[4*(y*a_width + x) + 2]);

            a_buffer[4*(y*a_width + x)] = average;
            a_buffer[4*(y*a_width + x) + 1] = average;
            a_buffer[4*(y*a_width+ x) + 2] = average;
        }
}

void convertBufferDouble2RGBA(uint8_t* a_buffer, size_t a_size)
{
    // checking for buffer granularity
    if (a_size % sizeof(double) != 0)
        return;

    uint64_t *tmpBuf = (uint64_t*)(a_buffer);

    for (size_t i = 0; i < a_size / sizeof(double); ++i)
    {
#if __BYTE_ORDER == __LITTLE_ENDIAN
        int64_t s = swap64(tmpBuf[i]);
#else
        int64_t s = tmpBuf[i];
#endif
        double f = *((double *)&s);

        uint64_t val = convertDouble2RGBA(f);

        tmpBuf[i] = (val << 32) & 0xffffffff00000000;
    }
}

void convertBufferDouble2RGB(uint8_t* a_buffer, size_t a_size)
{
    // checking for buffer granularity
    if (a_size % sizeof(double) != 0)
        return;

    uint64_t *tmpBuf = (uint64_t*)(a_buffer);

    for (size_t i = 0; i < a_size / sizeof(double); ++i)
    {
#if __BYTE_ORDER == __LITTLE_ENDIAN
        int64_t s = swap64(tmpBuf[i]);
#else
        int64_t s = tmpBuf[i];
#endif
        double f = *((double *)&s);

        uint8_t red, green, blue;

        convertDouble2RGB(f, red, green, blue);

        a_buffer[i*8]     = red;
        a_buffer[i*8 + 1] = green;
        a_buffer[i*8 + 2] = blue;
        a_buffer[i*8 + 3] = 0x00;
        a_buffer[i*8 + 4] = 0x00;
        a_buffer[i*8 + 5] = 0x00;
        a_buffer[i*8 + 6] = 0x00;
        a_buffer[i*8 + 7] = 0x00;
    }
}

void convertBufferShort2RGB(uint8_t* a_buffer, size_t a_size, uint8_t* a_destBuffer, bool a_gray)
{
    // checking for buffer granularity
    if (a_size % sizeof(uint16_t) != 0)
        return;

    uint16_t *tmpBuf = (uint16_t*)(a_buffer);

    convertBufferShort  ptrConvertFunction = convertShort2Grayscale;

    RGBPixel pixel;

    if (!a_gray)
        ptrConvertFunction = convertShort2RGB;

    for (size_t i = 0; i < a_size / sizeof(uint16_t); ++i)
    {
#if __BYTE_ORDER == __LITTLE_ENDIAN
        uint16_t s = swap16(tmpBuf[i]);
#else
        uint16_t s = tmpBuf[i];
#endif

        ptrConvertFunction(s, pixel);

        a_destBuffer[i*4]     = pixel.red;
        a_destBuffer[i*4 + 1] = pixel.green;
        a_destBuffer[i*4 + 2] = pixel.blue;
        a_destBuffer[i*4 + 3] = 0x00;
    }
}

void convertBufferShortSZ2RGB(uint8_t* a_buffer, size_t a_size, double a_bzero, double a_bscale, uint8_t* a_destBuffer, bool a_gray)
{
    // checking for buffer granularity
    if (a_size % sizeof(uint16_t) != 0)
        return;

    uint16_t *tmpBuf = (uint16_t*)(a_buffer);

    convertBufferShortSZ  ptrConvertFunctionSZ = convertShortSZ2Grayscale;

    RGBPixel pixel;

    if (!a_gray)
        ptrConvertFunctionSZ = convertShortSZ2RGB;

    for (size_t i = 0; i < a_size / sizeof(uint16_t); ++i)
    {
#if __BYTE_ORDER == __LITTLE_ENDIAN
        uint16_t s = swap16(tmpBuf[i]);
#else
        uint16_t s = tmpBuf[i];
#endif

        ptrConvertFunctionSZ(s, a_bscale, a_bzero, pixel);

        a_destBuffer[i*4]     = pixel.red;
        a_destBuffer[i*4 + 1] = pixel.green;
        a_destBuffer[i*4 + 2] = pixel.blue;
        a_destBuffer[i*4 + 3] = 0x00;
    }
}

void convertBufferByte2RGB(uint8_t* a_buffer, size_t a_size, uint8_t* a_destBuffer)
{
    RGBPixel pixel;

    for (size_t i = 0; i < a_size; ++i)
    {
        convertByte2Grayscale(a_buffer[i], pixel);

        a_destBuffer[i*4]     = pixel.red;
        a_destBuffer[i*4 + 1] = pixel.green;
        a_destBuffer[i*4 + 2] = pixel.blue;
        a_destBuffer[i*4 + 3] = 0x00;
    }
}

void convertBufferByteSZ2RGB(uint8_t* a_buffer, size_t a_size, int8_t a_bzero, int8_t a_bscale, uint8_t* a_destBuffer)
{
    RGBPixel pixel;

    for (size_t i = 0; i < a_size; ++i)
    {
        convertByteSZ2Grayscale(a_buffer[i], a_bzero, a_bscale, pixel);

        a_destBuffer[i*4]     = pixel.red;
        a_destBuffer[i*4 + 1] = pixel.green;
        a_destBuffer[i*4 + 2] = pixel.blue;
        a_destBuffer[i*4 + 3] = 0x00;
    }
}

void convertBufferInt2RGB(uint8_t* a_buffer, size_t a_size)
{
    // checking for buffer granularity
    if (a_size % sizeof(int32_t) != 0)
        return;

    uint32_t *tmpBuf = (uint32_t*)(a_buffer);

    for (size_t i = 0; i < a_size / sizeof(uint32_t); ++i)
    {
#if __BYTE_ORDER == __LITTLE_ENDIAN
        uint32_t s = swap32(tmpBuf[i]);
#else
        uint32_t s = tmpBuf[i];
#endif

        uint8_t red, green, blue;

        convertInt2RGB(s, red, green, blue);

        a_buffer[i*4]     = red;
        a_buffer[i*4 + 1] = green;
        a_buffer[i*4 + 2] = blue;
        a_buffer[i*4 + 3] = 0x00;
    }
}

void convertBufferLong2RGB(uint8_t* a_buffer, size_t a_size)
{
    // checking for buffer granularity
    if (a_size % sizeof(int64_t) != 0)
        return;

    uint64_t *tmpBuf = (uint64_t*)(a_buffer);

    for (size_t i = 0; i < a_size / sizeof(uint64_t); ++i)
    {
#if __BYTE_ORDER == __LITTLE_ENDIAN
        uint64_t s = swap64(tmpBuf[i]);
#else
        uint64_t s = tmpBuf[i];
#endif

        uint8_t red, green, blue;

        convertLong2RGB(s, red, green, blue);

        a_buffer[i*8]     = red;
        a_buffer[i*8 + 1] = green;
        a_buffer[i*8 + 2] = blue;
        a_buffer[i*8 + 3] = 0x00;
        a_buffer[i*8 + 4] = 0x00;
        a_buffer[i*8 + 5] = 0x00;
        a_buffer[i*8 + 6] = 0x00;
        a_buffer[i*8 + 7] = 0x00;
    }
}

std::string char2hex(const uint8_t a_char)
{
    const uint8_t hexPattern[0x10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

    std::string retStr = "";

    retStr += hexPattern[(a_char & 0xF0) >> 4];
    retStr += hexPattern[(a_char & 0x0F)];

    return retStr;
}

void char2hex(const uint8_t a_char, uint8_t& a_hexCharHigh, uint8_t& a_hexCharLow)
{
    const uint8_t hexPattern[0x10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

    a_hexCharHigh = hexPattern[(a_char & 0xF0) >> 4];
    a_hexCharLow = hexPattern[(a_char & 0x0F)];
}

int32_t convertBuffer2HexString(uint8_t* a_buffer, uint8_t* a_output, size_t size, uint32_t a_align)
{
    const uint8_t minBlockSize = 8;
    const uint8_t spaceSize = 8;

    uint32_t index = 0, indexOut = 0;

    uint8_t charHi = 0, charLow = 0;

    int32_t blockSize = a_align * minBlockSize;
    size_t blockAmount = size / blockSize;
    int32_t blockBufferSize = blockSize*3 + minBlockSize + spaceSize + 1;
    int32_t lastBlockSize = size % blockSize;

    uint8_t* blockBuffer = nullptr;

    size_t i, b;

    blockBuffer = new uint8_t[blockBufferSize];

    if (blockBuffer == nullptr)
        return 0;

    for (b = 0; b < blockAmount; ++b)
    {
        for (i = 0; i < blockSize; ++i)
        {
            if (i != 0 && i % a_align == 0)
                blockBuffer[index++] = HEX_DELIM_SYMBOL;

            char2hex(a_buffer[b*blockSize + i], charHi, charLow);

            blockBuffer[index++] = charHi;
            blockBuffer[index++] = charLow;
        }

        blockBuffer[index++] = HEX_DELIM_SYMBOL;

        for (size_t i = 0; i < spaceSize; ++i)
            blockBuffer[index++] = HEX_DELIM_SYMBOL;

        for (size_t i = 0; i < blockSize; ++i)
            blockBuffer[index++] = char2alphanum(a_buffer[b * blockSize + i]);

        blockBuffer[index] = '\n';
        index = 0;

        for (size_t i = 0; i < blockBufferSize; ++i)
            a_output[indexOut++] = blockBuffer[i];
    }

    // forming the last block of aligned (e.g. 4-byte hex block)
    index = 0;

    for (size_t i = 0; i < blockSize; ++i)
        blockBuffer[i] = 0;

    if (lastBlockSize > 0)
    {
        for (size_t j = 0; j < lastBlockSize; ++j)
        {
            if (j != 0 && j % a_align == 0)
                blockBuffer[index++] = HEX_DELIM_SYMBOL;

            char2hex(a_buffer[b * blockSize + j], charHi, charLow);

            blockBuffer[index++] = charHi;
            blockBuffer[index++] = charLow;
        }

        blockBuffer[index++] = HEX_DELIM_SYMBOL;

        for (size_t i = 0; i < spaceSize + (blockSize - lastBlockSize); ++i)
            blockBuffer[index++] = HEX_DELIM_SYMBOL;

        for (size_t i = 0; i < blockSize; ++i)
            blockBuffer[index++] = char2alphanum(a_buffer[b * blockSize + i]);

        blockBuffer[index] = '\n';
        index = 0;

        for (size_t i = 0; i < lastBlockSize; ++i)
            a_output[indexOut++] = blockBuffer[i];
    }
    a_output[indexOut] = 0x00;
    // end of forming last block size

    if (blockBuffer != nullptr)
        delete [] blockBuffer;

    return index;
}

std::string convertBuffer2HexString(uint8_t* a_buffer, size_t size, uint32_t a_align)
{
    std::string retStr = "";

    const uint8_t minBlockSize = 8;
    const uint8_t spaceSize = 8;

    uint32_t index = 0;

    uint8_t charHi = 0, charLow = 0;

    int32_t blockSize = a_align * minBlockSize;
    size_t blockAmount = size / blockSize;
    int32_t blockBufferSize = blockSize*3 + minBlockSize + spaceSize + 1;
    int32_t lastBlockSize = size % blockSize;

    uint8_t* blockBuffer = nullptr;

    size_t i, b;

    blockBuffer = new uint8_t[blockBufferSize];

    if (blockBuffer == nullptr)
        return 0;

    for (b = 0; b < blockAmount; ++b)
    {
        for (i = 0; i < blockSize; ++i)
        {
            if (i != 0 && i % a_align == 0)
                blockBuffer[index++] = HEX_DELIM_SYMBOL;

            char2hex(a_buffer[b*blockSize + i], charHi, charLow);

            blockBuffer[index++] = charHi;
            blockBuffer[index++] = charLow;
        }

        blockBuffer[index++] = HEX_DELIM_SYMBOL;

        for (size_t i = 0; i < spaceSize; ++i)
            blockBuffer[index++] = HEX_DELIM_SYMBOL;

        for (size_t i = 0; i < blockSize; ++i)
            blockBuffer[index++] = char2alphanum(a_buffer[b * blockSize + i]);

        blockBuffer[index] = '\n';
        index = 0;

        retStr += libnfits::int2hex<size_t>(b*blockSize) + ": ";
        for (size_t i = 0; i < blockBufferSize; ++i)
            retStr += blockBuffer[i];
    }

    // forming the last block of aligned (e.g. 4-byte hex block)
    index = 0;

    for (size_t i = 0; i < blockSize; ++i)
        blockBuffer[i] = 0;

    if (lastBlockSize > 0)
    {
        for (size_t j = 0; j < lastBlockSize; ++j)
        {
            if (j != 0 && j % a_align == 0)
                blockBuffer[index++] = HEX_DELIM_SYMBOL;

            char2hex(a_buffer[b * blockSize + j], charHi, charLow);

            blockBuffer[index++] = charHi;
            blockBuffer[index++] = charLow;
        }

        blockBuffer[index++] = HEX_DELIM_SYMBOL;

        for (size_t i = 0; i < spaceSize + (blockSize - lastBlockSize); ++i)
            blockBuffer[index++] = HEX_DELIM_SYMBOL;

        for (size_t i = 0; i < blockSize; ++i)
            blockBuffer[index++] = char2alphanum(a_buffer[b * blockSize + i]);

        blockBuffer[index] = '\n';
        index = 0;

        retStr += libnfits::int2hex<size_t>(b*blockSize) + ": ";
        for (size_t i = 0; i < lastBlockSize; ++i)
            retStr += blockBuffer[i];
    }
    // end of forming last block size

    if (blockBuffer != nullptr)
        delete [] blockBuffer;

    return retStr;
}

void normalizeFloatBuffer(uint8_t* a_buffer, size_t a_size, float a_min, float a_max, float a_minNew, float a_maxNew)
{
    // checking for buffer granularity
    if (a_size % sizeof(float) != 0)
        return;

    float *tmpFloatBuf = (float*)(a_buffer);
    uint32_t *tmpIntBuf = (uint32_t*)(a_buffer);

    for (size_t i = 0; i < a_size / sizeof(float); ++i)
    {
#if __BYTE_ORDER == __LITTLE_ENDIAN
        int32_t s = swap32(tmpIntBuf[i]);
#else
        int32_t s = tmpIntBuf[i];
#endif
        float f = *((float *)&s);

        if (!std::isnan(f))
            tmpFloatBuf[i] = normalizeValue<float>(f, a_min, a_max, a_minNew, a_maxNew);
    }
}

void normalizeDoubleBuffer(uint8_t* a_buffer, size_t a_size, double a_min, double a_max, double a_minNew, double a_maxNew)
{
    // checking for buffer granularity
    if (a_size % sizeof(double) != 0)
        return;

    double *tmpFloatBuf = (double*)(a_buffer);
    uint64_t *tmpIntBuf = (uint64_t*)(a_buffer);

    for (size_t i = 0; i < a_size / sizeof(float); ++i)
    {
#if __BYTE_ORDER == __LITTLE_ENDIAN
        int64_t s = swap64(tmpIntBuf[i]);
#else
        int364_t s = tmpIntBuf[i];
#endif
        double f = *((double *)&s);

        tmpFloatBuf[i] = normalizeValue<double>(f, a_min, a_max, a_minNew, a_maxNew);
    }
}


//// use for debug purposes only, slow function
int32_t dumpFloatDataBuffer(const uint8_t* a_buffer, size_t a_size, const std::string& a_filename, uint32_t a_rowSize)
{
    int32_t retVal = FITS_GENERAL_SUCCESS;

    std::ofstream dumpFile;

    std::string rowStr = "";

    float min = 0.0f, max = 0.0f;

    // checking for buffer granularity
    if (a_size % sizeof(float) != 0)
        return FITS_GENERAL_ERROR;

    int32_t *tmpBuf = (int32_t*)(a_buffer);

    dumpFile.open(a_filename);

    if (!dumpFile)
        return FITS_GENERAL_ERROR;

    size_t size = a_size / sizeof(float);

    for (size_t i = 0; i < size; ++i)
    {
#if __BYTE_ORDER == __LITTLE_ENDIAN
        int32_t s = swap32(tmpBuf[i]);
#else
        int32_t s = tmpBuf[i];
#endif
        float f = *((float *)&s);

        if (f > max)
            max = f;

        if (f < min)
            min = f;

        rowStr += formatFloatString(f, 6);
        rowStr += "    ";

        if (i > 0 && ((i + 1) % a_rowSize == 0 || i == size - 1))
        {
            rowStr += '\n';
            dumpFile << rowStr;
            rowStr = "";
        }
    }

    dumpFile << "\n\nMin: " << min << " , max: " << max;

    dumpFile.close();

    return retVal;
}

}
