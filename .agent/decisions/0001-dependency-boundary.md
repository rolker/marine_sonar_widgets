# 0001 — Dependency boundary: Qt + colormap + messages only, no rqt/rclcpp/rosbag2

## Status

Accepted

## Context

`marine_sonar_widgets` exists so that one implementation of the sonar-display
widgets (scrolling backscatter waterfall, water-column echogram) and the
target-marking core feeds **two consumers**:

- **Live**: the `rqt_operator_tools` rqt plugins (`rqt_sonar_waterfall`,
  `rqt_marine_sonar`), fed from ROS subscriptions during a survey.
- **Offline**: the `marine_perception_tools` sidescan/MBES viewer, fed from
  rosbag2 playback for post-survey review and annotation.

If the library pulled in `rqt`, it could not link into the offline bag
application; if it pulled in `rclcpp` or `rosbag2`, it would couple the rendering
core to a particular ROS runtime / playback path and defeat the "one
implementation, two consumers" goal. The pose source differs between the two
(live TF vs. an offline bag pose table), so that wiring must stay outside the
library and be supplied *to* the widget by the caller.

## Decision

The library depends only on:

- **Qt5** (Widgets, Gui, OpenGL) — the widgets are `QWidget` / `QOpenGLWidget`.
- **`marine_colormap`** — the shared scalar-field colormap library/shaders.
- **`marine_acoustic_msgs`** — the `RawSonarImage` input type.
- **`marine_interfaces`** (added in PR-C, when the `contact_builder` /
  `Contact`-emitting core lands).

The library must **not** depend on `rqt`, `rclcpp`, or `rosbag2`. ROS
subscriptions, bag playback, and pose-source acquisition (live TF vs. offline
bag pose table) live in the consumers and are passed into the widgets.

A change that pulls any of `rqt` / `rclcpp` / `rosbag2` into this package is a
design regression: push that wiring up into the consumer instead.

## Consequences

- The same widget code links into both a live rqt plugin and an offline Qt bag
  viewer; behavior is identical because there is one implementation.
- Consumers own the ROS/bag plumbing and feed the widget decoded pings + a pose
  source. This is more wiring on the consumer side, but it is the price of
  keeping the core runtime-agnostic.
- CI for this package clones only `marine_colormap` as a source sibling;
  `marine_acoustic_msgs` resolves via rosdep. No `rqt`/`rclcpp`/`rosbag2`
  toolchain is needed to build or test the library, keeping its CI light.
- This mirrors the workspace ADR-0001 framework-agnostic-library pattern that
  `marine_colormap` follows; recording it as a project-level ADR makes the
  boundary discoverable to future contributors of this repo.
