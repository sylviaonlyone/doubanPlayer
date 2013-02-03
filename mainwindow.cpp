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
#define PLAYLIST "playlist"
#define LIST_SIZE() (sid.size())

using namespace std;


static QString randomUID = 0;


MainWindow::MainWindow()
  : index(0)
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
    setupPlaylist('n');
    //pHttpGet->HttpGet(QUrl(QString("http://robtowns.com/music/blind_willie.mp3")),
    //                  QString("blind_willie.mp3"));

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
    //if(!QPixmap(ALBUM_PICTURE))
    //    cout<<"Error for creating pixmap!!!"<<endl;

    //mainUi.pic->setPixmap(QPixmap(ALBUM_PICTURE));

    //display picture fisrt, then start to play
    //mediaObject->play();
}

//!call back of Http get finished signal
void MainWindow::getFinished(bool error)
{
    if(error)
    {
        cerr<<"Http request error!"<<endl;
        return;
    }

    //cout<<"Get finished, playing"<<endl;
    startPlaylist();
    startDownload();
}

//!
void MainWindow::tick(qint64 time)
{
    QTime displayTime(0, (time / 60000) % 60, (time / 1000) % 60);

    mainUi.timeLcd->display(displayTime.toString("mm:ss"));
}

//!send Http request at the end of each song playing
void MainWindow::sourceChanged(const Phonon::MediaSource &source)
{
#ifdef _DEBUG_
    cout<<"sourceChanged"<<endl;
#endif
    mainUi.timeLcd->display("00:00");

    pHttpReqestShort->HttpRequest(QUrl(DOUBAN_URL), setHttpArguments('e'));
}

//!the queue is empty, but it doesn't have to mean all songs are played, so we still pass 'e' to setHttpArguments()
void MainWindow::aboutToFinish()
{
    cout<<"Play about to finish at:"<<index<<endl;
    if (index < LIST_SIZE())
    {
      pHttpReqestShort->HttpRequest(QUrl(DOUBAN_URL), setHttpArguments('e'));
    }
    else if (index == LIST_SIZE())
    {
      cout<<"request a new list!"<<endl;
      //Get a new play list, reset all interal data
      clearPlaylist();
      pHttpReqestLong->HttpRequest(QUrl(DOUBAN_URL), setHttpArguments('p'));
    }
    //int index = sources.indexOf(mediaObject->currentSource());

    ////set current song operation type to 'p' (playing)
    //type[index] = 'p';
    ////send a message to server
    ////index to next song
    //index++;
    //if (sources.size() > index)
    //    mediaObject->setCurrentSource(sources[index]);
    //else
    //    //get playlist again at the end of list
    //    setupPlaylist('p');
}

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

//! set Http request header.
//available OpType inputs are:
//n: request new list
//e: end of playing current source
//Note: p(request a new list again) is config inside this function
QByteArray MainWindow::setHttpArguments(char opType)
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
    pt:0.0    passed time?
    pb:64     passed bits?
    channel:1001853
    from:mainsite
    r:b310166935
    http://douban.fm/j/mine/playlist?type=e&sid=1451667&channel=1002048&pt=250.2&pb=64&from=mainsite&r=d451e382de
    */
    QByteArray arg;

    //set hard code to a valid channel
    //1000947 : So quit to sleep 
    arg += "channel=1000947&";

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

    //set from to main web site
    arg += "from=mainsite&";

    switch(opType)
    {
      case 'n':
      {
        arg += "sid=&";
        //set pt(passed time?)
        arg += "pt=0.0&";
      }
      break;
      case 's':
      case 'e':
      case 'p':
      {
        arg += "sid=" + sid[index];
        arg += "&";

        arg += "pt=" + QString::number(length[index]);
        arg += "&";
        if (opType == 'p')
        {
          //we have go to the end of the list,
          //request a new list at this point.
          //reset index
          index = 0;
        }
        ++index;

        arg += "pb=64&";
          //if(opType == 's' || opType == 'p')
          //{
          //    //|sid:type
          //    arg += "&h=";
          //    for(int i = 0; i <= index; i++)
          //    {
          //        arg += "|" + sid[i] + ":" + type[i];
          //    }
          //}
      }
      break;
      default:
      {
        cerr<<"Un-supported op-type:"<<opType<<" detected!!!"<<endl;
      }
      break;
    }

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
        QString mp3Title = QString::fromUtf8(content.mid(indexStart, indexEnd-indexStart).constData());

