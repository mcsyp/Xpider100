#include "optiserver.h"
#include <QTcpSocket>
#include <qdebug.h>
#include <stdio.h>
OptiService::OptiService(QObject *parent) :QTcpServer(parent)
{
  client_=NULL;
  connect(this,SIGNAL(newConnection()),this,SLOT(onNewConnection()));
  connect(&protocol_,SIGNAL(PayloadReady(int,QByteArray&)),
          this,SLOT(onPayloadReady(int,QByteArray&)));
}
OptiService::~OptiService(){
  ResetServer();
}
void OptiService::ResetServer(){
  //reset client
  if(client_)client_->disconnectFromHost();
  client_=NULL;

  //reset server
  if(this->isListening()){
    //stop server and reset
    this->close();
    printf("[%s,%d] Opti server stoped.\n",__FUNCTION__,__LINE__);
  }
}

int OptiService::StartServer(){
  //step1. reset server
  ResetServer();

  //step2. start listening
  this->listen(QHostAddress::Any,SERVER_PORT);
  printf("[%s,%d] Opti server started on %d\n",__FUNCTION__,__LINE__,SERVER_PORT);
  return this->serverPort();
}

void OptiService::onNewConnection(){
  QTcpSocket *ptr = this->nextPendingConnection();
  if(client_!=NULL){
    client_->disconnectFromHost();
  }
  client_ = ptr;
  connect(client_,SIGNAL(readyRead()),this, SLOT(onClientReadyRead()));
  connect(client_,SIGNAL(disconnected()),this,SLOT(onClientDisconnected()));
  client_->write("Hey bro, opti server connected.\n");
}

void OptiService::onPayloadReady(int cmdid,QByteArray & payload){
  printf("======cmd id:%d======\n",cmdid);
  printf("%s\n",QString(payload).toLatin1().data());
}

void OptiService::onClientDisconnected(){
  if(client_)client_=NULL;
}

void OptiService::onClientReadyRead(){
  QByteArray rx_raw = client_->read(RX_MAX_SIZE);
  protocol_.PushToProtocol(rx_raw);
}

