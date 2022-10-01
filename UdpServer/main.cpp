#include <QCoreApplication>
#include "udpserver.h"
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    UdpServer server;
    return a.exec();
}
