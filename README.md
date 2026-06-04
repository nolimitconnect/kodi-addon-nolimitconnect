# NoLimitConnect — Kodi Binary Add-on

> **Status:** Early Development — not yet published

A Kodi binary add-on that brings the [NoLimitConnect](https://nolimitconnect.org) peer-to-peer communication platform directly into Kodi. Users can text, voice-chat, video-call, share files, and stream media — all without registration, ads, or central data collection — without ever leaving the Kodi interface.

---

## Features

Seven NoLimitConnect plugins are exposed as native Kodi services:

| Plugin | Slot | Description |
|---|---|---|
| **Messenger** | 8 | Text, voice, and video messaging (session-based) |
| **PushToTalk** | 9 | VOIP audio push-to-talk |
| **PersonFileXfer** | 10 | Direct person-to-person file transfer |
| **CamServer** | 11 | Webcam broadcast to the network |
| **FileShareServer** | 12 | Shared-file server and media stream source |
| **VideoChat** | 15 | Video chat with motion detection and recording |
| **VoicePhone** | 16 | VOIP audio-only phone call |

---

## How It Works

1. On first launch the add-on prompts for a display name (or reuses one if already configured).
2. The add-on connects to the `nolimitconnect.net` Random Connect host and announces the user to the network.
3. Peers discover each other through the network host directory; communication is direct (P2P) whenever possible, relayed otherwise.
4. All Qt-based UI from the original NoLimitConnect desktop app is replaced with Kodi's native GUI system.

No account creation, no email, no phone number required.

---

## Requirements

| Requirement | Notes |
|---|---|
| Kodi 21 (Omega) or later | Binary add-on ABI compatibility |
| C++17 compiler | GCC 11+ or Clang 14+ |
| CMake 3.20+ | Build system |
| libOpus | Audio codec (VOIP) |
| FFmpeg | Media pipeline |
| OpenSSL / mbedTLS | Encrypted transport |
| Linux x86-64, ARM64 | Primary targets; Windows/Android planned |

---

## Building

> Full build instructions: [docs/developer-docs/building.md](docs/developer-docs/building.md)

```bash
# Clone
git clone https://github.com/nolimitconnect/kodi-addon-nolimitconnect.git
cd kodi-addon-nolimitconnect

# Configure (point to your Kodi build tree)
cmake -B build \
      -DCMAKE_BUILD_TYPE=Release \
      -DKODI_SOURCE_DIR=/path/to/kodi-source \
      -DKODI_BUILD_DIR=/path/to/kodi/build

# Build
cmake --build build -j$(nproc)
```

---

## Documentation

Full documentation is available at **[nolimitconnect.github.io/kodi-addon-nolimitconnect](https://nolimitconnect.github.io/kodi-addon-nolimitconnect)** (served via MkDocs).

To run the docs locally:

```bash
pip install mkdocs-material
mkdocs serve
```

Then open <http://127.0.0.1:8000>.

---

## Related Projects

| Project | Description |
|---|---|
| [NoLimitConnect](https://github.com/nolimitconnect/NoLimitConnect) | The original Qt desktop application |
| [nolimitconnect.org](https://nolimitconnect.org) | Project website and documentation |

---

## License

This project is dual-licensed. Author's original code is released under the NoLimitConnect source license. Third-party components (Opus, FFmpeg, RNNoise, WebRTC AEC) retain their respective open-source licenses. See [LICENSE](LICENSE) for details.
