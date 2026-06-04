#include <kodi/AddonBase.h>

#include "addon/NlcAddon.h"

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
    m_addon.Initialize();
    return ADDON_STATUS_OK;
  }

  ADDON_STATUS SetSetting(const std::string& settingName,
                          const kodi::addon::CSettingValue& settingValue) override
  {
    (void)settingName;
    (void)settingValue;
    return ADDON_STATUS_OK;
  }

private:
  nlc::NlcAddon m_addon;
};

ADDONCREATOR(CNoLimitConnectAddon)
