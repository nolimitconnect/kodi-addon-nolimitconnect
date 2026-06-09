#!/usr/bin/env python3
import ctypes
import os
import threading
import traceback

import xbmc
import xbmcaddon
import xbmcgui
import xbmcvfs

ADDON = xbmcaddon.Addon()
ADDON_ID = ADDON.getAddonInfo("id")
ADDON_NAME = ADDON.getAddonInfo("name")

HOME_WINDOW_ID = 10000
COMMAND_PROPERTY = "nlc.command"
STATUS_PROPERTY = "nlc.service_status"

STATUS_IDLE = "idle"
STATUS_STARTING = "starting"
STATUS_RUNNING = "running"
STATUS_SHUTTING_DOWN = "shutting_down"

FILTER_ALL = "All"
FILTER_INFO = "Info+"
FILTER_WARN = "Warn+"
FILTER_ERROR = "Error"

CONTROL_POWER = 101
CONTROL_BACK = 201
CONTROL_CLEAR = 103
CONTROL_LOG = 108
CONTROL_FILTER_ALL = 104
CONTROL_FILTER_INFO = 105
CONTROL_FILTER_WARN = 106
CONTROL_FILTER_ERROR = 107

_NATIVE_LIB = None


def log(message, level=xbmc.LOGINFO):
    xbmc.log(f"[{ADDON_ID}] {message}", level)


def notify(message):
    safe = message.replace(",", " ")
    xbmc.executebuiltin(f"Notification({ADDON_NAME},{safe},3000)")


def home_window():
    return xbmcgui.Window(HOME_WINDOW_ID)


def get_home_property(name):
    try:
        return home_window().getProperty(name).strip()
    except Exception:
        return ""


def set_home_property(name, value):
    try:
        home_window().setProperty(name, str(value))
    except Exception:
        pass


def read_recent_debug_lines(max_lines=350):
    log_path = xbmcvfs.translatePath("special://logpath/kodi.log")
    if not log_path or not os.path.exists(log_path):
        return ["Kodi log file not found yet"]

    try:
        with open(log_path, "r", encoding="utf-8", errors="replace") as log_file:
            lines = log_file.readlines()
    except Exception as ex:
        return [f"Unable to read Kodi log: {ex}"]

    interesting = []
    for line in lines:
        lowered = line.lower()
        if (
            "[service.binary.nolimitconnect]" in lowered
            or "[vxdebug]" in lowered
            or "nlcrunnowtrigger" in lowered
            or "ptopengine" in lowered
        ):
            interesting.append(line.rstrip("\n"))

    if not interesting:
        interesting = ["No NLC/Vx debug lines found yet"]

    return interesting[-max_lines:]


def filter_log_lines(lines, selected_filter):
    if selected_filter == FILTER_ALL:
        return lines

    filtered = []
    for line in lines:
        lowered = line.lower()
        if selected_filter == FILTER_ERROR and "error" in lowered:
            filtered.append(line)
        elif selected_filter == FILTER_WARN and (
            "warning" in lowered or "error" in lowered
        ):
            filtered.append(line)
        elif selected_filter == FILTER_INFO and (
            "info" in lowered or "warning" in lowered or "error" in lowered
        ):
            filtered.append(line)

    return filtered or [f"No lines for filter: {selected_filter}"]


def get_native_library():
    global _NATIVE_LIB
    if _NATIVE_LIB is not None:
        return _NATIVE_LIB

    addon_path = xbmcvfs.translatePath(ADDON.getAddonInfo("path"))
    library_path = os.path.join(addon_path, "libkodi-addon-nolimitconnect.so")
    if not os.path.exists(library_path):
        return None

    try:
        rtld_global = getattr(os, "RTLD_GLOBAL", 0)
        rtld_lazy = getattr(os, "RTLD_LAZY", 1)
        lib = ctypes.CDLL(library_path, mode=rtld_global | rtld_lazy)
        lib.NlcRunAppShutdown.argtypes = []
        lib.NlcRunAppShutdown.restype = ctypes.c_int
        _NATIVE_LIB = lib
        return _NATIVE_LIB
    except Exception as ex:
        log(f"Unable to load native library for shutdown: {ex}", xbmc.LOGERROR)
        return None


def trigger_native_shutdown():
    lib = get_native_library()
    if lib is None:
        return False

    try:
        result = lib.NlcRunAppShutdown()
        log(f"NlcRunAppShutdown returned {result}", xbmc.LOGINFO)
        return result >= 0
    except Exception as ex:
        log(f"Native shutdown raised exception: {ex}", xbmc.LOGERROR)
        return False


