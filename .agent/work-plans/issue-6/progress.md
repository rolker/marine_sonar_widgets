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
