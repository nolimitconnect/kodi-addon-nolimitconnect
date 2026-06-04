---
title: Messenger
---

# Messenger

<span class="slot-badge">SLOT 8 — ePluginTypeMessenger</span>

## What It Does

Messenger is a session-based messaging plugin that supports text, voice, and video messaging between two NLC peers. A Messenger session is initiated by one peer offering a session; the other peer accepts or declines via the offer system.

Once a session is active, the conversation window persists for the duration of the session. Messages, audio clips, and video frames sent within the session are delivered in order over the encrypted P2P connection.

---

## Features

- **Text messaging** — UTF-8 messages with emoji support
- **Voice messages** — Opus-encoded audio recorded and sent within the session
- **Video snippets** — Short video clips attached to the message thread
- **Session-based** — The conversation is tied to the session lifetime; history is stored locally
- **Encrypted** — All traffic is end-to-end encrypted over the NLC transport layer

---

## How Sessions Work

Messenger uses the NLC offer/session pipeline:

```
User A                              User B
  │                                    │
  │──── PktPluginOffer (type=8) ──────►│
  │                                    │  (offer appears in offer list)
  │◄─── PktPluginOfferReply (accept) ──│
  │                                    │
  │  fromGuiStartPluginSession(8, ...) │
  │                                    │
  │◄══════ Messenger session active ══►│
  │                                    │
  │◄───── PktVoiceReq / text pkts ────►│
  │                                    │
  │──── session end ───────────────────│
```

Both peers must have Messenger enabled (permission level ≥ the requester's permission) for the session to be accepted.

---

## Kodi UI Implementation

In the original NLC desktop app, Messenger is rendered as a Qt applet widget. In the Kodi add-on this is replaced with:

| NLC Qt element | Kodi equivalent |
|---|---|
| Messenger applet window | `CGUIDialog` |
| Message list view | `CGUIListContainer` with custom skin item |
| Text input field | `CGUIEditControl` |
| Voice record button | `CGUIButtonControl` → audio capture |
| Session status indicator | `CGUILabelControl` |

---

## Audio in Messenger

Voice messages use the same Opus pipeline as [VoicePhone](voice-phone.md) and [PushToTalk](push-to-talk.md):

- Capture: 16 kHz mono PCM at 60 ms frames
- Encode: Opus with WebRTC AEC and RNNoise suppression
- Transport: `PktVoiceReq` packets over the P2P connection
- Decode: Opus → 16 kHz PCM → Kodi audio output

---

## Configuration

| Setting | Description | Default |
|---|---|---|
| Minimum permission | Minimum peer permission level to accept Messenger offers | Guest |
| Save history | Store session message history locally | Enabled |
| Notification sound | Play audio notification on incoming message | Enabled |

---

## Related

- [Push To Talk](push-to-talk.md) — voice-only VOIP
- [Voice Phone](voice-phone.md) — dedicated audio-only call
- [Video Chat](video-chat.md) — full video session
- [NLC session flow docs](https://nolimitconnect.org/technical/README-SESSION-FLOW/)
