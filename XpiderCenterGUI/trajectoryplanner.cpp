#include "trajectoryplanner.h"

TrajectoryPlanner::TrajectoryPlanner()
{
  max_dis_id = 0;
  max_dis_x = 0;
  max_dis_y = 0;
  min_dis = 0.26;             //20cm
  min_target_dis = 0.8;      //6cm
  wait_max_number = 2;
}

int TrajectoryPlanner::Plan(xpider_opti_t info[], int info_len, xpider_tp_t out_action[],int out_size){

  qDebug()<<"info_len:"<<info_len;

  //step1:找到最远的距离点,定为标准点
  float target_dis = 0;
  for (int i=0; i<info_len; i++) {
    if (info[i].valid_target == true) {
      float target_dis_ = sqrt(pow(info[i].target_x,2)+pow(info[i].target_y,2));
      if (target_dis_ > target_dis) {
        target_dis = target_dis_;                //找到最大的距离值
        max_dis_x = info[i].target_x;
        max_dis_y = info[i].target_y;
      }
    }
  }

  //step2:计算要行走的优先级.    优先级数组长度和info[]相同，只有行走的有优先级，不行走的置为0
  float priority[200];
  float A[200];
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
      A[i] = A1;
      float delta_rad1, delta_rad2;
      delta_rad1 = info[i].theta-A1;// - info[i].theta;
      delta_rad2 = delta_rad1<0 ? M_PI*2.0f+delta_rad1 : delta_rad1-M_PI*2.0f;
      delta_rad1 = abs(delta_rad1)-abs(delta_rad2)>0 ? delta_rad2 : delta_rad1;
      out_action[i].id = info[i].id;
      out_action[i].delta_theta = delta_rad1;
      out_action[i].detla_step = 2;
    }
  }

  //step3:路径规划.
  for (int i = 0; i<info_len; i++) {
    if (info[i].valid_target == true) {          //i为需要移动的
      qDebug()<<"info["<<i<<"]:      ID:"<<info[i].id;
      qDebug()<<"info["<<i<<"]:       x:"<<info[i].x;
      qDebug()<<"info["<<i<<"]:       x:"<<info[i].y;
      for (int j=0; j<info_len; j++) {
        if (info[i].id != info[j].id) {
          float D_dist = sqrt(pow((info[i].x-info[j].x),2)+pow((info[i].y-info[j].y),2));
          float d_dist = sqrt(pow(info[i].x-info[i].target_x,2)+pow(info[i].y-info[i].target_y,2));
          if (d_dist < min_target_dis) {         //如果近似到达终点，优先级降为0.
            qDebug()<<"d_dist:"<<d_dist;
            priority[i] = 0;
          }

          if (D_dist < min_dis) {
            qDebug()<<"D_dist:"<<D_dist;
            if (info[j].valid_target == false) {  //j是静止的蜘蛛
              float L = 0.15f;
              float R = 0.09f;
              float x1 = info[i].x + L*cos(A[i]);
              float y1 = info[i].y + L*sin(A[i]);
              float r = sqrt(pow(info[j].x - x1, 2) + pow(info[j].y - y1, 2));//判断j是否位于i前面的圆内.
              if (r < R) {                                 //j在圆内，i转90°走4步.
                out_action[i].delta_theta = out_action[i].delta_theta + M_PI/2;
                out_action[i].detla_step = 2;
              }                                  //j不在圆内，按预定角度步数走.
            } else {                             //j是运动的蜘蛛
              float L = 0.15f;
              float R = 0.09f;
              float x1 = info[i].x + L*cos(A[i]);
              float y1 = info[i].y + L*sin(A[i]);
              float r = sqrt(pow(info[j].x - x1, 2) + pow(info[j].y - y1, 2));
              if (r < R) {
                if (priority[i] < priority[j]) {
                  out_action[j].detla_step = 0;    //此时i先走，i应检测前方
                  float L = 0.15f;
                  float R = 0.09f;
                  float x1 = info[i].x + L*cos(A[i]);
                  float y1 = info[i].y + L*sin(A[i]);
                  for (int k = 0; k<info_len; k++) {
                    float r = sqrt(pow(info[k].x - x1, 2) + pow(info[k].y - y1, 2));
                    if (r < R) {
                      qDebug()<<"r--i:"<<r;
                      out_action[i].detla_step = 0;
                      out_action[i].delta_theta = out_action[i].delta_theta + M_PI/2;
                      A[i] = A[i] + M_PI/2;
                    }
                  }
              } else {
                  out_action[i].detla_step = 0;    //此时j先走，j应检测前方
                  float L = 0.15f;
                  float R = 0.09f;
                  float x1 = info[j].x + L*cos(A[j]);
                  float y1 = info[j].y + L*sin(A[j]);
                  for (int k = 0; k<info_len; k++) {
                    float r = sqrt(pow(info[k].x - x1, 2) + pow(info[k].y - y1, 2));
                    if (r < R) {
                      qDebug()<<"r--j:"<<r;
                      out_action[j].detla_step = 0;
                      out_action[j].delta_theta = out_action[i].delta_theta + M_PI/2;
                      A[j] = A[j] + M_PI/2;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }

//            if (info[j].valid_target == false) {       //此时j为静止的蜘蛛;第一种情况j不位于i与目标点的连线上。i可正常行走

//            }
//              out_action[i].delta_theta = out_action[i].delta_theta + M_PI/2;   //此处应该是距离目标的角度
//              while (out_action[i].delta_theta > M_PI) out_action[i].delta_theta = out_action[i].delta_theta - M_PI;
//              while (out_action[i].delta_theta < M_PI) out_action[i].delta_theta = out_action[i].delta_theta + M_PI;
//              qDebug()<<"+++++++++++++++++++++";

//              float L = 0.15f;
//              float R = 0.175f;
//              float x1 = info[i].x + L*cos(out_action[i].delta_theta);
//              float y1 = info[i].y + L*sin(out_action[i].delta_theta);
//              qDebug()<<"x1:"<<x1;
//              qDebug()<<"y1:"<<y1;
//              for (int k=0; k<info_len; k++) {
//                if (k!=i) {
//                  float r = sqrt(pow(info[j].x - x1, 2) + pow(info[j].y - y1, 2));
//                  qDebug()<<"r:"<<r;
//                  if (r < R) {
//                    out_action[i].detla_step = 0;//有一只则置为0.
//                    break;
//                  } else {
//                    out_action[i].detla_step = 3;
//                  }
//                }
//              }
//            } else {
//              if (priority[i]<priority[j]) out_action[j].detla_step = 0;
//              if (priority[i]>priority[j]) out_action[i].detla_step = 0;
//            }
//          }
//        }
//      }
//    }
//  }

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
//    qDebug()<<"info["<<i<<"]:      ID:"<<info[i].id;
//    qDebug()<<"info["<<i<<"]:       x:"<<info[i].x;
//    qDebug()<<"info["<<i<<"]:       x:"<<info[i].y;

    qDebug()<<"out_action["<<i<<"]:         ID:"<<out_action[i].id;
    qDebug()<<"out_action["<<i<<"]:delta_theta:"<<out_action[i].delta_theta;
    qDebug()<<"out_action["<<i<<"]:          θ:"<<(out_action[i].delta_theta)*(180/M_PI);
    qDebug()<<"out_action["<<i<<"]: detla_step:"<<out_action[i].detla_step;
  }
  return info_len;
}
