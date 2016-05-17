#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    ui->connect->setEnabled(true);
    ui->disconnect->setEnabled(false);
    ui->send->setEnabled(false);

    socket = new QTcpSocket(this);
    connect(socket, SIGNAL(readyRead()), this, SLOT(Ready()));
    connect(socket, SIGNAL(connected()), this, SLOT(Connected()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(Disconnected()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(SocketError(QAbstractSocket::SocketError)));
}

void Dialog::on_connect_clicked()
{
    socket->connectToHost(ui->host->text(),ui->port->text().toShort());
}

void Dialog::SocketError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::information(this, "Error", "The host was not found");
        break;
    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::information(this, "Error", "The connection was refused by the peer.");
        break;
    default:
        QMessageBox::information(this, "Error", "The following error occurred: "+socket->errorString());
    }
}

void Dialog::Connected()
{
        ui->connect->setEnabled(false);
        ui->disconnect->setEnabled(true);
        ui->send->setEnabled(false);
        blockSize = 0;
        ui->msgLog->clear();
        AddToLog("Connected to "+socket->peerAddress().toString()+":"+QString::number(socket->peerPort()),Qt::green);

        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out << (quint16)0;
        out << (quint8)commAutchReq;
        out << ui->name->text();
        name = ui->name->text();
        out.device()->seek(0);
        out << (quint16)(block.size() - sizeof(quint16));
        socket->write(block);

}

void Dialog::Ready()
{
    QDataStream in(socket);

    if (blockSize == 0) {
        if (socket->bytesAvailable() < (int)sizeof(quint16))
            return;
        in >> blockSize;
        qDebug() << "blockSize now " << blockSize;
    }

    if (socket->bytesAvailable() < blockSize)
        return;
    else
        blockSize = 0;

    quint8 command;
    in >> command;
    qDebug() << "Received command " << command;

    switch (command)
    {
        case commAutchSuccess:
        {


            QString users;
            in >> users;
            if (users != " ")
            {
                QStringList l =  users.split(",");
                for (int i = 0; i < l.size(); ++i) {
                  AddToLog(l.at(i),Qt::gray,"usr");
                }
            }

            QString msgs;
            in >> msgs;
            if (msgs != " ")
            {
                QStringList list =  msgs.split("!_next_msg_!");
                for (int i = 0; i < list.size(); ++i) {
                    AddToLog(list.at(i),Qt::gray,"msg");
                }
            }
            ui->send->setEnabled(true);
            AddToLog("Enter as "+name,Qt::green);
        }
        break;

        case commServerMessage:
        {
            QString message;
            in >> message;
            AddToLog("[Server Message]: "+message, Qt::red);
        }
        break;

        case commMessage:
        {
            QString user;
            in >> user;
            QString message;
            in >> message;
            if (user != this->name)
            {
            AddToLog("["+user+"]: "+message);
            } else
                AddToLog("["+user+"]: "+message, Qt::blue);
        }
        break;

        case commUserJoin:
        {
            QString name;
            in >> name;
            AddToLog(name+" joined", Qt::green);
        }
        break;

        case commUserLeft:
        {
            QString name;
            in >> name;
            AddToLog(name+" left", Qt::green);
        }
        break;

        case commErrNameInvalid:
        {
            QMessageBox::information(this, "Error", "This name is invalid.");
            socket->disconnectFromHost();
        }
        break;

        case commErrNameUsed:
        {
            QMessageBox::information(this, "Error", "This name is already used.");
            socket->disconnectFromHost();
        }
        break;
    }
}

void Dialog::Disconnected()
{
    ui->connect->setEnabled(true);
    ui->disconnect->setEnabled(false);
    ui->send->setEnabled(false);
    ui->msgLog->clear();
    AddToLog("Disconnected from "+socket->peerAddress().toString()+":"+QString::number(socket->peerPort()), Qt::green);
}

void Dialog::on_disconnect_clicked()
{
    socket->disconnectFromHost();
}

void Dialog::on_send_clicked()
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << (quint16)0;
    out << (quint8)commMessage;
    out << ui->msgEdit->document()->toPlainText();
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));
    socket->write(block);
    ui->msgEdit->clear();
}

void Dialog::AddToLog(QString text, QColor color, QString check)
{
    if (check == ""){
    ui->msgLog->insertItem(0, QTime::currentTime().toString()+" "+text);
    } else
        if (check == "usr"){
            ui->msgLog->insertItem(0, text + " is connected");
        } else {
            ui->msgLog->insertItem(0, text);
        }
    ui->msgLog->item(0)->setTextColor(color);
    if (color == Qt::blue)
    {
    ui->msgLog->item(0)->setTextAlignment(Qt::AlignRight);
    } else
        if (color == Qt::black)
        {
            ui->msgLog->item(0)->setTextAlignment(Qt::AlignLeft);
        } else
            if (color == Qt::gray && check == "msg"){
                ui->msgLog->item(0)->setTextAlignment(Qt::AlignLeft);
            } else
                if (color == Qt::gray && check == "usr"){
                    ui->msgLog->item(0)->setTextAlignment(Qt::AlignCenter);
                } else {
                    ui->msgLog->item(0)->setTextAlignment(Qt::AlignCenter);
                }
}

Dialog::~Dialog()
{
    delete ui;
}
