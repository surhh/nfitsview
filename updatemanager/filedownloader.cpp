#include "filedownloader.h"

FileDownloader::FileDownloader(QUrl a_url, QObject* parent):
    QObject(parent),
    m_netAccessManager(nullptr),
    m_url(a_url)
{
    m_netAccessManager = new QNetworkAccessManager;

    if (m_netAccessManager != nullptr)
        connect(m_netAccessManager, SIGNAL(finished(QNetworkReply*)),this, SLOT(fileDownloaded(QNetworkReply*)));
}

FileDownloader::FileDownloader(QObject *parent):
    QObject(parent)
{

}

FileDownloader::~FileDownloader()
{
    delete m_netAccessManager;
}

void FileDownloader::fileDownloaded(QNetworkReply* a_pReply)
{
    m_downloadedData = a_pReply->readAll();

    a_pReply->deleteLater();

    emit downloaded();
}

QByteArray FileDownloader::getDownloadedData() const
{
    return m_downloadedData;
}

void FileDownloader::doRequest()
{
    QNetworkRequest request(m_url);

    m_netAccessManager->get(request);
}

void FileDownloader::setCheckURL(const QString &a_url)
{
    m_url = QUrl(a_url);
}
