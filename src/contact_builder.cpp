// Copyright 2026 Roland Arsenault
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "marine_sonar_widgets/contact_builder.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

namespace marine_sonar_widgets
{

marine_interfaces::msg::Contact make_box_contact(
  const std::vector<MapPoint> & points, const std::string & id,
  const std::string & source, const std::string & frame, double stamp_s)
{
  marine_interfaces::msg::Contact c;
  c.header.frame_id = frame;
  const double t = std::max(0.0, stamp_s);   // guard negative -> nanosec wraparound
  c.header.stamp.sec = static_cast<int32_t>(t);
  c.header.stamp.nanosec =
    static_cast<uint32_t>((t - static_cast<double>(c.header.stamp.sec)) * 1e9);
  c.id = id;
  c.source = source;
  c.existence_probability = 1.0f;            // human-drawn
  c.origin_kind = marine_interfaces::msg::Contact::ORIGIN_HUMAN;
  c.status = marine_interfaces::msg::Contact::STATUS_PROPOSED;

  // BOX shape + centroid pose from the drawn footprint.
  c.shape.type = marine_interfaces::msg::Shape::BOX;
  c.kinematics.orientation_availability =
    marine_interfaces::msg::Kinematics::ORIENTATION_UNAVAILABLE;
  c.kinematics.pose.pose.orientation.w = 1.0;

  if (!points.empty()) {
    double min_x = points.front().x;
    double max_x = min_x;
    double min_y = points.front().y;
    double max_y = min_y;
    for (const auto & p : points) {
      min_x = std::min(min_x, p.x);
      max_x = std::max(max_x, p.x);
      min_y = std::min(min_y, p.y);
      max_y = std::max(max_y, p.y);
    }
    c.kinematics.pose.pose.position.x = 0.5 * (min_x + max_x);
    c.kinematics.pose.pose.position.y = 0.5 * (min_y + max_y);
    c.shape.dimensions.x = max_x - min_x;
    c.shape.dimensions.y = max_y - min_y;
    c.shape.dimensions.z = 0.0;
  }

  // geo_pose unresolved (lat/lon resolution is a follow-up): NaN latitude marks it.
  c.geo_pose.position.latitude = std::numeric_limits<double>::quiet_NaN();
  return c;
}

}  // namespace marine_sonar_widgets
