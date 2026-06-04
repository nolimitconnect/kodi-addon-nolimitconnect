---
title: Plugins
---

# Plugin Reference

The Kodi add-on exposes seven NoLimitConnect plugins. Each plugin occupies a specific slot in the NLC plugin system (slots 0–47 are "announced" — visible to the network with configurable permission levels).

## Plugin Overview

<div class="plugin-grid">

<div class="plugin-card">
  <span class="slot-badge">SLOT 8</span>
  <h3>💬 Messenger</h3>
  Text, voice, and video messaging within a persistent session.
  <br><a href="messenger/">Documentation →</a>
</div>

<div class="plugin-card">
  <span class="slot-badge">SLOT 9</span>
  <h3>🎙️ Push To Talk</h3>
  VOIP push-to-talk with Opus audio and noise suppression.
  <br><a href="push-to-talk/">Documentation →</a>
</div>

<div class="plugin-card">
  <span class="slot-badge">SLOT 10</span>
  <h3>📁 Person File Xfer</h3>
  Direct peer-to-peer file transfer.
  <br><a href="person-file-xfer/">Documentation →</a>
</div>

<div class="plugin-card">
  <span class="slot-badge">SLOT 11</span>
  <h3>📷 Cam Server</h3>
  Webcam broadcast to network peers.
  <br><a href="cam-server/">Documentation →</a>
</div>

<div class="plugin-card">
  <span class="slot-badge">SLOT 12</span>
  <h3>📂 File Share Server</h3>
  Shared file library and streamable media source.
  <br><a href="file-share-server/">Documentation →</a>
</div>

<div class="plugin-card">
  <span class="slot-badge">SLOT 15</span>
  <h3>🎥 Video Chat</h3>
  Video chat with motion detection and recording.
  <br><a href="video-chat/">Documentation →</a>
</div>

<div class="plugin-card">
  <span class="slot-badge">SLOT 16</span>
  <h3>📞 Voice Phone</h3>
  VOIP audio-only phone call with echo cancellation.
  <br><a href="voice-phone/">Documentation →</a>
</div>

</div>

---

## NLC Plugin System Background

NLC supports up to 48 "announced" plugin slots (0–47). Announced plugins are visible to the network and subject to the four-level permission system (Ignore / Guest / Friend / Admin). Slots 48+ are reserved for internal or non-announced plugins.

The seven plugins above are a subset of the 17 currently active NLC plugin slots. The add-on focuses on the communication and media-sharing plugins that map naturally to a media center use case.

For a full description of the plugin architecture see the [NoLimitConnect plugin system docs](https://nolimitconnect.org/technical/plugin-system-overview/).

---

## Permission Levels

Each plugin respects the NLC permission threshold. Users can set the minimum permission level required for a peer to initiate each plugin:

| Level | Typical use |
|---|---|
| **Ignore** | Block all contact |
| **Guest** | Allow unknown peers (default) |
| **Friend** | Verified contacts only |
| **Admin** | Reserved for elevated access |

Plugin-level permissions are configured in the add-on settings screen.
