#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);

   MainWindow w;
   w.show();

    //svaki klijent poziva broadcastAll
//    w.broadcastAll();

    return a.exec();
}
