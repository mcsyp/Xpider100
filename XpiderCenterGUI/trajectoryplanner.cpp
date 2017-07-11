#include "trajectoryplanner.h"

TrajectoryPlanner::TrajectoryPlanner()
{
  max_dis_id = 0;
  max_dis_x = 0;
  max_dis_y = 0;
  min_dis = 0.18;             //20cm
  min_target_dis = 0.10;      //6cm
  target_len_ = 0;
  wait_max_number = 2;
}

bool TrajectoryPlanner::GenerateTargetList(xpider_opti_t info[], int info_len){
  float target_dis = 0;
  target_len_ = 0;
  for (int i=0; i<info_len; i++){
    if(info[i].valid_target == true) {
      float target_dis_ = sqrt(pow(info[i].target_x,2)+pow(info[i].target_y,2));
      if (target_dis_ > target_dis){
        target_dis = target_dis_;                //找到最大的距离值
        max_dis_id = info[i].id;          //记录最大距离值的ID
        max_dis_x = info[i].target_x;
        max_dis_y = info[i].target_y;
      }
      target_list_[target_len_].id = info[i].id;
      target_list_[target_len_].target_x = info[i].target_x;
      target_list_[target_len_].target_y = info[i].target_y;
      target_len_++;
    }
  }
  return true;
}

