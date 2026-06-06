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
  bool OnNlcUiEntered();
  bool OnNlcUiExited();
  bool IsNlcUiActive() const;
  std::optional<addon::ToGuiEvent> PollGuiEvent();

  const core::SignOnSnapshot& GetSignOnSnapshot() const;

private:
  void WireBridgeHandlers();
  void InitializeStorageAndEngine();
  void UpdateEngineIdentityFromSettings();
  void MaybeStartEngineUserLogon();
  void EmitDisplayNamePromptEvent(const std::string& reason);
  void EmitStatusEvent(const std::string& message);

  std::optional<std::string> LoadConfiguredDisplayName() const;
  std::optional<std::string> LoadConfiguredMoodMessage() const;
  std::optional<std::string> LoadLastRandomConnectHost() const;

  std::string BuildUserSpecificDir(const std::string& userName) const;
  std::string BuildUserXferDir(const std::string& userName) const;

  core::SignOnFlow m_signOnFlow;
  addon::FromGuiBridge m_fromGuiBridge;
  addon::ToGuiBridge m_toGuiBridge;
  bool m_engineStorageInitialized{false};
  bool m_engineUserLoggedOn{false};
  std::string m_assetsDir;
  std::string m_rootDataDir;
  std::string m_userSpecificDir;
  std::string m_userXferDir;
  bool m_isNlcUiActive{false};
};

} // namespace nlc
