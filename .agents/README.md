# Agent Guide — marine_sonar_widgets

Per-repo orientation for AI agents. Read this before changing the package.
Workspace-wide rules live in the workspace `AGENTS.md`; this file covers what is
specific to `marine_sonar_widgets`.

## What this repo is

A shared, **framework-agnostic** Qt/OpenGL sonar-display library plus a
target-marking core. The whole point is **one implementation, two consumers**:

- **Live**: the `rqt_operator_tools` rqt plugins (`rqt_sonar_waterfall`,
  `rqt_marine_sonar`), fed from ROS subscriptions during a survey.
- **Offline**: the `marine_perception_tools` sidescan/MBES viewer, fed from
  rosbag2 playback for post-survey review and annotation.

Both render the same waterfall / echogram and emit the same
`marine_interfaces/msg/Contact` when a target is marked.

## The load-bearing rule: the dependency boundary

The library may depend on **Qt, `marine_colormap`, `marine_acoustic_msgs`, and
`marine_interfaces` only**. It must **not** depend on `rqt`, `rclcpp`, or
`rosbag2`. That boundary is exactly what lets a live rqt plugin and an offline
bag application both link it. A change that pulls in any of those is a design
regression — push the ROS/rqt/bag wiring up into the consumer instead, and keep
pose-source (live TF vs offline bag pose table) supplied *to* the widget by the
caller.

## Current state

**Widgets extracted (PR-A).** The `SHARED` `marine_sonar_widgets` library now
holds the GPU display widgets moved (behavior-preserving) out of
`rqt_operator_tools`: `WaterfallWidget`/`WaterfallBuffer`/`WaterfallModel`,
`ColorMap`/`GpuColorMap`, `RowExtractor`, `PingPairer`, `TopicFilter`, plus
`EchogramWidget`/`Ping` — with their 12 gtests. Deps: Qt5 + `marine_colormap` +
`marine_acoustic_msgs`. License: BSD-3-Clause (matches the origin packages).

Landed since:
- **PR-B** (#1) — marking mode: `std::optional<WorldPose>` on `WaterfallRow`,
  `setMarkMode()` + `boxMarked()` on `WaterfallWidget` (pose-source-agnostic).
- **PR-C** (#1) — `make_box_contact` + `MapPoint` extracted from
  `marine_perception_tools`; adds the `marine_interfaces` dep (Apache-2.0; the
  package is now mixed-license, see the LICENSE files).
- **Contact overlay** (#6) — `setContacts(ContactBox)` draws already-marked
  contacts on the waterfall (forward of the marking inversion).

Still to land:
- **Consumer thinning** (separate PRs in `rqt_operator_tools`): delete the moved
  sources there and depend on this library; same for `marine_perception_tools`.

When adding sources, preserve each file's original license header and keep the
dependency boundary above (see [ADR 0001](../.agent/decisions/0001-dependency-boundary.md)).

## Layer / build

- Layer: `ui_ws` (operator station only — keeps the Qt dependency off the
  boat-side host).
- Default branch: `jazzy`. All changes via PR.
- Build: `colcon build --packages-up-to marine_sonar_widgets`; test:
  `colcon test --packages-select marine_sonar_widgets`.
