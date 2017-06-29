#include <QCoreApplication>
#include <QThread>
#include <stdio.h>
#include "xpidersimserver.h"
int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);
  XpiderSimServer server;
  server.listen(QHostAddress::LocalHost,9000);
  return a.exec();
}
