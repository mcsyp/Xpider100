#include "commanddegree.h"
#include <QTextStream>
#include <map>
#include <xpider_ctl/xpider_info.h>
#include <xpider_ctl/xpider_protocol.h>
#include <xpidersocketthread.h>
#include <math.h>
#include <time.h>

const QString CommandDegree::KEY = QString("degree");
const QString CommandDegree::ALL = QString("all");
const QString CommandDegree::RAND = QString("rand");

CommandDegree::CommandDegree(QObject* parent):CommandParser(parent){
  QTextStream text;
  text.setString(&example_);
  text<<"degree [index] [angle]"<<endl;
  text<<"eg: degree all 10"<<endl;
  text<<"eg: aim 10 180"<<endl;
}

bool CommandDegree::Exec(QStringList argv){
  if(argv.length()==0 ||
     argv.size()<MIN_LEN ||
     argv[0].isEmpty()){
    return false;
  }
  //step1 check first key
  if(argv[0]!=KEY)return false;

  //step2 check index ?.,m b
  int id=0;
  bool is_all_included=false;
  if(argv[1]==ALL){
    is_all_included=true;
  }else{
    id = argv[1].toInt();
  }
  //step3. check target
  float target_theta = 0;
  if(argv.contains(RAND)){
    static unsigned int rand_counter=0;
    rand_counter++;
    srand(rand_counter+time(NULL));
    target_theta = 2*M_PI*static_cast<float>(rand())/static_cast<float>(RAND_MAX);
  }else{
    target_theta = fmod(argv[2].toFloat(),360.0f)*M_PI/180.0f;
  }

  qDebug()<<tr("[%1,%2] executing degree %3 %4")
            .arg(__FILE__).arg(__LINE__)
            .arg(argv[1])
            .arg(target_theta);

  //step4. duplicate xpider_queue
  std::vector<xpider_opti_t> local_xpider_queue;
  const TrajectoryThread* ptr_planner = OptiService::Singleton()->Planner();
  if(ptr_planner==NULL)return false;
  for(int i=0;i<10;++i){
    //retry 10 times
    local_xpider_queue =  ptr_planner->xpider_queue_;
    if(local_xpider_queue.size()>0){
      break;
    }
    QThread::msleep(5);
  }
  if(local_xpider_queue.size()==0)return false;


  //step4. compute target
  for(int i=0;i<local_xpider_queue.size();++i){
    xpider_opti_t x = local_xpider_queue[i];
    if(is_all_included || x.id == id){
      float delta = ComputeDelta(x,target_theta);
      SendCommand(x.id,delta);
      //qDebug()<<tr("[%1,%2] sending %3 to xpider_%4").arg(__FILE__).arg(__LINE__).arg(delta).arg(x.id);
    }
  }

  return true;
}

void CommandDegree::SendCommand(int id, float delta_theta)
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

float CommandDegree::ComputeDelta(const xpider_opti_t& xpider, float heading_theta)
{
  float delta_rad1, delta_rad2;
  delta_rad1 = xpider.theta-heading_theta;// - info[i].theta;
  delta_rad2 = delta_rad1<0 ? M_PI*2.0f+delta_rad1 : delta_rad1-M_PI*2.0f;
  delta_rad1 = abs(delta_rad1)-abs(delta_rad2)>0 ? delta_rad2 : delta_rad1;
  return delta_rad1;
}

