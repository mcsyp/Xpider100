#include "serverxpider.h"
#include <qdebug.h>
#include <stdio.h>

#define DEBUG 1
ServerXpider::ServerXpider(QObject *parent) :QTcpServer(parent)
{
}
ServerXpider::~ServerXpider(){
  ResetServer();
}
void ServerXpider::ResetServer(){
  if(this->isListening()){
    //stop server and reset
    this->close();
    printf("[Xpider] server stoped.\n");
  }
  threadpool_.clear();
}

int ServerXpider::StartServer(){
  //step1. reset server
  ResetServer();

  //step2. start listening
  this->listen(QHostAddress::Any,XPIDER_PORT);
  threadpool_.setMaxThreadCount(MAX_THREADPOOL);
  printf("[Xpider] server started on %d, threadpool size %d\n",XPIDER_PORT, threadpool_.maxThreadCount());
  return this->serverPort();
}

void ServerXpider::incomingConnection(qintptr socket){
  printf("[Xpider] new client socket received %d\n",socket);

  //step1. call the base incomming socket
  //QTcpServer::incomingConnection(socket);

  //step2. init the socket
  ClientXpider * xpider = new ClientXpider;
  xpider->socketDescriptor_ = socket;

  //step3. start the runnable
  threadpool_.start(xpider);
}

void ClientXpider::run(){
  QTcpSocket socket;
  char rx_buffer[RX_MAX_SIZE];

  //step1. setup socket
  socket.setSocketDescriptor(socketDescriptor_);

  //setp2. loop for messages
  while(socket.isOpen() && socket.waitForReadyRead()){
    int rx_len = socket.bytesAvailable();
#if DEBUG
    printf("[Xpider] received new message, available bytes:%d\n",rx_len);
#endif
    socket.read(rx_buffer,RX_MAX_SIZE);
    rx_buffer[rx_len] = '\0';
#if DEBUG
    printf("rx message:%s\n",rx_buffer);
#endif
  }
  printf("[Xpider] client socket %d disconneceted\n",socketDescriptor_);
}



