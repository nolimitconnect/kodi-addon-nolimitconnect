/*
 *  Copyright (C) 2020 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include <memory>

class LoggerInstance
{
public:
    static void Log(int loglevel, const char *format, ...);

    static void log(int loglevel, const char* message);

    static void warn(const char *format, ...);

    static void debug(const char *format, ...);

    static void error(const char *format, ...);
};

using Logger = std::shared_ptr<LoggerInstance>;
