#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#pragma pack(1)


#include <vector>
#include <condition_variable>
#include <mutex>

#include <string>
#ifdef __WIN64__
    #include<winsock2.h>
    #pragma comment(lib,"ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
#endif

constexpr const int WINDOW_SIZE= 5;
constexpr const int DATA_SIZE =1434;
constexpr const int PACKET_SIZE =DATA_SIZE + 8;


struct packet{
    uint32_t sequence_number;
    uint32_t data_length;
    char data[DATA_SIZE];
};

struct ACK{
    char data[4];
    uint32_t sequence_number;
};



class UdpClient
{
public:

    explicit UdpClient();
    ~UdpClient();
    void setServerAddress(const std::string& address,unsigned short port);
    void file_to_packets(const std::string filename);
    bool connecToHost();


    friend void receiveACK(UdpClient* client);
    friend void sendPackets(UdpClient* client);
    friend void updateAckTable(UdpClient* client,int index,int size);
    friend void initAckTable(UdpClient* client);
    friend void setAckTable(int index,UdpClient* client);
    friend void retransmitPackets(int index,UdpClient* client) ;
    friend bool checkAckTable(UdpClient* client,int size);

private:

    std::mutex mutex;
    std::condition_variable wait;
    std::vector<packet*> packets;
    struct sockaddr_in server_addr;
    bool ack_table[WINDOW_SIZE];

#ifdef __WIN64__
    WSADATA *wsa;
    SOCKET m_socketfd;
#else
    int m_socketfd;
#endif


    bool isFinished;
    std::string filename;
    uint32_t receivedAck;
    uint32_t fileSize;

};

void updateAckTable(UdpClient* client,int index,int size = WINDOW_SIZE);
void sendPackets(UdpClient* client);
void initAckTable(UdpClient* client);
void setAckTable(UdpClient* client,int index);
void retransmitPackets(int index,UdpClient* client) ;
bool checkAckTable(UdpClient* client,int size = WINDOW_SIZE);
void receiveACK(UdpClient* client);

#endif // UDPCLIENT_H
