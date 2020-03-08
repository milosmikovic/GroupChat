#include "SocketClient.h"

SocketClient::SocketClient(qintptr socketfd, QObject *parent) : QTcpSocket(parent),mSockedfd(socketfd){
    setSocketDescriptor(socketfd);
    connect(this, &SocketClient::readyRead,this,&SocketClient::emitReadSig);
    connect(this, &SocketClient::stateChanged,this,&SocketClient::emitChangedSig);
}

//proveriti da li radi ispravno uvek...
void SocketClient::emitReadSig(){
    emit ReadyReadSig(this);
}


//proveriti da li radi ispravno uvek...
void SocketClient::emitChangedSig(int S){
    emit StateChangedSig(this, S);
}

int SocketClient::sockedfd(){
    return mSockedfd;
}