int TrajectoryPlanner::Plan(xpider_opti_t info[], int info_len, xpider_tp_t out_action[],int out_size){
  GenerateTargetList(info, info_len);

  float priority[100] = {};
  qDebug()<<"target_len:"<<target_len_;
  qDebug()<<"info_len:"<<info_len;

  //step1:保存要移动的输入列表.
  for(int i=0; i<target_len_; i++) {
    for(int j=0; j<info_len; j++) {
      if(target_list_[i].id==info[j].id) {
        info_move[i].id = info[j].id;
        info_move[i].theta = info[j].theta;
        info_move[i].x = info[j].x;
        info_move[i].y = info[j].y;
      }
    }
  }

  //step2:计算要行走的优先级.    优先级数组长度和info[]相同，只有行走的有优先级，不行走的置为0
  for (int i=0; i<target_len_; i++) {
    float p = sqrt(pow((info_move[i].x-max_dis_x),2)+pow((info_move[i].y-max_dis_y),2));\
    priority[info_move[i].id] = p;      //优先级下标代表id！！！
  }

  //step3:计算旋转角度，行走步数.
  float info_target_dis = 0;
  float A2[target_len_];
  for (int i=0; i<target_len_; i++) {
    info_target_dis = sqrt(pow((target_list_[i].target_y-info_move[i].y),2)+pow((target_list_[i].target_x-info_move[i].x),2));
    float A1 = acos((target_list_[i].target_x-info_move[i].x)/info_target_dis);
    if (target_list_[i].target_y-info_move[i].y<0) {
      A1 = 2.0f*M_PI-A1;                               //A1~(0,2M_PI)
    }
    A2[i] = A1;
    float delta_rad1, delta_rad2;
    delta_rad1 = info_move[i].theta-A1;// - info[i].theta;
    delta_rad2 = delta_rad1<0 ? M_PI*2.0f+delta_rad1 : delta_rad1-M_PI*2.0f;
    delta_rad1 = abs(delta_rad1)-abs(delta_rad2)>0 ? delta_rad2 : delta_rad1;

    out_action[i].id = target_list_[i].id;
    out_action[i].delta_theta = delta_rad1;
    out_action[i].detla_step = 5;
  }

  //step4:计算每个id之间的距离值.
  for (int i=0; i<target_len_; i++) {
    for (int j=0; j<info_len; j++) {
      if (info_move[i].id!=info[j].id) {
        float D_dist = sqrt(pow((info_move[i].x-info[j].x),2)+pow((info_move[i].y-info[j].y),2));
        float d_dist = sqrt(pow(info_move[i].x-target_list_[i].target_x,2)+pow(info_move[i].y-target_list_[i].target_y,2));
        if (d_dist < min_target_dis) {         //如果近似到达终点，优先级降为0.
          priority[i] = 0;
        }
        if (D_dist<min_dis) {
          qDebug()<<"D_dist"<<D_dist;
          if (priority[info_move[i].id] < priority[info_move[j].id]) {
            out_action[i].detla_step = 0;      //此处可解决行进过程中的碰撞问题
            qDebug()<<"+++++++++++++++++++++++++++++++++++++++";
            break;
          } else {                             //此处解决行进与静止的碰撞问题
            out_action[i].delta_theta = out_action[i].delta_theta + M_PI/2;
            float L = 0.15f;
            float R = 0.075f;
            float x1 = info_move[i].x + L*cos(out_action[i].delta_theta);
            float y1 = info_move[i].y + L*sin(out_action[i].delta_theta);
            for (int k=0; k<info_len; k++) {
              if (k!=i) {
                float r = sqrt(pow(info[j].x - x1, 2) + pow(info[j].y - y1, 2));
                if (r < R) {
                  out_action[i].detla_step = 0;
                  break;
                } else {
                  out_action[i].detla_step = 3;
                }
              }
            }
            qDebug()<<"----------------------------------------";
            break;
          }
        }
      }
    }
  }

  //step5:若长时间等待，转角90°

//  float L = 0.15f;
//  float R = 0.075f;
//  for (int i=0; i<target_len_; i++) {
//    if (id_to_id[out_action[i].id]>5) {
//      id_to_id[out_action[i].id] = 0;
//      A2[i] = A2[i] + M_PI/2.0f;

//      float x1 =info_move[i].x + L*cos(A2[i]);
//      float y1 =info_move[i].y + L*sin(A2[i]);

//      qDebug()<<"x1:"<<x1;
//      qDebug()<<"y1:"<<y1;
//      qDebug()<<"ID:"<<info_move[i].id;

//      //此处判断转角后，前方有无障碍物
//      for (int j=0; j<info_len; j++) {
//        float r = sqrt(pow(info[j].x - x1, 2) + pow(info[j].y - y1, 2));
//        qDebug()<<"r:"<<r;
//        if (r < R) {
//          out_action[i].delta_theta = out_action[i].delta_theta + M_PI/2;
//          out_action[i].detla_step = 0;
//          break;
//        } else {
//          out_action[i].delta_theta = out_action[i].delta_theta + M_PI/2;
//          out_action[i].detla_step = 3;
//          break;
//        }
//      }
//    }
//  }

  for (int i=0; i<target_len_; i++){
//    qDebug()<<"target_list_["<<i<<"]:      ID:"<<target_list_[i].id;
//    qDebug()<<"target_list_["<<i<<"]:target_x:"<<target_list_[i].target_x;
//    qDebug()<<"target_list_["<<i<<"]:target_y:"<<target_list_[i].target_y;
    qDebug()<<"out_action["<<i<<"]:       ID:"<<out_action[i].id;
    qDebug()<<"out_action["<<i<<"]:delta_theta:"<<out_action[i].delta_theta;
    qDebug()<<"out_action["<<i<<"]:          θ:"<<(out_action[i].delta_theta)*(180/M_PI);
    qDebug()<<"out_action["<<i<<"]: detla_step:"<<out_action[i].detla_step;
//    qDebug()<<"info_move["<<i<<"]:         ID:"<<info_move[i].id;
//    qDebug()<<"info_move["<<i<<"]:delta_theta:"<<info_move[i].theta;
//    qDebug()<<"info_move["<<i<<"]:          θ:"<<(info_move[i].theta)*(180/M_PI);
//    qDebug()<<"info_move["<<i<<"]:          x:"<<info_move[i].x;
//    qDebug()<<"info_move["<<i<<"]:          y:"<<info_move[i].y;
//    qDebug()<<'\n';
  }
  return target_len_;
}
