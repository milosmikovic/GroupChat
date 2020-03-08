#include "SocketServer.h"
#include "SocketClient.h"
#include <QTextStream>
#include <QDebug>

SocketServer::SocketServer(QObject *parent)
    : QTcpServer(parent)
{
}

bool SocketServer::startServer(quint16 port)
{
    return listen(QHostAddress::Any, port);
}

void SocketServer::readClientsData(){
    QFile clientsData("clients.txt");
    clientsData.open(QFile::ReadOnly | QFile::Text);
    QTextStream in(&clientsData);
    QString line;
    while(true){
        line = in.readLine();
        if(line.isEmpty()){
            break;
        }
        QList<QString> splitedLine = line.split(":");
        QString nickname = splitedLine[0];
        QString username = splitedLine[1];
        QString password = splitedLine[2];

        clientData tmpData = {
            nullptr,
            nickname,
            username,
            password
        };

        mClientsData.append(tmpData);
    }

    for(auto &i : mClientsData){
        qDebug() << i.clientSocket << i.clintNickname << i.clientUsername << i.clientPassword;
    }
}

void SocketServer::incomingConnection(qintptr socketfd)
{
    qDebug() << "Povezan klijent sa fd:" << socketfd;
    auto socket = new SocketClient(socketfd, this);
    mSockets << socket;

    connect(socket, &SocketClient::ReadyReadSig,this,&SocketServer::broadcastAll);
    connect(socket, &SocketClient::StateChangedSig,this,&SocketServer::clientDisconected);
}

