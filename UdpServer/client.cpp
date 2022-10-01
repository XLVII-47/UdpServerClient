#include "client.h"
#include <QDebug>
#include <unistd.h>
#include <QThread>
#include <iostream>


Client::Client(std::string filename,int apsize,QObject *parent)
    :QObject{parent}
    ,last_sequence{0}
    ,last_block{0}
{

    controllpacketsize= apsize;
    packetsize= apsize;
    file.setFileName(QString::fromStdString(filename));

    if(!file.open(QIODeviceBase::WriteOnly | QIODeviceBase::Truncate)){;
        qDebug() << "could not open file";
        exit(EXIT_FAILURE);
    }

    initWindow();

}

Client::~Client()
{
    for(int i= 0 ;i < waitingWrite.size(); i++){

        for(int j = 0 ; j < WINDOW_SIZE; j++){
            delete waitingWrite[i][j];
        }
        delete[] waitingWrite[i];
    }
    file.close();
}

bool Client::checkWindow()
{

    if(controllpacketsize > 0){
        int ll = controllpacketsize < WINDOW_SIZE ? controllpacketsize:WINDOW_SIZE;

        for(int i=0 ; i< ll ;i++){
            if(window[i] == nullptr) return false;
        }
        //if all element are true of ackTable which means is full
        //pass to next window


        //waitingWrite.push(window);
        waitingWrite.push_back(window);
        last_sequence+= ll;
        controllpacketsize-=ll;

        initWindow();

        //qDebug()<< "last_sequence," << last_sequence << " controllpacketsize : " << controllpacketsize;
        return true;
    }

    return false;
}

void Client::initWindow()
{

    window = new packet*[WINDOW_SIZE];
    for(int i=0;i < WINDOW_SIZE ;i++){
        window[i] = nullptr;
    }
}

void Client::setWindow(int index, packet *p)
{

    if( window[index%WINDOW_SIZE] == nullptr){
        window[index%WINDOW_SIZE] = p;
        checkWindow();
    }else{
        delete p;
    }

}

void Client::handle_write()
{

    int ii=0;

    int repeat_count= 3;
    std::unique_lock<std::mutex> lk(mutex);
    while (ii < packetsize && file.isOpen())
    {
        wait.wait_until(lk, std::chrono::system_clock::now() + std::chrono::milliseconds(1000));

        int len = waitingWrite.size();

        qDebug() << "writing.. "<<QThread::currentThread() << "new_block_size "<<len << "written_last_block "<< last_block;

        if(len == last_block){
            repeat_count--;
            if(repeat_count == 0){
                qDebug() << "File: "<< file.fileName()<<" transfer completed. "<<QThread::currentThread();
                break;
            }
        }else{
            repeat_count = 3;
        }

        for(int j= last_block; j < len ; j++){
            packet **tmpwindow = waitingWrite[j];

            for(int i=0 ; i < WINDOW_SIZE ; i++){

                if(tmpwindow[i] !=nullptr){
                    if(tmpwindow[i]->data_length >0){
                        file.write(tmpwindow[i]->data,tmpwindow[i]->data_length);

                        ii++;
                    }
                    else
                        break;
                }else
                    break;
            }
            last_block++;
        }
        file.flush();
    }

    if(repeat_count != 0)
        qDebug() << "File: "<< file.fileName()<<" transfer completed. "<<QThread::currentThread();

    emit finished();
}

