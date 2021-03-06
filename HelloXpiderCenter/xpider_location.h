#ifndef XPIDER_LOCATION_H
#define XPIDER_LOCATION_H

#include <map>
#include <vector>
#include "global_xpier.h"

#define ROBOT_LOOSE_FRAME_COUNT 30
#define MARKER_MAX_DISTANCE 0.055f
#define ROBOT_PLACE_WIDTH 0.15

class XpiderLocation {
public:
//  enum RobotStatus {
//    kOffline,
//    kOnline,
//    kLosePos,
//  };
//
//  struct XpiderStruct {             //xpider info from optitrac
//    bool online;
//    bool located;
//    uint32_t id;
//    float x;                        //in meter
//    float y;                        //in meter
//    float theta;                    //[0, 2pi]
//  };
  struct Point{
    float x;
    float y;
  };
  typedef std::vector<Point> LandmarkList;

private:
  std::vector<Point> initialize_points_;
  std::map<int, xpider_opti_t> *robot_data_;
  std::vector<std::vector<Point> > robot_init_location;

public:
  explicit XpiderLocation();

  bool GetRobotLocation(std::vector<xpider_opti_t> & raw_info, uint32_t id_list[],
                        uint32_t id_len, std::vector<xpider_opti_t> &out_list);
  void GenerateInitLocation(float center_x, float center_y, uint8_t rows, uint8_t cols);
  LandmarkList & Landmarks(){return initialize_points_;}
};

#endif // XPIDER_LOCATION_H
