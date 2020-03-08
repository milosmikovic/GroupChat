#include <QCoreApplication>
#include "SocketServer.h"


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

//    pokrecemo server
    SocketServer Server;
    if (!Server.startServer(4567)) {
        qDebug() << "Greska:" << Server.errorString();
        return 1;
    }
    qDebug() << "Server listening :)";
    Server.readClientsData();

    return a.exec();
}
