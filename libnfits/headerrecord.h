#ifndef LIBNFITS_HEADERRECORD_H
#define LIBNFITS_HEADERRECORD_H

#include <cstdint>

#include "defs.h"
#include "keywords.h"
#include "helperfunctions.h"

namespace libnfits
{

struct HeaderRecordData
{
    std::string keyword;
    std::string value;
    std::string comment;

    HeaderRecordData():
        keyword(""), value(""), comment("")
    {
    }

    HeaderRecordData(const std::string& a_strKeywoard, const std::string& a_strValue, const std::string& a_strComment):
        keyword(a_strKeywoard), value(a_strValue), comment(a_strComment)
    {
    }

    void appendKeyword(const std::string& a_strAppend)
    {
        keyword += a_strAppend;
    }

    void appendValue(const std::string& a_strAppend)
    {
        value += a_strAppend;
    }

    void appendComment(const std::string& a_strAppend)
    {
        comment += a_strAppend;
    }

    void clear()
    {
        keyword.clear();
        value.clear();
        comment.clear();
    }
};

class HeaderRecord
{
private:
    std::string                 m_strData;
    HeaderRecordData            m_headerRecordData;
    bool                        m_bContinue;

public:
    HeaderRecord();
    ~HeaderRecord();

    void operator = (const std::string& a_strData);

    void setData(const std::string& a_strData);
    std::string getData() const;
    int32_t parse();
    HeaderRecordData getDataRecord() const;
    std::string getKeyword() const;
    std::string getValueString() const;
    template<typename T> T getValue(bool& a_successFlag) const
    {
        return convertStringMulti<T>(m_headerRecordData.value, a_successFlag);
    }
    std::string getComment() const;
    std::vector<std::string> getDataRecordComponentsList() const;
    std::string getTextFromErrorCode(int32_t a_errorCode) const;
    void setContinueFlag();
    void clearContinueFlag();
    bool getContinueFlag() const;
    void changeContinueFlag(bool a_flag);
    void clear();
};

}
#endif // LIBNFITS_HEADERRECORD_H
