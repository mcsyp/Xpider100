#include <QCoreApplication>
#include "opticlient.h"
int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);
  OptiClient client;
  client.TryConnectToHost("127.0.0.1",8000);
  return a.exec();
}
