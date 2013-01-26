/****************************************************************************
* Filename:  mainwindow.cpp
* Description: Define main window display, functionality
* Version:  0.1
* Created:  15/05/2011
* Revision: none
* Compiler: QT
* Author:   Mason Zhang
* Company:  NA
****************************************************************************/

#include <QtGui>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <iostream>
#include "mainwindow.h"

#define RAND_RANGE 16
#define DECIMAL_RANGE 10
#define DOUBAN_URL "http://douban.fm/j/mine/playlist"
#define ALBUM_PICTURE "picture.jpg"
#define PLAYLIST "playlist"

using namespace std;


static QString randomUID = 0;


MainWindow::MainWindow()
{
    mainUi.setupUi(this);
    audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
    mediaObject = new Phonon::MediaObject(this);

    //http access use
    pHttpReqestLong = new HttpAccess;
    pHttpReqestShort = new HttpAccess;
    pHttpGet = new HttpAccess;

    mediaObject->setTickInterval(1000);
    connect(mediaObject, SIGNAL(tick(qint64)), this, SLOT(tick(qint64)));
    connect(mediaObject, SIGNAL(stateChanged(Phonon::State,Phonon::State)),
            this, SLOT(stateChanged(Phonon::State,Phonon::State)));
    connect(mediaObject, SIGNAL(currentSourceChanged(Phonon::MediaSource)),
            this, SLOT(sourceChanged(Phonon::MediaSource)));
    connect(mediaObject, SIGNAL(aboutToFinish()), this, SLOT(aboutToFinish()));
    connect(pHttpReqestLong, SIGNAL(finished(bool)), this, SLOT(processPlaylist(bool)));
    connect(pHttpGet, SIGNAL(finished(bool)), this, SLOT(getFinished(bool)));

    Phonon::createPath(mediaObject, audioOutput);

    setupActions();
    setupUi();
    cout<<"reuest new playlist"<<endl;
    setupPlaylist('n');
}

void MainWindow::stateChanged(Phonon::State newState, Phonon::State oldState)
{
#ifdef _DEBUG_
    cout<<"From "<<oldState<<" To "<<newState<<endl;
#else
    switch (newState) {
        case Phonon::ErrorState:
            if (mediaObject->errorType() == Phonon::FatalError) {
                QMessageBox::warning(this, tr("Fatal Error"),
                mediaObject->errorString());
            } else {
                QMessageBox::warning(this, tr("Error"),
                mediaObject->errorString());
            }
            break;
        case Phonon::BufferingState:
                break;
        default:
            ;
    }

    //set song title and album picture
    int index = sources.indexOf(mediaObject->currentSource());

    //cout<<index<<qPrintable(title[index])<<endl;
    mainUi.titleLabel->setText(title[index]);

    //pHttpGet->HttpGet(picture[index]);
#endif
}

void MainWindow::processPicture()
{
    if(!QPixmap(ALBUM_PICTURE))
        cout<<"Error for creating pixmap!!!"<<endl;

    mainUi.pic->setPixmap(QPixmap(ALBUM_PICTURE));

    //display picture fisrt, then start to play
    mediaObject->play();
}

//!call back of Http get finished signal
void MainWindow::getFinished(bool error)
{
    if(error)
    {
        cerr<<"Http request error!"<<endl;
        return;
    }

    startDownload();
}

//!
void MainWindow::tick(qint64 time)
{
    QTime displayTime(0, (time / 60000) % 60, (time / 1000) % 60);

    mainUi.timeLcd->display(displayTime.toString("mm:ss"));
}

//!
void MainWindow::sourceChanged(const Phonon::MediaSource &source)
{
#ifndef _DEBUG_
    cout<<"sourceChanged"<<endl;
#endif
    mainUi.timeLcd->display("00:00");
}

//!
void MainWindow::aboutToFinish()
{
    int index = sources.indexOf(mediaObject->currentSource());

    //set current song operation type to 'p' (playing)
    type[index] = 'p';

    //send a message to server
    pHttpReqestShort->HttpRequest(QUrl(DOUBAN_URL), setHttpArguments('e'));


    //index to next song
    index++;

    if (sources.size() > index)
        mediaObject->setCurrentSource(sources[index]);
    else
        //get playlist again at the end of list
        setupPlaylist('p');
}
//![16]

void MainWindow::setupActions()
{
    connect(mainUi.nextButton, SIGNAL(released()), this, SLOT(on_nextButton_released()));
    connect(mainUi.trashButton, SIGNAL(released()), this, SLOT(on_nextButton_released()));
}

