---
title: File Share Server
---

# File Share Server

<span class="slot-badge">SLOT 12 — ePluginTypeFileShareServer</span>

## What It Does

FileShareServer exposes a local file library to permitted NLC peers. Peers can browse the shared library, download individual files, or stream media files directly into their Kodi player — without any file having to be fully downloaded first.

This plugin is also the primary source for remote media streaming within the add-on. When a peer streams a shared file, Kodi receives it through a virtual file I/O layer that presents the remote file as if it were local.

---

## Features

- **File library browsing** — peers can list and search shared files
- **Peer-to-peer download** — files transfer directly over the encrypted P2P connection
- **Live media streaming** — Kodi can play a remote shared file while it streams, with seek support
- **Selective sharing** — share specific folders; nothing outside the configured share roots is accessible
- **Permission-controlled** — only peers meeting the minimum permission threshold can access the library

---

## Streaming Architecture

The virtual stream path allows Kodi to seek and play remote files without downloading them first:

```
Kodi Player                  NLC VirtStreamMgr            Remote FileShareServer
     │                              │                              │
     │── open("remote://file") ────►│                              │
     │                              │── PktFileGetReq ────────────►│
     │                              │◄── PktFileGetReply ──────────│
     │                              │                              │
     │── read(offset, len) ────────►│── PktFileChunkReq ──────────►│
     │                              │◄── PktFileChunkReq (data) ───│
     │◄── bytes ───────────────────│                              │
     │                              │                              │
     │── seek(newPos) ─────────────►│── PktStreamCtrlReq(seek) ───►│
     │                              │◄── PktStreamCtrlReply ───────│
     │                              │                              │
```

The NLC `VirtStreamMgr` maintains a stream cache so the player can re-read already-received data without re-requesting it from the server. The server-side `FileInfoXferMgr` handles seek requests by repositioning its file handle and restarting chunk transmission from the new offset.

---

## Kodi UI Implementation

| Action | Kodi equivalent |
|---|---|
| Browse peer's shared library | `CGUIWindow` with `CGUIListContainer` |
| Play a remote media file | Kodi media player via `CFileItem` with virtual stream URL |
| Download a file | Progress dialog → save to download path |
| Configure share roots | Add-on settings screen |
| Add media to Kodi library | Kodi library scan on download completion |

### Integration with Kodi's Media Library

Shared files served over FileShareServer can be added to Kodi's media library as a network source. The virtual stream URL scheme (`nlcstream://`) is registered by the add-on so Kodi treats remote NLC files the same as local or HTTP sources.

---

## Configuration

| Setting | Description | Default |
|---|---|---|
| Minimum permission | Minimum peer permission to browse and download | Friend |
| Share roots | List of local folders to share | (none) |
| Max concurrent streams | Simultaneous streaming clients | 4 |
| Max concurrent downloads | Simultaneous download clients | 8 |
| Download path | Where downloaded files are saved | `special://userdata/addon_data/nlc/downloads/` |

---

## Security Notes

!!! warning "Share roots are accessible to all permitted peers"
    Any peer whose permission level meets the threshold can browse and download everything inside a configured share root. Only add folders you are comfortable sharing. The NLC engine does not permit path traversal outside configured roots.

---

## Related

- [Person File Xfer](person-file-xfer.md) — send a specific file to one peer directly
- [Cam Server](cam-server.md) — live webcam broadcast (cam recordings can be shared via FileShareServer)
- [NLC video stream docs](https://nolimitconnect.org/technical/README-VIDEO-STREAM/)
- [NLC media flow docs](https://nolimitconnect.org/technical/README-MEDIA-FLOW/)
