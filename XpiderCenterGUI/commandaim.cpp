#include "commandaim.h"
#include <QTextStream>
#include <map>
#include <xpider_ctl/xpider_info.h>
#include <xpider_ctl/xpider_protocol.h>
#include <xpidersocketthread.h>
#include <math.h>

const QString CommandAim::KEY = QString("aim");
const QString CommandAim::ALL = QString("all");

CommandAim::CommandAim(QObject* parent):CommandParser(parent){
  QTextStream text;
  text.setString(&example_);
  text<<"aim [index] [x] [y]"<<endl;
  text<<"eg: aim all 0.12 -1.2"<<endl;
  text<<"eg: aim 10 0.2 0.23"<<endl;
}

bool CommandAim::Exec(QStringList argv){
  if(argv.length()==0 ||
     argv.size()<MIN_LEN ||
     argv[0].isEmpty()){
    return false;
  }
  //step1 check first key
  if(argv[0]!=KEY)return false;

  //step2 check index
  int id=0;
  bool is_all_included=false;
  if(argv[1]==ALL){
    is_all_included=true;
  }else{
    id = argv[1].toInt();
  }
  //step3. check target
  float target_x = argv[2].toFloat();
  float target_y = argv[3].toFloat();

  qDebug()<<tr("[%1,%2] executing aim %3 %4 %5")
            .arg(__FILE__).arg(__LINE__)
            .arg(argv[1])
            .arg(target_x)
            .arg(target_y);

  //step4. duplicate xpider_queue
  std::vector<xpider_opti_t> local_xpider_queue;
  const TrajectoryThread* ptr_planner = OptiService::Singleton()->Planner();
  if(ptr_planner==NULL){
    qDebug()<<tr("[%1,%2] No planner thread found").arg(__FILE__).arg(__LINE__);
    return false;
  }
  for(int i=0;i<10;++i){
    //retry 10 times
    local_xpider_queue =  ptr_planner->xpider_queue_;
    if(local_xpider_queue.size()>0){
      break;
    }
    QThread::msleep(5);
  }
  if(local_xpider_queue.size()==0){
    qDebug()<<tr("[%1,%2] No xpider found in the list.").arg(__FILE__).arg(__LINE__);
    return false;
  }
  //step4. compute target
  for(int i=0;i<local_xpider_queue.size();++i){
    xpider_opti_t x = local_xpider_queue[i];
    if(is_all_included || x.id == id){
      float delta = ComputeDelta(x,target_x,target_y);
      SendCommand(x.id,delta);
      //qDebug()<<tr("[%1,%2] sending %3 to xpider_%4").arg(__FILE__).arg(__LINE__).arg(delta).arg(x.id);
    }
  }
  qDebug()<<tr("[%1,%2]aim command successfully done").arg(__FILE__).arg(__LINE__);

  return true;
}

void CommandAim::SendCommand(int id, float delta_theta)
{
  if(id<0 && id>=XpiderSocketThread::socket_list_.size()){
    return;
  }

  QByteArray tx_pack;
  uint8_t* tx_buffer;
  uint16_t tx_length;
  XpiderInfo info;
  XpiderProtocol  protocol;
  protocol.Initialize(&info);

  //step1.set target angle & transform to tx buffer
  info.rotate_speed = 100;
  info.rotate_rad = delta_theta;
  info.walk_speed = 0;
  info.walk_step = 0;
  protocol.GetBuffer(protocol.kAutoMove, &tx_buffer, &tx_length);

  tx_pack.append((char*)tx_buffer,tx_length);

  XpiderSocketThread *x = XpiderSocketThread::socket_list_.at(id);
  if(x){
    x->SendMessage(tx_pack);
  }
}

float CommandAim::ComputeDelta(const xpider_opti_t& xpider, float target_x, float target_y)
{
  float distance = sqrt(pow((target_x-xpider.x),2)+pow((target_y-xpider.x),2));
  float A1 = acos((target_x-xpider.x)/distance);
  if (target_y-xpider.y<0) {
    A1 = 2.0f*M_PI-A1;                             //A1~(0,2M_PI)
  }
  float delta_rad1, delta_rad2;
  delta_rad1 = xpider.theta-A1;// - info[i].theta;
  delta_rad2 = delta_rad1<0 ? M_PI*2.0f+delta_rad1 : delta_rad1-M_PI*2.0f;
  delta_rad1 = abs(delta_rad1)-abs(delta_rad2)>0 ? delta_rad2 : delta_rad1;
  return delta_rad1;
}

