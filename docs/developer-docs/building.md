---
title: Building
---

# Building the Add-on

## Prerequisites

| Requirement | Version | Notes |
|---|---|---|
| Kodi source | 21 (Omega) | Checked out and built |
| CMake | 3.20+ | |
| GCC | 11+ | Or Clang 14+ |
| libOpus | any recent | `apt install libopus-dev` |
| FFmpeg | 6.x | `apt install libavcodec-dev libavformat-dev libswresample-dev` |
| OpenSSL | 3.x | `apt install libssl-dev` |

### Optional (for video plugins)

| Requirement | Notes |
|---|---|
| V4L2 headers | `apt install libv4l-dev` (camera capture on Linux) |
| libjpeg-turbo | `apt install libjpeg-turbo8-dev` (MJPEG encoding) |

---

## Step 1 — Clone

```bash
git clone https://github.com/nolimitconnect/kodi-addon-nolimitconnect.git
cd kodi-addon-nolimitconnect
```

---

## Step 2 — Build Kodi (if not already built)

The add-on needs a Kodi binary add-on development tree. If you haven't built Kodi yet:

```bash
# Clone Kodi
git clone https://github.com/xbmc/xbmc.git /path/to/kodi-source
cd /path/to/kodi-source

# Configure (headless / depends build)
mkdir build && cd build
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DENABLE_INTERNAL_FLATBUFFERS=ON

# Build (this takes a while)
cmake --build . -j$(nproc)
```

For a full Kodi build guide see the [Kodi wiki](https://kodi.wiki/view/HOW-TO:Compile_Kodi_for_Linux).

---

## Step 3 — Clone NoLimitConnect (engine library)

The add-on depends on `libptopengine` from the NLC repository:

```bash
git clone https://github.com/nolimitconnect/NoLimitConnect.git /path/to/NoLimitConnect
```

The CMake build will look for the NLC source tree at the path specified by `-DNLC_SOURCE_DIR`.

---

## Step 4 — Configure

```bash
cd /path/to/kodi-addon-nolimitconnect

cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DKODI_SOURCE_DIR=/path/to/kodi-source \
  -DKODI_BUILD_DIR=/path/to/kodi-source/build \
  -DNLC_SOURCE_DIR=/path/to/NoLimitConnect
```

### CMake Options

| Option | Default | Description |
|---|---|---|
| `KODI_SOURCE_DIR` | (required) | Path to the Kodi source tree |
| `KODI_BUILD_DIR` | (required) | Path to the Kodi build directory |
| `NLC_SOURCE_DIR` | (required) | Path to the NoLimitConnect source tree |
| `NLC_NETWORK_HOST_URL` | `nolimitconnect.net` | Default network host URL |
| `ENABLE_VIDEO_CHAT` | ON | Build VideoChat plugin |
| `ENABLE_CAM_SERVER` | ON | Build CamServer plugin |
| `CMAKE_INSTALL_PREFIX` | `/usr` | Install prefix |

### Kodi Source Auto-Detection

If `KODI_SOURCE_DIR` is not passed explicitly, the add-on CMake will try these paths in order:

1. `$KODI_SOURCE_DIR` environment variable
2. `../kodi-source` relative to this repository
3. `$HOME/kodi-source`
4. `/home/$USER/kodi-source`

Recommended explicit configure (most reliable):

```bash
cmake -B build -DKODI_SOURCE_DIR=/home/<user>/kodi-source
```

---

## Step 5 — Build

```bash
cmake --build build -j$(nproc)
```

The output is a shared library:

```
build/libkodi-addon-nolimitconnect.so
```

---

## Step 6 — Install

```bash
# Install add-on files to Kodi's add-on directory
cmake --install build

# Or manually copy
mkdir -p ~/.kodi/addons/service.binary.nolimitconnect
cp build/libkodi-addon-nolimitconnect.so ~/.kodi/addons/service.binary.nolimitconnect/
cp addon.xml ~/.kodi/addons/service.binary.nolimitconnect/
cp -r resources/ ~/.kodi/addons/service.binary.nolimitconnect/
```

---

## Step 7 — Enable in Kodi

1. Open Kodi
2. Go to **Settings → Add-ons → My add-ons → Services**
3. Find **NoLimitConnect** and click **Enable**
4. On first launch you will be prompted for a display name

---

## Development Builds (Debug)

```bash
cmake -B build-debug \
  -DCMAKE_BUILD_TYPE=Debug \
  -DKODI_SOURCE_DIR=/path/to/kodi-source \
  -DKODI_BUILD_DIR=/path/to/kodi-source/build \
  -DNLC_SOURCE_DIR=/path/to/NoLimitConnect

cmake --build build-debug -j$(nproc)
```

Debug builds include NLC engine logging. Log output goes to Kodi's log file (`~/.kodi/temp/kodi.log`).

---

## Troubleshooting

### `KODI_SOURCE_DIR` not found

Make sure you pass the correct absolute path and that the directory contains a `xbmc/` subfolder.

### Missing `libptopengine`

Ensure `NLC_SOURCE_DIR` points to a complete NoLimitConnect checkout. The CMake will attempt to build `libptopengine` as a subproject.

### Audio not working

Check that the user running Kodi has access to ALSA/PulseAudio devices. On some systems:

```bash
usermod -aG audio $USER
```

### Camera not detected

Verify V4L2 devices exist:

```bash
v4l2-ctl --list-devices
```

---

## Further Reading

- [Architecture](architecture.md) — how the add-on is structured internally
- [NoLimitConnect build docs](https://nolimitconnect.org/developer-docs/)
- [Kodi binary add-on development](https://kodi.wiki/view/Add-on_development)
