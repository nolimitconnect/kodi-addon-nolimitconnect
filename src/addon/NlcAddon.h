#pragma once

#include <optional>
#include <string>

#include "addon/BridgeTypes.h"
#include "addon/FromGuiBridge.h"
#include "addon/ToGuiBridge.h"
#include "core/SignOnFlow.h"

namespace kodi::addon
{
class CSettingValue;
}

namespace nlc
{

class NlcAddon
{
public:
  void Initialize();
  void Shutdown();

  bool HandleSettingChanged(const std::string& settingName,
                            const kodi::addon::CSettingValue& settingValue);
  bool DispatchGuiCommand(const addon::FromGuiCommand& command);
  std::optional<addon::ToGuiEvent> PollGuiEvent();

  const core::SignOnSnapshot& GetSignOnSnapshot() const;

private:
  void WireBridgeHandlers();
  void EmitDisplayNamePromptEvent(const std::string& reason);
  void EmitStatusEvent(const std::string& message);

  std::optional<std::string> LoadConfiguredDisplayName() const;

  core::SignOnFlow m_signOnFlow;
  addon::FromGuiBridge m_fromGuiBridge;
  addon::ToGuiBridge m_toGuiBridge;
};

} // namespace nlc
