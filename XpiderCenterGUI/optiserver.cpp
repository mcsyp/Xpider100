#include "optiserver.h"
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <qdebug.h>
#include <stdio.h>

#include "global_xpier.h"
#include "xpider_ctl/xpider_info.h"
#include "xpider_ctl/xpider_protocol.h"

OptiService::OptiService(QObject *parent) :QTcpServer(parent)
{
  client_=NULL;
  connect(this,SIGNAL(newConnection()),this,SLOT(onNewConnection()));
  connect(&protocol_,SIGNAL(PayloadReady(int,QByteArray&)),
          this,SLOT(onPayloadReady(int,QByteArray&)));

  planner_.moveToThread(&planner_thread_);
  connect(this,SIGNAL(plannerUpdate()),&planner_,SLOT(onXpiderPlannerUpdated()));
  time_.start();
  planner_thread_.start();

  xpider_location_=NULL;
}
OptiService::~OptiService(){
  //reset client
  if(client_)client_->disconnectFromHost();

  //reset server
  if(this->isListening()){
    //stop server and reset
    this->close();
    qDebug()<<tr("[%1,%2] Opti server stoped.\n").arg(__FUNCTION__).arg(__LINE__);
  }
  ptr_cmd_thread_->exit();
  ptr_cmd_thread_->deleteLater();
  delete ptr_cmd_thread_;

  planner_thread_.exit();
  planner_thread_.deleteLater();
  QThread::msleep(100);
  if(xpider_location_)delete xpider_location_;
}

