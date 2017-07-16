#include "commandwalk.h"

#include "commandwalk.h"
#include <QTextStream>
#include <map>
#include <xpider_ctl/xpider_info.h>
#include <xpider_ctl/xpider_protocol.h>
#include <xpidersocketthread.h>
#include <math.h>

const QString CommandWalk::KEY = QString("walk");
const QString CommandWalk::ALL = QString("all");

CommandWalk::CommandWalk(QObject* parent):CommandParser(parent){
  QTextStream text;
  text.setString(&example_);
  text<<"walk [index] [step]"<<endl;
  text<<"eg: walk all 10"<<endl;
  text<<"eg: wakk 4 -10"<<endl;
}

bool CommandWalk::Exec(QStringList argv){
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
  int delta_step = argv[2].toInt();
  if(abs(delta_step)<=1){
    qDebug()<<tr("[%1,%2] invalid walk step:%3").arg(__FILE__).arg(__LINE__).arg(delta_step);
    return false;
  }

  qDebug()<<tr("[%1,%2] executing walk %3 %4")
            .arg(__FILE__).arg(__LINE__)
            .arg(argv[1])
            .arg(delta_step);

  //step4. walk
  for(int i=0;i<XpiderSocketThread::socket_list_.size();++i){
    if(is_all_included || i == id){
      SendCommand(i,delta_step);
      qDebug()<<tr("[%1,%2] sending %3 to xpider_%4").arg(__FILE__).arg(__LINE__).arg(delta_step).arg(i);
    }
  }

  return true;
}

void CommandWalk::SendCommand(int id, int step)
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
  info.rotate_speed = 0;
  info.rotate_rad = 0;
  info.walk_speed = OptiService::XPIDER_WALK_SPEED;
  info.walk_step = step;
  protocol.GetBuffer(protocol.kAutoMove, &tx_buffer, &tx_length);

  tx_pack.append((char*)tx_buffer,tx_length);

  XpiderSocketThread *x = XpiderSocketThread::socket_list_.at(id);
  if(x){
    x->SendMessage(tx_pack);
  }
}
