#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H

#include <QObject>
#include <QByteArray>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

class FileDownloader : public QObject
{
    Q_OBJECT

public:
    explicit FileDownloader(QUrl a_url, QObject* parent = nullptr);
    virtual ~FileDownloader();
    void doRequest();
    QByteArray getDownloadedData() const;

signals:
    void downloaded();

private slots:
    void fileDownloaded(QNetworkReply* a_pReply);

private:
    QNetworkAccessManager  *m_netAccessManager;
    QUrl                    m_url;
    QByteArray              m_downloadedData;
};

#endif // FILEDOWNLOADER_H
