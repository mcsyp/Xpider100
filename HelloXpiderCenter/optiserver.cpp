#include "optiserver.h"
#include <QTcpSocket>
#include <qdebug.h>
#include <stdio.h>
#include "global_xpier.h"
OptiService::OptiService(QObject *parent) :QTcpServer(parent)
{
  client_=NULL;
  connect(this,SIGNAL(newConnection()),this,SLOT(onNewConnection()));
  connect(&protocol_,SIGNAL(PayloadReady(int,QByteArray&)),
          this,SLOT(onPayloadReady(int,QByteArray&)));
}
OptiService::~OptiService(){
  StopServer();
}
void OptiService::StopServer(){
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
  StopServer();

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
  std::vector<xpider_opti_t> opti_info_list;
  opti_info_list.clear();

  switch(cmdid){
  case SERVER_UPLAOD_REQ:
    QString message(payload);
    QStringList record_list = message.split('\n');
    for(int i=0;i<record_list.size();++i){
      QString record_line = record_list[i];
      record_line.remove('\"');

      QStringList value_list = record_line.split(',');
      if(value_list.size()<3)break;
      xpider_opti_t opti_info;
      opti_info.theta = value_list[0].toFloat();
      opti_info.x  = value_list[1].toFloat();
      opti_info.y  = value_list[2].toFloat();

      //save to list
      opti_info_list.push_back(opti_info);
      emit xpiderUpdate(i,opti_info.theta,opti_info.x,opti_info.y);
    }
    //step1.call tracing processor

    //step2.call trajectory planner


    //step3.call all xpiders to move

    break;
  }
}

void OptiService::onClientDisconnected(){
  if(client_)client_=NULL;
}

void OptiService::onClientReadyRead(){
  QByteArray rx_raw = client_->read(RX_MAX_SIZE);
  protocol_.PushToProtocol(rx_raw);
}

