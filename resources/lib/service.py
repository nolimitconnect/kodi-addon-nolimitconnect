#!/usr/bin/env python3
import os
import random
import socket
import traceback

import xbmc
import xbmcaddon
import xbmcvfs

ADDON = xbmcaddon.Addon()
ADDON_ID = ADDON.getAddonInfo("id")
ADDON_NAME = ADDON.getAddonInfo("name")
PROFILE_DIR = xbmcvfs.translatePath(ADDON.getAddonInfo("profile"))

DEFAULT_MOOD_MESSAGE = "Let's Communicate!"
DEFAULT_NETWORK_HOST_URL = "ptop://nolimitconnect.net:45124"
DEFAULT_CONNECTION_TEST_URL = "ptop://nolimitconnect.xyz:45124"
DEFAULT_CONNECTION_MODE = "connection_test"


def _setting_keys(setting_id):
    return (
        setting_id,
        f"{ADDON_ID}.{setting_id}",
        f"general.{setting_id}",
        f"identity.{setting_id}",
        f"network.{setting_id}",
        f"startup.{setting_id}",
    )


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


def ensure_profile_dir():
    if PROFILE_DIR and not os.path.exists(PROFILE_DIR):
        os.makedirs(PROFILE_DIR, exist_ok=True)


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

    if get_setting("connection_mode", "").strip() == "":
        set_setting("connection_mode", DEFAULT_CONNECTION_MODE)


def validate_configuration():
    errors = []
    warnings = []

    user_name = get_setting("user_name", "").strip()
    mood_message = get_setting("mood_message", "")
    listen_port = parse_port(get_setting("listen_port", ""))
    network_host_url = get_setting("network_host_url", "").strip()
    connection_test_url = get_setting("connection_test_url", "").strip()
    connection_mode = get_setting("connection_mode", DEFAULT_CONNECTION_MODE).strip()
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

    if connection_mode not in ("connection_test", "direct"):
        errors.append("Connection mode must be connection_test or direct")

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
    else:
        notify(ADDON_NAME, "Startup config logged")

    set_bool_setting("startup_run_now", False)


def run_service():
    monitor = xbmc.Monitor()
    ensure_profile_dir()
    ensure_default_configuration()

    log("NoLimitConnect addon service started (python bootstrap)")
    log("Waiting for startup run trigger")

    previous_run_trigger = get_bool_setting("startup_run_now")

    while not monitor.abortRequested():
        if monitor.waitForAbort(1):
            break

        current_run_trigger = get_bool_setting("startup_run_now")
        if current_run_trigger and not previous_run_trigger:
            handle_startup_trigger()
        previous_run_trigger = current_run_trigger

    log("NoLimitConnect addon service stopped")


if __name__ == "__main__":
    try:
        run_service()
    except Exception:
        log("Unhandled exception in NoLimitConnect service bootstrap", xbmc.LOGERROR)
        log(traceback.format_exc(), xbmc.LOGERROR)
