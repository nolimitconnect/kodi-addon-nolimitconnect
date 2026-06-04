---
title: Person File Xfer
---

# Person File Xfer

<span class="slot-badge">SLOT 10 — ePluginTypePersonFileXfer</span>

## What It Does

PersonFileXfer provides direct peer-to-peer file transfer between two NLC users. Files are sent directly over the encrypted P2P connection — no cloud storage, no intermediary server holds the data. If a direct TCP connection is available between the peers, files travel that path; otherwise the NLC relay infrastructure is used.

---

## Features

- **Direct P2P transfer** — data goes peer-to-peer whenever possible
- **No size limit** — transfer size is bounded only by disk space and connection speed
- **Encrypted in transit** — all data is encrypted over the NLC transport layer
- **Resumable** — interrupted transfers can be resumed from the last acknowledged chunk
- **Any file type** — no content filtering or type restriction

---

## Transfer Flow

```
Sender                              Receiver
  │                                    │
  │── PktFileGetReq (file metadata) ──►│
  │                                    │  (Kodi dialog: accept/decline)
  │◄── PktFileGetReply (accept) ───────│
  │                                    │
  │── PktFileChunkReq (data chunks) ──►│  (progress bar)
  │── PktFileChunkReq ────────────────►│
  │   ...                              │
  │── PktFileChunkFinalReq ───────────►│
  │                                    │
  │◄── PktFileChunkAck ────────────────│
  │                                    │
  │         transfer complete          │
```

The NLC engine handles chunking, acknowledgement, and flow control. The Kodi add-on provides the UI for browsing local files to send and displaying incoming transfer requests.

---

## Kodi UI Implementation

| Action | Kodi equivalent |
|---|---|
| Browse file to send | `CGUIDialogFileBrowser` |
| Incoming transfer request | `CGUIDialogYesNo` (accept/decline) |
| Transfer progress | `CGUIProgressControl` in a dialog |
| Transfer history | `CGUIListContainer` |

Received files are saved to a configurable local path (default: Kodi download folder). Received media files can optionally be added to the Kodi media library automatically.

---

## Configuration

| Setting | Description | Default |
|---|---|---|
| Minimum permission | Minimum peer permission to send files to you | Friend |
| Download path | Local folder for received files | `special://userdata/addon_data/nlc/downloads/` |
| Auto-add to library | Add received media to Kodi library | Disabled |
| Max concurrent transfers | Number of simultaneous file transfers | 2 |

---

## Related

- [File Share Server](file-share-server.md) — shared library accessible to all permitted peers
- [Messenger](messenger.md) — send small files/media within a messaging session
