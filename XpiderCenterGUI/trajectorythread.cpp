#include "trajectorythread.h"
#include "xpidersocketthread.h"
#include <QDebug>
#include "xpider_ctl/xpider_info.h"
#include "xpider_ctl/xpider_protocol.h"
#include "optiserver.h"
TrajectoryThread::TrajectoryThread(QObject* parent):QThread(parent){

}
void TrajectoryThread::run(){
  if(xpider_queue_.size()==0 || target_queue_.size()==0){
    return;
  }
  //step1.call trajectory planner
  const int action_size = xpider_queue_.size();
  xpider_tp_t action_list[action_size];
  int action_len = planner_.Plan(xpider_queue_,target_queue_,action_list,action_size);

  //step2.call all xpiders to move
  for(int i=0;i<action_len;++i){
    XpiderSocketThread * socket = XpiderSocketThread::socket_list_.at(action_list[i].id);
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
      qDebug()<<tr("[%1,%2]sending package:%3").arg(__FILE__).arg(__LINE__).arg(QString(tx_pack));
      //step3. tx pack
      socket->SendMessage(tx_pack);
    }
  }
}
