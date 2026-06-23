# Plan: marine_sonar_widgets — shared sonar display widgets + marking core

## Issue

https://github.com/rolker/marine_sonar_widgets/issues/1

## Context

A scaffold `marine_sonar_widgets` package (empty, lint-clean, in `ui_ws`) already
exists. The widgets to extract live in `rqt_operator_tools` (same `ui_ws` layer):

- `rqt_sonar_waterfall`: `WaterfallWidget` + `WaterfallBuffer` + `WaterfallModel`
  (waterfall_model.hpp) + `ColorMap` + `GpuColorMap` + `RowExtractor` + `PingPairer`
  + `TopicFilter` (both pure-logic, no rclcpp) — 10 source files, 11 tests.
  `ControlPanel` and `HistorySpinbox` depend on `marine_radar_control_msgs` (outside
  the allowed boundary) and stay in `rqt_sonar_waterfall`.
- `rqt_marine_sonar`: `EchogramWidget` + `Ping` — 3 source files, 2 tests.
  `EchogramWidget` already includes `rqt_sonar_waterfall/color_map.hpp`; include
  paths update as part of the move.

`make_box_contact` + `MapPoint` live in `marine_perception_tools/src/contact_store.*`
(internal, not installed). `ContactStore`'s save/load uses rclcpp serialization and
stays in `marine_perception_tools`.

Audit of `WaterfallRow` consumers: `rqt_sonar_waterfall` (creates rows) + its tests
(instantiate rows directly). `rqt_marine_sonar` uses its own `DecodedPing`.
`marine_perception_tools` uses a parallel `QImage` + `WaterfallIndex` path.
No external library consumers → adding an optional field is backward-compatible.

## Approach

Three PRs, each leaving the tree buildable. PR-A lands first (extraction); PR-B and
PR-C may overlap after PR-A merges.

### PR-A — Widget extraction + plugin thinning

1. **Populate `marine_sonar_widgets`** — copy moved sources, rewrite include guards
   and namespace from `rqt_sonar_waterfall::` → `marine_sonar_widgets::` for all
   moved files. File set: `waterfall_widget`, `waterfall_buffer`, `waterfall_model`,
   `color_map`, `gpu_color_map`, `row_extractor`, `ping_pairer`, `topic_filter`,
   `echogram_widget`, `ping` (10 headers + 10 .cpp files in `include/` and `src/`).
2. **Wire `CMakeLists.txt`** — SHARED library with deps Qt5 (Widgets Gui OpenGL),
   `marine_colormap`, `marine_acoustic_msgs`, `marine_interfaces`; mirror
   `marine_colormap`'s export setup. **No** `rclcpp`, `rqt`, or `rosbag2`.
3. **Update `package.xml`** — add `qtbase5-dev`, `libqt5-opengl-dev`,
   `marine_colormap`, `marine_acoustic_msgs`, `marine_interfaces`.
4. **Move 12 test files** to `marine_sonar_widgets/test/`:
   `test_decode_samples`, `test_combine_rows`, `test_ground_resample`,
   `test_single_beam_extractor`, `test_waterfall_buffer`, `test_color_map`,
   `test_waterfall_widget`, `test_ping_pairer`, `test_topic_filter`,
   `test_gpu_color_map` from `rqt_sonar_waterfall/test/`;
   `test_echogram_widget`, `test_ping` from `rqt_marine_sonar/test/`.
   Update include paths in each test.
5. **Thin `rqt_sonar_waterfall`** — remove moved sources and tests; add
   `marine_sonar_widgets` dep; keep `sonar_waterfall_plugin`, `control_panel`,
   `history_spinbox`, `test_control_panel`. Update plugin `#include` paths.
6. **Thin `rqt_marine_sonar`** — remove `echogram_widget`, `ping`, their tests;
   replace `rqt_sonar_waterfall` dep with `marine_sonar_widgets`. Update plugin
   `#include` paths and `find_package`.
7. **CI** — add `marine_sonar_widgets/.github/workflows/ci.yml`; clone
   `rqt_operator_tools`, `marine_colormap`, `marine_acoustic_msgs`,
   `marine_interfaces` as sibling source deps (mirror `rqt_sonar_waterfall` CI).
8. **ADR** — add `marine_sonar_widgets/.agent/decisions/0001-dependency-boundary.md`
   capturing the no-rqt/rclcpp/rosbag2 constraint and its rationale. The boundary
   is already in `.agents/README.md` but an ADR in the project repo makes it
   discoverable by future contributors and consistent with workspace ADR-0001.
   (Recommendation: a brief project-level ADR is warranted; `marine_colormap` uses
   the workspace ADR-0001 umbrella because it predates per-repo ADRs — this library
   should set the standard going forward.)

### PR-B — Marking mode + WaterfallRow world-geometry extension

9. **Extend `WaterfallRow`** — add `std::optional<WorldPose> world_pose = std::nullopt`
   where `WorldPose` carries `{double x, y, heading_rad}` in the map frame.
   All existing consumers compile unchanged (additive, default nullopt).
   The offline consumer fills it from a bag pose table; the live consumer from TF.
   Neither is wired here — the widget only inverts pixel→map when `world_pose` is set.
10. **Add marking mode to `WaterfallWidget`** — `setMarkMode(bool)` + `boxMarked(QRectF)`
    signal. On drag-release, invert pixel column → (sample, across-track) → map using
    the buffered rows' `world_pose`; emit a `map_rect` in the consumer-supplied frame.
    Rows with `world_pose == nullopt` yield a null rect (signal not emitted).
11. **Tests** — gtest for `WaterfallWidget` marking: construct a widget with fake rows
    carrying synthetic `world_pose`, simulate a drag, assert `boxMarked` rect corners.

