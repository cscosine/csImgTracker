#pragma once
#include <Eigen/Core>
#include <vector>

struct TimestampAndPoints {
  double timestamp;
  std::vector<Eigen::Vector2f> points;
};
