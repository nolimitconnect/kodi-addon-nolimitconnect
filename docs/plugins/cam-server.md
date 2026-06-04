---
title: Cam Server
---

# Cam Server

<span class="slot-badge">SLOT 11 — ePluginTypeCamServer</span>

## What It Does

CamServer broadcasts a live webcam stream to permitted NLC peers. Any peer with sufficient permission can connect as a viewer and receive the MJPEG stream. Multiple viewers can connect simultaneously — the server sends to all connected clients.

CamServer is a one-way broadcast (server → clients). For bidirectional video see [VideoChat](video-chat.md).

---

## Features

- **Live MJPEG broadcast** — low-latency webcam stream to multiple peers simultaneously
- **Multi-client** — any number of permitted peers can view at once
- **Client count indicator** — the add-on shows how many peers are currently watching
- **Motion detection** (optional) — frame differencing to detect activity and trigger recording
- **On-demand start/stop** — broadcast can be started and stopped at any time

---

## Stream Architecture

```
Webcam device (V4L2 / platform capture)
       │
       ▼
Camera capture thread (JPEG frames)
       │
       ▼
CamServer plugin
       ├──► Client A (PktCamFrameReq over P2P connection)
       ├──► Client B
       └──► Client C
```

The NLC engine handles multi-client fan-out and per-client packet transmission. The Kodi add-on provides:
1. Camera device selection UI
2. A live preview window using Kodi's render surface
3. The client count badge in the Kodi OSD

---

## Kodi UI Implementation

| NLC Qt element | Kodi equivalent |
|---|---|
| CamServer applet | `CGUIWindow` with video render texture |
| Camera device selector | `CGUIDialogSelect` |
| Client count badge | `CGUILabelControl` overlay |
| Start/stop button | `CGUIButtonControl` |
| Motion detection toggle | `CGUIToggleButtonControl` |

---

## Relationship with VideoChat

CamServer (slot 11) and VideoChat (slot 15) both involve camera capture but differ in purpose:

| | CamServer (11) | VideoChat (15) |
|---|---|---|
| Direction | One-way broadcast | Bidirectional |
| Viewers | Multiple simultaneous | One peer (1:1) |
| Session model | Server-client | Offer/accept session |
| Audio | No | Yes |
| Motion detection | Optional | Yes |
| Recording | Optional | Yes |

---

## Configuration

| Setting | Description | Default |
|---|---|---|
| Minimum permission | Minimum peer permission to view your cam | Friend |
| Camera device | System camera device to capture from | Default |
| Frame rate | Target broadcast frame rate (fps) | 15 |
| JPEG quality | Compression quality (1–100) | 75 |
| Enable motion detection | Detect frame differences | Disabled |
| Auto-record on motion | Start recording when motion detected | Disabled |

---

## Related

- [Video Chat](video-chat.md) — bidirectional video session
- [File Share Server](file-share-server.md) — share media files (including recorded cam sessions)
- [NLC MJPEG video docs](https://nolimitconnect.org/technical/README-MJPEG-VIDEO/)
