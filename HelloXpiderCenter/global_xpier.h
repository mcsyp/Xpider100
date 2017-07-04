#ifndef GLOBAL_XPIER_H
#define GLOBAL_XPIER_H

typedef struct xpider_opti_s{        //xpider info from optitrac
    uint32_t id;
    float theta;                     //[-pi, pi]
    float x;                         //in meter
    float y;                         //in meter
    int loose_count;
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

#endif // GLOBAL_XPIER_H
