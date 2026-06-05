---
title: Implementation Progress
---

# Implementation Progress Tracker

This page is the long-running execution tracker for the Kodi binary add-on implementation.

## Current Snapshot

- Date: 2026-06-05
- Overall phase: Foundation and integration planning
- Build/runtime context:
  - Kodi source: `/home/brett/kodi-source` (user-provided)
  - Kodi build: `/home/brett/build` (user-provided)
  - NLC source: `/home/brett/NoLimitConnect` (user-provided)
  - Add-on repo: `/home/brett/kodi-addon-nolimitconnect` (user-provided)
- Network bootstrap target: `nolimitconnect.net`
- UI migration direction: Qt desktop UI patterns translated into Kodi native GUI windows/dialogs

## Milestones

| ID | Milestone | Status | Notes |
|---|---|---|---|
| M0 | Project scaffolding and docs baseline | In Progress | Documentation and architecture plan exist |
| M1 | Buildable add-on skeleton (load/unload cleanly) | In Progress | Skeleton compiles; add-on is visible in Kodi service list; load/unload behavior validation still pending |
| M2 | Engine bridge integration (`IToGui`/`IFromGui`) | In Progress | Typed command/event scaffolding landed; entrypoint now drains and logs bridge events |
| M3 | Identity + network sign-on flow | In Progress | Display name persisted, prompt-or-join path wired, and dev-stub fallback identity added |
| M4 | Core dialogs and settings UX | Not Started | Network setup, permissions, join host |
| M5 | Audio pipeline (capture/playback) | Not Started | Required for PushToTalk and VoicePhone |
| M6 | Plugin delivery (8, 9, 10, 11, 12, 15, 16) | Not Started | Implement in dependency order |
| M7 | Packaging, QA, and install docs | Not Started | Local install, smoke tests, release notes |

## Plugin Delivery Plan

| Order | Slot | Plugin | Status | Dependency Notes |
|---|---|---|---|---|
| 1 | 8 | Messenger | Not Started | Needs sessions + messaging UI + engine events |
| 2 | 16 | VoicePhone | Not Started | Needs stable audio path + session control |
| 3 | 9 | PushToTalk | Not Started | Reuses VoicePhone audio path with PTT behavior |
| 4 | 10 | PersonFileXfer | Not Started | Needs peer targeting + transfer state UI |
| 5 | 12 | FileShareServer | Not Started | Needs virtual stream/file protocol integration |
| 6 | 11 | CamServer | Not Started | Needs camera capture + outbound media path |
| 7 | 15 | VideoChat | Not Started | Highest complexity: camera + render + motion + recording |

## Active Work Queue

| Priority | Task | Status | Owner |
|---|---|---|---|
| P0 | Confirm source paths in this environment and normalize to workspace paths | Completed | Copilot + User |
| P0 | Create initial code skeleton under `src/addon`, `src/gui`, `src/audio`, `src/stream` | Completed | Copilot |
| P0 | Define first-run username + auto-join flow state machine | Completed (wired scaffold) | Copilot |
| P1 | Specify event map for `IToGui` callbacks -> Kodi GUI messages | Completed (scaffold) | Copilot |
| P1 | Specify command map for Kodi actions -> `IFromGui` commands | Completed (scaffold) | Copilot |
| P1 | Add milestone checklists and acceptance criteria | Not Started | Copilot |
| P1 | Wire Kodi settings/profile persistence for display name | Completed | Copilot |

## Decisions Log

| Date | Decision |
|---|---|
| 2026-06-04 | Maintain a persistent task/progress tracker in this repository and update continuously during implementation |
| 2026-06-04 | Keep network onboarding aligned with NLC behavior: reuse existing username or prompt once, then join `nolimitconnect.net` |
| 2026-06-04 | Replace Qt UI surfaces with Kodi-native dialogs/windows and bridge events via `IToGui`/`IFromGui` |

## Risks and Unknowns

| Risk | Impact | Mitigation |
|---|---|---|
| Path mismatch between user-provided checkout locations and active workspace | Build/config churn | Keep both logical and workspace-resolved path notes; verify before each build step |
| Kodi GUI thread safety violations from engine callbacks | Instability/crashes | Enforce message marshaling to Kodi main thread for all GUI updates |
| Audio latency/echo complexity across VOIP plugins | Poor call quality | Land shared audio pipeline first; add test harness before plugin-level polish |
| Video plugin scope (capture/render/motion/record) | Schedule overrun | Stage CamServer before VideoChat and keep recording optional at first |

