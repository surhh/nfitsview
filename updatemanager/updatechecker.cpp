#include "updatechecker.h"

UpdateChecker::UpdateChecker():
    m_checkURL(""),
    m_updateURL("")
{

}

UpdateChecker::~UpdateChecker()
{
    m_checkURL = "";
    m_updateURL = "";
}

void UpdateChecker::setCheckURL(const QString& a_url)
{
    m_checkURL = a_url;
    m_fileDownloader.setCheckURL(m_checkURL);
}

void UpdateChecker::setUpdateURL(const QString& a_url)
{
    m_updateURL = a_url;
}

QString UpdateChecker::getCheckURL() const
{
    return m_checkURL;
}

QString UpdateChecker::getUpdateURL() const
{
    return m_updateURL;
}

int32_t UpdateChecker::checkUpdate(int32_t a_curVer, int32_t& a_majorVer, int32_t& a_minorVer)
{
    int32_t checkRes = 0;

    QByteArray data = m_fileDownloader.getDownloadedData();

    int32_t newVer = data.toInt();

    a_majorVer = newVer / 10;
    a_minorVer = newVer % 10;

    if (a_curVer < newVer)
        checkRes = 1;

    return checkRes;
}
