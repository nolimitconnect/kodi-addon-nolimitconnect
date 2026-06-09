#!/usr/bin/env python3
import os
import random
import sys
import socket
import traceback
import ctypes
import xml.etree.ElementTree as ET

import xbmc
import xbmcaddon
import xbmcgui
import xbmcvfs

ADDON = xbmcaddon.Addon()
ADDON_ID = ADDON.getAddonInfo("id")
ADDON_NAME = ADDON.getAddonInfo("name")
PROFILE_DIR = xbmcvfs.translatePath(ADDON.getAddonInfo("profile"))
USERDATA_SETTINGS_PATH = xbmcvfs.translatePath(
    os.path.join(ADDON.getAddonInfo("profile"), "settings.xml")
)

DEFAULT_MOOD_MESSAGE = "Let's Communicate!"
DEFAULT_NETWORK_HOST_URL = "ptop://nolimitconnect.net:45124"
DEFAULT_CONNECTION_TEST_URL = "ptop://nolimitconnect.xyz:45124"
DEFAULT_CONNECTION_MODE = "0"

HOME_WINDOW_ID = 10000
COMMAND_PROPERTY = "nlc.command"
STATUS_PROPERTY = "nlc.service_status"

STATUS_IDLE = "idle"
STATUS_STARTING = "starting"
STATUS_RUNNING = "running"
STATUS_ERROR = "error"
STATUS_SHUTTING_DOWN = "shutting_down"

_NATIVE_TRIGGER_LIB = None
_NATIVE_TRIGGER_LOAD_ERROR = ""
_NATIVE_TRIGGER_ENABLED = os.environ.get("NLC_ENABLE_NATIVE_TRIGGER", "").strip() == "1"
_NATIVE_TRIGGER_STAGE_RAW = os.environ.get("NLC_NATIVE_TRIGGER_STAGE", "").strip()


def native_stage_from_env():
    if _NATIVE_TRIGGER_STAGE_RAW == "":
        return None

    try:
        return int(_NATIVE_TRIGGER_STAGE_RAW)
    except ValueError:
        return None


def format_native_result(code, detail):
    if detail:
        return f"code={code} ({detail})"
    return f"code={code}"


def get_native_trigger_library():
    global _NATIVE_TRIGGER_LIB
    global _NATIVE_TRIGGER_LOAD_ERROR
    if _NATIVE_TRIGGER_LIB is not None:
        return _NATIVE_TRIGGER_LIB

    addon_path = xbmcvfs.translatePath(ADDON.getAddonInfo("path"))
    library_path = os.path.join(addon_path, "libkodi-addon-nolimitconnect.so")
    log(f"Native trigger load probe: {library_path}", xbmc.LOGINFO)
    if not os.path.exists(library_path):
        _NATIVE_TRIGGER_LOAD_ERROR = f"library missing at {library_path}"
        log(f"Native trigger library missing: {library_path}", xbmc.LOGWARNING)
        return None

    try:
        rtld_global = getattr(os, "RTLD_GLOBAL", 0)
        rtld_lazy = getattr(os, "RTLD_LAZY", 1)
        lib = ctypes.CDLL(library_path, mode=rtld_global | rtld_lazy)
        lib.NlcRunNowTrigger.argtypes = [
            ctypes.c_char_p,
            ctypes.c_char_p,
            ctypes.c_char_p,
            ctypes.c_char_p,
            ctypes.c_char_p,
        ]
        lib.NlcRunNowTrigger.restype = ctypes.c_int
        lib.NlcRunAppShutdown.argtypes = []
        lib.NlcRunAppShutdown.restype = ctypes.c_int
        _NATIVE_TRIGGER_LIB = lib
        _NATIVE_TRIGGER_LOAD_ERROR = ""
        log("Native trigger library load probe succeeded", xbmc.LOGINFO)
        return _NATIVE_TRIGGER_LIB
    except Exception as ex:
        _NATIVE_TRIGGER_LOAD_ERROR = f"load error: {ex}"
        log(f"Failed to load native trigger library: {ex}", xbmc.LOGERROR)
        return None


