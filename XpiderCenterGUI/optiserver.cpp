#include "optiserver.h"
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <qdebug.h>
#include <QTextStream>
#include <QFile>

#include <stdio.h>

#include "global_xpider.h"
#include "xpider_ctl/xpider_info.h"
#include "xpider_ctl/xpider_protocol.h"

OptiService::OptiService(QObject *parent) :QTcpServer(parent)
{
  client_=NULL;
  connect(this,SIGNAL(newConnection()),this,SLOT(onNewConnection()));
  connect(&protocol_,SIGNAL(PayloadReady(int,QByteArray&)),
          this,SLOT(onPayloadReady(int,QByteArray&)));

  time_.start();

  ptr_location_=NULL;
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

  QThread::msleep(100);
  if(ptr_location_)delete ptr_location_;
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

  //step3. start some socket threads
  const int host_size=22;
  const char* host_list[]={"192.168.1.22",
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
                          "192.168.1.69"};
  do{
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

  //init xpider location (the land mark loacation )
  do{
    ptr_location_ = new XpiderLocation();
    ptr_location_->GenerateInitLocation(0,0,5,5);
    XpiderLocation::LandmarkList &list = ptr_location_->Landmarks();

    int counter=0;
    QJsonDocument jdoc;
    QJsonArray jarray;
    for(auto iter=list.begin();iter!=list.end();++iter){
      XpiderLocation::Point point = *iter;
      QJsonObject jobj;

      if(counter<host_size){
        QString str_id = QString(host_list[counter]);
        jobj["id"]= str_id.split('.').last();
      }else{
        jobj["id"]= QString("[%1]").arg(counter);
      }
      jobj["x"] = point.x;
      jobj["y"] = point.y;
      //emit landmarkUpdate(count,point.x,point.y);
      jarray.append(jobj);
      ++counter;
    }
    jdoc.setArray(jarray);
    emit landmarkListUpdate(QString(jdoc.toJson()));
  }while(0);

  //init universal retry timer
  timer_retry_.start(XpiderSocketThread::INTERVAL_RETRY);

  //init command framework background thread
  ptr_cmd_thread_ = new CommandThread;

  //init planner thread
  ptr_planner_thread_ = new TrajectoryThread;

  //clear the target mask
  ui_target_mask_.clear();
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
  if(SERVER_UPLAOD_REQ==cmdid){
    std::vector<xpider_opti_t> opti_info_list;
    opti_info_list.clear();

    //step1.parse the payload infomation
    do{
      QTextStream stream;
      QString str_payload(payload);
      stream.setString(&str_payload);
      while(!stream.atEnd()){
        QString record_line = stream.readLine();
        record_line.remove('\"');

        QStringList value_list = record_line.split(',');
        if(value_list.size()<3)break;

        xpider_opti_t opti_info;
        opti_info.x  = value_list[0].toFloat();
        opti_info.y  = value_list[1].toFloat();
        opti_info.theta = value_list[2].toFloat();

        //save to list
        opti_info_list.push_back(opti_info);
      }
    }while(0);


    //step2. compute the available xpider id list
    const int id_size = XpiderSocketThread::socket_list_.size();
    uint32_t id_array[id_size];
    int id_len=AvailableXpiderSocketID(id_array,id_size);

    typedef struct temp_map_val_s{
      xpider_opti_t *ptr_xpider;
      xpider_target_point_t * ptr_target;
    }temp_map_val_t;
    QMap<QString, temp_map_val_t> temp_raw_map;

    //step3.call tracing processor
    if(opti_info_list.size() && id_len>0){
      //step1. match all xpiders
      ptr_planner_thread_->xpider_queue_.clear();
      ptr_location_->GetRobotLocation(opti_info_list,id_array,id_len,ptr_planner_thread_->xpider_queue_);

      //step2. match all targets
      ptr_planner_thread_->target_queue_.clear();
      SyncXpiderTarget(ptr_planner_thread_->xpider_queue_,ptr_planner_thread_->target_queue_);

      for(int i=0;i<ptr_planner_thread_->xpider_queue_.size();++i){
        xpider_opti_t xpider = ptr_planner_thread_->xpider_queue_[i];
        xpider_target_point_t target = ptr_planner_thread_->target_queue_[i];
        //also save it to a list, but only pointer
        QString str_key = PointToString(QPointF(xpider.x,xpider.y));
        temp_map_val_t value;
        value.ptr_xpider = &xpider;
        value.ptr_target = &target;
        temp_raw_map.insert(str_key,value);//we are using point as an ID of the xpider!!
      }
    }

    //step4. wrap in JSON and push to signal
    do{
      QJsonDocument jdoc;
      QJsonArray jarray;
      for(auto iter = opti_info_list.begin();iter!=opti_info_list.end();++iter){
        xpider_opti_t raw = *iter;
        QJsonObject jobj;

        jobj["theta"]= raw.theta;
        jobj["x"] = raw.x;
        jobj["y"] = raw.y;

        //update ID
        //HASH MAP checking is MUCH MUCH faster!!
        QString str_key = PointToString(QPointF(raw.x,raw.y));
        if(temp_raw_map.contains(str_key)){
          temp_map_val_t value = temp_raw_map.value(str_key);
          jobj["id"]=static_cast<int>(value.ptr_xpider->id);
          jobj["target_x"] = value.ptr_target->target_x;
          jobj["target_y"] = value.ptr_target->target_y;
        }else{
           jobj["id"]=-1;
           jobj["target_x"] = 0;
           jobj["target_y"] = 0;
        }

        //save to JARRAY
        jarray.push_back(jobj);
      }
      jdoc.setArray(jarray);
      emit xpiderListUpdate(QString(jdoc.toJson()));
    }while(0);


    //step5. check if we need to update all xpiders' plan
    int current_time = time_.elapsed();
    if(current_time-last_trigger_>INTERVAL_POST_TASK && is_planner_running_ && ptr_planner_thread_){
      //every 5sec
      last_trigger_ = current_time;
      ptr_planner_thread_->start();
    }

  }//end if(SERVER_UPLAOD_REQ==cmdid)
}

void OptiService::SyncXpiderTarget(std::vector<xpider_opti_t> &xpider_list, std::vector<xpider_target_point_t> &target_list)
{
  if(xpider_list.size()<=0)return;
  target_list.clear();
  for(auto iter=xpider_list.begin();iter!=xpider_list.end();++iter){
    xpider_opti_t xpider = *iter;
    xpider_target_point_t target;
    target.id = xpider.id;
    if(ui_target_mask_.count(xpider.id)){
      QPointF pos = ui_target_mask_.value(xpider.id);
      target.target_x = pos.x();
      target.target_y = pos.y();
    }else{
      target.target_x = xpider.x;
      target.target_y = xpider.y;
    }
    target_list.push_back(target);
  }
}

int OptiService::AvailableXpiderSocketID(uint32_t id_array[], int id_size)
{
  if(id_array==NULL || id_size<=0)return 0;
  int id_index=0;
  int id_len=0;
  for(auto iter=XpiderSocketThread::socket_list_.begin();
      iter!=XpiderSocketThread::socket_list_.end();
      ++iter)
  {
    XpiderSocketThread* xpider = *iter;
    if(xpider && xpider->Available()){
      id_array[id_len] = id_index;
      ++id_len;
    }
    ++id_index;
  }
  return id_len;
}

QString OptiService::PointToString(const QPointF &point){
  return QString("(%1,%2)").arg(point.x()).arg(point.y());
}

void OptiService::onClientDisconnected(){
  if(client_)client_=NULL;
}

void OptiService::onClientReadyRead(){
  if(client_==NULL)return;
  QByteArray rx_raw = client_->read(RX_MAX_SIZE);
  if(rx_raw.size()==0)return;
  protocol_.PushToProtocol(rx_raw);
}

void OptiService::pushTarget(unsigned int id, float x, float y){
  ui_target_mask_.insert(id,QPointF(x,y));
}

void OptiService::removeTarget(unsigned int id){
  //step1. remove target mask
  if(ui_target_mask_.count(id)){
    ui_target_mask_.remove(id);
  }

  //step2. stop the xpider
  XpiderSocketThread* x= XpiderSocketThread::socket_list_.at(id);
  if(x)x->StopWalking();
}

void OptiService::clearTargets(){
  XpiderSocketThread::XpiderList & xpider_list = XpiderSocketThread::socket_list_;
  ui_target_mask_.clear();
  for(auto iter=xpider_list.begin();iter!=xpider_list.end();++iter){
    XpiderSocketThread* socket = *iter;
    if(socket){
      socket->StopWalking();
    }
  }
}

void OptiService::enablePlanner(bool b){
  is_planner_running_ = b;
}

void OptiService::runCommandText(QString cmd_text){
  qDebug()<<tr("[%1,%2] command text is:%3").arg(__FILE__).arg(__LINE__).arg(cmd_text);
  ptr_cmd_thread_->StartCommandChain(cmd_text);
}

bool OptiService::csvLoadTargets(QString path)
{
  qDebug()<<tr("[%1,%2]loading target records from %3").arg(__FILE__).arg(__LINE__).arg(path);
  QFile file(path);
  if(!file.open(QIODevice::ReadOnly)){
    qDebug()<<tr("[%1,%2]Fail to load the input file:%3").arg(__FILE__).arg(__LINE__).arg(path);
    return false;
  }
  QTextStream text(&file);

  clearTargets();
  while(!text.atEnd()){
    QStringList record_list= text.readLine().split(",");
    unsigned int id = static_cast<unsigned int>(record_list[0].toInt());
    float x = record_list[1].toFloat();
    float y = record_list[2].toFloat();
    pushTarget(id,x,y);
  }
  file.close();
  return true;
}

bool OptiService::csvSaveTargets(QString path)
{
  qDebug()<<tr("[%1,%2]saving target records to %3").arg(__FILE__).arg(__LINE__).arg(path);
  QFile file(path);
  if(!file.open(QIODevice::WriteOnly)){
    qDebug()<<tr("[%1,%2]Fail to load the input file:%3").arg(__FILE__).arg(__LINE__).arg(path);
    return false;
  }
  QTextStream text(&file);
  QList<unsigned int> key_list = ui_target_mask_.keys();
  for(int i=0;i<key_list.size();++i){
    unsigned int id = key_list.at(i);
    QPointF p = ui_target_mask_.value(id);
    text<<id<<","<<p.x()<<","<<p.y()<<endl;

  }
  file.close();
  return true;
}


