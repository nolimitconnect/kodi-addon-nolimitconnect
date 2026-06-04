---
title: Architecture
---

# Architecture

This document describes how the Kodi add-on integrates the NLC engine, the IToGui/IFromGui bridge design, threading model, and the key implementation challenges for each subsystem.

---

## High-Level Design

```
┌──────────────────────────────────────────────────────────────┐
│                        Kodi Process                          │
│                                                              │
│  ┌────────────────────────────────────────────────────────┐  │
│  │               kodi-addon-nolimitconnect                │  │
│  │                                                        │  │
│  │  ┌─────────────────────┐   ┌──────────────────────┐   │  │
│  │  │   Kodi GUI Layer    │   │   NLC Engine          │   │  │
│  │  │                     │   │   (libptopengine)     │   │  │
│  │  │  CGUIWindow/Dialog  │◄──│                      │   │  │
│  │  │  CGUIControls       │──►│   PluginMgr          │   │  │
│  │  │  Skin XML           │   │   Plugins 8–16       │   │  │
│  │  │  IAEStream (audio)  │   │   MediaProcessor     │   │  │
│  │  │  render surface     │   │   NetServicesMgr     │   │  │
│  │  └─────────────────────┘   └──────────────────────┘   │  │
│  │                                                        │  │
│  │  ToGuiBridge   (IToGui  → Kodi events/callbacks)       │  │
│  │  FromGuiBridge (IFromGui → NLC engine commands)        │  │
│  │                                                        │  │
│  │  KodiAudioCapture  (mic PCM → MediaProcessor)          │  │
│  │  KodiAudioOutput   (mixer PCM → IAEStream)             │  │
│  │  NlcStreamFile     (VirtStreamMgr → CFileItem)         │  │
│  └────────────────────────────────────────────────────────┘  │
│                                                              │
│  Kodi core: GUI, audio engine, filesystem, media player     │
└──────────────────────────────────────────────────────────────┘
```

---

## IToGui / IFromGui Bridge

The NLC engine communicates with its host application through two pure-virtual C++ interfaces defined in `libs/GuiInterface/`:

### `IToGui` — Engine → GUI

The engine calls methods on `IToGui` to notify the GUI of events:

```cpp
// Examples from IToGui.h
virtual void toGuiNetworkState( ENetworkState netState ) = 0;
virtual void toGuiPluginSessionStarted( EPluginType pluginType, VxGUID& onlineId ) = 0;
virtual void toGuiPluginSessionEnded( EPluginType pluginType, VxGUID& onlineId ) = 0;
virtual void toGuiRxedPluginOffer( PluginOfferInfo& offerInfo ) = 0;
virtual void toGuiInstMsg( VxGUID& srcOnlineId, std::string& msg ) = 0;
virtual void toGuiMotionDetected( EPluginType pluginType, VxGUID& onlineId ) = 0;
```

`ToGuiBridge` implements `IToGui` and translates each callback into a Kodi event: posting a `CGUIMessage`, triggering a `CGUIDialogKaiToast` notification, or updating a GUI control's data model.

Because the engine calls `IToGui` methods from its own network/worker threads, all Kodi GUI calls must be marshalled to the Kodi main thread. `ToGuiBridge` uses Kodi's `CGUIWindowManager::SendThreadMessage()` for this.

### `IFromGui` — GUI → Engine

The GUI calls methods on `IFromGui` to command the engine:

```cpp
// Examples from IFromGui.h
virtual void fromGuiStartPluginSession( EPluginType pluginType, VxGUID& onlineId ) = 0;
virtual void fromGuiStopPluginSession( EPluginType pluginType, VxGUID& onlineId ) = 0;
virtual void fromGuiSendTextMessage( VxGUID& destOnlineId, std::string& msg ) = 0;
virtual void fromGuiStartCamServer() = 0;
virtual void fromGuiStopCamServer() = 0;
virtual void fromGuiSendFile( VxGUID& destOnlineId, std::string& filePath ) = 0;
```

`FromGuiBridge` wraps `IFromGui` and is called directly by Kodi GUI event handlers (button clicks, dialog actions, keymap events).

---

## Threading Model

