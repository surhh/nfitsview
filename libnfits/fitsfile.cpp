#include "fitsfile.h"

#include "defs.h"

#include "image.h"

namespace libnfits
{

FitsFile::FitsFile():
    m_fileName(""), m_memoryBuffer(nullptr), m_fileSize(0), m_offset(0), m_callbackFunc(nullptr), m_callbackFuncParam(nullptr)
{

}

FitsFile::FitsFile(const std::string& a_fileName):
    m_fileName(a_fileName), m_memoryBuffer(nullptr), m_fileSize(0), m_offset(0), m_callbackFunc(nullptr), m_callbackFuncParam(nullptr)
{

}

FitsFile::~FitsFile()
{
    reset();
}

int32_t FitsFile::loadFile(const std::string& a_fileName)
{
    int32_t retVal = m_mapFile.loadFile(a_fileName);

    if (retVal != FITS_MEMORY_MAP_FILE_SUCCESS)
        return retVal;

    m_fileSize = m_mapFile.getFileSize();

    if (m_fileSize < FITS_BLOCK_SIZE)
        return FITS_FILE_WRONG_SIZE;

    m_memoryBuffer = m_mapFile.getMappedFileBuffer();

    m_fileName = a_fileName;

    setOffset(0);

    retVal = findAllHDUs();

    // return m_HDUs.size();  // this was the original way, but the implementation of further ideas requires other approach

    return retVal;
}

int32_t FitsFile::closeFile()
{
    int32_t resUnmap = m_mapFile.closeFile();

    if (resUnmap == FITS_MEMORY_MAP_FILE_SUCCESS)
        reset();

    return resUnmap;
}

uint32_t FitsFile::getNumberOfHDUs() const
{
    return m_HDUs.size();
}

int32_t FitsFile::setOffset(size_t a_offset)
{
    int32_t retVal;

    if (a_offset > m_fileSize)
        retVal = FITS_GENERAL_ERROR;
    else
    {
        m_offset = a_offset;
        retVal = FITS_GENERAL_SUCCESS;
    }

    return retVal;
}

size_t FitsFile::getOffset() const
{
    return m_offset;
}

size_t FitsFile::getSize() const
{
    return m_fileSize;
}

int32_t FitsFile::findHDU()
{
    int32_t retVal = FITS_GENERAL_SUCCESS;

    size_t  recordIndex = 0;
    bool    bEnd = false;
    uint8_t hduType = FITS_HDU_TYPE_PRIMARY;
    size_t  prevOffset = 0;

    Header  header;
    HDU     hdu;

    prevOffset = m_offset;
    hdu.setOffset(m_offset);

    while (!bEnd)
    {
        HeaderRecord hRecord;

        if (m_offset > (m_fileSize - FITS_HEADER_RECORD_SIZE))
            return FITS_HDU_OFFSET_ERROR;

        std::string strRecord = getStringFromBuffer(m_memoryBuffer, m_offset);

        hRecord.setData(strRecord);
        int32_t parseRes = hRecord.parse();

        if (parseRes != FITS_GENERAL_SUCCESS)
            return parseRes;

        // Checking the first record for the header to be inserted into the HDU
        // it should be either 'SIMPLE' or 'XTENSION' for the beginning of the header.
        // Otherwise we are not at the header start position which is wrong.
        // Or we continue identifying the correct image type
        std::string keyword = hRecord.getKeyword();
        std::string value = hRecord.getValueString();
        trimStringLeftRight(value, FITS_QUOTE_CHAR); // This quotation stripping is added here because we skipped stripping in header parsing
                                                     // in order to have 'VALUE' syntax in the header output.
                                                     // Commented trimStringLeftRight(strValue, FITS_QUOTE_CHAR); in headerrecord.cpp

        if (recordIndex == 0 && keyword != FITS_KEYWORD_SIMPLE && keyword != FITS_KEYWORD_XTENSION)
            return FITS_HDU_START_ERROR;
        else if (recordIndex == 0 && keyword == FITS_KEYWORD_SIMPLE)
            hduType = FITS_HDU_TYPE_PRIMARY;
        else if (recordIndex == 0 && keyword == FITS_KEYWORD_XTENSION && value == FITS_XTENSION_IMAGE)
            hduType = FITS_HDU_TYPE_IMAGE_XTENSION;
        else if (recordIndex == 0 && keyword == FITS_KEYWORD_XTENSION && value == FITS_XTENSION_TABLE)
            hduType = FITS_HDU_TYPE_ASCII_TABLE_XTENSION;
        else if (recordIndex == 0 && keyword == FITS_KEYWORD_XTENSION && value == FITS_XTENSION_BINTABLE)
            hduType = FITS_HDU_TYPE_BINARY_TABLE_XTENSION;

        header.addRecord(hRecord);

        ++recordIndex;
        m_offset = m_offset + FITS_HEADER_RECORD_SIZE;

        if (keyword == FITS_KEYWORD_END)
            bEnd = true;
    }

    m_offset = alignOffsetForward(m_offset);

    hdu.addHeader(header);
    hdu.setType(hduType);

    hdu.setData(m_memoryBuffer + hdu.getOffset());
    if (header.getNAXIS() != 0)
    {
        hdu.setPaylod(m_memoryBuffer + m_offset);
        hdu.setPayloadOffset(m_offset);
    }

    // Calculating the amount of real data after the header
    bool bSucess = false;
    int32_t naxis = header.getNAXIS();
    int32_t bitpix = std::abs(header.getBITPIX()) / 8;  // number of bytes for one entry
    size_t size = naxis ? bitpix : 0;

    for (int32_t i = 1; i <= naxis; ++i)
    {
        int32_t tmpSize = header.getKeywordValue<int32_t>(FITS_KEYWORD_NAXIS + std::to_string(i), bSucess);

        if (bSucess)
            size = size * tmpSize;
    }
    //

    // added to fix the missing miltiple HDU bug and wrong HDU sizes bug
    m_offset += size;
    m_offset = alignOffsetForward(m_offset);

    hdu.setSize(m_offset - prevOffset);
    // end of bugfixes

    m_HDUs.push_back(hdu);

    return retVal;
}

int32_t FitsFile::findPrimaryHDU()
{
    m_offset = 0;

    return findHDU();
}

int32_t FitsFile::findAllHDUs()
{
    int32_t tmpVal = FITS_GENERAL_ERROR;

    m_offset = 0;

    do
    {
        tmpVal = findHDU();
    }
    while (tmpVal == FITS_GENERAL_SUCCESS);

    if (m_HDUs.size() == 0)
        return FITS_GENERAL_ERROR;
    else
        return FITS_GENERAL_SUCCESS;
}

void FitsFile::reset()
{
    m_fileName.clear();
    m_memoryBuffer = nullptr;
    m_fileSize = 0;
    m_offset = 0;
    m_callbackFunc = nullptr;
    m_HDUs.clear();
    m_mapFile.closeFile();
}

int32_t FitsFile::exportImageHDU(uint32_t a_hduIndex)
{
    int32_t retVal = FITS_GENERAL_SUCCESS;
    bool bSuccess;
    //// TODO: it's needed to add also the FITS_HDU_TYPE_COMPRESSED_IMAGE_XTENSION support

    uint32_t axisesNumber = m_HDUs[a_hduIndex].getKeywordValue<uint32_t>(FITS_KEYWORD_NAXIS, bSuccess);

    uint8_t HDUtype = m_HDUs[a_hduIndex].getType();
    if ((HDUtype != FITS_HDU_TYPE_PRIMARY && HDUtype != FITS_HDU_TYPE_IMAGE_XTENSION) || (axisesNumber < 2 || !bSuccess))
        return FITS_PNG_HDU_NOT_IMAGE_ERROR;

    Image           image;
    std::string     fileName;

    fileName = m_fileName + "." + formatNumberString(a_hduIndex, 10) + ".png";

    std::vector<uint32_t> axises = m_HDUs[a_hduIndex].getAxises();

    if (axises.size() < 2)
        return FITS_PNG_HDU_NOT_IMAGE_ERROR;

    int32_t bitpix = m_HDUs[a_hduIndex].getKeywordValue<int32_t>(FITS_KEYWORD_BITPIX, bSuccess); // was int8_t before, int32_t is the right one

    if (!bSuccess)
        return FITS_PNG_EXPORT_ERROR;

    bool bZSuccess = false, bSSuccess = false;

    double bzero = m_HDUs[a_hduIndex].getKeywordValue<double>(FITS_KEYWORD_BZERO, bZSuccess);
    double bscale = m_HDUs[a_hduIndex].getKeywordValue<double>(FITS_KEYWORD_BSCALE, bSSuccess);

    image.setParameters(axises[0], axises[1], FITS_PNG_DEFAULT_PIXEL_DEPTH, bitpix);
    image.setData(m_HDUs[a_hduIndex].getPayload());
    image.setMaxDataBufferSize(m_fileSize);
    image.setBaseOffset(m_HDUs[a_hduIndex].getPayloadOffset());
    image.setCallbackFunction(m_callbackFunc, m_callbackFuncParam);

    if (bZSuccess)
        image.setBZero(bzero);

    if (bSSuccess)
        image.setBScale(bscale);

    retVal = image.exportPNG(fileName);

    return retVal;
}

int32_t FitsFile::exportAllImageHDUs()
{
    int32_t retVal = 0, err = 0;

    for (uint32_t i = 0; i < m_HDUs.size(); ++i)
    {
        int32_t resExport = exportImageHDU(i);

        if (resExport == FITS_GENERAL_SUCCESS)
            ++retVal;
        else if (resExport == FITS_PNG_EXPORT_ERROR)
            err = FITS_PNG_EXPORT_ERROR;
    }

    return (!err) ? retVal : err;
}

void FitsFile::setCallbackFunction(CallbackFunctionPtr a_callbackFunc, void* a_callbackFuncParam)
{
    m_callbackFunc = a_callbackFunc;
    m_callbackFuncParam = a_callbackFuncParam;
}

int32_t FitsFile::getHDU(uint32_t a_index, HDU& a_hdu) const
{
     if (a_index >= m_HDUs.size())
         return FITS_GENERAL_ERROR;

     a_hdu = m_HDUs[a_index];

     return FITS_GENERAL_SUCCESS;
}

std::string FitsFile::getFileName() const
{
    return m_fileName;
}

bool FitsFile::isOpen() const
{
    return m_memoryBuffer != nullptr ? true : false;
}

}
