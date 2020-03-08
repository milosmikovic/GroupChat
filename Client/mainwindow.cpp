#include "mainwindow.h"
#include <regex>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

//---pozadina aplikacije
    QPixmap background("../images/b3.jpg");
    background = background.scaled(this->size());
    QPalette palette;
    palette.setBrush(QPalette::Background, background);
    this->setPalette(palette);
//---

//prvo je vidljiv loginPage
    ui->stackedWidget->setCurrentWidget(ui->loginPage);

}
MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::broadcastAll(){
    QTextStream T(mSocket);
    T << "cao svima!!!";
    mSocket->flush();
}


void MainWindow::on_clear_clicked()
{
    //brise sadrzaj svih polja..
    ui->hostname->clear();
    ui->username->clear();
    ui->password->clear();
    ui->error_msg_line_2->clear();
    ui->port->setValue(0);
}

void MainWindow::fromServer(){
    QTextStream T(mSocket);
    auto text = T.readAll();

    if(text.startsWith("[clientDisconected]")){
            QList<QString> tmpList = text.split(":");
            QString nickLogOut = tmpList[1];

            QString disconectedMsg = "";
            disconectedMsg.append("User:[");
            disconectedMsg.append(nickLogOut);
            disconectedMsg.append("] disconnected...");
            ui->textBox->append(disconectedMsg);

            if(mOnlineUsers.removeOne(nickLogOut)){
                ui->OnlineUsersBox->clear();
                for(const auto &i : mOnlineUsers){
                    ui->OnlineUsersBox->append(i);
                }
            }else{
                qDebug() << "BUG!";
            }
    }
    else if(text.startsWith("[logAccepted]")){
        QList<QString> tmpList = text.split(":");
        mNicknameLog = tmpList[1];
        qDebug() << tmpList.size();
        for(auto i=2;i<tmpList.size();i++){
            if(!tmpList[i].isEmpty()){
                mOnlineUsers.append(tmpList[i]);
            }
        }

        qDebug() << mOnlineUsers << " ***logAccepted**";
        ui->stackedWidget->setCurrentWidget(ui->chatPage);
        for(const auto &i : mOnlineUsers){
            ui->OnlineUsersBox->append(i);
        }
        broadcastAll();
    }
    else if(text.startsWith("[NewClientOnline]")){
         QList<QString> tmpList = text.split(":");
         QString newUserOnline = tmpList[1];
         mOnlineUsers.append(newUserOnline);
         qDebug() << mOnlineUsers << " ***newClientsOnline**";
         ui->OnlineUsersBox->clear();
         for(const auto &i : mOnlineUsers){
             ui->OnlineUsersBox->append(i);
         }

    }
    else if(text.startsWith("[logDeclinedUsrPas]")){
        qDebug() << "USER PASS INCORECT!";
        ui->error_msg_line_2->setText("Missing account ...");
        mSocket->disconnectFromHost();
    }else if(text.startsWith("[logDeclinedInUse]")){
        mSocket->disconnectFromHost();
        ui->error_msg_line_2->setText("User is already active ...");
    }
    else if(text.startsWith("[PasswdIncorrect]:")){
         mSocket->disconnectFromHost();
         ui->error_msg_line_2->setText("Wrong password");
    }
    else{
        ui->textBox->append(text);
    }
}

void MainWindow::on_connect_button_clicked()
{
    //konektovanje na server i prikaz ChatBoxa..
    if(ui->username->text() == "" || ui->password->text() == "" || ui->hostname->text() == ""
             || ui->port->text() == ""){
        ui->error_msg_line_2->setText("You must enter all fields ...");
        return;
    }

    QString host = ui->hostname->text();
    qint16 port = ui->port->value();
    mSocket = new QTcpSocket(this);
    mSocket->connectToHost(host,port);

    mUsernameLog = ui->username->text();
    mPasswordLog = ui->password->text();

    connect(mSocket,&QTcpSocket::connected,this,&MainWindow::connectSuccesful);
    connect(mSocket, &QTcpSocket::readyRead,this,&MainWindow::fromServer);
}

void MainWindow::connectSuccesful(){
    QTextStream T(mSocket);
    T << "[logCheck]:" << mUsernameLog << ":" << mPasswordLog;
    mSocket->flush();
}

void MainWindow::on_send_clicked(){
    QString msg = ui->message->text();
    if(!msg.isEmpty()){
        QTextStream T(mSocket);
        T << msg;
        mSocket->flush();
        QString tmp = "[";
        tmp.append(mNicknameLog);
        tmp.append("]:");
        tmp.append(msg);
        ui->textBox->append(tmp);
    }
    ui->message->clear();
    ui->message->setFocus();
}

void MainWindow::on_signUp_clicked()
{
    //Otvara se prozor za prijavljivanje novih clanova :)
    on_clear_clicked();
    ui->stackedWidget->setCurrentWidget(ui->SignUpPage);
}

