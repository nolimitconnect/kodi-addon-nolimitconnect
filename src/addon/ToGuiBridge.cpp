#include "addon/ToGuiBridge.h"

namespace nlc::addon
{

void ToGuiBridge::PostEvent(const ToGuiEvent& event)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_events.push_back(event);
}

std::optional<ToGuiEvent> ToGuiBridge::TryPopEvent()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  if (m_events.empty())
  {
    return std::nullopt;
  }

  ToGuiEvent event = m_events.front();
  m_events.pop_front();
  return event;
}

std::size_t ToGuiBridge::PendingEventCount() const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_events.size();
}

const char* ToGuiBridge::ToString(ToGuiEventType eventType)
{
  switch (eventType)
  {
    case ToGuiEventType::kNetworkStateChanged:
      return "NetworkStateChanged";
    case ToGuiEventType::kPromptDisplayNameRequested:
      return "PromptDisplayNameRequested";
    case ToGuiEventType::kPluginSessionStarted:
      return "PluginSessionStarted";
    case ToGuiEventType::kPluginSessionEnded:
      return "PluginSessionEnded";
    case ToGuiEventType::kIncomingOffer:
      return "IncomingOffer";
    case ToGuiEventType::kInstantMessageReceived:
      return "InstantMessageReceived";
    case ToGuiEventType::kMotionDetected:
      return "MotionDetected";
    case ToGuiEventType::kStatusMessage:
      return "StatusMessage";
  }

  return "Unknown";
}

} // namespace nlc::addon
