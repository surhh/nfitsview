#ifndef LIBNFITS_HELPERIO_H
#define LIBNFITS_HELPERIO_H

#include <cstdint>
#include <string>

#if defined(__WIN32__) || defined(__WIN64__)
#include <windows.h>
#endif

#define FILE_READ_ERROR     (-1)

namespace libnfits
{

class MapFile
{
private:
    uint8_t        *m_memoryBuffer;
    int32_t         m_fileDesc;
    std::string     m_fileName;
    size_t          m_fileSize;
    bool            m_rwFlag;

#if defined(__WIN32__) || defined(__WIN64__)
    HANDLE          m_fileHandle;
    HANDLE          m_mapHandle;
    void*           m_pMapAddress;
#endif

private:
#if defined(__unix__) || defined(__APPLE__)
    int32_t mapFileToMemoryU();
    int32_t unmapFileFromMemoryU();
#elif defined(__WIN32__) || defined(__WIN64__)
    int32_t mapFileToMemoryW();
    int32_t unmapFileFromMemoryW();
#endif

public:
    MapFile();
    ~MapFile();

    int32_t loadFile(const std::string& a_fileName, bool a_rwFlag = false);
    ssize_t loadFileRead(const std::string& a_fileName);
    int32_t closeFile();
    int32_t closeFileRead();
    uint8_t* getMappedFileBuffer() const;
    size_t getFileSize() const;

private:
    int32_t returnFileReadError();
};

}
#endif // LIBNFITS_HELPERIO_H
