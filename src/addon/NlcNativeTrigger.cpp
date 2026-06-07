#include <algorithm>
#include <cctype>
#include <cstdarg>
#include <cstdlib>
#include <cstdio>
#include <filesystem>
#include <string>

#include <kodi/AddonBase.h>

#include <PktLib/PktAnnounce.h>
#include <libptopengine/P2PEngine/P2PEngine.h>

namespace
{

std::string EnsureTrailingSlash(std::string value)
{
  if (!value.empty() && value.back() != '/')
  {
    value.push_back('/');
  }

  return value;
}

std::string TrimWhitespace(const std::string& value)
{
  const auto first = std::find_if_not(value.begin(), value.end(), [](unsigned char ch) {
    return std::isspace(ch) != 0;
  });

  if (first == value.end())
  {
    return {};
  }

  const auto last = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char ch) {
    return std::isspace(ch) != 0;
  }).base();

  return std::string(first, last);
}

std::string SanitizePathComponent(std::string value)
{
  for (char& ch : value)
  {
    const unsigned char ascii = static_cast<unsigned char>(ch);
    if (!(std::isalnum(ascii) != 0 || ch == '-' || ch == '_' || ch == '.'))
    {
      ch = '_';
    }
  }

  return value;
}

bool g_engineStarted = false;

const char* ToLogLevelLabel(ADDON_LOG level)
{
  switch (level)
  {
    case ADDON_LOG_DEBUG:
      return "DEBUG";
    case ADDON_LOG_INFO:
      return "INFO";
    case ADDON_LOG_WARNING:
      return "WARNING";
    case ADDON_LOG_ERROR:
      return "ERROR";
    case ADDON_LOG_FATAL:
      return "FATAL";
    default:
      return "UNKNOWN";
  }
}

void NlcLog(ADDON_LOG level, const char* format, ...)
{
  va_list args;
  va_start(args, format);

  char message[2048] = {0};
  std::vsnprintf(message, sizeof(message), format, args);

  va_end(args);

  AddonGlobalInterface* iface = kodi::addon::CPrivateBase::m_interface;
  if (iface != nullptr && iface->toKodi != nullptr && iface->toKodi->addon_log_msg != nullptr)
  {
    iface->toKodi->addon_log_msg(iface->toKodi->kodiBase, level, message);
    return;
  }

  std::fprintf(stderr, "[NLC][%s] %s\n", ToLogLevelLabel(level), message);
  std::fflush(stderr);
}

int GetTriggerStage()
{
  const char* raw = std::getenv("NLC_NATIVE_TRIGGER_STAGE");
  if (raw == nullptr || *raw == '\0')
  {
    return 0;
  }

  try
  {
    int parsed = std::stoi(raw);
    if (parsed < 0)
    {
      return 0;
    }

    if (parsed > 4)
    {
      return 4;
    }

    return parsed;
  }
  catch (...)
  {
    return 0;
  }
}

} // namespace

extern "C" int NlcRunNowTrigger(const char* addonPath,
                                 const char* profileDir,
                                 const char* userName,
                                 const char* moodMessage,
                                 const char* preferredHost)
{
  try
  {
    const std::string addonPathValue = EnsureTrailingSlash(addonPath != nullptr ? addonPath : "");
    const std::string rootDataDir = EnsureTrailingSlash(profileDir != nullptr ? profileDir : "");
    const std::string trimmedUserName = TrimWhitespace(userName != nullptr ? userName : "");
    const std::string trimmedMoodMessage = TrimWhitespace(moodMessage != nullptr ? moodMessage : "");
    const std::string trimmedPreferredHost = TrimWhitespace(preferredHost != nullptr ? preferredHost : "");

    if (addonPathValue.empty() || rootDataDir.empty() || trimmedUserName.empty())
    {
      NlcLog(ADDON_LOG_ERROR, "NlcRunNowTrigger: invalid input parameters");
      return 0;
    }

    const int triggerStage = GetTriggerStage();
    NlcLog(ADDON_LOG_INFO, "NlcRunNowTrigger: stage=%d", triggerStage);

#if defined(NLC_ENABLE_DEV_STUBS)
    if (triggerStage >= 1)
    {
      NlcLog(ADDON_LOG_WARNING,
             "NlcRunNowTrigger: dev-stub build detected; skipping native engine stages");
      return 0;
    }
#endif

    const std::string appDataDir = rootDataDir + "app/";
    const std::string xferRootDir = rootDataDir + "xfer/";
    std::filesystem::create_directories(appDataDir);
    std::filesystem::create_directories(xferRootDir);

    if (triggerStage < 1)
    {
      NlcLog(ADDON_LOG_INFO, "NlcRunNowTrigger: preflight only (stage < 1)");
      return 0;
    }

    if (!g_engineStarted)
    {
      NlcLog(ADDON_LOG_INFO, "NlcRunNowTrigger: entering fromGuiAppStartup");
      GetPtoPEngine().fromGuiAppStartup(addonPathValue, appDataDir, true);
      g_engineStarted = true;
      NlcLog(ADDON_LOG_INFO, "NlcRunNowTrigger: engine startup initialized");
    }

    if (triggerStage < 2)
    {
      NlcLog(ADDON_LOG_INFO, "NlcRunNowTrigger: stopping after startup stage");
      return 0;
    }

    const std::string safeUserName = SanitizePathComponent(trimmedUserName);
    const std::string userSpecificDir = rootDataDir + "app/users/" + safeUserName + "/";
    const std::string userXferDir = rootDataDir + "xfer/" + safeUserName + "/";

    std::filesystem::create_directories(userSpecificDir);
    std::filesystem::create_directories(userXferDir);
    NlcLog(ADDON_LOG_INFO, "NlcRunNowTrigger: entering user dir stage");
    GetPtoPEngine().fromGuiSetUserSpecificDir(userSpecificDir, true);
    GetPtoPEngine().fromGuiSetUserXferDir(userXferDir, true);

    if (triggerStage < 3)
    {
      NlcLog(ADDON_LOG_INFO, "NlcRunNowTrigger: stopping after user dir stage");
      return 0;
    }

    NlcLog(ADDON_LOG_INFO, "NlcRunNowTrigger: entering identity stage");
    GetPtoPEngine().fromGuiOnlineNameChanged(trimmedUserName.c_str());
    if (!trimmedMoodMessage.empty())
    {
      GetPtoPEngine().fromGuiMoodMessageChanged(trimmedMoodMessage.c_str());
    }

    if (triggerStage < 4)
    {
      NlcLog(ADDON_LOG_INFO, "NlcRunNowTrigger: stopping after identity stage");
      return 0;
    }

    NlcLog(ADDON_LOG_INFO, "NlcRunNowTrigger: entering user logon stage");
    PktAnnounce& myPktAnnounce = GetPtoPEngine().getMyPktAnnounce();
    GetPtoPEngine().fromGuiUserLoggedOn(myPktAnnounce.getVxNetIdent(), true);

    if (!trimmedPreferredHost.empty())
    {
      NlcLog(ADDON_LOG_INFO, "NlcRunNowTrigger: preferred host %s", trimmedPreferredHost.c_str());
    }

    NlcLog(ADDON_LOG_INFO, "NlcRunNowTrigger: native run trigger completed for user %s", trimmedUserName.c_str());
    return 1;
  }
  catch (const std::exception& ex)
  {
    NlcLog(ADDON_LOG_ERROR, "NlcRunNowTrigger exception: %s", ex.what());
    return -1;
  }
  catch (...)
  {
    NlcLog(ADDON_LOG_ERROR, "NlcRunNowTrigger exception: unknown");
    return -2;
  }
}