## Runtime Smoke Checklist

- [x] Service discovery test: add-on appears in Kodi Services list after install.
- [ ] Service startup log test: expect "NoLimitConnect addon service started" in Kodi log during startup.
- [x] Service startup log test: Kodi service manager starts NLC python bootstrap and emits startup/status logs.
- [ ] Fresh profile test: `display_name` empty -> add-on emits prompt request event and does not mark network online.
- [ ] Identity submit test: GUI sends `kProvideDisplayName` command -> setting persists and join command is dispatched.
- [ ] Returning user test: non-empty `display_name` at startup -> add-on dispatches join command automatically.
- [ ] Settings edit test: changing `display_name` through settings updates sign-on flow and re-triggers join dispatch.
- [ ] Event drain test: Kodi GUI loop polls and handles `kPromptDisplayNameRequested`, `kNetworkStateChanged`, and status events.

Note: With `NLC_ENABLE_DEV_STUBS` enabled, fresh profile startup currently auto-generates a temporary fallback display name (`kodi-dev-xxxxx`) and dispatches join to allow end-to-end flow testing before dialog UX is complete.
- [x] Event drain test: Kodi GUI loop polls and handles `kPromptDisplayNameRequested`, `kNetworkStateChanged`, and status events.

## Change Log

- 2026-06-04: Created initial implementation tracker with milestones, plugin order, decisions, and active work queue.
- 2026-06-04: Added Milestone M1 code skeleton (CMakeLists, addon.xml, addon entrypoint, core sign-on flow stub, gui/audio/stream placeholders).
- 2026-06-04: Added KODI source path auto-detection (env + common defaults) to CMake and validated configure with and without explicit `-DKODI_SOURCE_DIR`.
- 2026-06-04: Built `libkodi-addon-nolimitconnect.so` successfully in `build-autodetect` after aligning addon entrypoint with Kodi 21 API signatures.
- 2026-06-04: Started M2 by adding typed bridge scaffolding (`BridgeTypes`, `FromGuiBridge`, `ToGuiBridge`) and wiring `NlcAddon` command dispatch and event queue flow.
- 2026-06-04: Started M3 by adding `resources/settings.xml` with `display_name`, loading persisted display name during sign-on initialization, and handling runtime setting updates in the add-on entrypoint.
- 2026-06-04: Wired first-run bootstrap behavior so initialization now emits `kPromptDisplayNameRequested` when identity is missing, supports `kProvideDisplayName` command handling, and auto-dispatches `kJoinDefaultNetwork` when identity is available.
- 2026-06-04: Added minimal event-consumer loop in add-on entrypoint (`Create`/`SetSetting`) to drain `NlcAddon` events and log prompt/network/status transitions for runtime observability.
- 2026-06-04: Added dev-stub fallback identity behavior in `NlcAddon::Initialize` so missing `display_name` auto-populates a temporary value and joins network when `NLC_ENABLE_DEV_STUBS` is enabled.
- 2026-06-05: Manual runtime verification in Kodi confirmed `service.binary.nolimitconnect` is visible under Services after build/install.
- 2026-06-05: Kodi log confirms discovery/install of `service.binary.nolimitconnect v0.1.0`; however no NLC service startup marker was observed and a UI navigation attempt logged "Addon with id service.binary.nolimitconnect not found locally."
- 2026-06-05: Updated `addon.xml` service extension to use `library="libkodi-addon-nolimitconnect.so"` (instead of `library_linux`) to align with Kodi service addon loader expectations; rebuilt and reinstalled.
- 2026-06-05: Re-ran Kodi after manifest update; log still shows discovery/install of `service.binary.nolimitconnect` but no `NoLimitConnect addon service started` marker and no `CServiceAddonManager` startup for NLC.
- 2026-06-05: Root-caused service startup behavior in Kodi source: `CServiceAddonManager` starts only `.py` services. Switched add-on service entry to `resources/lib/service.py` bootstrap and added `xbmc.python` requirement.
- 2026-06-05: Fixed NoLimitConnect popup error (`TypeError: Invalid setting type`) by replacing `getSettingString` with compatibility-safe `getSetting`, adding exception guards, and confirming clean startup logs.
- 2026-06-05: Verified runtime markers: `CServiceAddonManager: starting service.binary.nolimitconnect`, `NoLimitConnect addon service started (python bootstrap)`, and successful native library load probe.

Note: Existing VS Code task `build-nlc-plugin` currently removes `resources/lib/service.py` during install. Use manual install command (without deleting service.py) until the task definition is updated.
