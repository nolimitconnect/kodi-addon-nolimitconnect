---
title: Reuse Matrix
---

# Code Reuse Matrix

This document maps upstream NoLimitConnect code to the Kodi add-on codebase.

## Scope Decision (2026-06-06)

- Full plugin compatibility is required for stable engine behavior.
- MVP UI exposure remains focused on slots 8, 9, 10, 11, 12, 15, 16.
- Non-MVP slots stay integrated but disabled by default until Kodi UX is implemented.
- The full upstream libs tree is expected to be imported into this repository.

## Vendor Layout Plan

| Source | Target in this repo | Intent |
|---|---|---|
| `/home/nolimit/NoLimitConnect/libs` | `src/libs` (current) | Vendored upstream engine and dependencies |
| `/home/nolimit/NoLimitConnect/nolimitgui/src` | Reference only | Port Qt behavior to Kodi windows/dialogs |

Notes:
- `NLC_LIBS_DIR` in CMake allows later relocation (for example to `third_party/nlc/libs`) without source-code path changes.
- Apply durable implementation changes in this repository, not in `/home/nolimit/kodi-source`.

## Core Bridge Mapping

| Upstream path | Current/target add-on path | Action | Status |
|---|---|---|---|
| `libs/GuiInterface/IToGui.h` | `src/addon/ToGuiBridge.h` + `src/addon/ToGuiBridge.cpp` | Implement callback translation to Kodi events | In progress |
| `libs/GuiInterface/IFromGui.h` | `src/addon/FromGuiBridge.h` + `src/addon/FromGuiBridge.cpp` | Implement GUI command dispatch into engine | In progress |
| `libs/GuiInterface/IFromGuiDefs.h` | `src/addon/BridgeTypes.h` | Keep enum/command translation alignment | In progress |
| `libs/libptopengine/NetServices/NetServicesMgr.*` | `src/core/SignOnFlow.*` + `src/addon/NlcAddon.*` | Reuse bootstrap semantics for network sign-on | In progress |

## Media and Streaming Mapping

| Upstream path | Current/target add-on path | Action | Status |
|---|---|---|---|
| `libs/libptopengine/MediaProcessor/MediaProcessor.*` | `src/audio/KodiAudioCapture.*` + `src/audio/KodiAudioOutput.*` | Connect mic/playback frames to engine media path | Not started |
| `libs/libptopengine/Plugins/VoiceFeedMgr.*` | `src/audio/KodiAudioOutput.*` | Adapt voice playback timing and buffering | Not started |
| `libs/libptopengine/Plugins/PushToTalkFeedMgr.*` | `src/audio/KodiAudioCapture.*` | Adapt press-to-transmit behavior | Not started |
| `nolimitgui/src/VirtStream/*` | `src/stream/NlcStreamFile.*` | Port virtual stream file IO into Kodi file interface | In progress |
| `libs/libptopengine/Plugins/FileShareXfer*` | `src/stream/NlcStreamFile.*` + future transfer dialogs | Reuse file share transfer/session logic | Not started |

## Plugin Slot Mapping (MVP Exposed)

| Slot | Plugin | Upstream engine area | Kodi add-on area | Status |
|---|---|---|---|---|
| 8 | Messenger | `libs/libptopengine/Plugins` + message callbacks in `IToGui` | `src/gui` dialog and message event handlers | Not started |
| 9 | PushToTalk | `libs/libptopengine/PushToTalk` + `Plugins/PushToTalkFeedMgr.*` | `src/audio` + PTT dialog controls | Not started |
| 10 | PersonFileXfer | `libs/libptopengine/Plugins/FileInfoPersonFileXferMgr.*` | file transfer dialog + bridge commands | Not started |
| 11 | CamServer | `libs/libptopengine/MediaProcessor` + video plugin handlers | camera capture bridge + preview window | Not started |
| 12 | FileShareServer | `libs/libptopengine/Plugins/FileShareXfer*` | stream bridge and browse/play dialogs | Not started |
| 15 | VideoChat | `libs/libptopengine/Plugins/VideoFeedMgr.*` + media processor | render surface + session controls | Not started |
| 16 | VoicePhone | `libs/libptopengine/Plugins/VoiceFeedMgr.*` | call session UI + shared audio pipeline | Not started |

## Qt to Kodi UI Porting Seeds

| Upstream Qt source | Kodi replacement direction |
|---|---|
| `nolimitgui/src/Forms/AppletMultiMessenger.ui` | Kodi messenger dialog XML + list/edit controls |
| `nolimitgui/src/Forms/AppletPeerVoicePhone.ui` | Kodi voice call dialog with accept/end controls |
| `nolimitgui/src/Forms/AppletPeerVideoPhone.ui` | Kodi video call dialog + texture surface |
| `nolimitgui/src/Forms/FileXferWidget.ui` | Kodi transfer queue dialog |
| `nolimitgui/src/Forms/AppletFileShareClientView.ui` | Kodi remote file browser dialog |
| `nolimitgui/src/Forms/AppletCamClient.ui` | Kodi camera preview and publish dialog |

## Integration Checklist

- Import libs tree under `src/libs` (or set `-DNLC_LIBS_DIR=/path/to/libs`).
- Add CMake options to build against vendored libs paths.
- Compile and register all plugin handlers required by engine startup.
- Keep non-MVP slots disabled by default in plugin settings.
- Validate startup logs: no missing plugin manager or initialization errors.
