#include <QCoreApplication>
#include "xpiderclient.h"
int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);
  XpiderClient client;
  client.ConnectToHost("192.168.1.52",80);
  return a.exec();
}
