#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDataStream>
#include <QString>
#include <QMessageBox>
#include <QHostAddress>
#include <QTime>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

private slots:
    void Connected();
    void Disconnected();
    void Ready();
    void SocketError(QAbstractSocket::SocketError socketError);

    void on_connect_clicked();
    void on_disconnect_clicked();
    void on_send_clicked();



public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();


private:
    Ui::Dialog *ui;
    QTcpSocket *socket;
    quint16 blockSize;
    QString name;
    void AddToLog(QString text, QColor color = Qt::black, QString check = "");

    static const quint8 commAutchReq = 1;
    static const quint8 commUserJoin = 2;
    static const quint8 commUserLeft = 3;
    static const quint8 commMessage = 4;
    static const quint8 commServerMessage = 5;
    static const quint8 commAutchSuccess = 6;
    static const quint8 commErrNameInvalid = 201;
    static const quint8 commErrNameUsed = 202;

};

#endif // DIALOG_H
