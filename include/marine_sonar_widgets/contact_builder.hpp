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

#ifndef MARINE_SONAR_WIDGETS__CONTACT_BUILDER_HPP_
#define MARINE_SONAR_WIDGETS__CONTACT_BUILDER_HPP_

#include <string>
#include <vector>

#include "marine_interfaces/msg/contact.hpp"

// Framework-agnostic target-marking core for the shared sonar widgets. Turns a
// drawn map-frame box (the WaterfallWidget marking mode emits one as a QRectF,
// the offline viewer as corner points) into the unified
// marine_interfaces/msg/Contact, so the live rqt plugin and the offline viewer
// produce the same contact. Depends only on marine_interfaces (+ STL) — no rqt /
// rclcpp / rosbag2 — per the library's dependency boundary (ADR-0001).

namespace marine_sonar_widgets
{

// A 2D point in the map (e.g. bizzy/map ENU) plane, metres.
struct MapPoint
{
  double x = 0.0;
  double y = 0.0;
};

// Build a human-origin BOX Contact from the map-frame footprint of a drawn box.
// `points` are the box corners (or any point set) in map metres; the contact's
// kinematics pose is their centroid, the BOX dimensions their extent. The contact
// is STATUS_PROPOSED, existence_probability 1.0, frame_id `frame`. `geo_pose` is
// left unresolved (latitude = NaN) — lat/lon resolution is a follow-up; same-datum
// overlay uses the map-frame pose directly.
marine_interfaces::msg::Contact make_box_contact(
  const std::vector<MapPoint> & points, const std::string & id,
  const std::string & source, const std::string & frame, double stamp_s);

}  // namespace marine_sonar_widgets

#endif  // MARINE_SONAR_WIDGETS__CONTACT_BUILDER_HPP_
