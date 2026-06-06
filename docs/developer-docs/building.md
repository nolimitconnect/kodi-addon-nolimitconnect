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

## Step 3 — Provide NLC libs tree

The add-on build expects an imported NoLimitConnect `libs` tree. Current default location in this repository is:

```bash
/path/to/kodi-addon-nolimitconnect/src/libs
```

If your libs tree is elsewhere, point CMake to it with `-DNLC_LIBS_DIR=/path/to/libs`.

---

## Step 4 — Configure

```bash
cd /path/to/kodi-addon-nolimitconnect

cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DKODI_SOURCE_DIR=/path/to/kodi-source \
  -DNLC_LIBS_DIR=/path/to/kodi-addon-nolimitconnect/src/libs
```

### CMake Options

| Option | Default | Description |
|---|---|---|
| `KODI_SOURCE_DIR` | (required) | Path to the Kodi source tree |
| `NLC_LIBS_DIR` | `src/libs` | Path to imported NoLimitConnect `libs` folder |
| `NLC_ENABLE_DEV_STUBS` | `ON` | Enable startup/dev scaffolding behavior while UI flows are incomplete |

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
  -DNLC_LIBS_DIR=/path/to/kodi-addon-nolimitconnect/src/libs \
  -DNLC_ENABLE_DEV_STUBS=ON

cmake --build build-debug -j$(nproc)
```

Debug builds include NLC engine logging. Log output goes to Kodi's log file (`~/.kodi/temp/kodi.log`).

---

## Troubleshooting

## Runtime Verification Hooks (Current Scaffold)

Until full Kodi window navigation is wired, use debug settings to validate runtime behavior:

1. Open add-on settings.
2. Toggle `Debug: Simulate NLC UI Active` ON.
3. Confirm logs show join activity only after this action.
4. Toggle `Debug: Simulate NLC UI Active` OFF.
5. Toggle `Debug: Emit Test Message Event` ON while UI is inactive.
6. Confirm a Kodi global toast appears (`New message received`).
7. Toggle `Debug: Emit Test Incoming Offer` ON while UI is inactive.
8. Confirm a Kodi warning toast appears (`Incoming call or offer`).

These hooks are temporary scaffolding to validate deferred login and out-of-plugin notifications.

### `KODI_SOURCE_DIR` not found

Make sure you pass the correct absolute path and that the directory contains a `xbmc/` subfolder.

### Missing NLC libs include paths

Ensure `NLC_LIBS_DIR` points to a complete `libs` tree containing at least `GuiInterface`, `CoreLib`, and `libptopengine`.

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
