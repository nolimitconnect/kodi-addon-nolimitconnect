---
title: Video Chat
---

# Video Chat

<span class="slot-badge">SLOT 15 — ePluginTypeVideoChat</span>

## What It Does

VideoChat is a bidirectional, session-based video call plugin. It provides full audio/video communication between two NLC peers, with optional motion detection on the incoming video stream and optional session recording.

VideoChat follows the NLC offer/session pipeline: one peer sends an offer; the other accepts or declines. Once the session is active, both peers exchange video frames and audio packets.

---

## Features

- **Full duplex video and audio** — simultaneous bidirectional video + Opus VOIP
- **Motion detection** — configurable per-session; detected motion can trigger recording
- **Session recording** — record the full video session to a local AVI file
- **Offer/accept model** — incoming video call shows in the Kodi offer list, just like a phone call
- **Single-session enforcement** — only one active VideoChat session at a time per peer pair

---

## Call Flow

```
Caller                              Receiver
  │                                    │
  │── PktPluginOffer (type=15) ───────►│
  │                                    │  (Kodi ringing notification)
  │◄── PktPluginOfferReply (accept) ───│
  │                                    │
  │  fromGuiStartPluginSession(15, ...) │
  │                                    │
  │◄═══════ VideoChat session active ══►│
  │                                    │
  │◄──── PktVoiceReq (audio) ─────────►│  (Opus 16kHz mono)
  │◄──── PktCamFrameReq (video) ──────►│  (MJPEG or H.264 frames)
  │                                    │
  │──── session end ───────────────────│
```

---

## Media Pipelines

### Audio
Same Opus pipeline as [VoicePhone](voice-phone.md):
- 16 kHz mono PCM → WebRTC AEC → RNNoise → Opus encode → P2P packet
- Received packet → Opus decode → 16 kHz PCM → Kodi audio output

### Video
- **Capture:** Platform camera (V4L2 on Linux) → JPEG / H.264 frame encode
- **Transmit:** `PktCamFrameReq` packets over P2P connection
- **Receive:** Decode frames → render to Kodi texture surface

---

## Motion Detection

When motion detection is enabled the NLC engine applies frame differencing on the incoming video stream. If the fraction of changed pixels exceeds a threshold the engine fires a `toGuiMotionDetected` callback. The add-on responds by:

1. Highlighting the video window
2. Optionally starting a recording session

Motion sensitivity is configurable in the add-on settings.

---

## Kodi UI Implementation

| NLC Qt element | Kodi equivalent |
|---|---|
| VideoChat applet | Full-screen or windowed `CGUIWindow` |
| Local video preview | Kodi render texture (picture-in-picture corner) |
| Remote video | Full render surface |
| Call controls (mute, end, record) | `CGUIButtonControl` row / OSD |
| Incoming call notification | `CGUIDialogKaiToast` + offer list |
| Recording indicator | `CGUILabelControl` overlay ("REC ●") |

---

## Recording

Recorded sessions are saved as AVI files to the configured recording path. File names include a timestamp and the peer's display name. Recordings can be played back immediately in Kodi or shared via [FileShareServer](file-share-server.md).

---

## Configuration

| Setting | Description | Default |
|---|---|---|
| Minimum permission | Minimum peer permission to initiate a video call | Friend |
| Recording path | Folder for saved session recordings | `special://userdata/addon_data/nlc/recordings/` |
| Motion detection | Enable motion detection on incoming video | Disabled |
| Motion sensitivity | Pixel-change threshold (0–100) | 20 |
| Auto-record on motion | Start recording when motion is detected | Disabled |
| Default camera | Camera device for video capture | Default |
| Video quality | Encoding quality preset | Medium |

---

## Related

- [Cam Server](cam-server.md) — one-way webcam broadcast
- [Voice Phone](voice-phone.md) — audio-only call
- [Messenger](messenger.md) — session-based messaging with optional voice/video snippets
- [NLC session flow docs](https://nolimitconnect.org/technical/README-SESSION-FLOW/)
- [NLC camera capture docs](https://nolimitconnect.org/technical/README-CAMERA-CAPTURE/)
