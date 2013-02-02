/****************************************************************************
* Filename:  httpaccess.cpp
* Description: Define methods for http accessing
* Version:  0.1
* Created:  05/15/2011
* Revision: none
* Compiler: QT
* Author:   Mason Zhang
* Company:  NA
****************************************************************************/

#include <iostream>
#include <QFileInfo>
#include <QHttpRequestHeader>
#include <QHttpResponseHeader>
#include <QByteArray>
#include "httpaccess.h"

#define ALBUM_PICTURE "picture.jpg"

using namespace std;

HttpAccess::HttpAccess(QObject *parent): 
    QObject(parent),
    iRequestId(0),
    iGetId(0),
    pReply(NULL)
{
    pHttp = new QHttp(this);

    connect(pHttp,SIGNAL(requestStarted(int)), this, SLOT(requestStarted(int)));
    connect(pHttp,SIGNAL(readyRead(const QHttpResponseHeader&)),
            this, SLOT(readyRead(const QHttpResponseHeader&)));
    connect(pHttp, SIGNAL(done(bool)), this, SLOT(done(bool)));
    connect(pHttp, SIGNAL(stateChanged(int)), this, SLOT(stateChanged(int)));
}

void HttpAccess::HttpRequest(const QUrl& url, const QByteArray& data)
{
    QFileInfo fileInfo(url.path());
    QString fileName = fileInfo.fileName();
    if(fileName.isEmpty())
    {
        cerr<<fileName.toAscii().constData()<<" is empty"<<endl;
        return;        
    }

    pFile = new QFile(fileName);
    if(!pFile->open(QIODevice::WriteOnly))
    {
        cerr<<"Unable to save the file"<<endl;
        delete pFile;
        pFile = NULL;
        return;
    }

    //cout<<"http request host "<<qPrintable(url.host())<<endl;  //douban.fm
    //cout<<"http request path "<<qPrintable(url.path())<<endl; ///j/mine/playlist

    QHttpRequestHeader header("POST", url.path());

    pHttp->setHost(url.host(), url.port(80));
    //obsolete Content-Type
    //header.setValue("Content-Type", "application/x-www-form-urlencoded");
    header.setValue("Content-Type", "application/json; charset=utf-8");
    header.setValue("Host", url.host());

    cout<<"Request para: "<<data.data()<<endl;
    iRequestId = pHttp->request(header,data,pFile);
    pHttp->close();
}

void HttpAccess::HttpGet(const QUrl& url, const QString& name)
{
    QFileInfo file(name);
    if (file.exists())
    {
        cerr<<name.toUtf8().constData()<<" exist!"<<endl;
        emit finished(true);
        return;
    }
    pFile = new QFile(name);
    if(!pFile->open(QIODevice::WriteOnly))
    {
        cerr<<"Unable to save the file"<<endl;
        delete pFile;
        pFile = 0;
        return;
    }
    cout<<"Getting "<<name.toUtf8().constData()<<endl;

    //pHttp->setHost(url.host());
    cout<<"Http Get host "<<qPrintable(url.host())<<endl;
    cout<<"Http Get path"<<qPrintable(url.path())<<endl;
    //iGetId = pHttp->get(url.path(), pFile);
    //pHttp->close();


    ///////////////////
    pReply = qnam.get(QNetworkRequest(url));
    connect(pReply, SIGNAL(finished()), this, SLOT(httpGetFinished()));
    connect(pReply, SIGNAL(readyRead()), this, SLOT(httpGetReadyRead()));
    connect(pReply, SIGNAL(downloadProgress(qint64,qint64)),
             this, SLOT(updateDataReadProgress(qint64,qint64)));
}

void HttpAccess::done(bool error)
 {
     if(error)
     {
         cerr<<"Error: "<<qPrintable(pHttp->errorString())<<endl;
     }
     else
     {
         cout<<"Request Done"<<endl;
     }

     if (pFile !=NULL)
     {
        pFile->flush();
        pFile->close();
        delete pFile;
        pFile = NULL;
     }

     emit finished(error);
 }

void HttpAccess::requestStarted(int id)
{

    //cout<<"bytesAvailable "<<pHttp->bytesAvailable()<<endl;
    if(id == iRequestId)
    {
        cout<<"Request started"<<endl;
    }
    else if (id == iGetId)
    {
        cout<<"Get started"<<endl;
    }
}

void HttpAccess::readyRead(const QHttpResponseHeader& resp)
{
    int len = 0;
    cout<<"bytesAvailable "<<pHttp->bytesAvailable()<<endl;

    QByteArray bytes = pHttp->readAll();
    cout<<"content:\n"<<bytes.data()<<endl;
    len = bytes.length();
    //len = http->read(pData, http->bytesAvailable());
    cout<<"length: "<<len<<endl;
    char *pData = new char(len);
    memcpy(pData, bytes.data(),len);
    if(len > 0)
    {
        emit readed(pData, len);
    }
    else{
        cerr<<"error occurs on reading"<<endl;
    }
    return;
}

void HttpAccess::stateChanged(int state)
{
#if 0
    switch(state)
    {
      case QHttp::Unconnected:
        cout<<"unconnected"<<endl;
      break;
      case QHttp::HostLookup:
        cout<<"HostLookup"<<endl;
      break;
      case QHttp::Connecting:
        cout<<"Connecting"<<endl;
      break;
      case QHttp::Sending:
        cout<<"Sending"<<endl;
      break;
      case QHttp::Connected:
        cout<<"Connected"<<endl;
      break;
      case QHttp::Closing:
        cout<<"Closing"<<endl;
      break;
      default:
        cout<<"unknown state!"<<endl;
      break;
    }
#endif
}

void HttpAccess::httpGetFinished()
{
    //cout<<"Network reply finished"<<endl;
    if (pFile !=NULL)
    {
       pFile->flush();
       pFile->close();
       delete pFile;
       pFile = NULL;
    }
    pReply->deleteLater();
    pReply = NULL;

    //error=false
    emit finished(false);
}

void HttpAccess::httpGetReadyRead()
{
    // this slot gets called every time the QNetworkReply has new data.
    // We read all of its new data and write it into the file.
    // That way we use less RAM than when reading it at the finished()
    // signal of the QNetworkReply
    if (pFile)
        pFile->write(pReply->readAll());
}

void HttpAccess::updateDataReadProgress(qint64 bytesRead, qint64 totalBytes)
{
    static int prevPercent = 0; 
    int percent = static_cast<float>(bytesRead)/static_cast<float>(totalBytes) * 100.00;
    if (percent != prevPercent)
    {
        cout<<"Downloaded "<<bytesRead<<" of totalBytes "<<totalBytes<<", ";
        cout<<percent<<"%"<<endl;
        prevPercent = percent;
    }
}

HttpAccess::~HttpAccess()
{
    delete pHttp;
}

