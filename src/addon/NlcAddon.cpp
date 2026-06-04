#include "addon/NlcAddon.h"

namespace nlc
{

void NlcAddon::Initialize()
{
  WireBridgeHandlers();

  m_signOnFlow.Reset();
  m_signOnFlow.Begin(LoadConfiguredDisplayName());
  EmitStatusEvent("Sign-on flow initialized");
}

void NlcAddon::Shutdown()
{
  EmitStatusEvent("Addon shutdown requested");
  m_signOnFlow.Reset();
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
  m_fromGuiBridge.RegisterHandler(addon::FromGuiCommandType::kJoinDefaultNetwork,
                                  [this](const addon::FromGuiCommand&) {
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

std::optional<std::string> NlcAddon::LoadConfiguredDisplayName() const
{
  // Persistence wiring will use Kodi addon profile settings in M3.
  return std::nullopt;
}

} // namespace nlc