class NlcMainWindow(xbmcgui.WindowXMLDialog):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.monitor = xbmc.Monitor()
        self._closed = False
        self._refresh_thread = None
        self._start_sent = False
        self.log_filter = FILTER_ALL
        self.log_cleared = False
        self.back_actions = {
            int(getattr(xbmcgui, "ACTION_NAV_BACK", 92)),
            int(getattr(xbmcgui, "ACTION_PREVIOUS_MENU", 10)),
            int(getattr(xbmcgui, "ACTION_BACKSPACE", 110)),
        }

    def onInit(self):
        self.setProperty("nlc.debug.filter", self.log_filter)
        self.refresh_status()
        self.refresh_log()
        self.send_start_command_once()
        self._refresh_thread = threading.Thread(target=self._refresh_loop, daemon=True)
        self._refresh_thread.start()

    def send_start_command_once(self):
        if self._start_sent:
            return
        self._start_sent = True
        set_home_property(COMMAND_PROPERTY, "start_engine")
        set_home_property(STATUS_PROPERTY, STATUS_STARTING)
        self.refresh_status("Starting engine")
        notify("Connecting in background")

    def onClick(self, control_id):
        if control_id == CONTROL_POWER:
            self.request_shutdown()
            return

        if control_id == CONTROL_BACK:
            self.close_window()
            return

        if control_id == CONTROL_LOG:
            self.open_log_dialog()
            return

        if control_id == CONTROL_CLEAR:
            self.log_cleared = True
            self.setProperty(
                "nlc.debug.log",
                "Log view cleared. Kodi log file is unchanged.",
            )
            return

        if control_id == CONTROL_FILTER_ALL:
            self.log_filter = FILTER_ALL
        elif control_id == CONTROL_FILTER_INFO:
            self.log_filter = FILTER_INFO
        elif control_id == CONTROL_FILTER_WARN:
            self.log_filter = FILTER_WARN
        elif control_id == CONTROL_FILTER_ERROR:
            self.log_filter = FILTER_ERROR
        else:
            return

        self.setProperty("nlc.debug.filter", self.log_filter)
        self.refresh_log()

    def onAction(self, action):
        if int(action.getId()) in self.back_actions:
            self.close_window()

    def request_shutdown(self):
        set_home_property(COMMAND_PROPERTY, "shutdown_app")
        set_home_property(STATUS_PROPERTY, STATUS_SHUTTING_DOWN)
        self.refresh_status("Shutting down")
        notify("Shutting down NoLimitConnect")
        trigger_native_shutdown()
        self.close_window()

    def open_log_dialog(self):
        addon_path = xbmcvfs.translatePath(ADDON.getAddonInfo("path"))
        dialog = NlcLogWindow("nlc_log_dialog.xml", addon_path, "Default", "1080i")
        try:
            dialog.doModal()
        finally:
            del dialog

    def close_window(self):
        self._closed = True
        self.close()

    def _refresh_loop(self):
        while not self._closed and not self.monitor.abortRequested():
            if self.monitor.waitForAbort(1):
                return
            self.refresh_status()
            if not self.log_cleared:
                self.refresh_log()

    def refresh_status(self, prefix=""):
        status = get_home_property(STATUS_PROPERTY) or STATUS_IDLE
        if prefix:
            text = f"{prefix} | Service status: {status}"
        else:
            text = f"Service status: {status}"
        self.setProperty("nlc.main.status", text)

    def refresh_log(self):
        lines = read_recent_debug_lines()
        lines = filter_log_lines(lines, self.log_filter)
        self.setProperty("nlc.debug.log", "\n".join(lines))


class NlcLogWindow(xbmcgui.WindowXMLDialog):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.monitor = xbmc.Monitor()
        self._closed = False
        self._refresh_thread = None

    def onInit(self):
        self.refresh_log()
        self._refresh_thread = threading.Thread(target=self._refresh_loop, daemon=True)
        self._refresh_thread.start()

    def onClick(self, control_id):
        if control_id == 101:
            self.close_window()

    def onAction(self, action):
        if int(action.getId()) in {
            int(getattr(xbmcgui, "ACTION_NAV_BACK", 92)),
            int(getattr(xbmcgui, "ACTION_PREVIOUS_MENU", 10)),
            int(getattr(xbmcgui, "ACTION_BACKSPACE", 110)),
        }:
            self.close_window()

    def _refresh_loop(self):
        while not self._closed and not self.monitor.abortRequested():
            if self.monitor.waitForAbort(1):
                return
            self.refresh_log()

    def refresh_log(self):
        lines = filter_log_lines(read_recent_debug_lines(), FILTER_ALL)
        self.setProperty("nlc.log.text", "\n".join(lines))

    def close_window(self):
        self._closed = True
        self.close()


def run_launcher():
    addon_path = xbmcvfs.translatePath(ADDON.getAddonInfo("path"))
    window = NlcMainWindow("nlc_main_window.xml", addon_path, "Default", "1080i")

    try:
        window.doModal()
    finally:
        del window


if __name__ == "__main__":
    try:
        run_launcher()
    except Exception:
        log("Unhandled exception in addon launcher", xbmc.LOGERROR)
        log(traceback.format_exc(), xbmc.LOGERROR)
        notify("Launcher failed; check Kodi log")
