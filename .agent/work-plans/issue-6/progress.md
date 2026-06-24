---
issue: 6
---

# Issue #6 — Existing-contact overlay on WaterfallWidget

## Local Review (Pre-Push)
**Status**: complete
**When**: 2026-06-24 02:15 +00:00
**By**: Claude Code Agent (Claude Opus)
**Verdict**: changes-requested

**Branch**: feature/issue-6 at `20cb2d1`
**Mode**: pre-push
**Depth**: Deep (reason: 368 changed lines ≥ 200)
**Must-fix**: 1 | **Suggestions**: 4
**Round**: 1 | **Ship**: continue — one genuine correctness must-fix in the core forward projection (slant-mode gate); mechanical fix, but warrants a re-read.

### Findings
- [ ] (must-fix) Slant-mode in-range gate compares ground lateral offset against a slant range, so a far-edge contact (altitude>0) can plot beyond the swath; gate on `disp > side_half` instead — `src/waterfall_widget.cpp:840`
- [ ] (suggestion) Tests use altitude=0 only, never exercising the slant-gate path, and assert no box y-position; add a slant+altitude edge-of-range case and a y-placement assertion — `test/test_waterfall_widget_contacts.cpp:62`
- [ ] (suggestion) `setContacts()` lacks the GUI-thread-only caveat that `add_row()` documents — `include/marine_sonar_widgets/waterfall_widget.hpp:139`
- [ ] (suggestion) New public `setContacts()`/`ContactBox` API missing from the README public-API lists — `README.md:22`, `.agents/README.md:41`
- [ ] (suggestion) Overlay is O(contacts × rows) with trig+sqrt per repaint (every update()); consider caching projected positions if counts grow — `src/waterfall_widget.cpp:791`

### Notes
- Geometry verified correct (both adversarial lenses + manual check): vertical `ridx→py` map, across-track port/starboard sign convention, slant↔ground `disp` conversion all exactly invert `pixel_to_map`. Only the gate threshold mixes units.
- Static analysis clean: ament_cpplint no problems; cppcheck's lone `useStlAlgorithm` hint is on a pre-existing context line (`src/waterfall_widget.cpp:366`, not in this diff) — dropped.
- Dependency boundary (ADR 0001) respected: no rqt/rclcpp/rosbag2; new test deps identical to sibling targets; `ContactBox` uses `QString`.

## Local Review (Pre-Push)
**Status**: complete
**When**: 2026-06-24 02:42 +00:00
**By**: Claude Code Agent (Claude Opus)
**Verdict**: approved

**Branch**: feature/issue-6 at `87ac351`
**Mode**: pre-push
**Depth**: Deep (reason: 423 changed lines ≥ 200; trig/geometry render path)
**Must-fix**: 0 | **Suggestions**: 1
**Round**: 2 | **Ship**: recommended — Round 1 must-fix fixed and regression-tested; static analysis clean; 5/5 contact tests pass; only one minor painter-hygiene suggestion remains.

### Findings
- [ ] (suggestion) `draw_contacts()` sets pen/brush without save/restore — harmless today (marking block overwrites) but fragile if a paint step is added after the overlay — `src/waterfall_widget.cpp:816`

### Notes
- Round 1 follow-up: all 5 prior findings addressed — slant gate now compares `disp > side_half` (`src/waterfall_widget.cpp:849`); `SlantModeGatesOnSlantRange` test + `mean_y` placement assertions added; GUI-thread caveat on `setContacts()` (`waterfall_widget.hpp:139`); public API in `README.md` + `.agents/README.md`; O(contacts×rows) perf documented in the hpp comment.
- Geometry re-verified by two disjoint-lens adversarial passes + manual trace: `draw_contacts()` exactly inverts `pixel_to_map()` (vertical `ridx→py`, across-track port/stbd sign, slant↔ground via `ground_range`); gate now compares same-unit quantities (`disp` vs `side_half`, both slant in slant mode, both ground in ground mode).
- Static analysis clean: ament_cpplint no problems; cppcheck clean on the source (the `TEST_F` "syntax error" is the known gtest-macro parse artifact, dropped).
- Tests pass: latest colcon test run shows `test_waterfall_widget_contacts` 5/5 PASSED; package 21/21 PASSED.
- No `plan.md` for #6 → Plan Drift skipped. ADR-0001 dependency boundary respected.
