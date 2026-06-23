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

**Scaffold only.** Empty buildable `ament_cmake` package (no library target
yet). The widgets, their dependencies, and gtests arrive via the extraction
issue (a behavior-preserving move out of `rqt_operator_tools`). When you add
sources:

- Declare the new deps in `package.xml` and wire the library + gtests in
  `CMakeLists.txt` (mirror `marine_colormap`'s `SHARED` library + export setup).
- Add the source-sibling clone steps to `.github/workflows/ci.yml`
  (`marine_colormap` etc.), the way `marine_perception_tools` CI does.
- Preserve each extracted file's original license header.

## Layer / build

- Layer: `ui_ws` (operator station only — keeps the Qt dependency off the
  boat-side host).
- Default branch: `jazzy`. All changes via PR.
- Build: `colcon build --packages-up-to marine_sonar_widgets`; test:
  `colcon test --packages-select marine_sonar_widgets`.
