/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include <cstdlib>

#include "CPUInfo.h"
#include "utils/log.h"
#include "utils/SysfsUtils.h"
//#include "utils/Temperature.h"
#include <string>
#include <string.h>

#if defined(TARGET_DARWIN)
#include <sys/types.h>
#include <sys/sysctl.h>
#if defined(__ppc__) || defined (TARGET_DARWIN_IOS)
#include <mach-o/arch.h>
#endif // defined(__ppc__) || defined (TARGET_DARWIN_IOS)
#ifdef TARGET_DARWIN_OSX
#include "platform/darwin/osx/smc.h"
#endif
#include "platform/linux/LinuxResourceCounter.h"
#endif

#if defined(TARGET_FREEBSD)
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/resource.h>
#endif

#if defined(TARGET_LINUX) && defined(HAS_NEON) && !defined(TARGET_ANDROID)
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>
#include <linux/auxvec.h>
#include <asm/hwcap.h>
#endif

#if defined(TARGET_ANDROID)
#include "platform/android/activity/AndroidFeatures.h"
#endif

#ifdef TARGET_WINDOWS
#include "platform/win32/CharsetConverter.h"
#include <algorithm>
#include <intrin.h>
#include <Pdh.h>
#include <PdhMsg.h>

#ifdef TARGET_WINDOWS_DESKTOP
#pragma comment(lib, "Pdh.lib")
#endif

#ifdef TARGET_WINDOWS_STORE
#include <winrt/Windows.Foundation.Metadata.h>
#include <winrt/Windows.System.Diagnostics.h>
#endif

// Defines to help with calls to CPUID
#define CPUID_INFOTYPE_STANDARD 0x00000001
#define CPUID_INFOTYPE_EXTENDED 0x80000001

// Standard Features
// Bitmasks for the values returned by a call to cpuid with eax=0x00000001
#define CPUID_00000001_ECX_SSE3  (1<<0)
#define CPUID_00000001_ECX_SSSE3 (1<<9)
#define CPUID_00000001_ECX_SSE4  (1<<19)
#define CPUID_00000001_ECX_SSE42 (1<<20)

#define CPUID_00000001_EDX_MMX   (1<<23)
#define CPUID_00000001_EDX_SSE   (1<<25)
#define CPUID_00000001_EDX_SSE2  (1<<26)

// Extended Features
// Bitmasks for the values returned by a call to cpuid with eax=0x80000001
#define CPUID_80000001_EDX_MMX2     (1<<22)
#define CPUID_80000001_EDX_MMX      (1<<23)
#define CPUID_80000001_EDX_3DNOWEXT (1<<30)
#define CPUID_80000001_EDX_3DNOW    (1<<31)


// Help with the __cpuid intrinsic of MSVC
#define CPUINFO_EAX 0
#define CPUINFO_EBX 1
#define CPUINFO_ECX 2
#define CPUINFO_EDX 3

#endif

#ifdef TARGET_POSIX
#include "ServiceBroker.h"
#include "settings/AdvancedSettings.h"
#include "settings/SettingsComponent.h"
#endif

#include "utils/StringUtils.h"

std::shared_ptr<CCPUInfo> CCPUInfo::GetCPUInfo()
{
    return std::make_shared<CCPUInfo>();
}

int CCPUInfo::GetUsedPercentage()
{
    return 0;
}

float CCPUInfo::GetCPUFrequency()
{
    return 0;
}

//bool CCPUInfo::GetTemperature( CTemperature& temperature )
//{
//    temperature = CTemperature::CreateFromFahrenheit(120);
//    return false;
//}

bool CCPUInfo::HasCoreId(int coreId) const
{
  for (const auto& core : m_cores)
  {
    if (core.m_id == coreId)
  return true;
}

  return false;
}

const CoreInfo CCPUInfo::GetCoreInfo(int coreId)
{
  CoreInfo coreInfo;

  for (auto& core : m_cores)
{
    if (core.m_id == coreId)
      coreInfo = core;
        }

  return coreInfo;
  }

std::string CCPUInfo::GetCoresUsageString()
  {
  std::string strCores;

  if (SupportsCPUUsage())
    {
    GetUsedPercentage(); // must call it to recalculate pct values

  if (!m_cores.empty())
  {
      for (const auto& core : m_cores)
    {
      if (!strCores.empty())
        strCores += ' ';
      if (core.m_usagePercent < 10.0)
        strCores += StringUtils::Format("#%d: %1.1f%%", core.m_id, core.m_usagePercent);
      else
        strCores += StringUtils::Format("#%d: %3.0f%%", core.m_id, core.m_usagePercent);
    }
  }
  else
  {
    strCores += StringUtils::Format("%3.0f%%", double(m_lastUsedPercentage));
  }
}

  return strCores;
  }
