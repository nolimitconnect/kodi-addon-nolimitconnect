---
title: Developer Docs
---

# Developer Documentation

Welcome to the kodi-addon-nolimitconnect developer documentation. This section covers the add-on's internal architecture, how it integrates with both Kodi and the NLC engine, and how to build and contribute to the project.

---

## Contents

- [Architecture](architecture.md) — engine integration, IToGui/IFromGui bridge, threading model
- [Building](building.md) — prerequisites, CMake configuration, build targets
- [Dialogs](dialogs.md) — Kodi dialog specifications for setup, permissions, and host join
- [Implementation Progress](progress.md) — long-running milestone tracker and task status

---

## Codebase Overview

```
kodi-addon-nolimitconnect/
├── CMakeLists.txt              # Top-level build definition
├── addon.xml                   # Kodi add-on metadata
├── src/
│   ├── addon/
│   │   ├── NlcAddon.cpp        # Add-on entry point (IAddonInstance)
│   │   ├── NlcAddon.h
│   │   ├── ToGuiBridge.cpp     # IToGui implementation → Kodi callbacks
│   │   ├── ToGuiBridge.h
│   │   ├── FromGuiBridge.cpp   # IFromGui calls into NLC engine
│   │   └── FromGuiBridge.h
│   ├── gui/
│   │   ├── WindowMain.cpp      # Main NLC window
│   │   ├── DialogMessenger.cpp
│   │   ├── DialogVoicePhone.cpp
│   │   ├── DialogVideoChat.cpp
│   │   ├── DialogFileXfer.cpp
│   │   ├── DialogCamServer.cpp
│   │   ├── DialogFileShare.cpp
│   │   └── DialogPushToTalk.cpp
│   ├── audio/
│   │   ├── KodiAudioCapture.cpp  # Platform mic → NLC MediaProcessor
│   │   └── KodiAudioOutput.cpp   # NLC mixer → Kodi IAEStream
│   └── stream/
│       └── NlcStreamFile.cpp     # Virtual file I/O for FileShareServer streaming
├── resources/
│   └── skins/
│       └── Default/
│           └── 1080i/
│               ├── NlcMain.xml
│               ├── NlcMessenger.xml
│               ├── NlcVoicePhone.xml
│               ├── NlcVideoChat.xml
│               └── ...
├── docs/                       # This documentation
└── README.md
```

---

## Key Dependencies

| Dependency | Purpose | Source |
|---|---|---|
| Kodi binary add-on API | Add-on lifecycle, GUI, audio | kodi-source |
| libptopengine | NLC networking + plugin engine | NoLimitConnect repo |
| libOpus | Audio codec | system / bundled |
| FFmpeg | Media encode/decode | system / bundled |
| OpenSSL or mbedTLS | Encrypted transport | system |
| WebRTC AEC | Acoustic echo cancellation | bundled in NLC |
| RNNoise | Noise suppression | bundled in NLC |

---

## Contributing

1. Fork the repository on GitHub
2. Create a feature branch: `git checkout -b feature/my-feature`
3. Follow the existing code style (clang-format, 4-space indent)
4. Submit a pull request with a clear description of the change

For large changes, open an issue first to discuss the approach.
