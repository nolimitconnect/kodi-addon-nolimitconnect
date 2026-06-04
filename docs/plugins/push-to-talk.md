---
title: Push To Talk
---

# Push To Talk

<span class="slot-badge">SLOT 9 — ePluginTypePushToTalk</span>

## What It Does

PushToTalk is a VOIP push-to-talk (PTT) plugin. Unlike [VoicePhone](voice-phone.md), which maintains a continuous bidirectional audio call, PushToTalk transmits audio only while the user holds a designated button. This is analogous to a walkie-talkie or radio-style communication.

PushToTalk is designed for low-latency, lightweight voice communication where full duplex is not needed.

---

## Features

- **Half-duplex voice** — transmit while key is held, receive when released
- **Opus audio codec** — same high-quality 16 kHz mono pipeline used by VoicePhone
- **WebRTC AEC** — acoustic echo cancellation on the capture path
- **RNNoise suppression** — AI-based background noise reduction
- **Low overhead** — no video, no session negotiation overhead

---

## Audio Pipeline

```
Hold PTT button
       │
       ▼
Microphone capture (10 ms AEC frames × 6 = 60 ms)
       │
       ▼
OpusCodec::encode → PktVoiceReq → txPacket to peer
                                         │
                             Peer receives PktVoiceReq
                                         │
                             OpusCodec::decode → PCM
                                         │
                             Kodi audio output (speaker)
```

The NLC engine's `PushToTalkFeedMgr` manages the encode/decode side. The Kodi add-on hooks into Kodi's audio capture API to feed PCM into the NLC `MediaProcessor`.

---

## Kodi UI Implementation

| NLC Qt element | Kodi equivalent |
|---|---|
| PTT applet | Floating `CGUIDialog` or OSD overlay |
| Hold-to-talk button | `CGUIButtonControl` with press/release events |
| Transmit indicator | `CGUIImage` (animated LED state) |
| Peer list | `CGUIListContainer` |

The PTT button can also be mapped to a remote control key via Kodi's keymap system, allowing hands-free operation from a couch remote.

---

## Keymap

A default keymap entry will be provided to bind PTT to a remote control button. Users can override this in Kodi's keymap XML:

```xml
<keymap>
  <global>
    <keyboard>
      <key id="65306">PushToTalk</key>  <!-- example: right Ctrl -->
    </keyboard>
  </global>
</keymap>
```

---

## Configuration

| Setting | Description | Default |
|---|---|---|
| Minimum permission | Minimum peer permission to send PTT to you | Friend |
| Remote key | Kodi key ID bound to PTT | (configurable) |
| Noise suppression | Enable RNNoise on capture | Enabled |

---

## Related

- [Messenger](messenger.md) — full session-based messaging including voice
- [Voice Phone](voice-phone.md) — full duplex VOIP call
- [NLC audio flow docs](https://nolimitconnect.org/technical/README-AUDIO-FLOW/)
