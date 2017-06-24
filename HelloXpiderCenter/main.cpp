#include <QCoreApplication>
#include  "serverxpider.h"
#include "protocolxpider.h"
#include <stdio.h>
int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);
  ServerXpider server_xpider;
  server_xpider.StartServer();

  printf("Length of the message head:%d\n",ProtocolXpider::MESSAGE_HEAD_LEN);
  return a.exec();
}
