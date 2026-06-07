---
title: Overview
---

# NoLimitConnect for Kodi — Overview

## What Is This Add-on?

**kodi-addon-nolimitconnect** is a Kodi binary add-on that brings the [NoLimitConnect (NLC)](https://nolimitconnect.org) peer-to-peer communication platform into the Kodi media center. Rather than running NLC as a separate desktop application, this add-on embeds NLC's C/C++ engine directly into Kodi so that users can text, voice-chat, video-call, transfer files, and share media — all inside the Kodi interface they already know.

!!! info "Upstream project"
    NoLimitConnect is an independent, user-hosted social platform built in C/C++ with Qt. The Kodi add-on replaces the Qt GUI layer with Kodi's native GUI while reusing the full NLC networking engine unchanged.

---

## Core Principles

The add-on inherits all of NoLimitConnect's core values:

| Principle | Meaning |
|---|---|
| **No registration** | Users choose a display name — no email, phone number, or account required |
| **No ads** | Zero advertising, tracking, or data mining |
| **User-controlled infrastructure** | Anyone can run a network host; `nolimitconnect.net` is the default but not mandatory |
| **Privacy by design** | All communication is encrypted end-to-end; no central logging |
| **Native performance** | Pure C/C++ engine with Opus audio, FFmpeg media, and OpenGL rendering |

---

## How It Works

### Service Run Behavior in Kodi

This add-on is a Kodi service add-on. In Kodi's Add-on Information screen, the rocket-icon Run control is disabled for service add-ons and is not used as the startup trigger.

To start the configured NLC startup/login flow, use:

- Settings -> Network -> Run Now

Run Now is a one-shot trigger. Turning it on executes startup validation/login and then automatically resets it to Off.

### 1. Sign-On Flow

```
Kodi starts add-on service
       │
       ▼
Load profile settings
  (display name + last random host)
       │
       ▼
Wait for user to open NLC UI
       │
       ▼
Is a display name configured?
  ├─ No  → Prompt user via Kodi dialog → Save display name
  └─ Yes → Use existing name
       │
       ▼
Join preferred random host
  (last joined host, default: nolimitconnect.net)
       │
       ▼
Add-on is online for that host
```

When the user enters NLC UI and triggers connect, the add-on resolves infrastructure services for the selected random-connect host:

- **ConnectionTestHost** — confirms the user's external IP address and whether their TCP listen port is reachable from the internet.
- **NetworkHost** — the directory service that stores and serves the list of active user hosts. All user hosts (RandomConnect, Group, ChatRoom) announce themselves here.

On successful join, that host becomes the new default (`last_random_connect_host`) for subsequent logins. Users can leave and join a different random host; the most recently joined host is used next time.

### 2. P2P vs. Relay Connections

NLC uses a hybrid networking model:

```
Peer A ──────── direct TCP/UDP ──────── Peer B
          (when both ports are open)

Peer A ── relay ── NLC relay server ── relay ── Peer B
          (when NAT or firewall blocks direct connection)
```

The engine automatically chooses the best path. Direct connections are always preferred for performance. Relay is used as a fallback — no manual configuration required.

### 3. Plugin System

NLC's functionality is organized into numbered plugin slots (0–47 are "announced" — visible to the network with permission levels). The Kodi add-on must remain compatible with the full plugin set required by the NLC engine runtime. For initial rollout, the Kodi UI and workflow focus on the seven core slots below while other plugins remain present but disabled by default.

| Slot | Plugin | Category | Initial UI State |
|---|---|---|
| 8 | Messenger | Peer-to-peer | Enabled |
| 9 | PushToTalk | Peer-to-peer | Enabled |
| 10 | PersonFileXfer | Peer-to-peer | Enabled |
| 11 | CamServer | Peer server | Enabled |
| 12 | FileShareServer | Peer server | Enabled |
| 15 | VideoChat | Peer-to-peer | Enabled |
| 16 | VoicePhone | Peer-to-peer | Enabled |

All non-MVP plugin slots are still built and initialized as required by the engine integration, but are set to disabled until their Kodi UX is implemented.

Each plugin slot has a corresponding permission level. Users can control which peers can use each plugin on their node (Ignore / Guest / Friend / Admin).

---

## Architecture

### NLC Engine vs. Kodi GUI

The NLC engine (`libptopengine`) is a self-contained C++ library that handles all networking, media encoding/decoding, and plugin logic. In the original desktop app it communicates with the Qt GUI through two interfaces:

- **`IToGui`** — engine → GUI callbacks (e.g., "session started", "message received", "connection status changed")
- **`IFromGui`** — GUI → engine commands (e.g., "start plugin session", "send file", "accept offer")

In this Kodi add-on those two interfaces are implemented using Kodi's native GUI APIs instead of Qt widgets.

```
┌─────────────────────────────────────────────────────┐
│                   Kodi Process                      │
│                                                     │
│  ┌──────────────────────────────────────────────┐  │
│  │           Kodi Add-on (this project)          │  │
│  │                                               │  │
│  │  ┌─────────────────┐   ┌──────────────────┐  │  │
│  │  │  Kodi GUI Layer │   │  NLC Engine       │  │  │
│  │  │  (windows,      │◄──│  (libptopengine)  │  │  │
│  │  │   dialogs,      │──►│                  │  │  │
│  │  │   controls)     │   │  Plugins 8–16    │  │  │
│  │  └─────────────────┘   └──────────────────┘  │  │
│  │                                               │  │
│  │  IToGui / IFromGui bridge (C++ interfaces)    │  │
│  └──────────────────────────────────────────────┘  │
│                                                     │
│  Kodi core APIs: GUI, audio, video, filesystem      │
└─────────────────────────────────────────────────────┘
```

### Qt → Kodi GUI Migration

The original NLC uses Qt's widget system for all UI. Key mapping between concepts:

| NLC / Qt concept | Kodi equivalent |
|---|---|
| `QDialog` / applet window | `CGUIWindow` or `CGUIDialog` |
| `QWidget` layout | XML skin layout with `<control>` elements |
| Qt signals/slots | Kodi `CGUIMessage` / `OnMessage()` |
| `QLabel`, `QTextEdit` | `CGUITextBox`, `CGUIEditControl` |
| `QListView` | `CGUIListContainer` / `CGUIFocusPlane` |
| Qt audio (`QAudioInput`) | Kodi `IAEStream` + platform audio capture |
| Qt video (`QVideoWidget`) | Kodi `IVideoPlayer` render surface |
| `QPushButton` | `CGUIButtonControl` |
| Qt threading | Kodi `CThread` / `CJobManager` |

### Media Pipeline

Audio and video use the NLC engine's existing media stack:

- **Audio:** 16 kHz mono PCM → Opus encode → P2P packet → Opus decode → 16 kHz PCM playback. AEC (WebRTC) and RNNoise suppression are applied on the capture path.
- **Video (VideoChat / CamServer):** Camera frame → MJPEG or H.264 encode → P2P packet → decode → render in Kodi window.
- **File streaming (FileShareServer):** Virtual file I/O layer presents remote files as a seekable byte stream; Kodi's media player reads through it as if it were a local file.

---

## Network Topology

```
                      nolimitconnect.net
                   ┌──────────────────────┐
                   │  NetworkHost (dir)   │
                   │  RandomConnectHost   │
                   │  ConnectionTestHost  │
                   └──────────────────────┘
                          │        │
              ┌───────────┘        └───────────┐
              │                                │
        Kodi User A                      Kodi User B
      (this add-on)                    (this add-on)
              │                                │
              └──── direct P2P (preferred) ────┘
              └─── or relay via NLC relay ─────┘
```

The `nolimitconnect.net` server hosts the infrastructure services (directory + connectivity test) and a default Random Connect host. Once two peers discover each other through the directory, their ongoing communication is direct — the central server is no longer in the data path.

---

## Permissions

NLC's four-level permission system is preserved in the add-on:

| Level | What peers at this level can do |
|---|---|
| **Ignore** | Cannot contact you at all |
| **Guest** | Default for unknown peers; limited plugin access |
| **Friend** | Full access to enabled plugins |
| **Admin** | Same as Friend; reserved for future elevated features |

Per-plugin permission thresholds let users fine-tune exactly who can initiate a Messenger session, file transfer, video call, etc.

---

## Further Reading

- [Plugin Reference](plugins/index.md) — all seven plugins in detail
- [Architecture Deep-Dive](developer-docs/architecture.md) — engine internals, IToGui/IFromGui bridge design
- [Building the Add-on](developer-docs/building.md) — build system setup and CMake options
- [NoLimitConnect technical docs](https://nolimitconnect.org/technical/) — upstream plugin system, session flow, audio flow, and more
