#ifndef SOCKETCLIENT_H
#define SOCKETCLIENT_H

#include <QTcpSocket>


class SocketClient : public QTcpSocket{
    Q_OBJECT
public:
    SocketClient(qintptr socketfd, QObject *parent = nullptr);
    int sockedfd();
public slots:
    void emitReadSig();
    void emitChangedSig(int S);
signals:
    void ReadyReadSig(SocketClient *);
    void StateChangedSig(SocketClient *, int);
private:
    int mSockedfd;
};

#endif // SOCKETCLIENT_H
