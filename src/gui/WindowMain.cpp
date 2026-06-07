#include "gui/WindowMain.h"

#include <atomic>
#include <chrono>
#include <deque>
#include <exception>
#include <functional>
#include <mutex>
#include <sstream>
#include <thread>

#include <kodi/AddonBase.h>
#include <kodi/gui/Window.h>

#include <CoreLib/VxDebug.h>

#include "addon/NlcAddon.h"

namespace nlc::gui
{

namespace
{

constexpr int kControlIdRunStartup = 101;
constexpr int kControlIdClose = 102;
constexpr int kControlIdClearLog = 103;
constexpr int kControlIdFilterAll = 104;
constexpr int kControlIdFilterInfoPlus = 105;
constexpr int kControlIdFilterWarnPlus = 106;
constexpr int kControlIdFilterErrorOnly = 107;
constexpr size_t kMaxLogLines = 350;

enum class LogFilterMode
{
  kAll,
  kInfoPlus,
  kWarnPlus,
  kErrorOnly
};

struct LogEntry
{
  uint32_t flags{0};
  std::string line;
};

class NlcDebugWindow final : public kodi::gui::CWindow, public ILogCallbackInterface
{
public:
  explicit NlcDebugWindow(const std::function<void()>& startupSequence)
    : kodi::gui::CWindow("nlc_debug_window.xml", "Default", false, false),
      m_startupSequence(startupSequence)
  {
  }

  ~NlcDebugWindow() override
  {
    m_refreshThreadRunning = false;
    if (m_refreshThread.joinable())
    {
      m_refreshThread.join();
    }

    if (m_logHandlerRegistered)
    {
      VxRemoveLogHandler(this);
      m_logHandlerRegistered = false;
    }
  }

  bool OnInit() override
  {
    SetProperty("nlc.debug.status", "Preparing startup sequence...");
    SetProperty("nlc.debug.log", "Waiting for Vx log output...");
    SetFilterMode(LogFilterMode::kAll);

    VxAddLogHandler(this);
    m_logHandlerRegistered = true;

    m_refreshThreadRunning = true;
    m_refreshThread = std::thread([this]() { RefreshLoop(); });

    RunStartupSequence();
    SetFocusId(kControlIdClose);
    return true;
  }

  bool OnClick(int controlId) override
  {
    if (controlId == kControlIdRunStartup)
    {
      RunStartupSequence();
      return true;
    }

    if (controlId == kControlIdClose)
    {
      Close();
      return true;
    }

    if (controlId == kControlIdClearLog)
    {
      {
        std::lock_guard<std::mutex> guard(m_logMutex);
        m_logLines.clear();
      }

      m_logDirty = true;
      SetProperty("nlc.debug.status", "Log view cleared");
      return true;
    }

    if (controlId == kControlIdFilterAll)
    {
      SetFilterMode(LogFilterMode::kAll);
      return true;
    }

    if (controlId == kControlIdFilterInfoPlus)
    {
      SetFilterMode(LogFilterMode::kInfoPlus);
      return true;
    }

    if (controlId == kControlIdFilterWarnPlus)
    {
      SetFilterMode(LogFilterMode::kWarnPlus);
      return true;
    }

    if (controlId == kControlIdFilterErrorOnly)
    {
      SetFilterMode(LogFilterMode::kErrorOnly);
      return true;
    }

    return false;
  }

  bool OnAction(ADDON_ACTION actionId) override
  {
    if (actionId == ADDON_ACTION_PREVIOUS_MENU || actionId == ADDON_ACTION_NAV_BACK)
    {
      Close();
      return true;
    }

    return false;
  }

