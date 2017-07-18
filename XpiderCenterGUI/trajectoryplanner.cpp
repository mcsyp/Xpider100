#include "trajectoryplanner.h"

TrajectoryPlanner::TrajectoryPlanner()
{
  max_dis_id = 0;
  max_dis_x = 0;
  max_dis_y = 0;
  min_dis = 0.20;             //20cm
  min_target_dis = 0.15;      //6cm
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

  //step2:计算要行走的优先级.(优先级数组长度和info[]相同，只有行走的有优先级，不行走的置为0)
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
  float L = 0.18f;
  float R = 0.09f;
  for (int i = 0; i<info_len; i++) {
    if (info[i].valid_target == true) {          //i为需要移动的.
      for (int j=0; j<info_len; j++) {
        if (info[i].id != info[j].id) {
          float D_dist = sqrt(pow((info[i].x-info[j].x),2)+pow((info[i].y-info[j].y),2));
//          float d_dist = sqrt(pow(info[i].x-info[i].target_x,2)+pow(info[i].y-info[i].target_y,2));
//          if (d_dist < min_target_dis) {         //如果近似到达终点，优先级降为0.
//            qDebug()<<"d_dist:"<<d_dist;
//            priority[i] = 0;
//          }

          if (D_dist < min_dis) {
            qDebug()<<"D_dist:"<<D_dist;

            //j是静止的蜘蛛.
            if (info[j].valid_target == false) {
              float x1 = info[i].x + L*cos(A[i]);
              float y1 = info[i].y + L*sin(A[i]);
              float r = sqrt(pow(info[j].x - x1, 2) + pow(info[j].y - y1, 2));
              qDebug()<<"r"<<r;
              if (r < R) {//判断j是否位于i前面的圆内.
                out_action[i].delta_theta = out_action[i].delta_theta + M_PI/2;
                float x1 = info[i].x + L*cos(A[i] + M_PI/2);//j在圆内，i转90°.
                float y1 = info[i].y + L*sin(A[i] + M_PI/2);//判断i转角后的前进方向是否有障碍物.
                for(int p=0; p<info_len; p++) {
                  float r = sqrt(pow(info[j].x - x1, 2) + pow(info[j].y - y1, 2));
                  if (r < R) {                              //有障碍物，再转90°.
                    out_action[i].delta_theta = out_action[i].delta_theta + M_PI/2;
                    out_action[i].detla_step = 0;
                  }
                }
              }                                   //j不在圆内，按预定角度步数走.
            } else {

              //j是运动的蜘蛛.
              float x1 = info[i].x + L*cos(A[i]);
              float y1 = info[i].y + L*sin(A[i]);
              float r = sqrt(pow(info[j].x - x1, 2) + pow(info[j].y - y1, 2));
              if (r < R) {                         //j在i的前方
                if (priority[i] < priority[j]) {
                  out_action[j].detla_step = 0;    //此时i先走，i应检测前方.
                  qDebug()<<"_+_+_+_+_+_+_+_+_+_+_+_+_+_+_+_+_+_+_";
                  wait_number[j]++;                //j等待计数器加一.
                  float x1 = info[i].x + L*cos(A[i]);
                  float y1 = info[i].y + L*sin(A[i]);
                  for (int k = 0; k<info_len; k++) {
                    float r = sqrt(pow(info[k].x - x1, 2) + pow(info[k].y - y1, 2));
                    if (r < R) {
                      out_action[i].detla_step = 0;
                      out_action[i].delta_theta = out_action[i].delta_theta + M_PI/2;
                      A[j] = A[j] + M_PI/2;
                    }
                  }
                } else {
                  out_action[i].detla_step = 0;    //此时j先走，j应检测前方.
                  qDebug()<<"-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=";
                  wait_number[i]++;                //i等待计数器加一.
                  float x1 = info[j].x + L*cos(A[j]);
                  float y1 = info[j].y + L*sin(A[j]);
                  for (int k = 0; k<info_len; k++) {
                    float r = sqrt(pow(info[k].x - x1, 2) + pow(info[k].y - y1, 2));
                    if (r < R) {
                      out_action[j].detla_step = 0;
                      out_action[j].delta_theta = out_action[i].delta_theta + M_PI/2;
                      A[j] = A[j] + M_PI/2;
                    }
                  }
                }
              } /*else {                       //j不在i前方，预测ij下一时刻是否会撞
                float L_number_step = 0.05;
                float x_i = info[i].x + L_number_step*cos(A[i]);
                float y_i = info[i].y + L_number_step*cos(A[i]);
                float x_j = info[j].x + L_number_step*cos(A[j]);
                float y_j = info[j].y + L_number_step*cos(A[j]);
                float D_forecase = sqrt(pow(x_i-x_j, 2) + pow(y_i-y_j, 2));
                if (D_forecase < 0.18) {
                  out_action[j].detla_step = 0;
                  out_action[j].delta_theta = out_action[i].delta_theta + M_PI/2;
                  wait_number[j]++;
                }
              }*/
            }
          }
        }
      }
    }
  }

  //step4:长时间等待，触发此处
  for (int i=0; i<info_len; i++) {
    if (wait_number[i] > 6) {
      qDebug()<<"+++++++++++++++++++++++++++++++";
      wait_number[i] = 0;
      out_action[i].detla_step = 2;
      out_action[i].delta_theta = A[i] + M_PI/2;
      for(int j=0; j<info_len; j++) {
        float x1 = info[j].x + L*cos(A[i]);
        float y1 = info[j].y + L*sin(A[i]);
        for (int k = 0; k<info_len; k++) {
          float r = sqrt(pow(info[k].x - x1, 2) + pow(info[k].y - y1, 2));
          if (r < R) {
            out_action[j].detla_step = 0;
            out_action[j].delta_theta = out_action[i].delta_theta + M_PI/2;
            A[j] = A[j] + M_PI/2;
          }
        }
      }
    }
  }

  //打印输出结果
  for (int i=0; i<info_len; i++){
    qDebug()<<"out_action["<<i<<"]:         ID:"<<out_action[i].id;
    qDebug()<<"out_action["<<i<<"]:delta_theta:"<<out_action[i].delta_theta;
    qDebug()<<"out_action["<<i<<"]:          θ:"<<(out_action[i].delta_theta)*(180/M_PI);
    qDebug()<<"out_action["<<i<<"]: detla_step:"<<out_action[i].detla_step;
  }
  return info_len;
}
