#include "addon/NlcAddon.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <filesystem>

#include <kodi/AddonBase.h>

#include <PktLib/PktAnnounce.h>
#include <libptopengine/P2PEngine/P2PEngine.h>

enum EAppDir : int
{
  eAppData = 1
};
void VxSetAppDirectory(EAppDir appDir, std::string setDir);
void VxSetRootDataStorageDirectory(const char* rootDataDir);
void VxSetRootUserDataDirectory(const char* rootUserDataDir);
void VxSetRootXferDirectory(const char* rootXferDir);

namespace nlc
{

namespace
{

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

std::string GenerateDevStubDisplayName()
{
  const auto now = std::chrono::system_clock::now();
  const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
  return "kodi-dev-" + std::to_string(seconds % 100000);
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

std::string EnsureTrailingSlash(std::string value)
{
  if (!value.empty() && value.back() != '/')
  {
    value.push_back('/');
  }

  return value;
}

} // namespace

void NlcAddon::Initialize()
{
  WireBridgeHandlers();
  InitializeStorageAndEngine();

  m_signOnFlow.Reset();
  m_signOnFlow.Begin(LoadConfiguredDisplayName(), LoadLastRandomConnectHost());

  if (m_signOnFlow.NeedsDisplayNamePrompt())
  {
#if defined(NLC_ENABLE_DEV_STUBS)
    const std::string fallbackName = GenerateDevStubDisplayName();
  kodi::addon::SetSettingString("user_name", fallbackName);
    m_signOnFlow.ProvideDisplayName(fallbackName);
    EmitStatusEvent("Using dev-stub fallback display name");
#else
    EmitDisplayNamePromptEvent("Display name is required before joining the network");
#endif
  }

  EmitStatusEvent("Sign-on flow initialized; waiting for NLC UI entry before network login");
}

void NlcAddon::Shutdown()
{
  EmitStatusEvent("Addon shutdown requested");
  m_signOnFlow.Reset();
}

bool NlcAddon::HandleSettingChanged(const std::string& settingName,
                                    const kodi::addon::CSettingValue& settingValue)
{
  if (settingName == "debug_nlc_ui_active")
  {
    if (settingValue.GetBoolean())
    {
      OnNlcUiEntered();
      EmitStatusEvent("Debug: simulated NLC UI entry");
    }
    else
    {
      OnNlcUiExited();
      EmitStatusEvent("Debug: simulated NLC UI exit");
    }

    return true;
  }

  if (settingName == "debug_emit_test_message")
  {
    if (settingValue.GetBoolean())
    {
      DispatchGuiCommand({addon::FromGuiCommandType::kSendTextMessage,
                          8,
                          std::optional<std::string>{"debug-peer"},
                          "debug-test-message"});
      kodi::addon::SetSettingBoolean("debug_emit_test_message", false);
      EmitStatusEvent("Debug: emitted test message event");
    }

    return true;
  }

  if (settingName == "debug_emit_test_offer")
  {
    if (settingValue.GetBoolean())
    {
      m_toGuiBridge.PostEvent({addon::ToGuiEventType::kIncomingOffer,
                               16,
                               std::optional<std::string>{"debug-peer"},
                               "debug-test-offer"});
      kodi::addon::SetSettingBoolean("debug_emit_test_offer", false);
      EmitStatusEvent("Debug: emitted test incoming offer event");
    }

    return true;
  }

  if (settingName == "network_host_url" || settingName == "last_random_connect_host")
  {
    const std::string trimmedHost = TrimWhitespace(settingValue.GetString());
    m_signOnFlow.SetPreferredRandomConnectHost(trimmedHost);
    kodi::addon::SetSettingString("network_host_url", m_signOnFlow.GetSnapshot().preferredRandomConnectHost);
    EmitStatusEvent("Updated preferred random connect host");
    return true;
  }

  if (settingName == "user_name")
  {
    const std::string trimmedValue = TrimWhitespace(settingValue.GetString());
    kodi::addon::SetSettingString("user_name", trimmedValue);
    m_signOnFlow.ProvideDisplayName(trimmedValue);
    UpdateEngineIdentityFromSettings();
    MaybeStartEngineUserLogon();

    if (m_signOnFlow.NeedsDisplayNamePrompt())
    {
      EmitDisplayNamePromptEvent(m_signOnFlow.GetSnapshot().lastError);
      return true;
    }

    EmitStatusEvent("User name updated; startup storage is ready");
    return true;
  }

  if (settingName == "mood_message")
  {
    const std::string trimmedValue = TrimWhitespace(settingValue.GetString());
    kodi::addon::SetSettingString("mood_message", trimmedValue);
    UpdateEngineIdentityFromSettings();
    EmitStatusEvent("Mood message updated");
    return true;
  }

  if (settingName != "display_name")
  {
    return false;
  }

  const std::string trimmedValue = TrimWhitespace(settingValue.GetString());
  kodi::addon::SetSettingString("user_name", trimmedValue);

  m_signOnFlow.ProvideDisplayName(trimmedValue);
  UpdateEngineIdentityFromSettings();
  MaybeStartEngineUserLogon();

  if (m_signOnFlow.NeedsDisplayNamePrompt())
  {
    EmitDisplayNamePromptEvent(m_signOnFlow.GetSnapshot().lastError);
    return true;
  }

  EmitStatusEvent("Display name updated; connect will occur when NLC UI is opened");

  return true;
}

bool NlcAddon::DispatchGuiCommand(const addon::FromGuiCommand& command)
{
  return m_fromGuiBridge.Dispatch(command);
}

bool NlcAddon::OnNlcUiEntered()
{
  return DispatchGuiCommand({addon::FromGuiCommandType::kEnterNlcUi, std::nullopt, std::nullopt, {}});
}

bool NlcAddon::OnNlcUiExited()
{
  return DispatchGuiCommand({addon::FromGuiCommandType::kExitNlcUi, std::nullopt, std::nullopt, {}});
}

bool NlcAddon::IsNlcUiActive() const
{
  return m_isNlcUiActive;
}

std::optional<addon::ToGuiEvent> NlcAddon::PollGuiEvent()
{
  return m_toGuiBridge.TryPopEvent();
}

const core::SignOnSnapshot& NlcAddon::GetSignOnSnapshot() const
{
  return m_signOnFlow.GetSnapshot();
}

void NlcAddon::WireBridgeHandlers()
{
  m_fromGuiBridge.RegisterHandler(addon::FromGuiCommandType::kEnterNlcUi,
                                  [this](const addon::FromGuiCommand&) {
                                    m_isNlcUiActive = true;
                                    m_signOnFlow.RequestNetworkJoinFromUiEntry();
                                    MaybeStartEngineUserLogon();

                                    if (m_signOnFlow.NeedsDisplayNamePrompt())
                                    {
                                      EmitDisplayNamePromptEvent("Display name is required before joining the network");
                                      return;
                                    }

                                    DispatchGuiCommand({addon::FromGuiCommandType::kJoinDefaultNetwork,
                                                        std::nullopt,
                                                        std::nullopt,
                                                        m_signOnFlow.GetSnapshot().preferredRandomConnectHost});
                                  });

  m_fromGuiBridge.RegisterHandler(addon::FromGuiCommandType::kExitNlcUi,
                                  [this](const addon::FromGuiCommand&) {
                                    m_isNlcUiActive = false;
                                    EmitStatusEvent("Exited NLC UI");
                                  });

  m_fromGuiBridge.RegisterHandler(addon::FromGuiCommandType::kProvideDisplayName,
                                  [this](const addon::FromGuiCommand& command) {
                                    const std::string trimmedName = TrimWhitespace(command.payload);
                                    kodi::addon::SetSettingString("user_name", trimmedName);
                                    m_signOnFlow.ProvideDisplayName(trimmedName);
                                    UpdateEngineIdentityFromSettings();
                                    MaybeStartEngineUserLogon();

                                    if (m_signOnFlow.NeedsDisplayNamePrompt())
                                    {
                                      EmitDisplayNamePromptEvent(m_signOnFlow.GetSnapshot().lastError);
                                      return;
                                    }

                                    EmitStatusEvent("Display name saved; connect will occur when NLC UI is opened");
                                  });

  m_fromGuiBridge.RegisterHandler(addon::FromGuiCommandType::kJoinDefaultNetwork,
                                  [this](const addon::FromGuiCommand& command) {
                                    if (!m_signOnFlow.GetSnapshot().displayName.has_value())
                                    {
                                      EmitDisplayNamePromptEvent("Display name is required before joining the network");
                                      return;
                                    }

                                    std::string hostToJoin = TrimWhitespace(command.payload);
                                    if (hostToJoin.empty())
                                    {
                                      hostToJoin = m_signOnFlow.GetSnapshot().preferredRandomConnectHost;
                                    }

                                    m_signOnFlow.MarkJoinedNetwork(hostToJoin);
                                    kodi::addon::SetSettingString("network_host_url", hostToJoin);

                                    m_toGuiBridge.PostEvent({
                                      addon::ToGuiEventType::kNetworkStateChanged,
                                      std::nullopt,
                                      std::nullopt,
                                      "Joined random connect host: " + hostToJoin
                                    });
                                  });

  m_fromGuiBridge.RegisterHandler(addon::FromGuiCommandType::kLeaveRandomConnectHost,
                                  [this](const addon::FromGuiCommand&) {
                                    m_signOnFlow.MarkLeftRandomConnectHost();
                                    m_toGuiBridge.PostEvent({
                                      addon::ToGuiEventType::kNetworkStateChanged,
                                      std::nullopt,
                                      std::nullopt,
                                      "Left random connect host"
                                    });
                                  });

  m_fromGuiBridge.RegisterHandler(addon::FromGuiCommandType::kJoinRandomConnectHost,
                                  [this](const addon::FromGuiCommand& command) {
                                    const std::string hostToJoin = TrimWhitespace(command.payload);
                                    if (hostToJoin.empty())
                                    {
                                      EmitStatusEvent("JoinRandomConnectHost ignored: host is empty");
                                      return;
                                    }

                                    DispatchGuiCommand({addon::FromGuiCommandType::kJoinDefaultNetwork,
                                                        std::nullopt,
                                                        std::nullopt,
                                                        hostToJoin});
                                  });

  m_fromGuiBridge.RegisterHandler(addon::FromGuiCommandType::kStartPluginSession,
                                  [this](const addon::FromGuiCommand& command) {
                                    m_toGuiBridge.PostEvent({
                                      addon::ToGuiEventType::kPluginSessionStarted,
                                      command.pluginSlot,
                                      command.peerId,
                                      command.payload
                                    });
                                  });

  m_fromGuiBridge.RegisterHandler(addon::FromGuiCommandType::kStopPluginSession,
                                  [this](const addon::FromGuiCommand& command) {
                                    m_toGuiBridge.PostEvent({
                                      addon::ToGuiEventType::kPluginSessionEnded,
                                      command.pluginSlot,
                                      command.peerId,
                                      command.payload
                                    });
                                  });

  m_fromGuiBridge.RegisterHandler(addon::FromGuiCommandType::kSendTextMessage,
                                  [this](const addon::FromGuiCommand& command) {
                                    m_toGuiBridge.PostEvent({
                                      addon::ToGuiEventType::kInstantMessageReceived,
                                      command.pluginSlot,
                                      command.peerId,
                                      command.payload
                                    });
                                  });

  m_fromGuiBridge.RegisterHandler(addon::FromGuiCommandType::kAcceptOffer,
                                  [this](const addon::FromGuiCommand& command) {
                                    m_toGuiBridge.PostEvent({
                                      addon::ToGuiEventType::kStatusMessage,
                                      command.pluginSlot,
                                      command.peerId,
                                      "Offer accepted"
                                    });
                                  });

  m_fromGuiBridge.RegisterHandler(addon::FromGuiCommandType::kDeclineOffer,
                                  [this](const addon::FromGuiCommand& command) {
                                    m_toGuiBridge.PostEvent({
                                      addon::ToGuiEventType::kStatusMessage,
                                      command.pluginSlot,
                                      command.peerId,
                                      "Offer declined"
                                    });
                                  });
}

void NlcAddon::InitializeStorageAndEngine()
{
  if (m_engineStorageInitialized)
  {
    return;
  }

  m_assetsDir = std::filesystem::path(kodi::addon::GetAddonPath()).lexically_normal().generic_string();
  m_rootDataDir = EnsureTrailingSlash(
      std::filesystem::path(kodi::addon::GetUserPath()).lexically_normal().generic_string());

  const std::string appDataDir = m_rootDataDir + "app/";
  const std::string xferRootDir = m_rootDataDir + "xfer/";

  std::filesystem::create_directories(appDataDir);
  std::filesystem::create_directories(xferRootDir);

  VxSetAppDirectory(eAppData, appDataDir);
  VxSetRootDataStorageDirectory(appDataDir.c_str());
  VxSetRootUserDataDirectory(appDataDir.c_str());
  VxSetRootXferDirectory(xferRootDir.c_str());

  GetPtoPEngine().fromGuiAppStartup(m_assetsDir, appDataDir, true);

  m_userSpecificDir = BuildUserSpecificDir(LoadConfiguredDisplayName().value_or(""));
  m_userXferDir = BuildUserXferDir(LoadConfiguredDisplayName().value_or(""));

  if (!m_userSpecificDir.empty())
  {
    std::filesystem::create_directories(m_userSpecificDir);
    GetPtoPEngine().fromGuiSetUserSpecificDir(m_userSpecificDir, true);
  }

  if (!m_userXferDir.empty())
  {
    std::filesystem::create_directories(m_userXferDir);
    GetPtoPEngine().fromGuiSetUserXferDir(m_userXferDir, true);
  }

  m_engineStorageInitialized = true;
  EmitStatusEvent("Engine storage directories initialized");
}

void NlcAddon::UpdateEngineIdentityFromSettings()
{
  const auto userName = LoadConfiguredDisplayName();
  const auto moodMessage = LoadConfiguredMoodMessage();

  if (userName.has_value())
  {
    GetPtoPEngine().fromGuiOnlineNameChanged(userName->c_str());
  }

  if (moodMessage.has_value())
  {
    GetPtoPEngine().fromGuiMoodMessageChanged(moodMessage->c_str());
  }

  if (userName.has_value())
  {
    m_userSpecificDir = BuildUserSpecificDir(*userName);
    m_userXferDir = BuildUserXferDir(*userName);

    if (!m_userSpecificDir.empty())
    {
      std::filesystem::create_directories(m_userSpecificDir);
      GetPtoPEngine().fromGuiSetUserSpecificDir(m_userSpecificDir, true);
    }

    if (!m_userXferDir.empty())
    {
      std::filesystem::create_directories(m_userXferDir);
      GetPtoPEngine().fromGuiSetUserXferDir(m_userXferDir, true);
    }
  }
}

void NlcAddon::MaybeStartEngineUserLogon()
{
  if (!m_engineStorageInitialized || m_engineUserLoggedOn)
  {
    return;
  }

  const auto userName = LoadConfiguredDisplayName();
  if (!userName.has_value())
  {
    return;
  }

  UpdateEngineIdentityFromSettings();
  PktAnnounce& myPktAnnounce = GetPtoPEngine().getMyPktAnnounce();
  GetPtoPEngine().fromGuiUserLoggedOn(myPktAnnounce.getVxNetIdent(), true);
  m_engineUserLoggedOn = true;
  EmitStatusEvent("Engine user logon started");
}

void NlcAddon::EmitStatusEvent(const std::string& message)
{
  m_toGuiBridge.PostEvent({addon::ToGuiEventType::kStatusMessage, std::nullopt, std::nullopt, message});
}

void NlcAddon::EmitDisplayNamePromptEvent(const std::string& reason)
{
  m_toGuiBridge.PostEvent({addon::ToGuiEventType::kPromptDisplayNameRequested,
                           std::nullopt,
                           std::nullopt,
                           reason});
}

std::optional<std::string> NlcAddon::LoadConfiguredDisplayName() const
{
  const std::string displayName = TrimWhitespace(kodi::addon::GetSettingString("user_name"));
  if (displayName.empty())
  {
    return std::nullopt;
  }

  return displayName;
}

std::optional<std::string> NlcAddon::LoadConfiguredMoodMessage() const
{
  const std::string moodMessage = TrimWhitespace(kodi::addon::GetSettingString("mood_message"));
  if (moodMessage.empty())
  {
    return std::nullopt;
  }

  return moodMessage;
}

std::optional<std::string> NlcAddon::LoadLastRandomConnectHost() const
{
  const std::string host = TrimWhitespace(kodi::addon::GetSettingString("network_host_url"));
  if (host.empty())
  {
    return std::nullopt;
  }

  return host;
}

std::string NlcAddon::BuildUserSpecificDir(const std::string& userName) const
{
  if (m_rootDataDir.empty() || userName.empty())
  {
    return {};
  }

  return m_rootDataDir + "app/users/" + SanitizePathComponent(userName) + "/";
}

std::string NlcAddon::BuildUserXferDir(const std::string& userName) const
{
  if (m_rootDataDir.empty() || userName.empty())
  {
    return {};
  }

  return m_rootDataDir + "xfer/" + SanitizePathComponent(userName) + "/";
}

} // namespace nlc
