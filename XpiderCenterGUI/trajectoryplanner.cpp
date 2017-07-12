#include "trajectoryplanner.h"

TrajectoryPlanner::TrajectoryPlanner()
{
  max_dis_id = 0;
  max_dis_x = 0;
  max_dis_y = 0;
  min_dis = 0.22;             //20cm
  min_target_dis = 0.15;      //6cm
  wait_max_number = 2;
}

int TrajectoryPlanner::Plan(xpider_opti_t info[], int info_len, xpider_tp_t out_action[],int out_size){

  //GenerateTargetList(info, info_len);

  float priority[200];
  qDebug()<<"info_len:"<<info_len;

  //step1:计算要行走的优先级.    优先级数组长度和info[]相同，只有行走的有优先级，不行走的置为0
  for (int i=0; i<info_len; i++) {
    if (info[i].valid_target == true) {
      float p = sqrt(pow((info[i].x-max_dis_x),2)+pow((info[i].y-max_dis_y),2));
      priority[i] = p;
    } else {
      priority[i] = 0;
    }
    qDebug()<<"priority:"<<priority[i];
  }

  //step2:计算旋转角度，行走步数.
  float info_target_dis = 0;
  for (int i=0; i<info_len; i++) {
    out_action[i].id = info[i].id;
    out_action[i].delta_theta = 0;
    out_action[i].detla_step = 0;
    if (info[i].valid_target == true) {
      info_target_dis = sqrt(pow((info[i].target_y-info[i].y),2)+pow((info[i].target_x-info[i].x),2));
      float A1 = acos((info[i].target_x-info[i].x)/info_target_dis);
      if (info[i].target_y-info[i].y<0) {
        A1 = 2.0f*M_PI-A1;                             //A1~(0,2M_PI)
      }
      float delta_rad1, delta_rad2;
      delta_rad1 = info[i].theta-A1;// - info[i].theta;
      delta_rad2 = delta_rad1<0 ? M_PI*2.0f+delta_rad1 : delta_rad1-M_PI*2.0f;
      delta_rad1 = abs(delta_rad1)-abs(delta_rad2)>0 ? delta_rad2 : delta_rad1;
      out_action[i].id = info[i].id;
      out_action[i].delta_theta = delta_rad1;
      out_action[i].detla_step = 4;
    }
  }

  //step3:计算每个id之间的距离值.
  for (int i = 0; i<info_len; i++) {
    if (info[i].valid_target == true) {      //当i为需要移动的
      for (int j=0; j<info_len; j++) {
        if (info[i].id != info[j].id) {
          float D_dist = sqrt(pow((info[i].x-info[j].x),2)+pow((info[i].y-info[j].y),2));
          float d_dist = sqrt(pow(info[i].x-info[i].target_x,2)+pow(info[i].y-info[i].target_y,2));
          qDebug()<<"D_dist:"<<D_dist;
          qDebug()<<"d_dist:"<<d_dist;
          if (d_dist < min_target_dis) {         //如果近似到达终点，优先级降为0.
            priority[i] = 0;
          }
          if (D_dist < min_dis) {
            if (info[j].valid_target == false) {       //此时j为静止的蜘蛛;i旋转90°，看有无障碍物

              out_action[i].delta_theta = out_action[i].delta_theta + M_PI/2;   //此处应该是距离目标的角度
              qDebug()<<"+++++++++++++++++++++";
              float L = 0.15f;
              float R = 0.175f;
              float x1 = info[i].x + L*cos(out_action[i].delta_theta);
              float y1 = info[i].y + L*sin(out_action[i].delta_theta);
              qDebug()<<"x1:"<<x1;
              qDebug()<<"y1:"<<y1;
              for (int k=0; k<info_len; k++) {
                if (k!=i) {
                  float r = sqrt(pow(info[j].x - x1, 2) + pow(info[j].y - y1, 2));
                  qDebug()<<"r:"<<r;
                  if (r < R) {
                    out_action[i].detla_step = 0;//有一只则置为0.
                    break;
                  } else {
                    out_action[i].detla_step = 3;
                  }
                }
              }
            } else {
              if (priority[i]<priority[j]) out_action[j].detla_step = 0;
              if (priority[i]>priority[j]) out_action[i].detla_step = 0;
            }
          }
        }
      }
    }
  }

            /*
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
  }*/

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

  for (int i=0; i<info_len; i++){
//    qDebug()<<"target_list_["<<i<<"]:      ID:"<<target_list_[i].id;
//    qDebug()<<"target_list_["<<i<<"]:target_x:"<<target_list_[i].target_x;
//    qDebug()<<"target_list_["<<i<<"]:target_y:"<<target_list_[i].target_y;
    qDebug()<<"out_action["<<i<<"]:         ID:"<<out_action[i].id;
    qDebug()<<"out_action["<<i<<"]:delta_theta:"<<out_action[i].delta_theta;
    qDebug()<<"out_action["<<i<<"]:          θ:"<<(out_action[i].delta_theta)*(180/M_PI);
    qDebug()<<"out_action["<<i<<"]: detla_step:"<<out_action[i].detla_step;
  }
  return info_len;
}
