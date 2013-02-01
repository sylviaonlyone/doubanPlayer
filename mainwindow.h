/****************************************************************************
* Filename:  httpaccess.h
* Description: Declare MainWindow class
* Version:  0.1
* Created:  05/15/2011
* Revision: none
* Compiler: QT
* Author:   Mason Zhang
* Company:  NA
****************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <phonon/audiooutput.h>
#include <phonon/audiodataoutput.h>
#include <phonon/seekslider.h>
#include <phonon/mediaobject.h>
#include <phonon/volumeslider.h>
#include <phonon/backendcapabilities.h>
#include <QList>
#include <QtGlobal>

#include "httpaccess.h"
#include "ui_mainwindow.h"

QT_BEGIN_NAMESPACE
class QAction;
class QTableWidget;
class QLCDNumber;
QT_END_NAMESPACE

//![0]

class MainWindow : public QMainWindow
{
//![0]
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

private slots:
    void on_nextButton_released();
    void stateChanged(Phonon::State newState, Phonon::State oldState);
    void tick(qint64 time);
    void sourceChanged(const Phonon::MediaSource &source);
    void aboutToFinish();

    void processPlaylist(bool);
    void processPicture();

    void getFinished(bool error);

private:
//!UI objects
    Ui::MainWindow mainUi;
    void setupActions();
    void setupUi();
    void setupPlaylist(const char);
    void startPlaylist();
    void startDownload();
    QByteArray setHttpArguments(const char);

//!Phonon objects
    Phonon::SeekSlider *seekSlider;
    Phonon::MediaObject *mediaObject;
    Phonon::AudioOutput *audioOutput;
    Phonon::VolumeSlider *volumeSlider;
    QList<Phonon::MediaSource> sources;
//!Playlist data objects
    HttpAccess *pHttpReqestLong;    //send long request, will receive a playlist
    HttpAccess *pHttpReqestShort;   //send short request, nothing recieve
    HttpAccess *pHttpGet;           //get data from URL
    QList<QUrl> url;
    QList<QUrl> picture;
    QList<QString> artist;
    QList<QString> year;
    QList<QString> title;
    QList<QString> album;
    QList<QString> sid;
    QList<QString> type; //this is user operation type, not directly from server
    QList<quint16> length; //length in seconds

//!misc objects
    QString currentFile;
};

#endif
