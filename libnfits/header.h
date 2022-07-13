#ifndef LIBNFITS_HEADER_H
#define LIBNFITS_HEADER_H

#include "headerrecord.h"

namespace libnfits
{

class Header
{
private:
    std::vector<HeaderRecord> m_headerRecords;

public:
    Header();
    ~Header();

    void addRecord(const HeaderRecord& a_record);
    int32_t removeRecord(uint32_t a_index);
    int32_t getRecord(uint32_t a_index, HeaderRecord& a_headerRecord) const;
    int32_t findRecordByKeyword(const std::string& a_strKeyword, HeaderRecord& a_headerRecord);
    int32_t getNAXIS();
    int32_t getBITPIX();
    std::vector<HeaderRecord> getHeaderRecords() const;
    void clear();
    void reset();


    template<typename T> T getKeywordValue(const std::string& a_strKeyword, bool& a_successFlag)
    {
        HeaderRecord    record;
        T               retVal;
        bool            bSuccess = false;

        int32_t resFound = findRecordByKeyword(a_strKeyword, record);

        if (resFound == FITS_RECORD_NOT_FOUND)
            return FITS_RECORD_NOT_FOUND;

        retVal = record.getValue<T>(bSuccess);

        a_successFlag = bSuccess;

        return retVal;
    }

};

}

#endif // LIBNFITS_HEADER_H
