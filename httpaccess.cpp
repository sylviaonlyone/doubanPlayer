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
#include <QByteArray>
#include "httpaccess.h"

#define ALBUM_PICTURE "picture.jpg"

using namespace std;

HttpAccess::HttpAccess(QObject *parent) : QObject(parent), iRequestId(0), iGetId(0)
{
    pHttp = new QHttp(this);

    connect(pHttp,SIGNAL(requestStarted(int)), this, SLOT(requestStarted(int)));
    connect(pHttp,SIGNAL(readyRead(const QHttpResponseHeader&)), this,
                        SLOT(readyRead(const QHttpResponseHeader&)));
    connect(pHttp, SIGNAL(done(bool)), this, SLOT(done(bool)));
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


    QFile *pFile = new QFile(fileName);
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
    fileVect<<pFile;
    pHttp->close();
}

void HttpAccess::HttpGet(const QUrl& url, const QString& name)
{
    QFileInfo file(name);
    if (file.exists())
    {
        emit finished(false);
        return;
    }
    QFile* pFile = new QFile(name);
    if(!pFile->open(QIODevice::WriteOnly))
    {
        cerr<<"Unable to save the file"<<endl;
        delete pFile;
        pFile = 0;
        return;
    }

    pHttp->setHost(url.host());
    //cout<<"Http Get host "<<qPrintable(url.host())<<endl;
    //cout<<"Http Get path"<<qPrintable(url.path())<<endl;
    iGetId = pHttp->get(url.path(), pFile);
    fileVect<<pFile;
    pHttp->close();
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

     for(int i = 0; i < fileVect.size(); i++)
     {
         QFile* p = fileVect.at(i);
         if (p !=NULL)
         {
            p->close();
            delete p;
            p = NULL;
         }
     }

     fileVect.clear();

     emit finished(error);
 }

void HttpAccess::requestStarted(int id)
{

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

HttpAccess::~HttpAccess()
{
    delete pHttp;
}