int OptiService::StartService(){
  emit serviceInitializing();

  time_.restart();
  last_trigger_=0;
  is_planner_running_=false;
  client_=NULL;

  //step2. start listening
  this->listen(QHostAddress::Any,SERVER_PORT);
  qDebug()<<tr("[%1,%2]Opti service started on %3").arg(__FILE__).arg(__LINE__).arg(SERVER_PORT);

  //init xpider location
  xpider_location_ = new XpiderLocation();
  xpider_location_->GenerateInitLocation(0.8, 0, 7, 6);
  XpiderLocation::LandmarkList &list = xpider_location_->Landmarks();
  int count=0;
  for(auto iter=list.begin();iter!=list.end();++iter){
    XpiderLocation::Point point = *iter;
    emit landmarkUpdate(count,point.x,point.y);
    ++count;
  }

  //step3. start some socket threads
  do{
    const int host_size=40;
    const char* host_list[]={ "192.168.1.22",
                              "192.168.1.23",
                              "192.168.1.50",
                              "192.168.1.51",
                              "192.168.1.52",
                              "192.168.1.53",
                              "192.168.1.54",
                              "192.168.1.55",
                              "192.168.1.56",
                              "192.168.1.57",
                              "192.168.1.58",
                              "192.168.1.59",
                              "192.168.1.60",
                              "192.168.1.61",
                              "192.168.1.62",
                              "192.168.1.63",
                              "192.168.1.64",
                              "192.168.1.65",
                              "192.168.1.66",
                              "192.168.1.67",
                              "192.168.1.68",
                              "192.168.1.69",
//                              "192.168.1.70",
                              "192.168.1.71",
                              "192.168.1.72",
//                              "192.168.1.73",
                              "192.168.1.74",
                              "192.168.1.75",
//                              "192.168.1.76",
                              "192.168.1.77",
//                              "192.168.1.78",
                              "192.168.1.79",
                              "192.168.1.80",
                              "192.168.1.81",
//                              "192.168.1.82",
                              "192.168.1.83",
                              "192.168.1.84",
                              "192.168.1.85",
                              "192.168.1.86",
                              "192.168.1.87",
                              "192.168.1.88",
                              "192.168.1.89",
                              "192.168.1.90",
                              "192.168.1.91",
                              "192.168.1.92",
//                              "192.168.1.93",
//                              "192.168.1.94",
//                              "192.168.1.95",
//                              "192.168.1.96",
//                              "192.168.1.97",
//                              "192.168.1.98",
//                              "192.168.1.99",
//                              "192.168.1.100",
//                              "192.168.1.101",
//                              "192.168.1.102",
//                              "192.168.1.103",
//                              "192.168.1.104",
                            };

    const int host_port=80;
    for(int i=0;i<host_size;++i){
      XpiderSocketThread * socket = new XpiderSocketThread();
      if(socket){
        QString name = host_list[i];
        socket->StartConnection(name,host_port);
        connect(&timer_retry_,SIGNAL(timeout()),socket,SLOT(onTimeoutRetry()));
      }
    }
  }while(0);

  timer_retry_.start(XpiderSocketThread::INTERVAL_RETRY);
  ptr_cmd_thread_ = new CommandThread;
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
  std::vector<xpider_opti_t> opti_info_list;
  std::vector<xpider_opti_t> xpider_info_list;
  opti_info_list.clear();

  //qDebug()<<tr("[%1,%2]rx_payload:").arg(__FILE__).arg(__LINE__);
  //qDebug()<<QString(payload)<<endl;
  //step1.find the alive xpider list
#if 1
  const int id_size = XpiderSocketThread::g_xpider_map_.size();
  uint32_t id_list[id_size];
  int id_len=0;

  //qDebug()<<tr("[%1,%2]alive xpiders are:").arg(__FILE__).arg(__LINE__);
  for(auto iter=XpiderSocketThread::g_xpider_map_.begin();
      iter!=XpiderSocketThread::g_xpider_map_.end();
      ++iter){
    XpiderSocketThread* xpider = iter->second;
    if(xpider && xpider->Available()){
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
    }

    //step2. wrap in JSON and push to signal
    QJsonDocument jdoc;
    QJsonArray jarray;
    for(auto iter = opti_info_list.begin();iter!=opti_info_list.end();++iter){
      xpider_opti_t raw = *iter;
      QJsonObject jobj;
      jobj["id"]=-1;
      jobj["theta"]= raw.theta;
      jobj["x"] = raw.x;
      jobj["y"] = raw.y;
      for(auto xiter=xpider_info_list.begin();xiter!=xpider_info_list.end();++xiter){
        xpider_opti_t xpider = *xiter;
        if(raw.x==xpider.x && raw.y == xpider.y){
          jobj["id"] = (int)xpider.id;
          break;
        }
      }
      jarray.push_back(jobj);
    }
    jdoc.setArray(jarray);
    emit xpiderListUpdate(QString(jdoc.toJson()));


    //step3. check if we need to update all xpiders' plan
    int current_time = time_.elapsed();
    if(current_time-last_trigger_>INTERVAL_POST_TASK && is_planner_running_){
      //every 5sec
      last_trigger_ = current_time;
      planner_.MoveToPostWork(target_map_,xpider_info_list);
      emit plannerUpdate();
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
  if(rx_raw.size()==0)return;
  //qDebug()<<tr("[%1,%2] rx_raw:").arg(__FILE__).arg(__LINE__)<<endl;
  //qDebug()<<QString(rx_raw)<<endl;
  protocol_.PushToProtocol(rx_raw);
  //qDebug()<<tr("====================================================")<<endl;
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
  XpiderSocketThread* x= XpiderSocketThread::Socket(id);
  if(x)x->StopWalking();
}

void OptiService::startPlanner(bool b){
  is_planner_running_ = b;
}

void OptiService::runCommandText(QString cmd_text){
  qDebug()<<tr("[%1,%2] command text is:%3").arg(__FILE__).arg(__LINE__).arg(cmd_text);
  emit commandRunning(true);
  ptr_cmd_thread_->StartCommandChain(cmd_text);
}


OptiPostWork::OptiPostWork(QObject *parent):QObject(parent){}

void OptiPostWork::MoveToPostWork(std::map<uint32_t, xpider_target_point_t> &target_map, std::vector<xpider_opti_t> &info_list){
  list_len_=0;
  for(auto iter = target_map.begin();iter!=target_map.end();++iter){
    target_list_[list_len_]  = iter->second;
    uint32_t id =target_list_[list_len_].id;
//    for(auto  info_iter=info_list.begin();info_iter!=info_list.end();++info_iter){
//      xpider_opti_t info = *info_iter;
//      if(info.id==id){
//        xpider_list_[list_len_] = info;
//        break;
//      }
//    }
    ++list_len_;
  }

  qDebug() << "len before reset" << info_list.size();
  for(auto i=0; i<info_list.size(); i++){
      xpider_list_[i] = info_list[i];
  }
  xpider_list_len_ = info_list.size();
  qDebug() << "list_len:" << list_len_;
  planner_.Reset(3.0,2.0,target_list_,list_len_);
}

void OptiPostWork::onXpiderPlannerUpdated(){
  if(list_len_<=0)return;

  //step1.call trajectory planner
  //TODO: call XIAO BO code
  const int action_size = list_len_;
  xpider_tp_t action_list[action_size];
  int len = planner_.Plan(xpider_list_, xpider_list_len_, action_list, action_size);

  //step2.call all xpiders to move
  for(int i=0;i<len;++i){
    XpiderSocketThread * socket = XpiderSocketThread::Socket(action_list[i].id);
    if(socket){

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
      socket->SendMessage(tx_pack);
    }
  }

}

