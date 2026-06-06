#pragma once

#include <optional>
#include <string>

namespace nlc::addon
{

enum class ToGuiEventType
{
  kNetworkStateChanged,
  kPromptDisplayNameRequested,
  kPluginSessionStarted,
  kPluginSessionEnded,
  kIncomingOffer,
  kInstantMessageReceived,
  kMotionDetected,
  kStatusMessage
};

struct ToGuiEvent
{
  ToGuiEventType type{ToGuiEventType::kStatusMessage};
  std::optional<int> pluginSlot;
  std::optional<std::string> peerId;
  std::string payload;
};

enum class FromGuiCommandType
{
  kEnterNlcUi,
  kExitNlcUi,
  kProvideDisplayName,
  kStartPluginSession,
  kStopPluginSession,
  kSendTextMessage,
  kAcceptOffer,
  kDeclineOffer,
  kJoinDefaultNetwork,
  kLeaveRandomConnectHost,
  kJoinRandomConnectHost
};

struct FromGuiCommand
{
  FromGuiCommandType type{FromGuiCommandType::kJoinDefaultNetwork};
  std::optional<int> pluginSlot;
  std::optional<std::string> peerId;
  std::string payload;
};

} // namespace nlc::addon
