#include "commandaim.h"
#include <QTextStream>
#include <map>
#include <xpider_ctl/xpider_info.h>
#include <xpider_ctl/xpider_protocol.h>
#include <xpidersocketthread.h>

const QString CommandAim::KEY = QString("led");
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

  //step3. parse the target
  float real_x = argv[2].toFloat();
  float real_y = argv[3].toFloat();
  qDebug()<<tr("[%1,%2] targeting to (%3,%4)").arg(__FILE__).arg(__LINE__).arg(real_x).arg(real_y);

  //step4. executate
  do{
    OptiService * serivce = OptiService::Singleton();
    //step1. clear all targets
    serivce->clearTargets();
    //step2.

  }while(0);

  return true;
}

void CommandAim::SendCommand(int id, float delta_theta)
{
  QByteArray tx_pack;
  uint8_t* tx_buffer;
  uint16_t tx_length;
  XpiderInfo info;
  XpiderProtocol  protocol;
  protocol.Initialize(&info);

  //step1.set target angle & transform to tx buffer
  info.rotate_speed = OptiService::XPIDER_ROTATE_SPEED;
  info.rotate_rad = delta_theta;
  info.walk_speed = 0;
  info.walk_step = 0;
  protocol.GetBuffer(protocol.kAutoMove, &tx_buffer, &tx_length);

  protocol.GetBuffer(protocol.kFrontLeds, &tx_buffer, &tx_length);
  tx_pack.append((char*)tx_buffer,tx_length);

  XpiderSocketThread *x = XpiderSocketThread::socket_list_.at(id);
  if(x){
    x->SendMessage(tx_pack);
  }
}

