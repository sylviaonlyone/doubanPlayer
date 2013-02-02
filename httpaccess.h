/****************************************************************************
* Filename:  httpaccess.h
* Description: Declare HttpAccess class
* Version:  0.1
* Created:  05/15/2011
* Revision: none
* Compiler: QT
* Author:   Mason Zhang
* Company:  NA
****************************************************************************/

#ifndef HTTPACCESS_H
#define HTTPACCESS_H

#include <QObject>
#include <QFile>
#include <QHttp>
#include <QUrl>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class HttpAccess : public QObject{
    Q_OBJECT
public:
    HttpAccess(QObject *parent = 0);
    ~HttpAccess();

    void HttpRequest(const QUrl& url, const QByteArray& data );
    QByteArray GetInputPara();
    void HttpGet(const QUrl& url, const QString& name);

signals:
    void readed(char * pBuf, int len);
    void finished(bool error);

private slots:
    void done(bool error);
    void requestStarted(int id);
    void readyRead(const QHttpResponseHeader& resp);
    void stateChanged(int state);
    void httpGetFinished();
    void httpGetReadyRead();
    void updateDataReadProgress(qint64 bytesRead, qint64 totalBytes);
private:
    QHttp* pHttp;
    //assume there is only one Http request at a time,
    //if it's not a case, we need design an alternative
    QFile *pFile;
    int iRequestId;
    int iGetId;
    QNetworkAccessManager qnam;
    QNetworkReply* pReply;
};
#endif // HTTPACCESS_H
