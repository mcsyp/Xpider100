#include "xpiderserver.h"
#include <qdebug.h>
#include <stdio.h>

#define DEBUG 1
XpiderServer::XpiderServer(QObject *parent) :QTcpServer(parent)
{
}
XpiderServer::~XpiderServer(){
  ResetServer();
}
void XpiderServer::ResetServer(){
  if(this->isListening()){
    //stop server and reset
    this->close();
    printf("[Xpider] server stoped.\n");
  }
  threadpool_.clear();
  clientlist_.clear();
}

int XpiderServer::StartServer(){
  //step1. reset server
  ResetServer();

  //step2. start listening
  this->listen(QHostAddress::Any,SERVER_PORT);
  threadpool_.setMaxThreadCount(SERVER_MAX_THREADPOOL);

  printf("[Xpider] server started on %d, threadpool size %d\n",SERVER_PORT, threadpool_.maxThreadCount());
  return this->serverPort();
}

void XpiderServer::incomingConnection(qintptr socket){
  printf("[Xpider] new client socket received %d\n",socket);

  //step1. call the base incomming socket
  //QTcpServer::incomingConnection(socket);

  //step2. init the socket
  XpiderClient * xpider = new XpiderClient;
  xpider->socket_descriptor_ = socket;
  xpider->server_ = this;
  clientlist_.push_back(xpider);//save the xpider to client list

  //step3. start the runnable
  threadpool_.start(xpider);
}

bool XpiderServer::RemoveClient(XpiderClient *client){
  for(auto iter=clientlist_.begin();iter!=clientlist_.end();++iter){
    XpiderClient * item = *iter;
    if(item==client){
      clientlist_.erase(iter);
      return true;
    }
  }
  return false;
}

void XpiderClient::run(){
  QTcpSocket socket;
  XpiderHdlcEncoder hdlc_encoder(this);

  QByteArray rx_message;

  //step1. setup socket reset worklist
  socket.setSocketDescriptor(socket_descriptor_);

  //step2. register to server
  Reset();

  //setp3. loop for messages
  while(socket.state()!=QTcpSocket::UnconnectedState){

    //step1. process rx data
    if(socket.waitForReadyRead(RX_TIMEOUT)){
      //if message arrives
      rx_message = socket.read(RX_MAX_SIZE);
      printf("[%s,%d] rx_message length:%d\n",__FUNCTION__,__LINE__,rx_message.size());
      //process the rx raw data
      hdlc_encoder.hdlc_.charReceiver(rx_message);
    }

    //step2. process tx tasks
    if(!tx_queue_.isEmpty()){
      //if tx queue is not empty, let socket output it.
      for(int i=0;i<tx_queue_.size();++i){
        const QByteArray array  =  tx_queue_.at(i);
        socket.write(array);
      }
      tx_queue_.clear();//clear the whole area
    }
  }

  //step4. un-register from server
  if(server_ && server_->RemoveClient(this)){
    printf("[%s,%d] client %d removed.\n",__FILE__,__LINE__,socket_descriptor_);
  }
  printf("[Xpider] client socket %d disconneceted\n",socket_descriptor_);
}

void XpiderClient::Reset(){
  tx_queue_.clear();
}

void XpiderClient::onDecodedMessage(QByteArray &dec_message, quint16 dec_length){
  char* raw_data = dec_message.data();
  raw_data[dec_length]='\0';
  printf("[%s,%d] rx_message:%s\n",__FUNCTION__,__LINE__,raw_data);
}
void XpiderClient::onEncodedMessage(QByteArray &enc_message){

}
