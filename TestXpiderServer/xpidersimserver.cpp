#include "xpidersimserver.h"
#include <stdio.h>
XpiderSimServer::XpiderSimServer(QObject* parent):QTcpServer(parent)
{
  connect(this,SIGNAL(newConnection()),this, SLOT(onNewConnection()));
}
void XpiderSimServer::onNewConnection(){
  QTcpSocket * socket = this->nextPendingConnection();
  connect(socket,SIGNAL(readyRead()),this,SLOT(onClientReadyRead()));
  connect(socket,SIGNAL(disconnected()),this,SLOT(onClientDisconnected()));
  printf("[%s,%d] new incomming socket\n",__FUNCTION__,__LINE__);
  socket->write("Hey baby, fuck you!");
  ptr_soket_=socket;
}
void XpiderSimServer::onClientDisconnected(){
  printf("[%s,%d] socket disconnected\n",__FUNCTION__,__LINE__);
}
void XpiderSimServer::onClientReadyRead(){
  //printf("[%s,%d] new incomming socket\n",__FUNCTION__,__LINE__);
  QByteArray rx_array = ptr_soket_->read(4096);
  qDebug()<<rx_array.toHex();
}
