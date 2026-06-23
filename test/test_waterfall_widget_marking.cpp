// Copyright 2026 University of New Hampshire
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
//    * Neither the name of the University of New Hampshire nor the names of its
//      contributors may be used to endorse or promote products derived from
//      this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

// Marking-mode test for the GPU WaterfallWidget: a left-drag rubber-band in
// mark mode must invert the marked pixel rect back into a map-frame box by
// reversing the render geometry (newest-at-top vertical scroll + centered
// across-track axis). The expected corners are hand-computed from the synthetic
// rows' WorldPose and geometry; a wrong inversion fails the corner asserts.
//
// The widget populates its render geometry (display_half_width_, ring_filled_,
// has_data_) during paintGL, so each test forces one offscreen render before
// simulating the drag. A real GL context is required; tests self-skip when none
// is available (headless without software GL), matching test_waterfall_widget.

#include <gtest/gtest.h>

#include <QApplication>
#include <QCoreApplication>
#include <QMouseEvent>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QPointF>
#include <QRectF>
#include <QSurfaceFormat>

#include <cmath>
#include <cstddef>
#include <memory>
#include <optional>

#include "marine_sonar_widgets/color_map.hpp"
#include "marine_sonar_widgets/waterfall_widget.hpp"

namespace
{

using marine_sonar_widgets::WaterfallRow;
using marine_sonar_widgets::WaterfallWidget;
using marine_sonar_widgets::WorldPose;

// The widget enforces a 256x256 minimum size, so resize() can't make it
// smaller; the test geometry is computed against the real on-screen size.
constexpr int kW = 256;      // widget width  (px); center column at 128
constexpr int kH = 256;      // widget height (px)
constexpr double kRange = 32.0;  // per-side slant range (m); px_per_unit == 4

// A flat-amplitude symmetric row with a known per-side range and no altitude
// (so the axis is slant == ground and across-track |d| maps straight to lateral
// metres). `samples` columns split evenly port|starboard at nadir.
WaterfallRow make_row(std::optional<WorldPose> pose, std::size_t samples = 64)
{
  WaterfallRow r;
  r.intensities.assign(samples, 1.0f);
  r.nadir_index = samples / 2;
  r.range_max = kRange;
  r.range_max_port = kRange;
  r.range_max_stbd = kRange;
  r.altitude = 0.0;            // no water column -> slant axis, ground == |d|
  r.world_pose = pose;
  return r;
}

// Subclass exposing the protected mouse handlers so a test can synthesize a
// drag without a running event loop (QTest dependency-free).
class TestableWaterfall : public WaterfallWidget
{
public:
  void press(const QPoint & p) {send(QEvent::MouseButtonPress, p);}
  void move(const QPoint & p) {send(QEvent::MouseMove, p);}
  void release(const QPoint & p) {send(QEvent::MouseButtonRelease, p);}

private:
  void send(QEvent::Type type, const QPoint & p)
  {
    QMouseEvent ev(
      type, QPointF(p), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    switch (type) {
      case QEvent::MouseButtonPress: mousePressEvent(&ev); break;
      case QEvent::MouseMove: mouseMoveEvent(&ev); break;
      case QEvent::MouseButtonRelease: mouseReleaseEvent(&ev); break;
      default: break;
    }
  }
};

bool gl_available()
{
  QSurfaceFormat fmt;
  fmt.setRenderableType(QSurfaceFormat::OpenGL);
  fmt.setVersion(3, 3);
  QOffscreenSurface surface;
  surface.setFormat(fmt);
  surface.create();
  if (!surface.isValid()) {
    return false;
  }
  QOpenGLContext ctx;
  ctx.setFormat(fmt);
  if (!ctx.create() || !ctx.makeCurrent(&surface)) {
    return false;
  }
  const QSurfaceFormat got = ctx.format();
  const bool ok =
    got.majorVersion() > 3 || (got.majorVersion() == 3 && got.minorVersion() >= 3);
  ctx.doneCurrent();
  return ok;
}

class WaterfallMarkingTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    if (QCoreApplication::instance() == nullptr) {
      static int argc = 1;
      static char arg0[] = "test_waterfall_widget_marking";
      static char * argv[] = {arg0, nullptr};
      app_ = std::make_unique<QApplication>(argc, argv);
    }
    if (!gl_available()) {
      GTEST_SKIP() << "No OpenGL 3.3 context available (headless without software GL).";
    }
  }

  // Force one offscreen render so the widget's geometry state is populated.
  static void render(WaterfallWidget & w)
  {
    w.resize(kW, kH);
    w.grabFramebuffer();
  }

  std::unique_ptr<QApplication> app_;
};

// pixel x -> signed display range d, mirroring the render math:
//   px_per_unit = (W/2) / half ; d = (px.x + 0.5 - W/2) / px_per_unit
double expected_d(int px_x, double half)
{
  const double cx = kW / 2.0;
  const double px_per_unit = (kW / 2.0) / half;
  return (static_cast<double>(px_x) + 0.5 - cx) / px_per_unit;
}

}  // namespace

