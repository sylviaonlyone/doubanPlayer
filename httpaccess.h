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
#include <QVector>

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
private:
    QHttp *pHttp;
    //there could be multiple requests coming at same time
    //asking for files.
    //id : request id
    //pd : file pointer for a certain request
    QVector<QFile*> fileVect;
    QFile *pFile;
    int iRequestId;
    int iGetId;
};
#endif // HTTPACCESS_H
