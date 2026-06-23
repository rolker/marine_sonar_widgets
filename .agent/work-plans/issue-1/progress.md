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

## Local Review (Pre-Push)
**Status**: complete
**When**: 2026-06-23 17:38 +00:00
**By**: Claude Code Agent (Claude Opus)
**Verdict**: approved

**Branch**: feature/issue-1 at `cb4e907`
**Mode**: pre-push
**Depth**: Deep (reason: large extraction — 6.3k lines, 41 files, cross-layer Qt/OpenGL/GPU shared library)
**Must-fix**: 0 | **Suggestions**: 5
**Round**: 1 | **Ship**: recommended — faithful byte-for-byte extraction; dependency boundary, license, and namespace all clean; no must-fix defects

Specialists: Static Analysis (cppcheck+xmllint), Governance, Plan Drift, Claude Adversarial ×2 (Lens A logic/move-fidelity, Lens B systemic/build). Lens A normalized each moved file for renames and byte-diffed against the live originals in `rqt_operator_tools` — zero residual diff, no dropped code, no logic change. Boundary (ADR-0001) honored: no rqt/rclcpp/rosbag2 includes. License reconciled to BSD-3-Clause consistently. CMake export wiring and test linkage correct.

### Findings
- [x] (suggestion) `qtbase5-dev` is build_depend only, but PUBLIC widget headers expose Qt types — add as `<depend>`/build_export so downstream `find_package` consumers resolve it — `package.xml:18` — FIXED: `qtbase5-dev` + `libqt5-opengl-dev` now full `<depend>`
- [x] (suggestion) Stale "Scaffold only" package description — widgets have now landed — `package.xml:11` — FIXED
- [x] (suggestion) Stale "Scaffold only" current-state section — update to post-extraction — `.agents/README.md:31-40` — FIXED (post-extraction state + PR-B/PR-C/thinning roadmap)
- [ ] (suggestion, → follow-up) Consumer thinning of `rqt_sonar_waterfall` / `rqt_marine_sonar` not done (separate repos; acceptable single-repo scoping) — required follow-up to avoid duplicated widget code — deferred to the rqt_operator_tools thinning PR (can't build until this lib is published + in the manifest)
- [x] (suggestion, low) `ament_export_dependencies(Qt5)` lacks `COMPONENTS`; a minimal downstream may miss `Qt5::Widgets/Gui/OpenGL` targets — `CMakeLists.txt:223` — FIXED: exports `Qt5Widgets Qt5Gui Qt5OpenGL`

**Resolution (2026-06-23):** 4 of 5 suggestions applied inline (Ship: recommended, 0 must-fix); rebuild + test green (258 tests, 0 failures). The 5th (consumer thinning) is the planned separate rqt_operator_tools PR. Ready to publish.

## Local Review (Pre-Push)
**Status**: complete
**When**: 2026-06-23 21:48 +00:00
**By**: Claude Code Agent (Claude Opus)
**Verdict**: changes-requested

**Branch**: feature/issue-1 at `9323b05`
**Mode**: pre-push
**Depth**: Deep (reason: 523 changed lines; correctness-critical render-geometry inversion)
**Must-fix**: 1 | **Suggestions**: 2
**Round**: 2 | **Ship**: continue — genuine correctness concern (cross-pass confirmed) in `pixel_to_map`; effectively round 1 of the PR-B diff (prior pre-push entry reviewed PR-A, now merged)

Reviews PR-B only (`9323b05`, target-marking mode); PR-A landed via PR #3 and is in `origin/jazzy`. Specialists: Static Analysis (cppcheck+xmllint — no findings on changed lines), Governance, Plan Drift (clean; matches plan steps 9-11), Claude Adversarial ×2 (Lens A logic, Lens B systemic — both independently flagged the must-fix). Inversion verified against the shader ring mapping (`gpu_color_map.cpp`), `project_row_into`, and the range-line overlay in `paintGL`; vertical mapping, port/starboard sign, and `world_pose` additivity all correct. ADR-0001 boundary honored (Qt-only, no rqt/rclcpp/rosbag2).

### Findings
- [ ] (must-fix) `pixel_to_map` mixes live `buffer_.rows()` with paint-time `ring_filled_`/`display_half_width_`; un-painted appends/clear between last paint and mark release silently mis-map the click to the wrong row's pose (live/non-frozen marking) — `src/waterfall_widget.cpp:728-754`
- [ ] (suggestion) Slant-mode slant→ground conversion is a no-op (`g.altitude` forced 0 when not ground); comment misleadingly claims water-column removal — `src/waterfall_widget.cpp:783-786`
- [ ] (suggestion) 4-corner bbox can under-cover the swept region on a curved/turning track (intermediate-row poses ignored) — `src/waterfall_widget.cpp:837-864`

## Local Review (Pre-Push)
**Status**: complete
**When**: 2026-06-23 22:47 +00:00
**By**: Claude Code Agent (Claude Opus)
**Verdict**: approved

**Branch**: feature/issue-1 at `d62c6e3`
**Mode**: pre-push
**Depth**: Deep (reason: correctness-critical render-geometry inversion in a Qt/OpenGL shared widget)
**Must-fix**: 0 | **Suggestions**: 4
**Round**: 3 | **Ship**: recommended — round-2 must-fix genuinely resolved (paint-time snapshot); remaining items are mechanical polish, not rising

Reviews PR-B at `d62c6e3` (target-marking + the review-fix commit) vs `origin/jazzy`. Specialists: Static Analysis (cppcheck — no actionable findings; all hits FPs), Governance, Plan Drift (on-plan, steps 9-11), Claude Adversarial ×2 (Lens A logic, Lens B systemic). Build green; 269 tests / 0 failures (the 6 marking GL tests compile+link but self-skip headless, like the sibling GL tests). The round-2 `pixel_to_map` mis-map is fixed: `update_paint_geometry()` snapshots each displayed row's pose+geometry at paint time and `pixel_to_map` inverts against that, not the live buffer; the `AppendAfterPaint` regression test discriminates against the old code (verified). Slant-altitude (true altitude carried) and curved-track bbox (per-pixel-row edge stepping; correct because the map distance is monotonic in |d|) suggestions from round 2 are also addressed. ADR-0001 boundary honored (Qt-only). Evaluated and rejected the `clear()`-before-repaint case as a must-fix: inversion against the old snapshot stays consistent with the un-repainted on-screen frame, which is the stated contract.

### Findings
- [ ] (suggestion) `pixel_to_map` still reads live `uniform_scale_`/`display_half_width_` (rest from snapshot); a uniform-scale toggle between paint and release can desync inversion from the painted frame — snapshot them into `PaintRow` — `src/waterfall_widget.cpp:789`
- [ ] (suggestion) `px.x()` not clamped to `[0,w-1]` (unlike `py`); off-widget drag corner inverts beyond the rendered swath — `src/waterfall_widget.cpp:796,883`
- [ ] (suggestion) `setMarkMode(true)` doesn't force a repaint; first drag on a quiescent/frozen view relies on a prior paint of `paint_rows_` — add a defensive `update()` — `src/waterfall_widget.cpp:716`
- [x] (suggestion) Comment says projection is "affine in pixel-x"; in slant mode it's nonlinear but monotonic in |d| (result still correct) — reword to "monotonic" — `src/waterfall_widget.cpp:857`

**Resolution (2026-06-23, `35adb1d`):** all 4 round-3 suggestions applied even though the verdict was already approved (cheap, and the first two are the same paint-time-vs-live class as the round-2 must-fix). Snapshot `uniform_scale_`+`display_half_width_` into `paint_uniform_scale_`/`paint_half_width_`; clamp `px.x()` to `[0,w-1]`; `setMarkMode()` repaints unconditionally; bbox comment reworded to "monotonic". Rebuild + full suite green (269 tests, 0 failures). PR-B ready to publish.
