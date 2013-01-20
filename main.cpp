/****************************************************************************
* Filename:  main.cpp
* Description: Play music from http:://douban.fm on mobile phone
* Version:  0.1
* Created:  05/15/2011
* Revision: none
* Compiler: QT
* Author:   Mason Zhang
* Company:  NA
****************************************************************************/

#include <QtGui>
#include "mainwindow.h"

//![1]
int main(int argv, char **args)
{
    QApplication app(argv, args);
    app.setApplicationName("Music Player");
    app.setQuitOnLastWindowClosed(true);

    MainWindow window;
#if defined(Q_OS_SYMBIAN)
    window.showMaximized();
#else
    window.show();
#endif   
    return app.exec(); 
}
//![1]
