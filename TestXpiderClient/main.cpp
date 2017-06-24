#include <QCoreApplication>
#include "xpiderclient.h"
int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);
  XpiderClient client;
  client.ConnectToHost("127.0.0.1",XpiderClient::SERVER_PORT);
  return a.exec();
}
