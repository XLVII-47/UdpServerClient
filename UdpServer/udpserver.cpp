
#include <QDebug>
#include <iostream>
#include <sstream>
#include <QtConcurrent>
#include "udpserver.h"

UdpServer::UdpServer(QObject *parent)
    : QObject{parent}
{
    m_socket = new QUdpSocket(this);
    m_socket->bind(QHostAddress("127.0.0.1"), 5001);

    connect(m_socket, &QUdpSocket::readyRead,this, &UdpServer::onReadyRead);

    qDebug() << "Server is listening....";
}



std::string UdpServer::mapKey(QHostAddress ha,int port){
    return ha.toString().toStdString() + ":" + std::to_string(port);
}


void UdpServer::onReadyRead()
{

    if(m_socket->bytesAvailable() >= PACKET_SIZE){

        QNetworkDatagram data;
        data = m_socket->receiveDatagram(PACKET_SIZE);

        packet *pack = new packet;


        memcpy(pack,data.data(),PACKET_SIZE);

        if(pack->data_length==0 && pack->sequence_number==0){

            std::stringstream ss(pack->data);
            std::string con,file,name;
            uint32_t size;
            ss >> con >> file >> name >> size;

            if(con == "CON"){


                if(map.find(mapKey(data.senderAddress(),data.senderPort())) == map.end()){

                    Client *newClient = new Client(name,size,this);

                    connect(newClient,&Client::finished,this,&UdpServer::handleFinishedClient);
                    map[mapKey(data.senderAddress(),data.senderPort())] = newClient;

                }else{
                    if(map[mapKey(data.senderAddress(),data.senderPort())] == nullptr){
                        Client *newClient = new Client(name,size,this);

                        connect(newClient,&Client::finished,this,&UdpServer::handleFinishedClient);
                        map[mapKey(data.senderAddress(),data.senderPort())] = newClient;
                        QFuture<void> future = QtConcurrent::run(&Client::handle_write, newClient);
                    }
                }


                ACK ack;

                ack.data[0]='C';
                ack.data[1]='O';
                ack.data[2]='N';
                ack.data[3]='\0';

                ack.sequence_number=0;


                m_socket->writeDatagram((const char*)&ack,sizeof(ACK),data.senderAddress(),data.senderPort());
            }else{
                delete pack;
            }


        }else{

            map[mapKey(data.senderAddress(),data.senderPort())]->setWindow(pack->sequence_number,pack);

            ACK ack;

            ack.data[0]='A';
            ack.data[1]='C';
            ack.data[2]='K';
            ack.data[3]='\0';

            ack.sequence_number=pack->sequence_number;
            m_socket->writeDatagram((const char*)&ack,sizeof(ACK),data.senderAddress(),data.senderPort());

        }
        return;
    }

}

void UdpServer::handleFinishedClient()
{
    Client* senderClient = reinterpret_cast<Client*>(sender());

    for(auto it = map.begin(); it != map.end();){
        if(it->second == senderClient){

            map[it->first] = nullptr;
            senderClient->deleteLater();
            //std::cout << "handleFinished" << std::endl;
            break;
        }else
            it++;
    }


}