### PR-C — `make_box_contact` extraction

12. **New header/cpp** — `marine_sonar_widgets/include/marine_sonar_widgets/contact_builder.hpp`
    + `src/contact_builder.cpp`: `MapPoint` struct + `make_box_contact()` function,
    extracted verbatim from `marine_perception_tools/src/contact_store.*`.
    Namespace: `marine_sonar_widgets`.
13. **Update `marine_perception_tools`** — `contact_store.hpp` replaces local `MapPoint`
    + `make_box_contact` with `#include <marine_sonar_widgets/contact_builder.hpp>` +
    a `using` alias. Add `marine_sonar_widgets` to `package.xml` and `CMakeLists.txt`.
14. **Move tests** — extract `make_box_contact` test cases from `test_contact_store.cpp`
    into `marine_sonar_widgets/test/test_contact_builder.cpp`; remaining
    `test_contact_store.cpp` cases (inBox, save/load) stay in `marine_perception_tools`.

## Files to Change

| File | Change |
|------|--------|
| `marine_sonar_widgets/CMakeLists.txt` | Wire library, deps, 12+ gtest targets |
| `marine_sonar_widgets/package.xml` | Add Qt, marine_colormap, marine_acoustic_msgs, marine_interfaces |
| `marine_sonar_widgets/include/marine_sonar_widgets/*.hpp` (×10) | Moved + include-guard/namespace renamed |
| `marine_sonar_widgets/src/*.cpp` (×10) | Moved + namespace renamed |
| `marine_sonar_widgets/test/test_*.cpp` (×12) | Moved + include paths updated |
| `marine_sonar_widgets/.github/workflows/ci.yml` | New; mirrors rqt_operator_tools CI |
| `marine_sonar_widgets/.agent/decisions/0001-*.md` | New project-level ADR |
| `rqt_sonar_waterfall/CMakeLists.txt` | Remove moved sources/tests; add marine_sonar_widgets dep |
| `rqt_sonar_waterfall/package.xml` | Replace moved deps with marine_sonar_widgets |
| `rqt_sonar_waterfall/src/sonar_waterfall_plugin.cpp` | Update includes |
| `rqt_marine_sonar/CMakeLists.txt` | Remove moved sources/tests; swap rqt_sonar_waterfall for marine_sonar_widgets |
| `rqt_marine_sonar/package.xml` | Same |
| `rqt_marine_sonar/src/marine_echogram_plugin.cpp` | Update includes |
| `marine_sonar_widgets/include/…/waterfall_model.hpp` | Add `WorldPose` + optional field (PR-B) |
| `marine_sonar_widgets/include/…/waterfall_widget.hpp` | Add `setMarkMode`, `boxMarked` (PR-B) |
| `marine_sonar_widgets/include/…/contact_builder.hpp` | New (PR-C) |
| `marine_sonar_widgets/src/contact_builder.cpp` | New (PR-C) |
| `marine_perception_tools/src/contact_store.hpp/cpp` | Replace local make_box_contact (PR-C) |
| `marine_perception_tools/package.xml` + `CMakeLists.txt` | Add marine_sonar_widgets dep (PR-C) |

## Principles Self-Check

| Principle | Consideration |
|---|---|
| A change includes its consequences | Tests physically move with the code they test; both plugin repos update in PR-A |
| Improve incrementally | Three atomic PRs; each leaves the tree buildable |
| Capture decisions, not just implementations | Project-level ADR for the dependency boundary (step 8) |
| Only what's needed | `control_panel` + `history_spinbox` stay in rqt_sonar_waterfall — they touch marine_radar_control_msgs which is outside the boundary; forcing them into marine_sonar_widgets would violate the boundary or expand it unnecessarily |

## ADR Compliance

| ADR | Triggered | How addressed |
|---|---|---|
| ADR-0001 (ADRs) | Yes | Project-level ADR captures the dependency boundary |
| ADR-0008 (ROS 2 conventions) | Yes | ament_cmake, BSD-3 license headers preserved, conventional package layout |
| ADR-0013 (progress.md vocab) | Yes | This entry and future progress entries follow the schema |

## Consequences

| If we change… | Also update… | Included? |
|---|---|---|
| `WaterfallRow` struct | All direct instantiation sites in tests (only rqt_sonar_waterfall tests, now moving) | Yes — moved tests updated simultaneously |
| `rqt_sonar_waterfall` public headers removed | `rqt_marine_sonar` includes (already `color_map.hpp`, `gpu_color_map.hpp`) | Yes — PR-A updates rqt_marine_sonar |
| `make_box_contact` namespace changes | `marine_perception_tools/contact_store.*` call sites | Yes — PR-C adds using alias |
| CI for marine_sonar_widgets | Must clone all 4 source deps (rqt_operator_tools, marine_colormap, marine_acoustic_msgs, marine_interfaces) | Yes — step 7 |

## Open Questions

- Should `topic_filter.*` move to marine_sonar_widgets? It depends only on STL (no rclcpp),
  but it does no sonar processing — it filters ROS-style topic strings. If the offline
  viewer never needs it, it's ROS-adjacent and arguably cleaner left in rqt_sonar_waterfall.
  Recommend: leave it in rqt_sonar_waterfall unless a second consumer emerges.
- Namespace: rename `rqt_sonar_waterfall::` → `marine_sonar_widgets::` for moved
  files (recommended — avoids a `marine_sonar_widgets` library with a `rqt_sonar_waterfall`
  namespace), or keep as-is for zero-churn move. Plan assumes rename.

## Estimated Scope

Three PRs. PR-A is the largest (file moves across three repos) but is mechanical.
PR-B and PR-C are independent of each other and may be reviewed in parallel after PR-A.
