#include "addon/NlcAddon.h"

#include <algorithm>
#include <cctype>
#include <chrono>

#include <kodi/AddonBase.h>

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

} // namespace

void NlcAddon::Initialize()
{
  WireBridgeHandlers();

  m_signOnFlow.Reset();
  m_signOnFlow.Begin(LoadConfiguredDisplayName());

  if (m_signOnFlow.NeedsDisplayNamePrompt())
  {
#if defined(NLC_ENABLE_DEV_STUBS)
    const std::string fallbackName = GenerateDevStubDisplayName();
    kodi::addon::SetSettingString("display_name", fallbackName);
    m_signOnFlow.ProvideDisplayName(fallbackName);
    EmitStatusEvent("Using dev-stub fallback display name");
    DispatchGuiCommand({addon::FromGuiCommandType::kJoinDefaultNetwork, std::nullopt, std::nullopt, {}});
#else
    EmitDisplayNamePromptEvent("Display name is required before joining the network");
#endif
  }
  else
  {
    DispatchGuiCommand({addon::FromGuiCommandType::kJoinDefaultNetwork, std::nullopt, std::nullopt, {}});
  }

  EmitStatusEvent("Sign-on flow initialized");
}

void NlcAddon::Shutdown()
{
  EmitStatusEvent("Addon shutdown requested");
  m_signOnFlow.Reset();
}

bool NlcAddon::HandleSettingChanged(const std::string& settingName,
                                    const kodi::addon::CSettingValue& settingValue)
{
  if (settingName != "display_name")
  {
    return false;
  }

  const std::string trimmedValue = TrimWhitespace(settingValue.GetString());
  kodi::addon::SetSettingString("display_name", trimmedValue);

  m_signOnFlow.ProvideDisplayName(trimmedValue);

  if (m_signOnFlow.NeedsDisplayNamePrompt())
  {
    EmitDisplayNamePromptEvent(m_signOnFlow.GetSnapshot().lastError);
    return true;
  }

  EmitStatusEvent("Display name updated");
  DispatchGuiCommand({addon::FromGuiCommandType::kJoinDefaultNetwork, std::nullopt, std::nullopt, {}});

  return true;
}

bool NlcAddon::DispatchGuiCommand(const addon::FromGuiCommand& command)
{
  return m_fromGuiBridge.Dispatch(command);
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
  m_fromGuiBridge.RegisterHandler(addon::FromGuiCommandType::kProvideDisplayName,
                                  [this](const addon::FromGuiCommand& command) {
                                    const std::string trimmedName = TrimWhitespace(command.payload);
                                    kodi::addon::SetSettingString("display_name", trimmedName);
                                    m_signOnFlow.ProvideDisplayName(trimmedName);

                                    if (m_signOnFlow.NeedsDisplayNamePrompt())
                                    {
                                      EmitDisplayNamePromptEvent(m_signOnFlow.GetSnapshot().lastError);
                                      return;
                                    }

                                    EmitStatusEvent("Display name saved");
                                    DispatchGuiCommand({addon::FromGuiCommandType::kJoinDefaultNetwork,
                                                        std::nullopt,
                                                        std::nullopt,
                                                        {}});
                                  });

  m_fromGuiBridge.RegisterHandler(addon::FromGuiCommandType::kJoinDefaultNetwork,
                                  [this](const addon::FromGuiCommand&) {
                                    if (!m_signOnFlow.GetSnapshot().displayName.has_value())
                                    {
                                      EmitDisplayNamePromptEvent("Display name is required before joining the network");
                                      return;
                                    }

                                    m_signOnFlow.MarkJoinedNetwork();
                                    m_toGuiBridge.PostEvent({
                                      addon::ToGuiEventType::kNetworkStateChanged,
                                      std::nullopt,
                                      std::nullopt,
                                      "Joined nolimitconnect.net"
                                    });
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
  const std::string displayName = TrimWhitespace(kodi::addon::GetSettingString("display_name"));
  if (displayName.empty())
  {
    return std::nullopt;
  }

  return displayName;
}

} // namespace nlc
