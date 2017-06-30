#include <QCoreApplication>
#include "opticlient.h"
int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);
  OptiClient client;
  client.TryConnectToHost("192.168.1.7",8000);
  return a.exec();
}
