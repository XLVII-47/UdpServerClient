#include "udpclient.h"
#include <thread>
#include <iostream>
#include <unistd.h>


UdpClient *client;


int main(int argc, char *argv[])
{
    std::string addr= "127.0.0.1",filename= "file.txt";
    int port = 5001;

    while(1){

//        std::cout << "Enter server address" << std::endl;
//        std::cin >> addr;
//        std::cout << "Enter server port" << std::endl;
//        std::cin >> port;
        std::cout << "Enter filename" << std::endl;
        std::cin >> filename;
        client = new UdpClient;
        client->setServerAddress(addr.c_str(),port);
        client->file_to_packets(filename);
        if(!client->connecToHost()){
            std::cout << "Could not connect to host.." <<std::endl;
            continue;
        }

        std::thread * t1 = new std::thread(sendPackets,client);
        std::thread * t2 = new std::thread(receiveACK,client);
        t1->join();
        t2->join();


        delete t1;
        delete t2;
        delete client;
        //std::cin >> port;
    }

 return 0;
}
