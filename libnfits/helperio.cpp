#include "helperio.h"
#include "defs.h"

#if defined(__unix__) || defined(__APPLE__)
#include <sys/mman.h>
#endif

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace libnfits
{

MapFile::MapFile():
    m_memoryBuffer(nullptr), m_fileName(""), m_fileSize(0), m_rwFlag(false), m_fileDesc(0)
{
#if defined(__WIN32__) || defined(__WIN64__)
    m_fileHandle = nullptr;
    m_mapHandle = nullptr;
    m_pMapAddress = nullptr;
#endif
}

MapFile::~MapFile()
{
    delete [] m_memoryBuffer;
}

#if defined(__unix__) || defined(__APPLE__)
int32_t MapFile::mapFileToMemoryU()
{
    struct stat sb;

    int32_t flag = (m_rwFlag) ? O_RDWR : O_RDONLY;
    m_fileDesc = open(m_fileName.c_str(), flag);

    if (m_fileDesc <= FITS_FILE_OPEN_STAT_SUCCESS)
        return FITS_MEMORY_MAP_FILE_OPEN_ERROR;

    // getting the file size
    int32_t statRes = fstat(m_fileDesc, &sb);
    if (statRes != FITS_FILE_OPEN_STAT_SUCCESS)
       return FITS_MEMORY_MAP_FILE_FSTAT_ERROR;

    m_fileSize = sb.st_size;

    // setting the RW or R-only mode
    int32_t rwFlag = (m_rwFlag) ? PROT_READ | PROT_WRITE : PROT_READ;

    // mapping the file, getting the pointer and converting it
    m_memoryBuffer = (uint8_t*)mmap(NULL, m_fileSize, rwFlag, MAP_PRIVATE, m_fileDesc, 0);

    if (m_memoryBuffer == MAP_FAILED)
        return FITS_MEMORY_MAP_FILE_MAP_ERROR;

    return FITS_MEMORY_MAP_FILE_SUCCESS;
}

int32_t MapFile::unmapFileFromMemoryU()
{
    if (m_fileDesc == 0)
        return FITS_MEMORY_MAP_FILE_IO_ERROR;

    int32_t resUnmap = munmap(m_memoryBuffer, m_fileSize);

    int32_t resClose = close(m_fileDesc);

    if ((resUnmap & resClose) != FITS_MEMORY_UNMAP_SUCCESS)
        return FITS_MEMORY_MAP_FILE_MAP_ERROR;

    m_memoryBuffer = nullptr;

    return FITS_MEMORY_MAP_FILE_SUCCESS;
}
#elif defined(__WIN32__) || defined(__WIN64__)
int32_t MapFile::mapFileToMemoryW()
{
    m_fileHandle = ::CreateFileA(m_fileName.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (m_fileHandle == NULL)
        return FITS_MEMORY_MAP_FILE_MAP_ERROR;

    m_fileSize = ::GetFileSize(m_fileHandle, NULL);

    m_mapHandle = ::CreateFileMapping(m_fileHandle, NULL, PAGE_READONLY, 0, 0, NULL);

    if (m_mapHandle == NULL)
      return FITS_MEMORY_MAP_FILE_MAP_ERROR;

    m_pMapAddress = ::MapViewOfFile(m_mapHandle, FILE_MAP_READ, 0, 0, m_fileSize);

    if (m_pMapAddress == nullptr)
        return FITS_MEMORY_MAP_FILE_MAP_ERROR;

    m_memoryBuffer = static_cast<uint8_t*>(m_pMapAddress);

    return FITS_MEMORY_MAP_FILE_SUCCESS;
}

int32_t MapFile::unmapFileFromMemoryW()
{
    bool bFlag = FITS_MEMORY_MAP_FILE_SUCCESS ;

    if (m_memoryBuffer != nullptr)
        bFlag = ::UnmapViewOfFile(m_memoryBuffer);

    if (!bFlag)
        return FITS_MEMORY_MAP_FILE_MAP_ERROR;

    if (m_mapHandle != NULL)
        bFlag = ::CloseHandle(m_mapHandle);

    if (!bFlag)
        return FITS_MEMORY_MAP_FILE_MAP_ERROR;

    if (m_fileHandle != NULL)
        bFlag = ::CloseHandle(m_fileHandle);

    if (!bFlag)
        return FITS_MEMORY_MAP_FILE_MAP_ERROR;

    m_memoryBuffer = nullptr;

    return FITS_MEMORY_MAP_FILE_SUCCESS;
}
#endif

int32_t MapFile::loadFile(const std::string& a_fileName, bool a_rwFlag)
{
    m_fileName = a_fileName;

#if defined(__unix__) || defined(__APPLE__)
    return mapFileToMemoryU();
#elif defined(__WIN32__) || defined(__WIN64__)
    return mapFileToMemoryW();
#else
    return FITS_MEMORY_MAP_FILE_MAP_ERROR;
#endif
}

int32_t MapFile::returnFileReadError()
{
    if (m_memoryBuffer != nullptr)
        delete [] m_memoryBuffer;

    close(m_fileDesc);

    return FITS_MEMORY_MAP_FILE_OPEN_ERROR;
}

ssize_t MapFile::loadFileRead(const std::string& a_fileName)
{
    m_fileName = a_fileName;

    m_fileDesc = open(a_fileName.c_str(), O_RDONLY);
    if (m_fileDesc == FILE_READ_ERROR)
        return FITS_MEMORY_MAP_FILE_OPEN_ERROR;

    /// Getting file size
    m_fileSize = lseek(m_fileDesc, 0, SEEK_END);
    if (m_fileSize == FILE_READ_ERROR)
        returnFileReadError();

    m_memoryBuffer = new uint8_t[m_fileSize];

    /// Moving file read pointer to the start
    if (lseek(m_fileDesc, 0, SEEK_SET) == FILE_READ_ERROR)
        returnFileReadError();

    /// Read the complete file into the memory
    size_t bytesRead = read(m_fileDesc, m_memoryBuffer, m_fileSize);
    if (bytesRead != m_fileSize)
        returnFileReadError();

    /// Closing the file
    int32_t res = close(m_fileDesc);

    if (res != 0)
        return FITS_MEMORY_MAP_FILE_MAP_ERROR;
    else
        return FITS_MEMORY_MAP_FILE_SUCCESS;
}

int32_t MapFile::closeFileRead()
{
    //// Commented this part because the file is being loaded completely and is closed aftwards.
    //// No need to call close() again, will cause an error

    /*
    int32_t res = close(m_fileDesc);

    int32_t retVal = FITS_MEMORY_MAP_FILE_SUCCESS;

    if (res != 0)
    {
        retVal = FITS_MEMORY_MAP_FILE_MAP_ERROR;
    }
    else
    {
        delete [] m_memoryBuffer;
        m_memoryBuffer = nullptr;
    }
    */

    delete [] m_memoryBuffer;

    m_memoryBuffer = nullptr;

    return FITS_MEMORY_MAP_FILE_SUCCESS;
    //retVal;
}

int32_t MapFile::closeFile()
{
#if defined(__unix__) || defined(__APPLE__)
    int32_t resUnmap = unmapFileFromMemoryU();

    if (resUnmap == FITS_MEMORY_MAP_FILE_SUCCESS)
        m_memoryBuffer = nullptr;

    return resUnmap;
#elif defined(__WIN32__) || defined(__WIN64__)
    int32_t resUnmap = unmapFileFromMemoryW();

    if (resUnmap == FITS_MEMORY_MAP_FILE_SUCCESS)
        m_memoryBuffer = nullptr;

    return resUnmap;
#else
    return FITS_MEMORY_MAP_FILE_MAP_ERROR;
#endif
}

uint8_t* MapFile::getMappedFileBuffer() const
{
    return m_memoryBuffer;
}

size_t MapFile::getFileSize() const
{
    return m_fileSize;
}

}
