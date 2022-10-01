#ifndef CLIENT_H
#define CLIENT_H
#include <QObject>
#include <QFile>
#include <vector>
#include <condition_variable>
#pragma pack(1)

//typedef struct client{
//   int fd;
//   packet *win;
//   bool ackTable[WINDOW_SIZE];
//   QTimer timer;
//   std::vector<packet**> buffer;
//}client;

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



class Client: public QObject
{
    Q_OBJECT
public:
    explicit Client(std::string filename,int apsize,QObject *parent = nullptr);
    ~Client();

    bool checkWindow();
    void initWindow();
    void setWindow(int index,packet * p);
    void handle_write();
    bool isFinished();

private:
    std::mutex  mutex;
    std::condition_variable wait;

    int last_sequence;
    packet **window;

    QFile file;
    std::vector<packet**> waitingWrite;
    int last_block;

    int controllpacketsize;
    int packetsize;


signals:
    void finished();


};

#endif // CLIENT_H