void SocketServer::broadcastAll(SocketClient *client){
    qDebug() << "ReadyRead client:" << client;
    QTextStream S(client);
    auto text = S.readAll();

    if(text.startsWith("[accCheck]")){
        auto socketTmp = client;
        mSockets.removeOne(client);
        mAccSockets << socketTmp;
        qDebug() << text;
        QList<QString> splited = text.split(":");
        QString clientNickname = splited[1];
        QString clientUsername = splited[2];
        QString clientPassword = splited[3];

        qDebug() << clientNickname << clientUsername << clientPassword;

        for(auto &i : mClientsData){
            if(clientNickname.compare(i.clintNickname) == 0 or clientPassword.compare(i.clientPassword) == 0
                     or clientUsername.compare(i.clientUsername) == 0){
                S << "INCORRECT";
                client->flush();
                return;
            }
        }

        QFile clientDataWrite("clients.txt");
        clientDataWrite.open(QFile::ReadWrite | QFile::Text | QFile::Append);
        QTextStream writeStream(&clientDataWrite);
        writeStream << clientNickname;
        writeStream << ":";
        writeStream << clientUsername;
        writeStream << ":";
        writeStream << clientPassword << "\n";
        writeStream.flush();
        clientDataWrite.close();

        clientData tmpData = {
            nullptr,
            clientNickname,
            clientUsername,
            clientPassword
        };

        mClientsData.append(tmpData);
        S << "OK";
        client->flush();
    }else if(text.startsWith("[logCheck]")){
        qDebug() << "LOG CHECK!";
        QList<QString> splited = text.split(":");
        QString user = splited[1];
        QString pass = splited[2];

        for(auto &i : mClientsData){
            if(i.clientUsername.compare(user) == 0 and i.clientPassword.compare(pass) == 0){
                if(i.clientSocket != nullptr){
                    qDebug() << "Nije se poklopio user jer je vec ulogovan na servis!";
                    mSockets.removeOne(client);
                    mAccSocketsDeclined << client;
                    S << "[logDeclinedInUse]";
                    client->flush();
                    return;
                }
                qDebug() << "Poklopio se USER!";
                i.clientSocket = client;

                //saljemo ulogovanom sve trenutne online
                QString onlineUsers = "";
                for(const auto &j: mClientsData){
                    if(j.clientSocket != nullptr and j.clientSocket != i.clientSocket){
                        onlineUsers.append(j.clintNickname);
                        onlineUsers.append(":");
                    }
                }
                S << "[logAccepted]:" << i.clintNickname << ":" << onlineUsers;
                client->flush();

                //saljemo ostalima novog ulogovanog
                for (const auto &j : mClientsData) {
                    if(j.clientSocket != client and j.clientSocket != nullptr){
                        QTextStream K(j.clientSocket);
                        K << "[NewClientOnline]:" << i.clintNickname;
                        j.clientSocket->flush();
                    }
                }
                return;
            }
            if(i.clientUsername.compare(user) == 0 and i.clientPassword.compare(pass) != 0){
                //postoji user ali je pogresna sifra
                qDebug() << "Poklopio se USER ali PASSWD pogresan!";
                mSockets.removeOne(client);
                mAccSocketsDeclined << client;
                S << "[PasswdIncorrect]:";
                client->flush();
                return;
            }

        }
        qDebug() << "Nije se poklopio user zbog nepostojeceg usra i pass!";
        mSockets.removeOne(client);
        mAccSocketsDeclined << client;
        S << "[logDeclinedUsrPas]";
        client->flush();
        return;
    }else if(text.startsWith("[deleteAcc]")){
        qDebug() << "**DELETE ACC" << text;
        auto socketTmp = client;
        mSockets.removeOne(client);
        mAccDeleted << socketTmp;

        QList<QString> splited = text.split(":");
        QString user = splited[1];
        QString pass = splited[2];

        //podaci posle eventualnog brisanja klijenta
        QList<QString> newData;
        //klijent za brisanje, ako se poklopi sve...
        clientData removedOne;

        for(auto &i : mClientsData){
            if(user.compare(i.clientUsername) == 0 and pass.compare(i.clientPassword) == 0){
                if(i.clientSocket != nullptr){
                    //acc nije izbrisan jer je klijent aktivan na njemu
                    S << "[activeAcc]";
                    client->flush();
                    return;
                }
                removedOne = i;
            }else{
                newData.append(i.clintNickname + ":" + i.clientUsername + ":" + i.clientPassword + "\n");
            }
        }
        //acc nije izbrisan jer ne postoji takav, tj. sa tim userom i pass
        if(!mClientsData.removeOne(removedOne)){
            S << "[accNotDeleted]";
            client->flush();
            return;
        }

        QFile f("clients.txt");
        f.open(QFile::ReadWrite | QFile::Text | QFile::Truncate);
        QTextStream in(&f);
        for(auto &i : newData){
            in << i;
        }
        S << "[accDeleted]";
        client->flush();
    }
    else{
        QString tmpNickname;
        for(const auto &i : mClientsData){
            if(i.clientSocket == client){
                tmpNickname = i.clintNickname;
            }
        }

        for (const auto &i : mClientsData) {
            if(i.clientSocket != client and i.clientSocket != nullptr){
                QTextStream K(i.clientSocket);
                K << "[" << tmpNickname << "]:" << text;
                i.clientSocket->flush();
            }
        }
    }
}


void SocketServer::clientDisconected(SocketClient *S, int ST){

    qDebug() << "Klijent je promenio stanje fd:"
             << S->sockedfd();
    if (ST == QTcpSocket::UnconnectedState) {
        qDebug() << "Klijent je diskonektovan:"
                 << S->sockedfd();
        if(mSockets.removeOne(S)){
            qDebug() << "Obrisan je clientSocket!";
            clientData pom;
            for(auto &i : mClientsData){
                if(i.clientSocket == S){
                    i.clientSocket = nullptr;
                    pom = i;
                    break;
                }
            }
            for (const auto &i : mSockets) {
                QTextStream K(i);
                K << "[clientDisconected]:"
                  << pom.clintNickname;
                i->flush();
                K.flush();
            }

        }else{
            if(mAccSockets.removeOne(S)){
                qDebug() << "Obrisan je accMake socket!";
            }
            if(mAccSocketsDeclined.removeOne(S)){
                qDebug() << "Obrisan je accMakeDeclined socket!";
            }
            if(mAccDeleted.removeOne(S)){
                qDebug() << "Obrisan je accDelete socket!";
            }
        }
    }
}
