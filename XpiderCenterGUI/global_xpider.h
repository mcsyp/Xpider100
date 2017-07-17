#ifndef GLOBAL_XPIER_H
#define GLOBAL_XPIER_H

typedef struct xpider_opti_s{        //xpider info from optitrac
    int id;

    uint8_t loose_count; //for YETIAN's location count

    float theta;                     //[0, 2pi]
    float x;                         //in meter
    float y;                         //in meter

    bool valid_target; //if the target x and y are valid
    float target_x;
    float target_y;
}xpider_opti_t;

typedef struct xpider_tp_s{          //输出的结构体
    int id;
    float delta_theta; //[-pi, pi]
    int detla_step;
}xpider_tp_t;


#endif // GLOBAL_XPIER_H
