# marine_sonar_widgets

Shared Qt/OpenGL **sonar-display widgets** and a **target-marking core** for the
UNH marine autonomy stack. One implementation, two kinds of consumer:

- **Live** — the `rqt_operator_tools` rqt plugins (`rqt_sonar_waterfall`,
  `rqt_marine_sonar`), fed from ROS subscriptions during a survey.
- **Offline** — the `marine_perception_tools` sidescan/MBES viewer, fed from
  rosbag2 playback for post-survey review and annotation.

Both render the same waterfall / echogram displays and produce the same
`marine_interfaces/msg/Contact` when an analyst marks a target — whether marking
live on the boat or offline from a bag.

## Contents (as extraction lands)

- `WaterfallWidget` — GPU scrolling backscatter waterfall (geometry-agnostic,
  newest-at-top), `marine_colormap` shader, gain/contrast/auto-range +
  ground-range/uniform-scale/range-lines/TVG.
- `EchogramWidget` — GPU water-column echogram (`addPings(RawSonarImage)`),
  auto-range / black-white / contrast / colormap / depth zoom-pan.
- **Marking** — `setMarkMode()` + `boxMarked(map_rect)`; per-row world-pose
  tagging supplied by the consumer (live TF / offline bag pose table), so the
  widget stays pose-source-agnostic.
- `make_box_contact(...)` — the shared `Contact` builder (`ORIGIN_HUMAN`,
  `Shape::BOX`); geo_pose resolved by the consumer.

## Dependency boundary

Qt + `marine_colormap` + `marine_acoustic_msgs` + `marine_interfaces`.
**No rqt, no rclcpp, no rosbag2** — that boundary is what lets a live rqt plugin
and an offline bag app both link the library.

Layer: `ui_ws`. Default branch: `jazzy`.

> Widgets and tests were moved out of `rqt_operator_tools`
> (behavior-preserving extraction). The package and the extracted code are
> BSD-3-Clause; per-file license headers travel with the extracted code.
