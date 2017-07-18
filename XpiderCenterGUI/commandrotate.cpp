#include "commandrotate.h"
#include <QTextStream>
#include <map>
#include <xpider_ctl/xpider_info.h>
#include <xpider_ctl/xpider_protocol.h>
#include <xpidersocketthread.h>
#include <math.h>

const QString CommandRotate::KEY = QString("rotate");
const QString CommandRotate::ALL = QString("all");

CommandRotate::CommandRotate(QObject* parent):CommandParser(parent){
  QTextStream text;
  text.setString(&example_);
  text<<"rotate [index] [speed[0-100]]"<<endl;
  text<<"eg: rotate all 32"<<endl;
  text<<"eg: rotate 10 -32"<<endl;
}

bool CommandRotate::Exec(QStringList argv){
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
  int rotate_speed = argv[2].toInt();
  if(abs(rotate_speed)<MIN_ROTATE_SPEED)rotate_speed=MIN_ROTATE_SPEED;
  if(abs(rotate_speed)>MAX_ROTATE_SPEED)rotate_speed=MAX_ROTATE_SPEED;
  qDebug()<<tr("[%1,%2] executing rotate %3 %4")
            .arg(__FILE__).arg(__LINE__)
            .arg(argv[1])
            .arg(rotate_speed);

  //step4. compute target
  for(int i=0;i<XpiderSocketThread::socket_list_.size();++i){
    if(is_all_included || i==id){
      SendCommand(i,rotate_speed);
    }
  }

  return true;
}

void CommandRotate::SendCommand(int id, int speed)
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
  info.rotate = speed;
  info.move = 0;
  protocol.GetBuffer(protocol.kMove, &tx_buffer, &tx_length);

  tx_pack.append((char*)tx_buffer,tx_length);

  XpiderSocketThread *x = XpiderSocketThread::socket_list_.at(id);
  if(x){
    x->SendMessage(tx_pack);
  }
  qDebug()<<tr("[%1,%2]sending command to %3 with speed %4")
            .arg(__FILE__).arg(__LINE__)
            .arg(id).arg(speed);

}

