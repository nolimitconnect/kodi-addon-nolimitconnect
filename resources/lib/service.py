#!/usr/bin/env python3
import ctypes
import os
import traceback

import xbmc
import xbmcaddon
import xbmcvfs

ADDON = xbmcaddon.Addon()
ADDON_ID = ADDON.getAddonInfo("id")


def log(message, level=xbmc.LOGINFO):
    xbmc.log(f"[{ADDON_ID}] {message}", level)


def try_load_native_library():
    addon_path = xbmcvfs.translatePath(ADDON.getAddonInfo("path"))
    lib_name = "libkodi-addon-nolimitconnect.so"
    lib_path = os.path.join(addon_path, lib_name)

    if not os.path.exists(lib_path):
        log(f"Native library missing: {lib_path}", xbmc.LOGWARNING)
        return

    try:
        ctypes.CDLL(lib_path)
        log(f"Native library load probe succeeded: {lib_path}")
    except OSError as err:
        log(f"Native library load probe failed: {err}", xbmc.LOGERROR)


def run_service():
    monitor = xbmc.Monitor()
    try:
        # getSetting is supported across Kodi Python API variants.
        display_name = ADDON.getSetting("display_name")
    except Exception as err:
        log(f"Unable to read display_name setting: {err}", xbmc.LOGWARNING)
        display_name = ""

    log("NoLimitConnect addon service started (python bootstrap)")
    log(f"Configured display_name='{display_name}'")
    try_load_native_library()

    while not monitor.abortRequested():
        if monitor.waitForAbort(10):
            break

    log("NoLimitConnect addon service stopped")


if __name__ == "__main__":
    try:
        run_service()
    except Exception:
        log("Unhandled exception in NoLimitConnect service bootstrap", xbmc.LOGERROR)
        log(traceback.format_exc(), xbmc.LOGERROR)
