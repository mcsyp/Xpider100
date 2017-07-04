#include "optiserver.h"
#include <QTcpSocket>
#include <qdebug.h>
#include <stdio.h>

#include "xpidersocket.h"
#include "global_xpier.h"
#include "xpider_ctl/xpider_info.h"
#include "xpider_ctl/xpider_protocol.h"

OptiService::OptiService(QObject *parent) :QTcpServer(parent)
{
  client_=NULL;
  connect(this,SIGNAL(newConnection()),this,SLOT(onNewConnection()));
  connect(&protocol_,SIGNAL(PayloadReady(int,QByteArray&)),
          this,SLOT(onPayloadReady(int,QByteArray&)));

  post_worker_.moveToThread(&worker_thread_);
  connect(this,SIGNAL(xpiderPlannerUpdate()),&post_worker_,SLOT(onXpiderPlannerUpdated()));
  time_.start();
  worker_thread_.start();

  xpider_location_=NULL;
}
OptiService::~OptiService(){
  StopServer();
  worker_thread_.quit();
  worker_thread_.wait();
  if(xpider_location_)delete xpider_location_;
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

  xpider_location_ = new XpiderLocation();
  xpider_location_->GenerateInitLocation(0,0,2,3);
  XpiderLocation::LandmarkList &list = xpider_location_->Landmarks();
  int count=0;
  for(auto iter=list.begin();iter!=list.end();++iter){
    XpiderLocation::Point point = *iter;
    emit landmarkUpdate(count,point.x,point.y);
    ++count;
  }

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
  //qDebug("[%1,%2]======cmd id:%3======\n").arg(__FILE__).arg(__LINE__).arg(cmdid);
  std::vector<xpider_opti_t> opti_info_list;
  std::vector<xpider_opti_t> xpider_info_list;
  opti_info_list.clear();

  //step1.find the alive xpider list
#if 1
  const int id_size = XpiderSocket::g_xpider_map_.size();
  uint32_t id_list[id_size];
  int id_len=0;

  //qDebug()<<tr("[%1,%2]alive xpiders are:").arg(__FILE__).arg(__LINE__);
  for(auto iter=XpiderSocket::g_xpider_map_.begin();
      iter!=XpiderSocket::g_xpider_map_.end();
      ++iter){
    XpiderSocket* xpider = iter->second;
    if(xpider && xpider->IsRunning()){
      id_list[id_len] = xpider->Id();
      ++id_len;
      //qDebug()<<tr("%1, ").arg(xpider->Id());
    }
  }
  //qDebug()<<endl;
#else
  const int id_size = XpiderSocket::g_xpider_map_.size();
  uint32_t id_list[]={1,2};
  int id_len=2;
  printf("[%s,%d]id_size=%d\n",__FILE__,__LINE__,id_size);
#endif

  if(SERVER_UPLAOD_REQ==cmdid){
    QString message(payload);
    QStringList record_list = message.split('\n');
    for(int i=0;i<record_list.size();++i){
      QString record_line = record_list[i];
      record_line.remove('\"');

      QStringList value_list = record_line.split(',');
      if(value_list.size()<3)break;
      xpider_opti_t opti_info;
      opti_info.x  = value_list[0].toFloat();
      opti_info.y  = value_list[1].toFloat();
      opti_info.theta = value_list[2].toFloat();

      //save to list
      opti_info_list.push_back(opti_info);
      //emit xpiderUpdate(i,opti_info.theta,opti_info.x,opti_info.y,false);
    }
#if 1
    //step1.call tracing processor
    //TODO: call YE TIAN code
    if(opti_info_list.size() && id_len>0){
      xpider_info_list.clear();
      xpider_location_->GetRobotLocation(opti_info_list,id_list,id_len,xpider_info_list);
      //step2. udpate UI
      for(auto iter=xpider_info_list.begin();iter!=xpider_info_list.end();++iter){
        xpider_opti_t info = *iter;
        emit xpiderUpdate(info.id,info.theta,info.x,info.y,true);
      }
    }

    //step3. check if we need to update all xpiders' plan
    int current_time = time_.elapsed();
    if(current_time-last_trigger_>INTERVAL_POST_TASK){
      //every 5sec
      last_trigger_ = current_time;
      post_worker_.MoveToPostWork(target_map_,xpider_info_list);
      emit xpiderPlannerUpdate();
    }
#endif
  }//end if(SERVER_UPLAOD_REQ==cmdid)
}

void OptiService::onClientDisconnected(){
  if(client_)client_=NULL;
}

void OptiService::onClientReadyRead(){
  if(client_==NULL)return;
  QByteArray rx_raw = client_->read(RX_MAX_SIZE);
  protocol_.PushToProtocol(rx_raw);
  //printf("[%s,%d]raw length:%d\n",__FILE__,__LINE__,rx_raw.size());
}

void OptiService::pushTarget(unsigned int id, float x, float y){
  xpider_target_point_t target;
  target.id = id;
  target.target_x = x;
  target.target_y = y;
  target_map_[id] = target;
}

void OptiService::removeTarget(unsigned int id){
  if(target_map_.count(id)){
    for(auto iter = target_map_.begin();iter!=target_map_.end();++iter){
      if(iter->first==id){
        target_map_.erase(iter++);
        break;
      }
    }
  }
}


OptiPostWork::OptiPostWork(QObject *parent):QObject(parent){}

void OptiPostWork::MoveToPostWork(std::map<uint32_t, xpider_target_point_t> &target_map, std::vector<xpider_opti_t> &info_list){
  list_len_=0;
  for(auto iter = target_map.begin();iter!=target_map.end();++iter){
    target_list_[list_len_]  = iter->second;
    uint32_t id =target_list_[list_len_].id;
    for(auto  info_iter=info_list.begin();info_iter!=info_list.end();++info_iter){
      xpider_opti_t info = *info_iter;
      if(info.id==id){
        xpider_list_[list_len_] = info;
        break;
      }
    }
    ++list_len_;
  }
  //printf("[%s,%d]list_len_ is :%d\n",__FUNCTION__,__LINE__,list_len_);

  planner_.Reset(3.0,2.0,target_list_,list_len_);
}

void OptiPostWork::onXpiderPlannerUpdated(){
  //printf("[%s,%d]xpider planner is triggered\n",__FILE__,__LINE__);
  if(list_len_<=0)return;

  //step1.call trajectory planner
  //TODO: call XIAO BO code
  const int action_size = list_len_;
  xpider_tp_t action_list[action_size];
  int len = planner_.Plan(xpider_list_,list_len_,action_list,action_size);

  //step2.call all xpiders to move
  for(int i=0;i<len;++i){
    XpiderSocket * socket = XpiderSocket::Search(action_list[i].id);
    if(socket){
      //printf("[%s,%d] tx command to [%d]\n",__FILE__,__LINE__,socket->Id());

      QByteArray tx_pack;
      uint8_t* tx_buffer;
      uint16_t tx_length;
      XpiderInfo info;
      XpiderProtocol  protocol;
      protocol.Initialize(&info);

      //step1.set target angle & transform to tx buffer
      info.rotate_speed = OptiService::XPIDER_ROTATE_SPEED;
      info.rotate_rad = action_list[i].delta_theta;
      info.walk_speed = OptiService::XPIDER_WALK_SPEED;
      info.walk_step = action_list[i].detla_step;
      protocol.GetBuffer(protocol.kAutoMove, &tx_buffer, &tx_length);

      //step2. set tx_pack
      tx_pack.append((char*)tx_buffer,tx_length);

      //step3. tx pack
      socket->AppendTxMessage(tx_pack);
    }
  }

}

