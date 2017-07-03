#ifndef TRAJECTORYPLANNER_H
#define TRAJECTORYPLANNER_H
#include <stdint.h>
#include <math.h>
#include <QDebug>
class TrajectoryPlanner
{

public:
    int max_dis_id;
    float max_dis_x;
    float max_dis_y;
    float min_dis;                       //安全距离

    typedef struct xpider_opti_s{        //输入，包括当前位置信息
        uint32_t id;
        float theta;                     //[-pi, pi]
        float x;                         //in meter
        float y;                         //in meter
    }xpider_opti_t;

    typedef struct xpider_tp_s{          //输出的结构体
        uint32_t id;
        float delta_theta; //[-pi, pi]
        int detla_step;
    }xpider_tp_t;

    typedef struct xpider_taget_point_s{   //输入，包括目的位置信息
        uint32_t id;
        float target_x;
        float target_y;
    }xpider_target_point_t;

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
    int Plan(xpider_opti_t info[],xpider_target_point_t target_list[],int info_len,xpider_tp_t out_action[],int out_size);
};

#endif // TRAJECTORYPLANNER_H
