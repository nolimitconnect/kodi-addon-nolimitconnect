#include "addon/FromGuiBridge.h"

#include <utility>

namespace nlc::addon
{

void FromGuiBridge::RegisterHandler(FromGuiCommandType commandType, CommandHandler handler)
{
  m_handlers[commandType] = std::move(handler);
}

bool FromGuiBridge::Dispatch(const FromGuiCommand& command) const
{
  const auto it = m_handlers.find(command.type);
  if (it == m_handlers.end())
  {
    return false;
  }

  it->second(command);
  return true;
}

const char* FromGuiBridge::ToString(FromGuiCommandType commandType)
{
  switch (commandType)
  {
    case FromGuiCommandType::kStartPluginSession:
      return "StartPluginSession";
    case FromGuiCommandType::kStopPluginSession:
      return "StopPluginSession";
    case FromGuiCommandType::kSendTextMessage:
      return "SendTextMessage";
    case FromGuiCommandType::kAcceptOffer:
      return "AcceptOffer";
    case FromGuiCommandType::kDeclineOffer:
      return "DeclineOffer";
    case FromGuiCommandType::kJoinDefaultNetwork:
      return "JoinDefaultNetwork";
  }

  return "Unknown";
}

} // namespace nlc::addon
