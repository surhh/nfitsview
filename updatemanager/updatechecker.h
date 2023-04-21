#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

#include <QString>
#include "filedownloader.h"

class UpdateChecker
{
private:
    QString         m_updateURL;
    QString         m_checkURL;
    FileDownloader  m_fileDownloader;

public:
    UpdateChecker();
    ~UpdateChecker();

    void setCheckURL(const QString& a_url);
    void setUpdateURL(const QString& a_url);
    QString getCheckURL() const;
    QString getUpdateURL() const;
    int32_t checkUpdate(int32_t a_curVer, int32_t& a_majorVer, int32_t& a_minorVer);
};

#endif // UPDATECHECKER_H
