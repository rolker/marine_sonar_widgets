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

#include <gtest/gtest.h>

#include <cmath>
#include <string>
#include <vector>

#include "marine_sonar_widgets/contact_builder.hpp"
#include "marine_interfaces/msg/contact.hpp"

using marine_sonar_widgets::MapPoint;
using marine_sonar_widgets::make_box_contact;
using Contact = marine_interfaces::msg::Contact;

// A two-corner box becomes a BOX contact with the centroid pose, the extent as
// dimensions, human/proposed metadata, and an unresolved (NaN) geo_pose.
TEST(ContactBuilder, BoxContactCentroidAndExtent)
{
  const std::vector<MapPoint> box{{10.0, 20.0}, {14.0, 26.0}};
  const Contact c = make_box_contact(box, "T-001", "sidescan.port", "bizzy/map", 1234.5);
  EXPECT_EQ(c.header.frame_id, "bizzy/map");
  EXPECT_EQ(c.id, "T-001");
  EXPECT_EQ(c.source, "sidescan.port");
  EXPECT_FLOAT_EQ(c.existence_probability, 1.0f);
  EXPECT_EQ(c.origin_kind, Contact::ORIGIN_HUMAN);
  EXPECT_EQ(c.status, Contact::STATUS_PROPOSED);
  EXPECT_EQ(c.shape.type, marine_interfaces::msg::Shape::BOX);
  EXPECT_NEAR(c.kinematics.pose.pose.position.x, 12.0, 1e-9);  // centroid
  EXPECT_NEAR(c.kinematics.pose.pose.position.y, 23.0, 1e-9);
  EXPECT_NEAR(c.shape.dimensions.x, 4.0, 1e-9);                // extent
  EXPECT_NEAR(c.shape.dimensions.y, 6.0, 1e-9);
  EXPECT_TRUE(std::isnan(c.geo_pose.position.latitude));       // unresolved
  EXPECT_EQ(c.header.stamp.sec, 1234);
  EXPECT_EQ(c.header.stamp.nanosec, 500000000u);  // 0.5 s fractional part
}

// All four corners (any point set) collapse to the same axis-aligned extent as
// the two diagonal corners, and a negative stamp is clamped to 0 (no nanosec
// wraparound).
TEST(ContactBuilder, FourCornersAndStampGuard)
{
  const std::vector<MapPoint> box{
    {10.0, 20.0}, {14.0, 20.0}, {14.0, 26.0}, {10.0, 26.0}};
  const Contact c = make_box_contact(box, "T-002", "sidescan.starboard", "bizzy/map", -5.0);
  EXPECT_NEAR(c.kinematics.pose.pose.position.x, 12.0, 1e-9);
  EXPECT_NEAR(c.kinematics.pose.pose.position.y, 23.0, 1e-9);
  EXPECT_NEAR(c.shape.dimensions.x, 4.0, 1e-9);
  EXPECT_NEAR(c.shape.dimensions.y, 6.0, 1e-9);
  EXPECT_EQ(c.header.stamp.sec, 0);       // negative clamped
  EXPECT_EQ(c.header.stamp.nanosec, 0u);
}

// An empty point set still yields a well-formed BOX contact: identity
// orientation, zero pose/dimensions, metadata intact.
TEST(ContactBuilder, EmptyPointsIsWellFormed)
{
  const Contact c = make_box_contact({}, "T-003", "src", "bizzy/map", 1.0);
  EXPECT_EQ(c.shape.type, marine_interfaces::msg::Shape::BOX);
  EXPECT_EQ(c.status, Contact::STATUS_PROPOSED);
  EXPECT_DOUBLE_EQ(c.kinematics.pose.pose.orientation.w, 1.0);
  EXPECT_DOUBLE_EQ(c.kinematics.pose.pose.position.x, 0.0);
  EXPECT_DOUBLE_EQ(c.kinematics.pose.pose.position.y, 0.0);
  EXPECT_DOUBLE_EQ(c.shape.dimensions.x, 0.0);
  EXPECT_DOUBLE_EQ(c.shape.dimensions.y, 0.0);
  EXPECT_TRUE(std::isnan(c.geo_pose.position.latitude));
}
