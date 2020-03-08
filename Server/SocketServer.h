#ifndef SOCKETSERVER_H
#define SOCKETSERVER_H

#include <QTcpServer>
#include <QMap>
#include <QFile>

struct clientData{
    QTcpSocket *clientSocket;
    QString clintNickname;
    QString clientUsername;
    QString clientPassword;


    //morao da se napravi ovaj op da bi mogli da vrsimo QList::removeOne()
    bool operator==(const clientData &other){
        if(clientSocket == other.clientSocket and clintNickname.compare(other.clintNickname) == 0 and
                clientUsername.compare(other.clientUsername) == 0 and clientPassword.compare(other.clientPassword) == 0){
            return true;
        }else{
            return false;
        }
    }
};

class SocketClient;

class SocketServer : public QTcpServer
{
public:
    SocketServer(QObject *parent = nullptr);
    bool startServer(quint16 port);
    void readClientsData();

private slots:
    void broadcastAll(SocketClient *client);
    void clientDisconected(SocketClient *S, int ST);
protected:
    void incomingConnection(qintptr handle);
private:
    QList<SocketClient *> mSockets;
    QList<SocketClient *> mAccSockets;
    QList<SocketClient *> mAccSocketsDeclined;
    QList<SocketClient *> mAccDeleted;
    QList<clientData> mClientsData;
};

#endif // SOCKETSERVER_H
