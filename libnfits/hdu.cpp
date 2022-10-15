#include "hdu.h"

namespace libnfits
{

HDU::HDU():
    m_dataBuffer(nullptr), m_payloadBuffer(nullptr), m_offset(0), m_payloadOffset(0), m_size(0),
    m_type(/*FITS_HDU_PRIMARY_HDU_INDEX*/0), m_bitpix(0), m_naxis(0)
{

}

HDU::~HDU()
{
    reset();
}

void HDU::addHeader(const Header& a_header)
{
    m_header = a_header;

    HeaderRecord hRecord;

    int32_t foundIndex = m_header.findRecordByKeyword(FITS_KEYWORD_SIMPLE, hRecord);

    if (foundIndex == FITS_HDU_PRIMARY_HDU_INDEX)
        if (hRecord.getKeyword() == FITS_KEYWORD_SIMPLE && hRecord.getValueString() == "T")
            m_type = FITS_HDU_TYPE_PRIMARY;

    m_naxis = m_header.getNAXIS();
    m_bitpix = m_header.getBITPIX();

    for (int32_t i = 1; i <= m_naxis; ++i)
    {
        bool bSuccess = false;

        uint32_t value = m_header.getKeywordValue<uint32_t>(FITS_KEYWORD_NAXIS + std::to_string(i), bSuccess);

        if (bSuccess)
            m_axises.push_back(value);
    }
}

Header HDU::getHeader() const
{
    return m_header;
}

void HDU::setData(const uint8_t* a_dataBuffer)
{
    m_dataBuffer = (uint8_t*)a_dataBuffer;
}

uint8_t* HDU::getData() const
{
    return (uint8_t*)m_dataBuffer;
}

void HDU::setPaylod(const uint8_t *a_payloadBuffer)
{
    m_payloadBuffer = (uint8_t*)a_payloadBuffer;
}

uint8_t* HDU::getPayload() const
{
    return (uint8_t*)m_payloadBuffer;
}

bool HDU::isPrimary() const
{
    return (m_type == FITS_HDU_TYPE_PRIMARY);
}

void HDU::setType(uint8_t a_type)
{
     m_type = a_type;
}

uint8_t HDU::getType() const
{
    return m_type;
}

std::vector<uint32_t> HDU::getAxises() const
{
    return m_axises;
}

void HDU::setOffset(size_t a_offset)
{
    m_offset = a_offset;
}

size_t HDU::getOffset() const
{
    return m_offset;
}

void HDU::setSize(size_t a_size)
{
    m_size = a_size;
}

void HDU::setPayloadOffset(size_t a_payloadOffset)
{
    m_payloadOffset = a_payloadOffset;
}

size_t HDU::getPayloadOffset() const
{
    return m_payloadOffset;
}

size_t HDU::getSize() const
{
    return m_size;
}

void HDU::reset()
{
    m_dataBuffer = nullptr;
    m_payloadBuffer = nullptr;
    m_offset = 0;
    m_payloadOffset = 0;
    m_size = 0;
    m_type = 0; //FITS_HDU_PRIMARY_HDU_INDEX;
    m_bitpix = 0;
    m_naxis = 0;
    m_axises.clear();
}

}
