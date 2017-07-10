#include "trajectoryplanner.h"

TrajectoryPlanner::TrajectoryPlanner()
{
  max_dis_id = 0;
  max_dis_x = 0;
  max_dis_y = 0;
  min_dis = 0.22;             //20cm
  min_target_dis = 0.06;      //6cm
  target_len_ = 0;
  wait_max_number = 2;
  memset(id_to_id, 0, 5050);
}

bool TrajectoryPlanner::Reset(float width, float height, xpider_target_point_t target_list[], int target_len){
  float target_dis = 0;
  for (int i=0; i<target_len; i++){
    float target_dis_ = sqrt(pow(target_list[i].target_x,2)+pow(target_list[i].target_y,2));
    if (target_dis_ > target_dis){
      target_dis = target_dis_;                //找到最大的距离值
      max_dis_id = target_list[i].id;          //记录最大距离值的ID
      max_dis_x = target_list[i].target_x;
      max_dis_y = target_list[i].target_y;
    }
    target_list_[i].id = target_list[i].id;
    target_list_[i].target_x = target_list[i].target_x;
    target_list_[i].target_y = target_list[i].target_y;
  }
  target_len_ = target_len;
    qDebug() << "target_len-----" << target_len_;
  return true;
}

int TrajectoryPlanner::Plan(xpider_opti_t info[], int info_len, xpider_tp_t out_action[],int out_size){
  qDebug() << "target len on plan:" <<  target_len_;
  //step1:计算当前位置和最大距离值点的距离，判断优先级
  float priority[info_len+5];
  for (int i=0; i<info_len; i++){
    float p = sqrt(pow((info[i].x-max_dis_x),2)+pow((info[i].y-max_dis_y),2));
    priority[i] = p;
    //qDebug()<<"priority["<<i<<"]"<<priority[i];
  }

  qDebug() << "info len:" << info_len;
  //target_len_ = 1;
  //step2:计算旋转角度，行走步数
  float info_target_dis;
  for (int i=0; i<info_len; i++) {
    for(int j=0; j<target_len_; j++) {
      if (info[i].id==target_list_[j].id) {
        info_target_dis = sqrt(pow((target_list_[j].target_y-info[i].y),2)+pow((target_list_[j].target_x-info[i].x),2));
        float A1 = acos((target_list_[j].target_x-info[i].x)/info_target_dis);
        if (target_list_[j].target_y-info[i].y<0) {
          A1 = 2.0f*PI-A1;                               //A1~(0,2PI)
        }
        //qDebug()<<"info[].theta"<<info[i].theta*180/PI;
        //qDebug()<<"A1:"<<A1*180/PI;
        float delta_rad1, delta_rad2;
        delta_rad1 = info[i].theta-A1;// - info[i].theta;
        delta_rad2 = delta_rad1<0 ? PI*2.0+delta_rad1 : delta_rad1-PI*2.0;
        delta_rad1 = abs(delta_rad1)-abs(delta_rad2)>0 ? delta_rad2 : delta_rad1;

        //qDebug()<<"Theta:"<<-delta_rad1*180/PI;
        qDebug() << "j:" << j;

        out_action[j].delta_theta = delta_rad1;
        out_action[j].id = target_list_[j].id;
        out_action[j].detla_step = 5;
      }
    }
  }

  //step3:计算每个id之间的距离值
  for (int i=0; i<info_len; i++) {
    for (int j=0; j<info_len; j++) {
      if (info[i].id<info[j].id) {
        float D_dist = sqrt(pow((info[i].x-info[j].x),2)+pow((info[i].y-info[j].y),2));
        qDebug()<<"ID_to_ID_distence:"<<D_dist;
        if (D_dist<min_dis){
          if (priority[i]<priority[j]) {
            out_action[j].detla_step = 0;    //低于安全距离的，优先级低置零
            //out_action[j].delta_theta = 0;
            id_to_id[j]++;
          } else {
            out_action[i].detla_step = 0;
            //out_action[i].delta_theta = 0;
            id_to_id[i]++;
          }
        }
      }
    }
  }

  //step4:若长时间等待，转角90°
  for (int i=0; i<target_len_; i++) {
    for (int j=0; j<info_len; j++) {
      if ((id_to_id[i] > wait_max_number)&&(target_list_[i].id==info[j].id)) {
        out_action[i].delta_theta = out_action[i].delta_theta - PI/2;
        out_action[i].detla_step = 3;
        id_to_id[i] = 0;
        qDebug()<<"+++";
      }
    }
  }

  //step5:计算每个ID与目标点之间的距离值
  /*for (int i=0; i<info_len; i++) {
    float aim_dis = sqrt(pow((info[i].x-target_list_[i].target_x),2)+pow((info[i].y-target_list_[i].target_y),2));
    //qDebug()<<"aim_dis["<<i<<"]"<<aim_dis;
    if (aim_dis<min_target_dis) {
      out_action[i].detla_step = 0;
      out_action[i].delta_theta = 0;
    }
  }*/
  for (int i=0; i<target_len_; i++){
    qDebug()<<"target_len"<<target_len_;
    qDebug()<<"out_action["<<i<<"]:         ID:"<<out_action[i].id;
    qDebug()<<"out_action["<<i<<"]:delta_theta:"<<out_action[i].delta_theta;
    qDebug()<<"out_action["<<i<<"]:          θ:"<<(out_action[i].delta_theta)*(180/PI);
    qDebug()<<"out_action["<<i<<"]: detla_step:"<<out_action[i].detla_step;
    qDebug()<<"id_to_id["<<i<<"]"<<id_to_id[i];
    qDebug()<<'\n';
  }
  return target_len_;
}
