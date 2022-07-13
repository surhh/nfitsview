#include "headerrecord.h"

#include <sstream>

namespace libnfits
{

HeaderRecord::HeaderRecord():
    m_strData(""), m_bContinue(false)
{

}

HeaderRecord::~HeaderRecord()
{

}

void HeaderRecord::operator = (const std::string& a_strData)
{
    this->m_strData = a_strData;
}

void HeaderRecord::setData(const std::string& a_strData)
{
    m_strData = a_strData;
}

std::string HeaderRecord::getData() const
{
    return m_strData;
}

int32_t HeaderRecord::parse()
{
    HeaderRecordData dataRecord;
    std::string strAfterKeyword = "";
    std::string strKeyword = "";
    std::string strValue = FITS_UNDEFINED_STR_VALUE;
    std::string strComment = "";

    int32_t retVal = FITS_GENERAL_SUCCESS;

    // processing record error cases: empty, long, wrong syntax
    if (m_strData.empty())
        return FITS_EMPTY_STRING_ERROR;

    if (m_strData.length() > FITS_HEADER_RECORD_SIZE)
        return FITS_RECORD_SIZE_ERROR;

    if (!recordSyntaxCheck(m_strData))
        return FITS_RECORD_SYNTAX_ERROR;

    // there is at least a keyword
    if (m_strData.length() >= FITS_MIN_KEYWORD_VALUE_STR_LENGTH &&
        m_strData[FITS_KEYWORD_END_POS] == FITS_HEADER_RECORD_ASSIGNMENT_CHAR &&
        m_strData[FITS_KEYWORD_END_POS + 1] == FITS_PADDING_SPACE_CHAR)
    {
        strKeyword = removeSymbols(getKeywordFromRecord(m_strData), FITS_PADDING_SPACE_CHAR);
        strAfterKeyword = getAfterKeywordFromRecord(m_strData);
    }
    else
    {
        // special keywords (e.g. END, CONTINUE, HISTORY, COMMENT) case handling
        size_t posFoundEnd = m_strData.find(FITS_KEYWORD_END);
        size_t posFoundContinue = m_strData.find(FITS_KEYWORD_CONTINUE);
        size_t posFoundHistory = m_strData.find(FITS_KEYWORD_HISTORY);
        size_t posFoundComment = m_strData.find(FITS_KEYWORD_COMMENT);

        if (posFoundEnd == 0)
        {
            strKeyword = FITS_KEYWORD_END;
            strAfterKeyword = m_strData.substr(posFoundEnd + std::string(FITS_KEYWORD_END).length());
        }
        else if (posFoundContinue == 0)
        {
            strKeyword = FITS_KEYWORD_CONTINUE;
            strAfterKeyword = m_strData.substr(posFoundContinue + std::string(FITS_KEYWORD_CONTINUE).length());
        }
        else if (posFoundHistory == 0)
        {
            strKeyword = FITS_KEYWORD_HISTORY;
            strAfterKeyword = m_strData.substr(posFoundHistory + std::string(FITS_KEYWORD_HISTORY).length());
        }
        else if (posFoundComment == 0)
        {
            strKeyword = FITS_KEYWORD_COMMENT;
            strAfterKeyword = m_strData.substr(posFoundComment + std::string(FITS_KEYWORD_COMMENT).length());
        }
        else
            strAfterKeyword = m_strData;
    }

    // finding the exact comment (if exists) start position
    int32_t posComment = findFirstCharOutOfQuotes(strAfterKeyword, FITS_COMMENT_START_CHAR);

    // processing the rest of the record
    if (posComment != -1)
        strComment = strAfterKeyword.substr(posComment + 1);

    if (posComment > 0)
        strValue = strAfterKeyword.substr(0, posComment - 1);
    else
        strValue = strAfterKeyword;

    // removing possible double quotes from the value field
    if (strValue.length() > 2)
        replaceSubstring(strValue, FITS_DOUBLE_QUOTE_CHAR, std::string(1, FITS_QUOTE_CHAR));

    // final processing of the keyword field
    dataRecord.keyword = strKeyword;

    // final processing of the value field
    trimStringLeftRight(strValue, FITS_PADDING_SPACE_CHAR);
    ////trimStringLeftRight(strValue, FITS_QUOTE_CHAR);       // Here we may have 2 cases, with '' or without, both can be ok
    trimStringRight(strValue, FITS_VALUE_CONTINUE_CHAR);
    dataRecord.value = strValue;

    // final prpcessing of the comment field, both-side trimming may be optional
    // trimStringLeftRight(strComment, FITS_PADDING_SPACE_CHAR);    // Uncomment this if needed to trim spaces both from left & right sides.
    trimStringRight(strComment, FITS_PADDING_SPACE_CHAR);           // Comment this line if the above one is uncommented.
    dataRecord.comment = strComment;

    // checking if the value or comment are continued to the next record item
    bool bContinueValue = isValueContinued(strValue);

    // changing continue flag for the record
    changeContinueFlag(bContinueValue);

    // trimming the continuation marker (by default '&')
    if (bContinueValue)
        trimStringRight(strValue, FITS_VALUE_CONTINUE_CHAR);

    // assigning the constructed final header record structure
    m_headerRecordData = dataRecord;

    m_strData.clear();

    return retVal;
}

HeaderRecordData HeaderRecord::getDataRecord() const
{
    return m_headerRecordData;
}

std::vector<std::string> HeaderRecord::getDataRecordComponentsList() const
{
    std::vector<std::string> retVec;

    retVec.push_back(m_headerRecordData.keyword);
    retVec.push_back(m_headerRecordData.value);
    retVec.push_back(m_headerRecordData.comment);

    return retVec;
}

std::string HeaderRecord::getTextFromErrorCode(int32_t a_errorCode) const
{
    std::string strError = "";

    switch (a_errorCode)
    {
        case FITS_GENERAL_SUCCESS:
            strError = FITS_MSG_GENERAL_SUCCESS;
            break;
        case FITS_ONLY_KEYWORD_SUCCESS:
            strError = FITS_MSG_ONLY_KEYWORD_SUCCESS;
            break;
        case FITS_GENERAL_ERROR:
            strError = FITS_MSG_GENERAL_ERROR;
            break;
        case FITS_EMPTY_STRING_ERROR:
            strError = FITS_MSG_EMPTY_STRING_ERROR;
            break;
        case FITS_RECORD_SYNTAX_ERROR:
            strError = FITS_MSG_GENERAL_RECORD_SIZE_ERROR;
            break;
        default:
            break;
    }

    return strError;
}

void HeaderRecord::setContinueFlag()
{
    changeContinueFlag(true);
}

void HeaderRecord::clearContinueFlag()
{
    changeContinueFlag(false);
}

bool HeaderRecord::getContinueFlag() const
{
    return m_bContinue;
}

void HeaderRecord::changeContinueFlag(bool a_flag)
{
    m_bContinue = a_flag;
}

void HeaderRecord::clear()
{
    m_bContinue = false;
    m_strData.clear();
    m_headerRecordData.clear();
}

std::string HeaderRecord::getKeyword() const
{
    return m_headerRecordData.keyword;
}

std::string HeaderRecord::getValueString() const
{
    return m_headerRecordData.value;
}

std::string HeaderRecord::getComment() const
{
    return m_headerRecordData.comment;
}

}
