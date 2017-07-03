#include "optiserver.h"
#include <QTcpSocket>
#include <qdebug.h>
#include <stdio.h>
#include "xpidersocket.h"
#include "global_xpier.h"
OptiService::OptiService(QObject *parent) :QTcpServer(parent)
{
  client_=NULL;
  connect(this,SIGNAL(newConnection()),this,SLOT(onNewConnection()));
  connect(&protocol_,SIGNAL(PayloadReady(int,QByteArray&)),
          this,SLOT(onPayloadReady(int,QByteArray&)));

  post_worker_.moveToThread(&worker_thread_);
  connect(this,SIGNAL(xpiderPlannerUpdate()),&post_worker_,SLOT(onXpiderPlannerUpdated()));
  time_.start();
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

  time_.restart();
  last_trigger_=0;
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
  printf("[%s,%d]======cmd id:%d======\n",__FILE__,__LINE__,cmdid);
  std::vector<xpider_opti_t> opti_info_list;
  std::vector<xpider_opti_t> xpider_info_list;
  opti_info_list.clear();
  xpider_info_list.clear();

  if(SERVER_UPLAOD_REQ==cmdid){
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
      printf("[%s,%d] %f,%f,%f\n",__FILE__,__LINE__,opti_info.theta,opti_info.x,opti_info.y);
      //emit xpiderUpdate(i,opti_info.theta,opti_info.x,opti_info.y);
    }
    //step1.call tracing processor
    //TODO: call YE TIAN code

    //step2. udpate UI
    for(auto iter=opti_info_list.begin();iter!=opti_info_list.end();++iter){
      xpider_opti_t info = *iter;
      emit xpiderUpdate(info.id,info.theta,info.x,info.y);
    }

    //step3. check if we need to update all xpiders' plan
    int current_time = time_.elapsed();
    if(current_time-last_trigger_>INTERVAL_POST_TASK){
      //every 5sec
      last_trigger_ = current_time;
      emit xpiderPlannerUpdate();
    }
  }//end if(SERVER_UPLAOD_REQ==cmdid)
}

void OptiService::onClientDisconnected(){
  if(client_)client_=NULL;
}

void OptiService::onClientReadyRead(){
  QByteArray rx_raw = client_->read(RX_MAX_SIZE);
  protocol_.PushToProtocol(rx_raw);
  printf("[%s,%d]raw length:%d\n",__FILE__,__LINE__,rx_raw.size());
}

OptiPostWork::OptiPostWork(QObject *parent):QObject(parent){}
void OptiPostWork::onXpiderPlannerUpdated(){
  //step1.call trajectory planner
  //TODO: call XIAO BO code

  //step2.call all xpiders to move
  for(auto iter=XpiderSocket::g_xpider_map_.begin();iter!=XpiderSocket::g_xpider_map_.end();++iter)
  {
    XpiderSocket *socket  = iter->second;
    if(socket){
      //TODO:create move and rotate commands

      //last step
      //socket.AppendTxMessage();
    }
  }
}