void MainWindow::on_buttonBox_accepted()
{
    //regex za sifru: min 8 karaktera, bar 1 veliko i jedno malo slovo i bar jedan broj, bez specijalnih karaktera:
    //^(?=.*[a-z])(?=.*[A-Z])(?=.*\d)[a-zA-Z\d]{8,}$

    //regex za username: izmedju 5 i 12 karaktera, mogu velika, mala slova i brojevi
    //[a-zA-Z\d]{5,12}

   // QRegExp userNameRegex("[a-zA-Z0-9]{5,12}");
    //QRegExp passwordRegex("^(?=.*[a-z])(?=.*[A-Z])(?=.*0-9)[a-zA-Z0-9]{8,}$");

    //Potvrda novog naloga (Treba napraviti novi nalog korisnika)

    //KOM
    mNickname = ui->nickname_line->text();
    mUsername = ui->username_line->text();
    mPassword = ui->password_line->text();

    QRegularExpression userNameRegex("[a-zA-Z0-9]{5,12}");
    QRegularExpression passwordRegex("^(?=.*[0-9])(?=.*[a-z])(?=.*[A-Z]).{8,}$");

    QRegularExpressionMatch matchNickName = userNameRegex.match(mNickname);
    QRegularExpressionMatch matchUserName = userNameRegex.match(mUsername);
    QRegularExpressionMatch matchPassword = passwordRegex.match(mPassword);
    //KOM

    if(mNickname.isEmpty() or mUsername.isEmpty() or mPassword.isEmpty()){
        qDebug() << "Nickname,user,pass: " <<  mNickname << mUsername << mPassword;
        on_buttonBox_rejected();
        ui->stackedWidget->setCurrentWidget(ui->SignUpPage);
        ui->error_msg_line->setText("You must enter all three fields ...");
    }

    //KOM
    else if(!matchNickName.hasMatch() or !matchUserName.hasMatch()){
        qDebug() << "Nickname,username:" <<  mNickname << mUsername;
        on_buttonBox_rejected();
        ui->stackedWidget->setCurrentWidget(ui->SignUpPage);
        ui->error_msg_line->setText("nick and username must be between 5 and 12 characters!");
    }
    else if(!matchPassword.hasMatch()){
        qDebug() << "Password:" <<  mPassword;
        on_buttonBox_rejected();
        ui->stackedWidget->setCurrentWidget(ui->SignUpPage);
        ui->error_msg_line->setText("pass must have min.8 characters,a capital letter and number!");
    }
    //KOM
    else{
        qDebug() << "Nickname,user,pass: " <<  mNickname << mUsername << mPassword;
        //TEST
        mSocketTmp = new QTcpSocket(this);
        mSocketTmp->connectToHost("localhost",4567);
        connect(mSocketTmp, &QTcpSocket::readyRead,this,&MainWindow::fromServerAccCheck);
        QTextStream T(mSocketTmp);
        T << "[accCheck]:" << mNickname << ":" << mUsername << ":" << mPassword;
        mSocketTmp->flush();
        //TEST
        ui->stackedWidget->setCurrentWidget(ui->loginPage);
    }
}

void MainWindow::fromServerAccCheck(){
    QTextStream T(mSocketTmp);
    auto text = T.readAll();
    if(text.startsWith("OK")){
        qDebug() << "OK!!!";
    }else{
        on_buttonBox_rejected();
        ui->stackedWidget->setCurrentWidget(ui->SignUpPage);
        ui->error_msg_line->setText("Acc vec postoji...");
    }
    mSocketTmp->disconnectFromHost();
}

void MainWindow::on_buttonBox_rejected()
{
    //Korisnik je odustao od pravljenja naloga
    //Vracamo se na glavni prozor aplikacije
    ui->password_line->clear();
    ui->username_line->clear();
    ui->nickname_line->clear();
    ui->error_msg_line->clear();
    ui->stackedWidget->setCurrentWidget(ui->loginPage);
}

void MainWindow::on_deleteAcc_button_clicked()
{
    //otvara se prozor za brisanje naloga
    on_clear_clicked();
    ui->stackedWidget->setCurrentWidget(ui->DeleteAccountPage);
}

void MainWindow::on_buttonBox_2_accepted()
{
    //Brise se nalog korisnika
    mSocketDeleteAcc = new QTcpSocket(this);
    mSocketDeleteAcc->connectToHost("localhost",4567);
    connect(mSocketDeleteAcc, &QTcpSocket::readyRead,this,&MainWindow::fromServerDeleteAcc);
    QTextStream T(mSocketDeleteAcc);
    QString deleteUser = ui->deleteUser_line->text();
    QString deletePass = ui->deletePasswd_line->text();
    T << "[deleteAcc]:" << deleteUser << ":" << deletePass;
    mSocketDeleteAcc->flush();
    ui->stackedWidget->setCurrentWidget(ui->loginPage);
}

void MainWindow::fromServerDeleteAcc(){
    QTextStream T(mSocketDeleteAcc);
    auto text = T.readAll();
    if(text.startsWith("[accDeleted]")){
        qDebug() << "*****USPESNO OBRISAN ACC*****!!!";
    }else if(text.startsWith("[activeAcc]")){
        ui->stackedWidget->setCurrentWidget(ui->DeleteAccountPage);
        ui->deleteUser_line->clear();
        ui->deletePasswd_line->clear();
        ui->error_msg_line_3->setText("The account is active, cannot be deleted!");
    }else{
        ui->stackedWidget->setCurrentWidget(ui->DeleteAccountPage);
        ui->deleteUser_line->clear();
        ui->deletePasswd_line->clear();
        ui->error_msg_line_3->setText("Missing account, deletion failed!");
    }
    mSocketDeleteAcc->disconnectFromHost();
}

void MainWindow::on_buttonBox_2_rejected()
{
    //Korisnik odustao od brisanja naloga
    //Vracamo se na glavni prozor
    ui->deleteUser_line->clear();
    ui->deletePasswd_line->clear();
    ui->error_msg_line_3->clear();
    ui->stackedWidget->setCurrentWidget(ui->loginPage);
}


void MainWindow::on_Back_button_clicked()
{   //Back dugmence na deleteAccPage
    ui->stackedWidget->setCurrentWidget(ui->loginPage);
}

void MainWindow::on_Back_button_2_clicked()
{   //Back dugmence na SignUpPage
    ui->stackedWidget->setCurrentWidget(ui->loginPage);
}
