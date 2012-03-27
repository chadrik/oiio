#include "mainwindow.h"
#include <QtGui/QApplication>
#include "mainwindow.h"
#include <iostream>
int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("OpenSource");
    QCoreApplication::setOrganizationDomain("oiio.org");
    QCoreApplication::setApplicationName("Frame Buffer");

    QApplication a(argc, argv);
    //a.setStyle("plastique");
    MainWindow w;
    w.show();
    return a.exec();
}
