#pragma once

#include <functional>
#include <unordered_map>

#include "addon/BridgeTypes.h"

namespace nlc::addon
{

class FromGuiBridge
{
public:
  using CommandHandler = std::function<void(const FromGuiCommand&)>;

  void RegisterHandler(FromGuiCommandType commandType, CommandHandler handler);
  bool Dispatch(const FromGuiCommand& command) const;
  static const char* ToString(FromGuiCommandType commandType);

private:
  std::unordered_map<FromGuiCommandType, CommandHandler> m_handlers;
};

} // namespace nlc::addon