#ifdef _DEBUG_
        cout<<mp3Title.toUtf8().constData()<<endl;
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
#ifdef _DEBUG_
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
#ifdef _DEBUG_
        cout<<mp3Album.toUtf8().constData()<<endl;
#endif
        album.append(mp3Album);
    }

    //find artist
    indexStart = 0, indexEnd = 0;

    while ((indexStart = content.indexOf("artist", indexStart)) != -1)
    {
        indexStart += 9;
        indexEnd = content.indexOf(",", indexStart) - 1;
        QString mp3Artist = QString::fromUtf8(content.mid(indexStart, indexEnd-indexStart));
#ifdef _DEBUG_
        cout<<mp3Artist.toUtf8().constData()<<endl;
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
#ifdef _DEBUG_
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

    //find length in seconds
    indexStart = 0, indexEnd = 0;

    while ((indexStart = content.indexOf("\"length\"", indexStart)) != -1)
    {
        indexStart += 9;
        indexEnd = content.indexOf(",", indexStart);
        QString len = QString(content.mid(indexStart, indexEnd-indexStart));
#ifdef _DEBUG_
        cout<<len.toAscii().constData()<<endl;
#endif
        bool ok;
        length.append(len.toUShort(&ok));
    }

    pPlaylist->close();
    delete pPlaylist;
    pPlaylist = 0;

    startDownload();
}

void MainWindow::startPlaylist()
{
    QFileInfo file(currentFile);
    if (!currentFile.isEmpty() && file.exists())
    {
        //if no source, set current souce, otherwise, add to queue
        if (mediaObject->currentSource().fileName().isEmpty())
        {
          mediaObject->setCurrentSource(currentFile);
        }
        else
        {
          mediaObject->enqueue(currentFile);
        }
        //cout<<"media name: "<<mediaObject->currentSource().fileName().toUtf8().constData()<<endl;
        cout<<"media name:"<<currentFile.toUtf8().constData()<<endl;
        if (mediaObject->state() == Phonon::StoppedState)
          mediaObject->play();
    }
    //else
    //{
    //    cerr<<"Play cannot find file:"<<currentFile.toUtf8().constData()<<endl;
    //}
}

//Save mp3 files to local path, then play files
void MainWindow::startDownload()
{
    static QList<QUrl>::const_iterator urlIter = url.begin();
    static QList<QString>::const_iterator titleIter = title.begin();

    if(urlIter != url.end() && titleIter != title.end())
    {
        if((*urlIter).isEmpty() || !(*urlIter).isValid() || (*titleIter).isEmpty())
        {
            cerr<<"Input URL or Title invalid!"<<endl;
            return;
        }

        currentFile = (*titleIter) + QString(".mp3");
        QUrl currentUrl = *urlIter;
        ++urlIter;
        ++titleIter;
        //cout<<"dowloading "<<currentFile.toUtf8().constData()<<endl;
        QFileInfo file(currentFile);
        if (!file.exists())
        {
          pHttpGet->HttpGet(currentUrl, currentFile);
        //pHttpGet->HttpGet(QUrl(QString("http://robtowns.com/music/blind_willie.mp3")),
        //                  QString("blind_willie.mp3"));
        }
        else
        {
          //the file already downloader, start to play
          getFinished(false);
        }
    }
    else
    {
      cout<<"download to end, stopped!"<<endl;
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

//!clear current playlist containers, all interal data, be prepare for
//a new list coming
void MainWindow::clearPlaylist()
{
    url.clear();
    picture.clear();
    artist.clear();
    year.clear();
    title.clear();
    album.clear();
    sid.clear();
    type.clear();
    length.clear();

    QFile playlist(PLAYLIST);
    playlist.remove();

}

MainWindow::~MainWindow()
{
    delete audioOutput;
    delete mediaObject;
    delete pHttpReqestLong;
    delete pHttpReqestShort;
    delete pHttpGet;

    QFile files;
#ifndef _DEBUG_
    files.remove(PLAYLIST);       //remove playlist
#endif
}
