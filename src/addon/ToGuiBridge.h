#pragma once

#include <cstddef>
#include <deque>
#include <mutex>
#include <optional>

#include "addon/BridgeTypes.h"

namespace nlc::addon
{

class ToGuiBridge
{
public:
  void PostEvent(const ToGuiEvent& event);
  std::optional<ToGuiEvent> TryPopEvent();
  std::size_t PendingEventCount() const;

  static const char* ToString(ToGuiEventType eventType);

private:
  mutable std::mutex m_mutex;
  std::deque<ToGuiEvent> m_events;
};

} // namespace nlc::addon
