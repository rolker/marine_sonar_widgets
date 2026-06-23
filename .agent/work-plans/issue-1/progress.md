---
issue: 1
---

# Issue #1 — marine_sonar_widgets: shared sonar display widgets + marking core

## Issue Review
**Status**: complete
**When**: 2026-06-23 00:00 +00:00
**By**: Claude Code Agent (Claude Sonnet)

**Issue**: #1
**Comment**: (best-effort post follows this entry; not recorded inline)
**Scope verdict**: well-scoped

The issue proposes extracting `WaterfallWidget` and `EchogramWidget` from `rqt_operator_tools` into a standalone `marine_sonar_widgets` library, adding a marking mode with `boxMarked` signal, and extracting `make_box_contact` from `marine_perception_tools`. The dependency boundary is explicit (Qt + marine_colormap + marine_acoustic_msgs + marine_interfaces; no rqt/rclcpp/rosbag2). A skeleton package already exists. Acceptance criteria are well-defined. The issue mirrors the `marine_colormap` consolidation pattern, which is a proven approach in this workspace.

### Actions
- [ ] Consider writing an ADR for the dependency boundary decision (no rqt/rclcpp/rosbag2 in the library) — this is an architectural constraint future contributors need to know about and is the kind of design decision ADR-0001 targets.
- [ ] When extending `WaterfallRow` to carry per-row world geometry, treat it as an interface change: audit existing consumers of `WaterfallRow` (in rqt_sonar_waterfall and anywhere else) before adding fields to ensure backward compatibility.
- [ ] Ensure "tests carried over" means the test files physically move with the widget source — not just mentioned in prose. The CI must run them from the new package.
