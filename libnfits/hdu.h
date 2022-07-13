#ifndef LIBNFITS_HDU_H
#define LIBNFITS_HDU_H

#include <cstdint>

#include "headerrecord.h"
#include "header.h"

namespace libnfits
{

class HDU
{
private:
    Header                  m_header;
    uint8_t*                m_dataBuffer;
    uint8_t*                m_payloadBuffer;
    size_t                  m_offset;
    size_t                  m_size;
    uint8_t                 m_type;
    int32_t                 m_bitpix;
    uint32_t                m_naxis;
    std::vector<uint32_t>   m_axises;

public:
    HDU();
    ~HDU();

    void addHeader(const Header& a_header);
    Header getHeader() const;
    void setData(const uint8_t* a_dataBuffer);
    uint8_t* getData() const;
    void setPaylod(const uint8_t* a_payloadBuffer);
    uint8_t* getPayload() const;
    bool isPrimary() const;
    void setType(uint8_t a_type);
    uint8_t getType() const;
    std::vector<uint32_t> getAxises() const;

    template<typename T> T getKeywordValue(const std::string& a_strKeyword, bool& a_successFlag)
    {
        return  m_header.getKeywordValue<T>(a_strKeyword, a_successFlag);
    }

    void setOffset(size_t a_offset);
    size_t getOffset() const;

    void setSize(size_t a_size);
    size_t getSize() const;

    void reset();
};

}
#endif // LIBNFITS_HDU_H
