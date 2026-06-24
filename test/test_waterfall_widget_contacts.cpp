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

// Existing-contact overlay test for the GPU WaterfallWidget (issue #6): a
// contact set via setContacts() must be drawn (magenta box) on the pass(es) that
// ensonified it, on the correct across-track side, and NOT drawn when it lies
// outside every row's range. Verified by rendering offscreen and scanning the
// grabbed framebuffer for magenta pixels. A real GL context is required; the test
// self-skips when none is available, matching the sibling widget tests.

#include <gtest/gtest.h>

#include <QApplication>
#include <QColor>
#include <QCoreApplication>
#include <QImage>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QSurfaceFormat>

#include <cmath>
#include <cstddef>
#include <memory>
#include <optional>

#include "marine_sonar_widgets/color_map.hpp"
#include "marine_sonar_widgets/waterfall_widget.hpp"

namespace
{

using marine_sonar_widgets::ContactBox;
using marine_sonar_widgets::WaterfallRow;
using marine_sonar_widgets::WaterfallWidget;
using marine_sonar_widgets::WorldPose;

constexpr int kW = 256;
constexpr int kH = 256;
constexpr double kRange = 32.0;  // per-side slant range (m)

WaterfallRow make_row(std::optional<WorldPose> pose, std::size_t samples = 64)
{
  WaterfallRow r;
  r.intensities.assign(samples, 1.0f);
  r.nadir_index = samples / 2;
  r.range_max = kRange;
  r.range_max_port = kRange;
  r.range_max_stbd = kRange;
  r.altitude = 0.0;            // slant == ground; across-track |d| is lateral m
  r.world_pose = pose;
  return r;
}

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

// A "magenta" overlay pixel: strong R + B, weak G (robust to AA blending).
bool is_magenta(QRgb p)
{
  return qRed(p) > 180 && qBlue(p) > 180 && qGreen(p) < 80;
}

struct MagentaStats
{
  int count = 0;
  double mean_x = 0.0;
  double mean_y = 0.0;
};

MagentaStats scan_magenta(const QImage & img)
{
  MagentaStats s;
  double sum_x = 0.0;
  double sum_y = 0.0;
  for (int y = 0; y < img.height(); ++y) {
    for (int x = 0; x < img.width(); ++x) {
      if (is_magenta(img.pixel(x, y))) {
        ++s.count;
        sum_x += x;
        sum_y += y;
      }
    }
  }
  if (s.count > 0) {
    s.mean_x = sum_x / s.count;
    s.mean_y = sum_y / s.count;
  }
  return s;
}

class WaterfallContactsTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    if (QCoreApplication::instance() == nullptr) {
      static int argc = 1;
      static char arg0[] = "test_waterfall_widget_contacts";
      static char * argv[] = {arg0, nullptr};
      app_ = std::make_unique<QApplication>(argc, argv);
    }
    if (!gl_available()) {
      GTEST_SKIP() << "No OpenGL 3.3 context available (headless without software GL).";
    }
  }

  // Five rows along a +pi/2 (north) track at x=0, y increasing. Heading +pi/2 =>
  // left vector (-1, 0): a contact with x>0 is starboard (right), x<0 is port.
  static void posed_rows(WaterfallWidget & w)
  {
    for (int i = 0; i < 5; ++i) {
      w.add_row(make_row(WorldPose{0.0, 1000.0 + 10.0 * i, M_PI / 2.0}));
    }
  }

  static QImage render(WaterfallWidget & w)
  {
    w.resize(kW, kH);
    return w.grabFramebuffer();
  }

  std::unique_ptr<QApplication> app_;
};

