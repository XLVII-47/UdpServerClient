#include <QCoreApplication>
#include "udpserver.h"

constexpr const int PORT = 5001;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    UdpServer server(PORT);
    return a.exec();
}
