#include <QCoreApplication>
#include  "xpiderserver.h"
#include <stdio.h>
int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);
  XpiderServer server_xpider;
  server_xpider.StartServer();
  return a.exec();
}
