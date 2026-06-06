#include <kodi/AddonBase.h>
#include <kodi/General.h>

#include "addon/NlcAddon.h"
#include "addon/ToGuiBridge.h"

namespace
{

void DrainAndLogGuiEvents(nlc::NlcAddon& addon)
{
  while (const auto event = addon.PollGuiEvent())
  {
    kodi::Log(ADDON_LOG_INFO,
              "NLC event: type=%s payload=%s",
              nlc::addon::ToGuiBridge::ToString(event->type),
              event->payload.c_str());

    if (event->type == nlc::addon::ToGuiEventType::kPromptDisplayNameRequested)
    {
      kodi::Log(ADDON_LOG_WARNING,
                "Display name prompt requested by sign-on flow: %s",
                event->payload.c_str());
    }
    else if (event->type == nlc::addon::ToGuiEventType::kNetworkStateChanged)
    {
      kodi::Log(ADDON_LOG_INFO,
                "Network state event received: %s",
                event->payload.c_str());
    }

    if (addon.IsNlcUiActive())
    {
      continue;
    }

    if (event->type == nlc::addon::ToGuiEventType::kInstantMessageReceived)
    {
      kodi::QueueNotification(QUEUE_INFO, "NoLimitConnect", "New message received");
    }
    else if (event->type == nlc::addon::ToGuiEventType::kIncomingOffer)
    {
      kodi::QueueNotification(QUEUE_WARNING, "NoLimitConnect", "Incoming call or offer");
    }
  }
}

} // namespace

class ATTR_DLL_LOCAL CNoLimitConnectAddon : public kodi::addon::CAddonBase
{
public:
  CNoLimitConnectAddon() = default;

  ~CNoLimitConnectAddon() override
  {
    m_addon.Shutdown();
  }

  ADDON_STATUS Create() override
  {
    kodi::Log(ADDON_LOG_INFO, "NoLimitConnect addon service started");
    m_addon.Initialize();
    DrainAndLogGuiEvents(m_addon);
    return ADDON_STATUS_OK;
  }

  ADDON_STATUS SetSetting(const std::string& settingName,
                          const kodi::addon::CSettingValue& settingValue) override
  {
    m_addon.HandleSettingChanged(settingName, settingValue);
    DrainAndLogGuiEvents(m_addon);
    return ADDON_STATUS_OK;
  }

private:
  nlc::NlcAddon m_addon;
};

ADDONCREATOR(CNoLimitConnectAddon)
