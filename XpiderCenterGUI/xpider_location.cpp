#include "xpider_location.h"

#include <QDebug>
#include <math.h>

XpiderLocation::XpiderLocation() {
  robot_data_ = new std::map<int, xpider_opti_t>();
}

void XpiderLocation::GenerateInitLocation(float center_x, float center_y, uint8_t rows, uint8_t cols) {
  Point top_left, temp_point;
  top_left.x = center_x - cols*ROBOT_PLACE_WIDTH/2.0f;
  top_left.y = center_y - rows*ROBOT_PLACE_WIDTH/2.0f;

  // qDebug() << "top_left_x:" << top_left.x << ", top_left_y:" << top_left.y;

  initialize_points_.clear();
  //initialize_points_.resize(rows);

  for(uint8_t i=0; i<rows; i++) {
    for(uint8_t j=0; j<cols; j++) {
      temp_point.x = top_left.x + ROBOT_PLACE_WIDTH*(j+0.5);
      temp_point.y = top_left.y + ROBOT_PLACE_WIDTH*(i+0.5);
      initialize_points_.push_back(temp_point);
    }
  }

//  for(uint8_t ii=0; ii<rows*cols; ii++) {
//    qDebug() << "x: " << initialize_points_[ii].x << "y:" << initialize_points_[ii].y;
//  }
}


bool XpiderLocation::GetRobotLocation(std::vector<xpider_opti_t> & raw_info, uint32_t id_list[],
                                      uint32_t id_len, std::vector<xpider_opti_t> &out_list) {
  int32_t best_markers_index = -1;
  uint32_t robot_index, temp_index=0;
  float distance, min_distance;

  std::map<int, xpider_opti_t>::iterator it;
  std::map<int, xpider_opti_t> *temp_robot_data = new std::map<int, xpider_opti_t>();

  // qDebug() << "Robot data size:" << robot_data_->size();

  out_list.clear();
  for(uint8_t i=0; i<id_len; i++) {
    best_markers_index = -1;
    robot_index = id_list[i];

    it = robot_data_->find(robot_index);
    if(it == robot_data_->end()) {
      /*
       * if there is no history data,
       * initialize the location.
       */

      // qDebug() << "goin init" << initialize_points_[robot_index].x << initialize_points_[robot_index].y;
      xpider_opti_t data;
      data.id = robot_index;
      data.loose_count = ROBOT_LOOSE_FRAME_COUNT;
      data.x = initialize_points_[robot_index].x;
      data.y = initialize_points_[robot_index].y;
      out_list.push_back(data);
    } else {
      /*
       * use history data
       */
      // qDebug() << "use old";
      out_list.push_back(it->second);
    }

    min_distance = MARKER_MAX_DISTANCE;
    for(uint8_t j=0; j<raw_info.size(); j++) {
      distance = sqrt(pow(out_list[temp_index].x-raw_info[j].x, 2)+pow(out_list[temp_index].y-raw_info[j].y, 2));
      if(distance < min_distance) {
        min_distance = distance;
        best_markers_index = j;
      }
    }

    if(best_markers_index != -1) {
      /*
       * if find the point, update robot location
       */
      // qDebug() << "match: " << robot_index;
      out_list[temp_index].loose_count = 0;
      out_list[temp_index].x = raw_info[best_markers_index].x;
      out_list[temp_index].y = raw_info[best_markers_index].y;
      out_list[temp_index].theta = raw_info[best_markers_index].theta;
      temp_robot_data->insert(std::pair<int, xpider_opti_t>(robot_index, out_list[temp_index]));
      temp_index ++;
    } else {
      /*
       * else remove the robot
       */
      if(out_list[temp_index].loose_count >= ROBOT_LOOSE_FRAME_COUNT) {
        out_list.pop_back();
      } else {
        out_list[temp_index].loose_count++;
        temp_robot_data->insert(std::pair<int, xpider_opti_t>(robot_index, out_list[temp_index]));
        temp_index ++;
      }
    }
  }

  //robot_data_->clear();
  delete robot_data_;
  robot_data_ = temp_robot_data;
  return true;
}
