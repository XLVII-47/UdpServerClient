#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QObject>
#include <QUdpSocket>
#include <QTimer>
#include <string>
#include <QNetworkDatagram>
#include <map>
#include "client.h"


class UdpServer : public QObject
{
    Q_OBJECT
public:


    explicit UdpServer(int part,QObject *parent = nullptr);
    std::string mapKey(QHostAddress ha,int port);

signals:

public slots:
    void onReadyRead();
    void handleFinishedClient();
private:
    QUdpSocket* m_socket;
    std::map<std::string, Client* > map;

};

#endif // UDPSERVER_H
