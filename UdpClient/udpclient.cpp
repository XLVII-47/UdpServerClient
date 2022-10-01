#include "udpclient.h"
#include <QFile>
#include <iostream>
#include <chrono>
#include <unistd.h>


UdpClient::UdpClient()
{

#ifdef __WIN64__

    wsa = new WSADATA;

    if (WSAStartup(MAKEWORD(2,2),wsa) != 0)
    {
        printf("Failed. Error Code : %d",WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    //create socket
    if ( (m_socketfd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
    {
        printf("socket() failed with error code : %d" , WSAGetLastError());
        exit(EXIT_FAILURE);
    }

#else
    isFinished = false;
    receivedAck = 0;
    fileSize = 0;

    if ((m_socketfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("UdpClient() socket creat");
        exit(EXIT_FAILURE);
    }

#endif
}

UdpClient::~UdpClient()
{
    close(m_socketfd);
    for(int i=0;i<packets.size();i++){
        delete packets[i];
    }
}

void UdpClient::setServerAddress(const std::string &address, unsigned short port)
{
#ifdef __WIN64__
    memset((char *) &server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.S_un.S_addr = inet_addr(address.c_str());
#else
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(address.c_str());
#endif
    //    fileSize=file_to_packets(packets, "file.txt");
    //    qDebug() << "file packet: "<<lastfile_packetsize;
}




void UdpClient::file_to_packets(const std::string filename)
{
    this->filename = filename;

    QFile file(filename.c_str());
    if (!file.open(QIODevice::ReadOnly)){
        std::cout << "file_to_packet could not open file" <<std::endl;
        return;
    }

    if (file.size() != 0)
    {
        uint seqNo = 0;
        while (!file.atEnd())
        {
            packet *npac = new packet;
            npac->data_length = file.read(npac->data, DATA_SIZE);
            npac->sequence_number = seqNo;

            packets.push_back(npac);
            seqNo++;
        }
    }

    fileSize = packets.size();
    file.close();
}

bool UdpClient::connecToHost()
{
    packet packet;

    ACK ACK;
    struct sockaddr sender;
#ifdef __WIN64__
    int  len;
#else
    socklen_t len;
#endif

    std::string s;
    s+=std::string("CON FILE ")+ filename+" "+std::to_string(fileSize);

    strcpy(packet.data,s.c_str());
    packet.data_length=0;
    packet.sequence_number=0;

    int maxTry = 20;
    while(maxTry--){
        sendto(m_socketfd, reinterpret_cast<char *>(&packet), PACKET_SIZE, 0x40 /*MSG_DONTWAIT*/, (const struct sockaddr *)&server_addr, sizeof(server_addr));

        recvfrom(m_socketfd, (char *)&ACK, sizeof(ACK),
                 0, (struct sockaddr *)&sender, &len);

        if(strcmp(ACK.data,"CON")==0 && ACK.sequence_number==0){
            std::cout << "Connected to server" <<std::endl;
            return true;
        }
    }
    return false;
}

void sendPackets(UdpClient *client)
{
    std::cout << "Starting sending file: " << client->filename << std::endl;
    int packetsize = client->packets.size();

    for (int i = 0; i < packetsize ;)
    {

        initAckTable(client);


        client->mutex.lock();
        int winsize = (packetsize-i) < WINDOW_SIZE? (packetsize-i) : WINDOW_SIZE;

        for (int j = i; j - i < winsize; j++)
        {
            //m_socket->writeDatagram(reinterpret_cast<char*>(packets[j]),PACKET_SIZE,QHostAddress::LocalHost, 5001);
            sendto(client->m_socketfd, reinterpret_cast<char *>(client->packets[j]), PACKET_SIZE, 0x40 /*MSG_DONTWAIT*/, (const struct sockaddr *)&client->server_addr, sizeof(client->server_addr));
        }
        client->mutex.unlock();

        std::unique_lock<std::mutex> lk(client->mutex);
        while (!checkAckTable(client,winsize))
        {
            client->wait.wait_until(lk, std::chrono::system_clock::now() + std::chrono::milliseconds(10));
            if (!checkAckTable(client,winsize))
            {
                retransmitPackets(i, client);
            }
        }

        lk.unlock();

        i += WINDOW_SIZE;
    }

    client->isFinished = true;

}



void receiveACK(UdpClient *client)
{


    client->receivedAck = 0;
    ACK buffer;
    struct sockaddr sender;

#ifdef __WIN64__
    int  len;
#else
    socklen_t len;
#endif

    while (client->receivedAck < client->fileSize && !client->isFinished)
    {
        recvfrom(client->m_socketfd, (char *)&buffer, sizeof(buffer),
                 0/*MSG_WAITALL*/, (struct sockaddr *)&sender, &len);

        if (strcmp(buffer.data, "ACK") == 0)
        {
            updateAckTable(client,buffer.sequence_number);
            client->receivedAck++;
        }
    }

    std::cout << "File transfer completed..." << std::endl;
}

void updateAckTable(UdpClient *client,int index,int size)
{
    client->mutex.lock();

    client->ack_table[index % WINDOW_SIZE] = true;
    
    if (checkAckTable(client,size))
    {
        client->wait.notify_one();
    }

    client->mutex.unlock();
}

void initAckTable(UdpClient *client)
{
    client->mutex.lock();

    for (int i = 0; i < WINDOW_SIZE; i++)
    {
        client->ack_table[i] = false;
    }

    client->mutex.unlock();
}

bool checkAckTable(UdpClient *client,int size)
{


    for (int i = 0; i < size; i++)
    {
        if (client->ack_table[i] == false)
            return false;
    }

    return true;
}

void retransmitPackets(int index, UdpClient *client)
{
    client->mutex.lock();
    for (int i = 0; i < WINDOW_SIZE; i++)
    {
        if (client->ack_table[i] == false)
        {
            sendto(client->m_socketfd, reinterpret_cast<char *>(client->packets[index + i])
                    ,PACKET_SIZE, 0x40 /*MSG_DONTWAIT*/, (const struct sockaddr *)&client->server_addr, sizeof(client->server_addr));
        }
    }
    client->mutex.unlock();
}
