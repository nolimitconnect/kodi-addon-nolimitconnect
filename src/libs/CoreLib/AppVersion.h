#pragma once

#define APP_NAME				"kodi-addon-nolimitconnect"
#define APP_TITLE				"kodi-addon-nolimitconnect"
#define APP_URL					"https://nolimitconnect.org"
#define APP_DOMAIN_NAME			"nolimitconnect"
#define APP_PACKAGE				"org.nolimitconnect.kodi-addon-nolimitconnect"

// NOTE: if you change the version then change it in 
// cmake/version.cmake and android/AndroidManifest.xml also
#define APP_VERSION				"1.1.2"
#define APP_MAJOR_VERSION		1
#define APP_MINOR_VERSION		1
#define APP_PATCH_VERSION		2
/* Version number: (major<<16) + (minor<<8) + patch */
#define APP_VERSION_BINARY		((APP_MAJOR_VERSION<<16) + (APP_MINOR_VERSION<<8) + APP_PATCH_VERSION)

#define PACKAGE_STRING			"kodi-addon-nolimitconnect 1.1.2"

/* redefine for different library needs */
#define PACKAGE_BUGREPORT		APP_URL
#define PACKAGE_NAME			APP_NAME
#define PACKAGE_TARNAME			APP_PACKAGE
#define PACKAGE_VERSION			APP_VERSION
#define BUILD_REVISION			APP_VERSION

/* The time this package was configured for a build */
#define BUILD_TIMESTAMP			"2026-06-07T06:42+0000"

