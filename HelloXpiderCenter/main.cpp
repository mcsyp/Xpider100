#include <QCoreApplication>
#include "xpiderpool.h"
#include "optiserver.h"
#include <stdio.h>
int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);
  XpiderPool server_xpider;
  server_xpider.StartConnection();
  OptiServer server_opti;
  server_opti.StartServer();
  return a.exec();
}
