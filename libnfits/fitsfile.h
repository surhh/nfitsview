#ifndef LIBNFITS_FITSFILE_H
#define LIBNFITS_FITSFILE_H

#include <string>
#include "helperio.h"
#include "hdu.h"

namespace libnfits
{

class FitsFile
{
private:
    std::string         m_fileName;
    MapFile             m_mapFile;
    uint8_t*            m_memoryBuffer;
    size_t              m_fileSize;
    size_t              m_offset;

    std::vector<HDU>    m_HDUs;

    CallbackFunctionPtr m_callbackFunc;
    void*               m_callbackFuncParam;

private:
    int32_t findHDU();
    int32_t findAllHDUs();
    int32_t findPrimaryHDU();

public:
    FitsFile();
    FitsFile(const std::string& a_fileName);
    ~FitsFile();

    int32_t loadFile(const std::string& a_fileName);
    int32_t closeFile();
    uint32_t getNumberOfHDUs() const;
    int32_t setOffset(size_t a_offset);
    size_t getOffset() const;
    size_t getSize() const;
    int32_t exportImageHDU(uint32_t a_hduIndex);
    int32_t exportAllImageHDUs();
    void setCallbackFunction(CallbackFunctionPtr a_callbackFunc, void* a_callbackFuncParam);
    int32_t getHDU(uint32_t a_index, HDU& a_hdu) const;
    std::string getFileName() const;
    bool isOpen() const;
    void reset();
};

}
#endif // LIBNFITS_FITSFILE_H