void MainWindow::setupUi(){
    mainUi.timeLcd->display("00:00");

    mainUi.pic->setScaledContents(true);

    /*mainUi.trashButton->setIcon(QIcon("trash.png"));
    mainUi.trashButton->setIconSize(QSize(50,50));
    //mainUi.trashButton->setAutoDefault(true);

    mainUi.favorButton->setIcon(QIcon("heart.png"));
    mainUi.favorButton->setIconSize(QSize(50,50));
    //mainUi.favorButton->setAutoDefault(true);

    mainUi.nextButton->setIcon(QIcon("next.png"));
    mainUi.nextButton->setIconSize(QSize(50,50));*/
    //mainUi.nextButton->setAutoDefault(true);
}

QByteArray MainWindow::setHttpArguments(const char opType)
{
    /* HTTP POST message format:
    r=0.1237416032899653&
    type=r&
    sid=491899&
    uid=26636761&
    h=491899:r&
    du=9&
    channel=0&res
    
    type:n
    sid:
    pt:0.0
    channel:1001853
    from:mainsite
    r:b310166935
http://douban.fm/j/mine/playlist?type=e&sid=1451667&channel=1002048&pt=250.2&pb=64&from=mainsite&r=d451e382de
    */
    QByteArray arg;

    /*set type, including
    p: playing(?), all songs played, need a new list
    r: rated(?), like current song
    u: unrate, unlike current song
    b: bye(?),
    s: skip,
    e: end, current song end playing
    n: new, return a new list
    */
    arg += "type=";
    arg += opType;
    arg +="&";

    //set sid
    arg += "sid=&";

    //set hard code to a valid channel
    arg += "channel=1002048&";

    //set pt(passed time?)
    arg += "pt=0.0&";

    //set from to main web site
    arg += "from=mainsite&";

    //set r, a 10 hex string
    QTime midnight(0, 0, 0);
    qsrand(midnight.secsTo(QTime::currentTime()));

    if(randomUID == 0)
    {
      randomUID = "r=";

      for(int i = 0; i < DECIMAL_RANGE; i++)
      {
        QString ch;
        // need a hex char
        ch.setNum(qrand()%RAND_RANGE, RAND_RANGE);
        randomUID += ch;
      }
    }
    arg += randomUID;

    switch(opType)
    {
    case 'n':
    //    arg += "&h=";
        break;
    case 's':
    case 'e':
    case 'p':
    {
        int index = sources.indexOf(mediaObject->currentSource());
        arg += "&sid=" + sid[index];

        //if(opType == 's' || opType == 'p')
        //{
        //    //|sid:type
        //    arg += "&h=";
        //    for(int i = 0; i <= index; i++)
        //    {
        //        arg += "|" + sid[i] + ":" + type[i];
        //    }
        //}
        break;
    }
    default:
        break;
    }

    return arg;
}


/*
* Parameter: QString
* Value    : "n" create a new playlist
*            "p" request a new list when finish current list
*/
void MainWindow::setupPlaylist(const char type)
{
#ifdef _DEBUG_
    //Get play list
    pHttpReqestLong->HttpRequest(QUrl(DOUBAN_URL), setHttpArguments(type));

#endif
#ifndef _DEBUG_
    processPlaylist(false);
#endif
}