  void onLogEvent(uint32_t u32LogFlags, const char* logMsg) override
  {
    if (!logMsg)
    {
      return;
    }

    std::ostringstream line;
    line << "[0x" << std::hex << u32LogFlags << "] " << logMsg;

    {
      std::lock_guard<std::mutex> guard(m_logMutex);
      m_logLines.push_back({u32LogFlags, line.str()});
      while (m_logLines.size() > kMaxLogLines)
      {
        m_logLines.pop_front();
      }
    }

    m_logDirty = true;
  }

private:
  void RunStartupSequence()
  {
    if (!m_startupSequence)
    {
      SetProperty("nlc.debug.status", "No startup callback configured");
      return;
    }

    SetProperty("nlc.debug.status", "Running P2P startup sequence...");
    try
    {
      m_startupSequence();
      SetProperty("nlc.debug.status", "Startup sequence finished (check logs below)");
    }
    catch (const std::exception& ex)
    {
      SetProperty("nlc.debug.status", std::string("Startup exception: ") + ex.what());
    }
    catch (...)
    {
      SetProperty("nlc.debug.status", "Startup exception: unknown error");
    }
  }

  void RefreshLoop()
  {
    while (m_refreshThreadRunning)
    {
      if (m_logDirty.exchange(false))
      {
        SetProperty("nlc.debug.log", ComposeLogText());
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }
  }

  bool ShouldInclude(uint32_t flags) const
  {
    const uint32_t priority = flags & LOG_PRIORITY_MASK;

    switch (m_filterMode.load())
    {
      case LogFilterMode::kAll:
        return true;
      case LogFilterMode::kInfoPlus:
        return (priority & (LOG_INFO | LOG_WARN | LOG_ERROR | LOG_FATAL | LOG_ASSERT)) != 0;
      case LogFilterMode::kWarnPlus:
        return (priority & (LOG_WARN | LOG_ERROR | LOG_FATAL | LOG_ASSERT)) != 0;
      case LogFilterMode::kErrorOnly:
        return (priority & (LOG_ERROR | LOG_FATAL | LOG_ASSERT)) != 0;
    }

    return true;
  }

  void SetFilterMode(LogFilterMode mode)
  {
    m_filterMode = mode;

    switch (mode)
    {
      case LogFilterMode::kAll:
        SetProperty("nlc.debug.filter", "All");
        break;
      case LogFilterMode::kInfoPlus:
        SetProperty("nlc.debug.filter", "Info+");
        break;
      case LogFilterMode::kWarnPlus:
        SetProperty("nlc.debug.filter", "Warn+");
        break;
      case LogFilterMode::kErrorOnly:
        SetProperty("nlc.debug.filter", "Error");
        break;
    }

    m_logDirty = true;
  }

  std::string ComposeLogText()
  {
    std::lock_guard<std::mutex> guard(m_logMutex);
    std::string text;
    for (const LogEntry& entry : m_logLines)
    {
      if (!ShouldInclude(entry.flags))
      {
        continue;
      }

      text += entry.line;
      text += "\n";
    }
    return text;
  }

  std::function<void()> m_startupSequence;
  std::atomic<bool> m_refreshThreadRunning{false};
  std::atomic<bool> m_logDirty{false};
  std::thread m_refreshThread;
  std::mutex m_logMutex;
  std::deque<LogEntry> m_logLines;
  std::atomic<LogFilterMode> m_filterMode{LogFilterMode::kAll};
  bool m_logHandlerRegistered{false};
};

} // namespace

WindowMain::WindowMain(NlcAddon& addon)
  : m_addon(addon)
{
}

bool WindowMain::Open()
{
  const bool entered = m_addon.OnNlcUiEntered();
  m_open = entered;
  return entered;
}

bool WindowMain::OpenStartupDebugWindow(const std::function<void()>& startupSequence)
{
  if (!Open())
  {
    kodi::Log(ADDON_LOG_ERROR, "NLC debug window open failed: unable to enter NLC UI state");
    return false;
  }

  {
    NlcDebugWindow window(startupSequence);
    window.DoModal();
  }

  Close();
  return true;
}

void WindowMain::Close()
{
  if (!m_open)
  {
    return;
  }

  m_addon.OnNlcUiExited();
  m_open = false;
}

} // namespace nlc::gui
