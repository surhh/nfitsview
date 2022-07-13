#include "header.h"

namespace libnfits
{

Header::Header()
{

}

Header::~Header()
{
    clear();
}

void Header::addRecord(const HeaderRecord& a_record)
{
    // Only non-empty records are suppoed to be added into the header info
    if (!a_record.getKeyword().empty() || !a_record.getValueString().empty() || !a_record.getComment().empty())
        m_headerRecords.push_back(a_record);
}

int32_t Header::removeRecord(uint32_t a_index)
{
    if (a_index > m_headerRecords.size() - 1)
        return FITS_GENERAL_ERROR;

    m_headerRecords.erase(m_headerRecords.begin() + a_index);

    return FITS_GENERAL_SUCCESS;
}

int32_t Header::getRecord(uint32_t a_index, HeaderRecord& a_headerRecord) const
{
    if (a_index > m_headerRecords.size() - 1)
        return FITS_GENERAL_ERROR;

    a_headerRecord = m_headerRecords[a_index];

    return FITS_GENERAL_SUCCESS;
}

int32_t Header::findRecordByKeyword(const std::string& a_strKeyword, HeaderRecord& a_headerRecord)
{
    int32_t index = 0;

    for (std::vector<HeaderRecord>::iterator it = m_headerRecords.begin(); it < m_headerRecords.end(); ++it, ++index)
        if ((*it).getDataRecord().keyword == a_strKeyword)
        {
            a_headerRecord = (*it);
            return index;
        }

    return FITS_RECORD_NOT_FOUND;
}

void Header::clear()
{
    m_headerRecords.clear();
}

void Header::reset()
{
    clear();
}

int32_t Header::getNAXIS()
{
    bool    bSuccess = false;

    int32_t foundVal = getKeywordValue<int32_t>(FITS_KEYWORD_NAXIS, bSuccess);

    if (bSuccess)
        return foundVal;
    else
        return FITS_RECORD_NOT_FOUND;
}

int32_t Header::getBITPIX()
{
    bool    bSuccess = false;

    int32_t foundVal = getKeywordValue<int32_t>(FITS_KEYWORD_BITPIX, bSuccess);

    if (bSuccess)
        return foundVal;
    else
        return FITS_RECORD_NOT_FOUND;
}

std::vector<HeaderRecord> Header::getHeaderRecords() const
{
    return m_headerRecords;
}

}