void MainWindow::processPlaylist(bool error)
{
    cout<<"get playlist, start processing list"<<endl;
    if (error) return;

    //check playlist file
    QFile *pPlaylist = new QFile(PLAYLIST);

    if(!pPlaylist->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        cerr<<"Unable to open the playlist file"<<endl;
        delete pPlaylist;
        pPlaylist = 0;
        return;
    }
    QByteArray content = pPlaylist->readAll();    

    content.replace("\\/","/");
    //find mp3's url
    int indexStart = 0, indexEnd = 0;

    //start point at 6 chars after "url"
    while ((indexStart = content.indexOf("\"url\"", indexStart)) != -1)
    {
        indexStart += 7;
        indexEnd = content.indexOf(",", indexStart) - 1;
        QUrl mp3Url = QUrl(content.mid(indexStart, indexEnd-indexStart));
#ifdef _DEBUG_
        cout<<content.mid(indexStart, indexEnd-indexStart).constData()<<endl;
#endif
        url.append(mp3Url);

        //init type
        type.append(0);
    }

    //find title
    indexStart = 0, indexEnd = 0;

    while ((indexStart = content.indexOf("\"title\"", indexStart)) != -1)
    {
        indexStart += 9;
        indexEnd = content.indexOf(",", indexStart) - 1;
        QString mp3Title = QString(content.mid(indexStart, indexEnd-indexStart));

#ifndef _DEBUG_
        cout<<mp3Title.toAscii().constData()<<endl;
#endif
        title.append(mp3Title); 
    }

    //find year
    indexStart = 0, indexEnd = 0;

    while ((indexStart = content.indexOf("public_time", indexStart)) != -1)
    {
        indexStart += 14;
        indexEnd = content.indexOf(",", indexStart) - 1;
        QString mp3Year = QString(content.mid(indexStart, indexEnd-indexStart));
#ifndef _DEBUG_
        cout<<mp3Year.toAscii().constData()<<endl;
#endif
        year.append(mp3Year);
    }

    //find album
    indexStart = 0, indexEnd = 0;

    while ((indexStart = content.indexOf("albumtitle", indexStart)) != -1)
    {
        indexStart += 13;
        indexEnd = content.indexOf(",", indexStart) - 1;
        QString mp3Album = QString(content.mid(indexStart, indexEnd-indexStart));
#ifndef _DEBUG_
        cout<<mp3Album.toAscii().constData()<<endl;
#endif
        album.append(mp3Album);
    }

    //find artist
    indexStart = 0, indexEnd = 0;

    while ((indexStart = content.indexOf("artist", indexStart)) != -1)
    {
        indexStart += 9;
        indexEnd = content.indexOf(",", indexStart) - 1;
        QString mp3Artist = QString(content.mid(indexStart, indexEnd-indexStart));
#ifdef _DEBUG_
        cout<<mp3Artist.toAscii().constData()<<endl;
#endif
        artist.append(mp3Artist);
    }

    //find picture
    indexStart = 0, indexEnd = 0;

    while ((indexStart = content.indexOf("picture", indexStart)) != -1)
    {
        indexStart += 10;
        indexEnd = content.indexOf(",", indexStart) - 1;
        QString pic = QString(content.mid(indexStart, indexEnd-indexStart));
#ifndef _DEBUG_
        cout<<pic.toAscii().constData()<<endl;
#endif
        picture.append(pic);
    }

    //find sid
    indexStart = 0, indexEnd = 0;

    while ((indexStart = content.indexOf("\"sid\"", indexStart)) != -1)
    {
        indexStart += 7;
        indexEnd = content.indexOf(",", indexStart) - 1;
        QString id = QString(content.mid(indexStart, indexEnd-indexStart));
#ifdef _DEBUG_
        cout<<id.toAscii().constData()<<endl;
#endif
        sid.append(id);
    }

    pPlaylist->close();
    delete pPlaylist;
    pPlaylist = 0;

    startDownload();
    //startPlaylist();
}

void MainWindow::startPlaylist()
{
    QList<QUrl>::const_iterator iter;
    for(iter = url.begin(); iter != url.end();++iter)
    {
        Phonon::MediaSource source(*iter);
        sources.append(source);
    }

    //mediaObject->stop();
    //mediaObject->clearQueue();

    //mediaObject->setCurrentSource(QString("http://robtowns.com/music/blind_willie.mp3"));
    mediaObject->setCurrentSource(QString("/home/oasis/viva_la_vida.mp3"));
    //mediaObject->play();
    //mediaObject->setCurrentSource(sources[0]);
}

//Save mp3 files to local path, then play files
void MainWindow::startDownload()
{
    static QList<QUrl>::const_iterator urlIter = url.begin();
    static QList<QString>::const_iterator titleIter = title.begin();

    if((*urlIter).isEmpty() || !(*urlIter).isValid() || (*titleIter).isEmpty())
    {
        cerr<<"Input URL or Title invalid!"<<endl;
        return;
    }

    if(urlIter != url.end() && titleIter != title.end())
    {
        cout<<"dowloading mp3 "<<(*titleIter).toAscii().constData()<<endl;

        QFileInfo fileInfo((*urlIter).path());
        QString fileName = fileInfo.fileName();
        pHttpGet->HttpGet(*urlIter, fileName);
    }
    else
    {
        ++urlIter;
        ++titleIter;
    }
}

void MainWindow::on_nextButton_released()
{

    if(mainUi.nextButton->isDown())
        return;

    mainUi.nextButton->setDown(true);

    int index = sources.indexOf(mediaObject->currentSource());

    //set current song operation type to 's' (skiped)
    type[index] = 's';

    //send a message to server
    pHttpReqestShort->HttpRequest(QUrl(DOUBAN_URL), setHttpArguments('s'));

    //index to next song
    index++;

    if (sources.size() > index)
        mediaObject->setCurrentSource(sources[index]);
    else
        //get playlist again at the end of list
        setupPlaylist('p');
}

MainWindow::~MainWindow()
{
    delete audioOutput;
    delete mediaObject;
    delete pHttpReqestLong;
    delete pHttpReqestShort;
    delete pHttpGet;

    QFile files;
    files.remove(ALBUM_PICTURE);  //remove picture.jpg
    //files.remove(PLAYLIST);       //remove playlist
}