def trigger_native_startup(config):
    if not _NATIVE_TRIGGER_ENABLED:
        return {
            "ok": False,
            "code": 0,
            "detail": "native trigger skipped (disabled)",
            "skipped": True,
        }

    configured_stage = native_stage_from_env()
    if configured_stage is None:
        return {
            "ok": False,
            "code": 0,
            "detail": "native trigger skipped (stage not configured)",
            "skipped": True,
        }

    lib = get_native_trigger_library()
    if lib is None:
        detail = _NATIVE_TRIGGER_LOAD_ERROR or "library unavailable"
        return {
            "ok": False,
            "code": -998,
            "detail": detail,
        }

    addon_path = xbmcvfs.translatePath(ADDON.getAddonInfo("path"))
    profile_dir = PROFILE_DIR
    mood_message = config["mood_message"] or ""
    preferred_host = config["network_host_url"] or ""

    log(f"Invoking NlcRunNowTrigger (stage={configured_stage})", xbmc.LOGINFO)

    try:
        result = lib.NlcRunNowTrigger(
            addon_path.encode("utf-8"),
            profile_dir.encode("utf-8"),
            config["user_name"].encode("utf-8"),
            mood_message.encode("utf-8"),
            preferred_host.encode("utf-8"),
        )
    except Exception as ex:
        log(f"Native startup trigger raised exception: {ex}", xbmc.LOGERROR)
        return {
            "ok": False,
            "code": -999,
            "detail": "ctypes exception",
        }

    log(f"NlcRunNowTrigger returned code {result}", xbmc.LOGINFO)

    native_details = {
        1: "startup/login dispatched",
        0: "preflight/partial stage (Python fallback)",
        -996: "native stage not set",
        -997: "native trigger disabled",
        -1: "invalid arguments",
        -2: "native exception",
    }
    detail = native_details.get(result, "unknown result")

    if result <= 0:
        log(
            f"Native startup trigger failed with {format_native_result(result, detail)}",
            xbmc.LOGERROR,
        )
        return {
            "ok": False,
            "code": result,
            "detail": detail,
        }

    log("Native startup trigger completed", xbmc.LOGINFO)
    return {
        "ok": True,
        "code": result,
        "detail": detail,
    }


def trigger_native_shutdown():
    lib = get_native_trigger_library()
    if lib is None:
        detail = _NATIVE_TRIGGER_LOAD_ERROR or "library unavailable"
        return {
            "ok": False,
            "code": -998,
            "detail": detail,
        }

    try:
        result = lib.NlcRunAppShutdown()
    except Exception as ex:
        log(f"Native shutdown trigger raised exception: {ex}", xbmc.LOGERROR)
        return {
            "ok": False,
            "code": -999,
            "detail": "ctypes exception",
        }

    native_details = {
        1: "shutdown dispatched",
        0: "engine already stopped",
        -1: "native exception",
        -2: "native exception",
    }
    detail = native_details.get(result, "unknown result")

    if result < 0:
        return {
            "ok": False,
            "code": result,
            "detail": detail,
        }

    return {
        "ok": True,
        "code": result,
        "detail": detail,
    }


def _setting_keys(setting_id):
    return (setting_id,)


def _legacy_setting_keys(setting_id):
    legacy = {
        "user_name": ("display_name", f"{ADDON_ID}.display_name", "general.display_name"),
        "network_host_url": (
            "last_random_connect_host",
            f"{ADDON_ID}.last_random_connect_host",
            "general.last_random_connect_host",
        ),
        "connection_mode": (
            f"{ADDON_ID}.connection_mode",
            "general.connection_mode",
            "network.connection_mode",
        ),
    }
    return legacy.get(setting_id, ())


def log(message, level=xbmc.LOGINFO):
    xbmc.log(f"[{ADDON_ID}] {message}", level)


