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

## Plan Authored
**Status**: complete
**When**: 2026-06-23 16:00 +00:00
**By**: Claude Code Agent (Claude Sonnet)

**Plan**: `.agent/work-plans/issue-1/plan.md` at `7709596`
**Branch**: feature/issue-1 at `7709596`
**Phases**: 3

### Open questions
- [ ] Should `topic_filter.*` move to marine_sonar_widgets (pure STL, no rclcpp) or stay in rqt_sonar_waterfall (ROS-adjacent, no known second consumer)?
- [ ] Namespace rename: move to `marine_sonar_widgets::` for extracted files (recommended) or keep `rqt_sonar_waterfall::` for zero-churn move?

## Plan Review
**Status**: complete
**When**: 2026-06-23 16:24 +00:00
**By**: Claude Code Agent (Claude Opus)
<!-- Independent: fresh-context Opus sub-agent reviewing a plan authored by a Sonnet invocation. The shared workspace $AGENT_NAME makes the name-match self-review heuristic a false positive here; this is NOT an author self-review. -->

**Plan**: `.agent/work-plans/issue-1/plan.md` at `7709596`
**PR**: PR-less (`--issue` / worktree mode; `gh` unauthenticated in this env — issue sourced from the `## Issue Review` entry + plan context, claims verified against the live `rqt_operator_tools` tree)
**Verdict**: changes-requested

### Findings
- [ ] (must-fix) License mismatch: scaffold declares Apache-2.0 (`package.xml:14`, CMakeLists header, LICENSE) but all extracted sources carry BSD-3-Clause headers and both origin packages are BSD-3-Clause; reconcile (likely set package to BSD-3-Clause) — `plan.md:136`
- [ ] (must-fix) `topic_filter` self-contradiction: steps 1/4 + Files table move it, but Open Questions recommends leaving it in `rqt_sonar_waterfall`; pick one and make steps consistent (adjust moving-test count 12→11 if it stays) — `plan.md:150`
- [ ] (suggestion) `marine_interfaces` declared in PR-A but unused until PR-C's `contact_builder`; declare it in PR-C per "only what's needed" — `plan.md:46`
- [ ] (suggestion) `echogram_widget.hpp` includes both `color_map.hpp` and `gpu_color_map.hpp`; note `gpu_color_map.hpp` in the include update too — `plan.md:18`
- [ ] (suggestion) Close the namespace-rename open question (rename recommended; `ping.cpp` is among files it touches) — `plan.md:154`

Verified against source: file targeting accurate (10 hpp + 10 cpp + 12 tests; staying set correct); all moving files are dependency-boundary-clean (`topic_filter`'s `marine_radar_control_msgs` reference is a topic-type string literal, not an include). Plan is structurally sound; resolve the two must-fix items inline before implementation.