// A starboard contact (x>0) within range draws a magenta box right of centre.
TEST_F(WaterfallContactsTest, StarboardContactDrawsRightOfCentre)
{
  WaterfallWidget w;
  w.set_color_map(marine_sonar_widgets::ColorMapType::Grayscale);
  posed_rows(w);
  render(w);  // populate paint_rows_

  // y=1020 matches the middle row's pose (rows at y=1000..1040), so the box sits
  // near mid-height: closest-approach ridx=2 of 5 -> py ~= 0.5*kH.
  w.setContacts({ContactBox{10.0, 1020.0, 4.0, 4.0, QString("T-1")}});
  const MagentaStats s = scan_magenta(render(w));

  ASSERT_GT(s.count, 0) << "in-range contact should draw a box";
  EXPECT_GT(s.mean_x, kW / 2.0) << "starboard contact must draw right of centre";
  EXPECT_GT(s.mean_y, 70.0);
  EXPECT_LT(s.mean_y, 190.0) << "mid-track contact should box near mid-height";
}

// Slant-display mode (altitude > 0): the in-range gate must use the SLANT range,
// not the raw ground offset. A contact whose ground offset is within the per-side
// range but whose slant exceeds it must NOT draw (the must-fix regression).
TEST_F(WaterfallContactsTest, SlantModeGatesOnSlantRange)
{
  WaterfallWidget w;
  w.set_color_map(marine_sonar_widgets::ColorMapType::Grayscale);
  w.set_ground_range(false);  // slant axis; side range stays the slant kRange
  const double alt = 20.0;
  for (int i = 0; i < 5; ++i) {
    WaterfallRow r = make_row(WorldPose{0.0, 1000.0 + 10.0 * i, M_PI / 2.0});
    r.altitude = alt;
    w.add_row(r);
  }
  render(w);

  // Ground offset 20 -> slant hypot(20,20)=28.3 < 32: in range, draws.
  w.setContacts({ContactBox{20.0, 1020.0, 4.0, 4.0, QString("in")}});
  EXPECT_GT(scan_magenta(render(w)).count, 0) << "slant within range should draw";

  // Ground offset 28 (< 32) but slant hypot(28,20)=34.4 > 32: beyond the swath,
  // must NOT draw. Gating on the ground offset (the bug) would draw it.
  w.setContacts({ContactBox{28.0, 1020.0, 4.0, 4.0, QString("out")}});
  EXPECT_EQ(scan_magenta(render(w)).count, 0)
    << "ground offset within range but slant beyond -> no draw";
}

// A port contact (x<0) within range draws left of centre.
TEST_F(WaterfallContactsTest, PortContactDrawsLeftOfCentre)
{
  WaterfallWidget w;
  w.set_color_map(marine_sonar_widgets::ColorMapType::Grayscale);
  posed_rows(w);
  render(w);

  w.setContacts({ContactBox{-10.0, 1020.0, 4.0, 4.0, QString("T-2")}});
  const MagentaStats s = scan_magenta(render(w));

  ASSERT_GT(s.count, 0);
  EXPECT_LT(s.mean_x, kW / 2.0) << "port contact must draw left of centre";
}

// A contact far beyond every row's range draws nothing.
TEST_F(WaterfallContactsTest, OutOfRangeContactDrawsNothing)
{
  WaterfallWidget w;
  w.set_color_map(marine_sonar_widgets::ColorMapType::Grayscale);
  posed_rows(w);
  render(w);

  w.setContacts({ContactBox{1000.0, 1020.0, 4.0, 4.0, QString("T-3")}});
  EXPECT_EQ(scan_magenta(render(w)).count, 0) << "out-of-range contact must not draw";
}

// Clearing the contacts removes the overlay.
TEST_F(WaterfallContactsTest, ClearRemovesOverlay)
{
  WaterfallWidget w;
  w.set_color_map(marine_sonar_widgets::ColorMapType::Grayscale);
  posed_rows(w);
  render(w);

  w.setContacts({ContactBox{10.0, 1020.0, 4.0, 4.0, QString("T-4")}});
  ASSERT_GT(scan_magenta(render(w)).count, 0);
  w.setContacts({});
  EXPECT_EQ(scan_magenta(render(w)).count, 0) << "empty setContacts clears the overlay";
}

}  // namespace