def get_setting(setting_id, default=""):
    for key in _setting_keys(setting_id):
        try:
            value = ADDON.getSetting(key)
        except Exception:
            continue
        if value != "":
            return value

    for key in _legacy_setting_keys(setting_id):
        try:
            value = ADDON.getSetting(key)
        except Exception:
            continue
        if value != "":
            return value

    return default


def set_setting(setting_id, value):
    text_value = str(value)
    for key in _setting_keys(setting_id):
        try:
            ADDON.setSetting(key, text_value)
            return
        except Exception:
            continue


def get_bool_setting(setting_id):
    return get_setting(setting_id, "false").lower() == "true"


def set_bool_setting(setting_id, value):
    set_setting(setting_id, "true" if value else "false")


def notify(heading, message):
    safe_heading = heading.replace(",", " ")
    safe_message = message.replace(",", " ")
    xbmc.executebuiltin(f"Notification({safe_heading},{safe_message},4000)")


def get_home_window():
    return xbmcgui.Window(HOME_WINDOW_ID)


def get_home_property(name):
    try:
        return get_home_window().getProperty(name).strip()
    except Exception:
        return ""


def set_home_property(name, value):
    try:
        get_home_window().setProperty(name, str(value))
    except Exception:
        pass


def clear_command_property():
    set_home_property(COMMAND_PROPERTY, "")


def set_service_status(status):
    set_home_property(STATUS_PROPERTY, status)


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


def ensure_profile_dir():
    if PROFILE_DIR and not os.path.exists(PROFILE_DIR):
        os.makedirs(PROFILE_DIR, exist_ok=True)


def migrate_legacy_action_settings():
    if not USERDATA_SETTINGS_PATH or not os.path.exists(USERDATA_SETTINGS_PATH):
        return

    try:
        tree = ET.parse(USERDATA_SETTINGS_PATH)
        root = tree.getroot()
    except Exception:
        return

    removed = False
    for setting in list(root.findall("setting")):
        if setting.get("id") == "startup_run_now":
            root.remove(setting)
            removed = True

    if removed:
        try:
            tree.write(USERDATA_SETTINGS_PATH, encoding="utf-8", xml_declaration=True)
            log("Removed legacy startup_run_now setting from userdata")
        except Exception:
            pass


def get_profile_username():
    for label in ("System.ProfileName", "System.UserName", "System.FriendlyName"):
        try:
            value = xbmc.getInfoLabel(label).strip()
        except Exception:
            value = ""
        if value:
            return value
    return ""


def utf8_char_len(value):
    return len(value)


def is_valid_username(value):
    return 4 <= utf8_char_len(value) <= 31


def is_valid_mood_message(value):
    return utf8_char_len(value) <= 31


def parse_port(value):
    try:
        return int(value)
    except (TypeError, ValueError):
        return None


def parse_connection_mode(value):
    text = str(value).strip().lower()
    if text in ("1", "direct"):
        return "direct"
    return "connection_test"


def is_port_free(port):
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
            sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 0)
            sock.bind(("", port))
        return True
    except OSError:
        return False


def find_random_tcp_port(min_port=10000, max_port=65535, attempts=64):
    for _ in range(attempts):
        candidate = random.randint(min_port, max_port)
        if is_port_free(candidate):
            return candidate
    return random.randint(min_port, max_port)


def ensure_default_configuration():
    profile_username = get_profile_username()
    current_username = get_setting("user_name", "").strip()
    if not is_valid_username(current_username) and is_valid_username(profile_username):
        set_setting("user_name", profile_username)
        current_username = profile_username

    if get_setting("mood_message", "").strip() == "":
        set_setting("mood_message", DEFAULT_MOOD_MESSAGE)

    current_port = parse_port(get_setting("listen_port", ""))
    if current_port is None or current_port < 10000 or not is_port_free(current_port):
        set_setting("listen_port", find_random_tcp_port())

    if get_setting("network_host_url", "").strip() == "":
        set_setting("network_host_url", DEFAULT_NETWORK_HOST_URL)

    if get_setting("connection_test_url", "").strip() == "":
        set_setting("connection_test_url", DEFAULT_CONNECTION_TEST_URL)

    current_mode = get_setting("connection_mode", "").strip().lower()
    if current_mode == "":
        set_setting("connection_mode", DEFAULT_CONNECTION_MODE)
    elif current_mode in ("direct", "connection_test"):
        set_setting("connection_mode", "1" if current_mode == "direct" else "0")


