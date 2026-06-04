---
title: Voice Phone
---

# Voice Phone

<span class="slot-badge">SLOT 16 — ePluginTypeVoicePhone</span>

## What It Does

VoicePhone is an audio-only VOIP phone call plugin. It provides a full duplex, real-time voice call between two NLC peers using the Opus audio codec with WebRTC acoustic echo cancellation (AEC) and RNNoise background noise suppression.

VoicePhone is the audio-only complement to [VideoChat](video-chat.md). It uses the same underlying audio pipeline but without any camera capture or video transmission.

---

## Features

- **Full duplex VOIP** — simultaneous bidirectional voice
- **Opus codec** — 16 kHz mono, 60 ms frames, high quality at low bitrate
- **WebRTC AEC** — acoustic echo cancellation prevents speaker audio feeding back into the microphone
- **RNNoise suppression** — AI-based noise reduction on the capture path
- **Offer/accept model** — incoming calls appear in Kodi's notification system
- **Single-session enforcement** — only one active VoicePhone call at a time (same peer pair)

---

## Call Flow

```
Caller                              Receiver
  │                                    │
  │── PktPluginOffer (type=16) ───────►│
  │                                    │  (Kodi ringing notification)
  │◄── PktPluginOfferReply (accept) ───│
  │                                    │
  │  fromGuiStartPluginSession(16, ...) │
  │                                    │
  │◄══════ VoicePhone session active ══►│
  │                                    │
  │◄──── PktVoiceReq (Opus frames) ───►│
  │                                    │
  │──── end call ──────────────────────│
```

---

## Audio Pipeline Detail

```
Microphone (platform capture)
       │  10 ms PCM frames
       ▼
WebRTC AEC (echo cancellation)
       │  6 × 10 ms frames accumulated → 60 ms
       ▼
RNNoise (background noise suppression)
       │
       ▼
OpusCodec::encode (960 samples @ 16 kHz)
       │
       ▼
PktVoiceReq → encrypted P2P packet → peer

─── receive path ───────────────────────────

Peer PktVoiceReq received
       │
       ▼
OpusCodec::decode → 960 PCM samples
       │
       ▼
Jitter buffer / mixer
       │
       ▼
Kodi audio output (IAEStream)
```

The NLC `VoiceFeedMgr` handles per-session encode/decode state. The Kodi add-on feeds raw PCM from Kodi's audio capture API into the NLC `MediaProcessor`.

---

## Kodi UI Implementation

| NLC Qt element | Kodi equivalent |
|---|---|
| VoicePhone applet | `CGUIDialog` (compact call window) |
| Mute button | `CGUIToggleButtonControl` |
| End call button | `CGUIButtonControl` |
| Call timer | `CGUILabelControl` |
| Incoming call notification | `CGUIDialogKaiToast` + offer list |

### Background Operation

VoicePhone can optionally run in the background while Kodi plays back media. In this mode the add-on reduces the NLC audio output volume to avoid competing with Kodi's media audio. This is controlled by Kodi's `IAEStream` volume envelope.

---

## Configuration

| Setting | Description | Default |
|---|---|---|
| Minimum permission | Minimum peer permission to call you | Friend |
| Noise suppression | Enable RNNoise on capture | Enabled |
| Echo cancellation | Enable WebRTC AEC | Enabled |
| Background mode | Allow calls while media is playing | Enabled |
| Ring tone | Audio file to play on incoming call | (built-in) |

---

## Comparison: VoicePhone vs. PushToTalk vs. Messenger

| | VoicePhone (16) | PushToTalk (9) | Messenger (8) |
|---|---|---|---|
| Duplex | Full duplex | Half duplex (PTT) | Full duplex |
| Session | Offer/accept | Always-on (when permitted) | Offer/accept |
| Video | No | No | Optional snippets |
| Text | No | No | Yes |
| Use case | Private phone call | Walkie-talkie style | Full messaging |

---

## Related

- [Push To Talk](push-to-talk.md) — half-duplex walkie-talkie style
- [Video Chat](video-chat.md) — add video to the call
- [Messenger](messenger.md) — full session with text, voice, and video
- [NLC audio flow docs](https://nolimitconnect.org/technical/README-AUDIO-FLOW/)
- [NLC Opus record/playback docs](https://nolimitconnect.org/technical/README-OPUS-FILE-RECORD-AND-PLAYBACK/)