// Across-track inversion: all rows share one pose, heading = +pi/2 (north), so
// across-track maps to map X (left vector = (-1,0); mx = pose.x + d) and map Y
// is the (single) pose.y. The marked x-span must equal the hand-computed d at
// each dragged column; y collapses to pose.y.
TEST_F(WaterfallMarkingTest, AcrossTrackInversionMatchesRenderGeometry)
{
  TestableWaterfall w;
  w.set_color_map(marine_sonar_widgets::ColorMapType::Grayscale);
  const WorldPose pose{100.0, 200.0, M_PI / 2.0};
  for (int i = 0; i < 5; ++i) {
    w.add_row(make_row(pose));
  }
  render(w);

  QRectF got;
  int hits = 0;
  QObject::connect(
    &w, &WaterfallWidget::boxMarked, &w, [&](QRectF r) {got = r; ++hits;});

  w.setMarkMode(true);
  w.press(QPoint(64, 40));
  w.move(QPoint(192, 200));
  w.release(QPoint(192, 200));

  ASSERT_EQ(hits, 1) << "boxMarked should fire once for a fully-posed region";

  // half == display_half_width_ == kRange (uniform scale, symmetric rows).
  const double d_left = expected_d(64, kRange);    // < 0 (port)
  const double d_right = expected_d(192, kRange);  // > 0 (starboard)
  const double mx_lo = pose.x + std::min(d_left, d_right);
  const double mx_hi = pose.x + std::max(d_left, d_right);

  EXPECT_NEAR(got.left(), mx_lo, 1e-6);
  EXPECT_NEAR(got.right(), mx_hi, 1e-6);
  EXPECT_NEAR(got.top(), pose.y, 1e-6);
  EXPECT_NEAR(got.bottom(), pose.y, 1e-6);
}

// Full 2D inversion: distinct per-row poses in Y exercise the vertical
// pixel->row selection (newest at top), while heading = +pi/2 maps across-track
// to X. The dragged box spans the full height, so the top corner lands on the
// newest row (highest Y pose) and the bottom corner on the oldest (lowest Y).
TEST_F(WaterfallMarkingTest, FullBoxSpansRowPosesAndAcrossTrack)
{
  TestableWaterfall w;
  w.set_color_map(marine_sonar_widgets::ColorMapType::Grayscale);
  // Row buf index 0 = oldest .. 4 = newest. pose.y increases with index, so the
  // newest (top of display) has the largest Y. pose.x constant.
  const double base_x = 500.0;
  for (int i = 0; i < 5; ++i) {
    w.add_row(make_row(WorldPose{base_x, 1000.0 + 10.0 * i, M_PI / 2.0}));
  }
  render(w);

  QRectF got;
  int hits = 0;
  QObject::connect(
    &w, &WaterfallWidget::boxMarked, &w, [&](QRectF r) {got = r; ++hits;});

  w.setMarkMode(true);
  w.press(QPoint(64, 0));
  w.move(QPoint(192, kH - 1));
  w.release(QPoint(192, kH - 1));

  ASSERT_EQ(hits, 1);

  // py = 0 -> newest row (buf 4, Y = 1040); py = kH-1 -> oldest (buf 0, Y = 1000).
  EXPECT_NEAR(got.top(), 1000.0, 1e-6);
  EXPECT_NEAR(got.bottom(), 1040.0, 1e-6);

  // Across-track corners map to X (same as the previous test).
  const double d_left = expected_d(64, kRange);
  const double d_right = expected_d(192, kRange);
  EXPECT_NEAR(got.left(), base_x + std::min(d_left, d_right), 1e-6);
  EXPECT_NEAR(got.right(), base_x + std::max(d_left, d_right), 1e-6);
}

// Rows without a WorldPose cannot be projected: marking over them must emit
// nothing (matching the reference SidescanWaterfall guard).
TEST_F(WaterfallMarkingTest, NoPoseEmitsNoSignal)
{
  TestableWaterfall w;
  w.set_color_map(marine_sonar_widgets::ColorMapType::Grayscale);
  for (int i = 0; i < 5; ++i) {
    w.add_row(make_row(std::nullopt));  // no world_pose
  }
  render(w);

  int hits = 0;
  QObject::connect(
    &w, &WaterfallWidget::boxMarked, &w, [&](QRectF) {++hits;});

  w.setMarkMode(true);
  w.press(QPoint(16, 10));
  w.move(QPoint(48, 50));
  w.release(QPoint(48, 50));

  EXPECT_EQ(hits, 0) << "no projectable pose -> no boxMarked";
}

// Mark mode off: a drag must not emit (the widget had no prior mouse behavior).
TEST_F(WaterfallMarkingTest, MarkModeOffEmitsNoSignal)
{
  TestableWaterfall w;
  for (int i = 0; i < 5; ++i) {
    w.add_row(make_row(WorldPose{0.0, 0.0, 0.0}));
  }
  render(w);

  int hits = 0;
  QObject::connect(
    &w, &WaterfallWidget::boxMarked, &w, [&](QRectF) {++hits;});

  // setMarkMode never called -> mark_mode_ == false.
  w.press(QPoint(16, 10));
  w.move(QPoint(48, 50));
  w.release(QPoint(48, 50));

  EXPECT_EQ(hits, 0);
}
