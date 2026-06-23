# Contributing to marine_sonar_widgets

This repo is developed as part of the
[ROS 2 Agent Workspace](https://github.com/rolker/ros2_agent_workspace). When
checked out inside that workspace, follow the workflow rules in the workspace
`AGENTS.md` (worktree isolation, branch naming, AI signatures, pre-commit
hooks). See [`.agents/README.md`](.agents/README.md) for the per-repo agent
guide.

## Quick rules

- All changes land via Pull Requests against the `jazzy` default branch.
- Run pre-commit hooks before committing (`pre-commit run --all-files`); never
  bypass with `--no-verify`.
- New source files carry their original license header. Widgets extracted from
  `rqt_operator_tools` keep that code's BSD-3-Clause header; new files added here
  use BSD-3-Clause (see `LICENSE`).
- One logical change per commit.

## License

Any contribution that you make to this repository will
be under the 3-Clause BSD License, as dictated by that
[license](https://opensource.org/licenses/BSD-3-Clause).
