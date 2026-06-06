#!/usr/bin/env python3
import os
import traceback

import xbmc
import xbmcaddon
import xbmcvfs

ADDON = xbmcaddon.Addon()
ADDON_ID = ADDON.getAddonInfo("id")
ADDON_NAME = ADDON.getAddonInfo("name")


def _setting_keys(setting_id):
    return (
        setting_id,
        f"{ADDON_ID}.{setting_id}",
        f"general.{setting_id}",
    )


def log(message, level=xbmc.LOGINFO):
    xbmc.log(f"[{ADDON_ID}] {message}", level)


def get_setting(setting_id, default=""):
    for key in _setting_keys(setting_id):
        try:
            value = ADDON.getSetting(key)
        except Exception:
            continue
        if value:
            return value

    return default


def get_bool_setting(setting_id):
    return get_setting(setting_id, "false").lower() == "true"


def set_bool_setting(setting_id, value):
    bool_text = "true" if value else "false"
    for key in _setting_keys(setting_id):
        try:
            ADDON.setSetting(key, bool_text)
        except Exception:
            continue


def notify(heading, message):
    # Builtin Notification is the most reliable way to surface a toast across skins.
    xbmc.executebuiltin(f"Notification({heading},{message},4000)")


def run_service():
    monitor = xbmc.Monitor()
    try:
        # getSetting is supported across Kodi Python API variants.
        display_name = get_setting("display_name", "")
    except Exception as err:
        log(f"Unable to read display_name setting: {err}", xbmc.LOGWARNING)
        display_name = ""

    log("NoLimitConnect addon service started (python bootstrap)")
    log(f"Configured display_name='{display_name}'")
    addon_path = xbmcvfs.translatePath(ADDON.getAddonInfo("path"))
    lib_path = os.path.join(addon_path, "libkodi-addon-nolimitconnect.so")
    if os.path.exists(lib_path):
        log(f"Native library present: {lib_path}")
    else:
        log(f"Native library missing: {lib_path}", xbmc.LOGWARNING)

    previous_ui_active = get_bool_setting("debug_nlc_ui_active")
    if previous_ui_active:
        log("Debug UI simulation already active at startup")

    while not monitor.abortRequested():
        if monitor.waitForAbort(1):
            break

        current_ui_active = get_bool_setting("debug_nlc_ui_active")
        if current_ui_active != previous_ui_active:
            previous_ui_active = current_ui_active
            if current_ui_active:
                log("Debug: simulated NLC UI entry")
                notify(ADDON_NAME, "Debug: simulated NLC UI entry")
            else:
                log("Debug: simulated NLC UI exit")
                notify(ADDON_NAME, "Debug: simulated NLC UI exit")

        if get_bool_setting("debug_emit_test_message"):
            log("Debug: emitted test incoming message event")
            notify(ADDON_NAME, "New message received")
            set_bool_setting("debug_emit_test_message", False)

        if get_bool_setting("debug_emit_test_offer"):
            log("Debug: emitted test incoming offer event")
            notify(ADDON_NAME, "Incoming call or offer")
            set_bool_setting("debug_emit_test_offer", False)

    log("NoLimitConnect addon service stopped")


if __name__ == "__main__":
    try:
        run_service()
    except Exception:
        log("Unhandled exception in NoLimitConnect service bootstrap", xbmc.LOGERROR)
        log(traceback.format_exc(), xbmc.LOGERROR)
