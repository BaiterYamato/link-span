> English translation of [coordination/claims/TIMER-001.md](TIMER-001.md). The Portuguese document remains the canonical project record.

#TIMER-001

- Status: done
- Closed: 2026-10-16 (CLAIMS-CLEANUP-001; evidence in the handoff, “Closure 2026-10-16” section)
- Agent: codex-windows-01
- Platform: Windows 11 / Codex / MinGW g++ 15.2 + Ninja
- Branch: agent/TIMER-001-frame-timers
- Started: 2026-07-13T01:29:08-03:00
- Depends on: EVENT-001 (PR #8; stacked branch)
- Files:
  - include/shiplua/timer/FrameTimerScheduler.h
  - src/timer/FrameTimerScheduler.cpp
  - tests/unit/FrameTimerSchedulerTests.cpp
  - tests/CMakeLists.txt
  - docs/api/frame-timers.md
  - coordination/claims/TIMER-001.md
  - coordination/handoffs/TIMER-001.md
  - coordination/STATUS.md
- Goal:
  - Implement deterministic timers per frame without host or Lua dependencies.
  - Support single run and repeat with safe cancellation.
  - Isolate callback failures and preserve healthy timers.
  - Define mutations during tick and restriction to the owning thread.
- Completed: 2026-07-13T01:32:44-03:00
