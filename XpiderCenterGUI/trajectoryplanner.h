#ifndef TRAJECTORYPLANNER_H
#define TRAJECTORYPLANNER_H
#include <stdint.h>
#include <math.h>
#include <QDebug>
#include "global_xpier.h"

const float PI=3.14159;

class TrajectoryPlanner {

public:
  static constexpr int MAX_TARGET_SIZE=200;
  TrajectoryPlanner();
    /*purpose: reset the planer with the area width and height
     * Target area info also included
     *input:
     * @width, the area width, in m
     * @height, the area height in meter
     * @target_list, the target points of all xpiders
     * @target_len, the target_list len
     *return
     * return true is reset success
     */

  bool Reset(float width, float height, xpider_target_point_t target_list[],int target_len);
    /*purpose: reset the planer with the area width and height
     * Target area info also included
     *input:
     * @info, the xpider loaction info from optitract analyer
     * @info_len, the length of the info list
     *output:
     * @out_action, the calculated action of all xpiders
     * @out_size, the size of the out_action array.
     *return
     * return the length of the out_action array
     */
  int Plan(xpider_opti_t info[], int info_len, xpider_tp_t out_action[], int out_size);
public:
  int max_dis_id;
  float max_dis_x;
  float max_dis_y;
  float min_dis;                       //每只之间的安全距离
  float min_target_dis;                //近似到达目的地的距离
  int wait_number;                     //等待最大次数
  xpider_target_point_t target_list_[MAX_TARGET_SIZE];
  int id_to_id[MAX_TARGET_SIZE];
  int target_len_;
};

#endif // TRAJECTORYPLANNER_H