def validate_configuration():
    errors = []
    warnings = []

    user_name = get_setting("user_name", "").strip()
    mood_message = get_setting("mood_message", "")
    listen_port = parse_port(get_setting("listen_port", ""))
    network_host_url = get_setting("network_host_url", "").strip()
    connection_test_url = get_setting("connection_test_url", "").strip()
    connection_mode = parse_connection_mode(get_setting("connection_mode", DEFAULT_CONNECTION_MODE))
    external_ip = get_setting("external_ip", "").strip()

    if not is_valid_username(user_name):
        errors.append("User Name must be 4 to 31 UTF-8 characters")

    if not is_valid_mood_message(mood_message):
        errors.append("Mood Message must be 0 to 31 UTF-8 characters")

    if listen_port is None:
        errors.append("Listen Port must be numeric")
    elif listen_port < 10000:
        errors.append("Listen Port must be 10000 or higher")

    if not network_host_url:
        errors.append("Network Host Url is required")

    if not connection_test_url:
        errors.append("Connection Test Url is required")

    if connection_mode == "direct" and not external_ip:
        warnings.append("External IP is empty while direct-connect mode is selected")

    return {
        "errors": errors,
        "warnings": warnings,
        "user_name": user_name,
        "mood_message": mood_message,
        "listen_port": listen_port,
        "network_host_url": network_host_url,
        "connection_test_url": connection_test_url,
        "connection_mode": connection_mode,
        "external_ip": external_ip,
    }


def print_configuration_summary(config):
    log("Startup configuration requested", xbmc.LOGINFO)
    log(f"User Name: {config['user_name']}")
    log(f"Mood Message: {config['mood_message']}")
    log(f"Listen Port: {config['listen_port']}")
    log(f"Network Host Url: {config['network_host_url']}")
    log(f"Connection Test Url: {config['connection_test_url']}")
    log(f"Connection Mode: {config['connection_mode']}")
    log(f"External IP: {config['external_ip'] or '<empty>'}")


def perform_startup_login(config):
    log("Startup login requested", xbmc.LOGINFO)
    log(f"Logging in as: {config['user_name']}")
    log(f"Joining preferred host: {config['network_host_url']}")

    if config["connection_mode"] == "direct":
        log(f"Direct-connect external IP: {config['external_ip'] or '<empty>'}")

    notify(ADDON_NAME, f"Startup login ready for {config['user_name']}")


def handle_startup_trigger():
    ensure_default_configuration()
    config = validate_configuration()

    for warning in config["warnings"]:
        log(f"Warning: {warning}", xbmc.LOGWARNING)

    for error in config["errors"]:
        log(f"Error: {error}", xbmc.LOGERROR)

    print_configuration_summary(config)

    if config["errors"]:
        notify(ADDON_NAME, "Startup config has validation errors")
        set_service_status(STATUS_ERROR)
        set_bool_setting("run_now", False)
        return False
    else:
        native_result = trigger_native_startup(config)
        if native_result["ok"]:
            status_text = (
                f"Native startup/login dispatched for {config['user_name']} "
                f"({format_native_result(native_result['code'], native_result['detail'])})"
            )
            notify(
                ADDON_NAME,
                status_text,
            )
            set_service_status(STATUS_RUNNING)
        else:
            if native_result.get("skipped"):
                log(
                    (
                        "Native trigger probe skipped "
                        f"({native_result.get('detail', 'no detail')}); "
                        "startup is handled by binary run_now setting callback"
                    ),
                    xbmc.LOGINFO,
                )
                status_text = "Native trigger skipped; running Python startup flow"
                notify(ADDON_NAME, status_text)
            else:
                status_text = (
                    "Native trigger failed "
                    f"({format_native_result(native_result['code'], native_result['detail'])}); "
                    "using Python fallback"
                )
                notify(
                    ADDON_NAME,
                    status_text,
                )

            if not native_result.get("skipped"):
                perform_startup_login(config)
            else:
                perform_startup_login(config)
            set_service_status(STATUS_RUNNING)

    set_bool_setting("run_now", False)
    return True