| Thread | Owner | Role |
|---|---|---|
| Kodi main thread | Kodi | GUI rendering and event dispatch |
| NLC engine thread | libptopengine | Network I/O, plugin processing |
| Audio capture thread | KodiAudioCapture | Mic PCM → MediaProcessor |
| Audio output thread | Kodi AE | Pull decoded PCM via IAEStream |
| Video capture thread | platform / V4L2 | Camera frames → CamServer / VideoChat |

**Critical rule:** Never call Kodi GUI APIs from the NLC engine thread. Use `CGUIWindowManager::SendThreadMessage()` or `CApplicationMessenger::PostMsg()` to marshal back to the Kodi main thread.

---

## Audio Integration

### Capture (Mic → NLC)

`KodiAudioCapture` opens a platform audio capture stream and produces 10 ms PCM frames at 16 kHz mono. These frames are fed into `MediaProcessor::fromGuiEchoCanceledSamplesThreaded()`. After 6 frames (60 ms) have accumulated, `MediaProcessor` encodes them with Opus and dispatches the packet to the appropriate plugin's feed manager.

### Playback (NLC → Speaker)

`KodiAudioOutput` registers as an `IAEStream` with Kodi's Audio Engine. The NLC `AudioMixerMgr` pulls mixed 60 ms PCM from the engine and writes it into the `IAEStream` buffer. Kodi's AE thread drains the buffer and sends it to the hardware.

```
KodiAudioCapture → MediaProcessor → Opus encode → PktVoiceReq → peer
peer → PktVoiceReq → Opus decode → AudioMixerMgr → KodiAudioOutput → IAEStream → speaker
```

---

## Video Integration

### Camera Capture

`KodiVideoCapture` (planned) wraps V4L2 (Linux) or platform-specific capture APIs to produce JPEG or raw frames. Frames are pushed into the NLC `CamServer` or `VideoChat` plugin's frame pipeline.

### Render Surface

Remote video frames received from VideoChat or CamServer are rendered to a Kodi texture using `CRenderCapture` / an OpenGL texture updated each frame. The texture is displayed in the skin XML via `<control type="image">` with a dynamic texture URL.

---

## File Streaming Integration

`NlcStreamFile` implements Kodi's `IFile` interface (`CFileFactory`-registered scheme `nlcstream://`). It delegates read/seek/open/close calls to `VirtStreamMgr`. This lets Kodi's built-in media player open a remote NLC file as if it were a local file, with full seek support.

```cpp
// Registered in add-on init:
CFileFactory::RegisterProtocol("nlcstream", new CNlcStreamFile());

// Kodi player opens:
CFileItem item("nlcstream://peer-id/file-hash", false);
g_application.PlayFile(item);
```

---

## Add-on Lifecycle

```
Kodi loads add-on (IAddonInstance::Create)
    │
    ▼
NlcAddon::Initialize()
    ├── Create ToGuiBridge (IToGui implementation)
    ├── Create FromGuiBridge (IFromGui wrapper)
    ├── Initialize libptopengine with engine settings
    │       (user name, network host URLs, listen port)
    ├── Start NLC engine (NetServicesMgr begins DNS + connect)
    └── Register nlcstream:// file protocol

Engine resolves nolimitconnect.net
    └── toGuiNetworkState(eNetworkStateOnline) → update Kodi status indicator

User navigates to NLC add-on in Kodi
    └── CGUIWindowManager opens WindowMain

User performs action (e.g., accept call)
    └── GUI calls FromGuiBridge → engine accepts offer → session starts
    └── Engine calls ToGuiBridge::toGuiPluginSessionStarted → open dialog

Kodi unloads add-on
    └── NlcAddon::Destroy() → engine shutdown → thread join
```

---

## Further Reading

- [Building the Add-on](building.md)
- [NoLimitConnect IToGui/IFromGui source](https://github.com/nolimitconnect/NoLimitConnect/tree/main/libs/GuiInterface)
- [NLC plugin system overview](https://nolimitconnect.org/technical/plugin-system-overview/)
- [NLC session flow](https://nolimitconnect.org/technical/README-SESSION-FLOW/)
- [NLC audio flow](https://nolimitconnect.org/technical/README-AUDIO-FLOW/)