def handle_command(command):
    normalized = (command or "").strip().lower()
    if normalized == "":
        return

    if normalized in ("start_engine", "start", "run_startup"):
        current_status = get_home_property(STATUS_PROPERTY)
        if current_status == STATUS_RUNNING:
            log("Command start_engine ignored; engine already running", xbmc.LOGINFO)
            notify(ADDON_NAME, "NoLimitConnect is already running")
        else:
            log("Command start_engine received", xbmc.LOGINFO)
            set_service_status(STATUS_STARTING)
            handle_startup_trigger()
        clear_command_property()
        return

    if normalized in ("stop_engine", "stop"):
        # Native stop/disconnect API is not wired yet; mark idle for UI flow.
        log("Command stop_engine received; native stop is not implemented yet", xbmc.LOGWARNING)
        notify(ADDON_NAME, "Stop requested; full native disconnect is pending")
        set_service_status(STATUS_IDLE)
        clear_command_property()
        return

    if normalized in ("shutdown_app", "shutdown", "power_off"):
        log("Command shutdown_app received", xbmc.LOGINFO)
        set_service_status(STATUS_SHUTTING_DOWN)
        shutdown_result = trigger_native_shutdown()
        if shutdown_result["ok"]:
            log(
                f"Native shutdown completed ({format_native_result(shutdown_result['code'], shutdown_result['detail'])})",
                xbmc.LOGINFO,
            )
        else:
            log(
                f"Native shutdown failed ({format_native_result(shutdown_result['code'], shutdown_result['detail'])})",
                xbmc.LOGERROR,
            )
        set_service_status(STATUS_IDLE)
        clear_command_property()
        return "exit"

    log(f"Ignoring unknown command: {normalized}", xbmc.LOGWARNING)
    clear_command_property()


def run_service():
    monitor = xbmc.Monitor()
    ensure_profile_dir()
    migrate_legacy_action_settings()
    ensure_default_configuration()

    log("NoLimitConnect addon service started (python bootstrap)")
    log("Waiting for run_now trigger")
    if get_home_property(STATUS_PROPERTY) == "":
        set_service_status(STATUS_IDLE)
    clear_command_property()

    if get_bool_setting("run_now"):
        set_service_status(STATUS_STARTING)
        handle_startup_trigger()

    previous_run_trigger = get_bool_setting("run_now")

    while not monitor.abortRequested():
        if monitor.waitForAbort(1):
            break

        if handle_command(get_home_property(COMMAND_PROPERTY)) == "exit":
            break

        current_run_trigger = get_bool_setting("run_now")
        if current_run_trigger and not previous_run_trigger:
            set_service_status(STATUS_STARTING)
            handle_startup_trigger()
        previous_run_trigger = current_run_trigger

    log("NoLimitConnect addon service stopped")


def run_startup_once():
    ensure_profile_dir()
    migrate_legacy_action_settings()
    ensure_default_configuration()
    log("NoLimitConnect startup button invoked")
    handle_startup_trigger()


if __name__ == "__main__":
    try:
        if any(argument == "run_startup" for argument in sys.argv[1:]):
            run_startup_once()
        else:
            run_service()
    except Exception:
        log("Unhandled exception in NoLimitConnect service bootstrap", xbmc.LOGERROR)
        log(traceback.format_exc(), xbmc.LOGERROR)
