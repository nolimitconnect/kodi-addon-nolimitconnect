#include "core/SignOnFlow.h"

#include <algorithm>
#include <cctype>

namespace nlc::core
{

void SignOnFlow::Reset()
{
  m_snapshot = {};
}

void SignOnFlow::Begin(const std::optional<std::string>& configuredDisplayName,
                      const std::optional<std::string>& configuredLastRandomConnectHost)
{
  m_snapshot.state = SignOnState::kLoadProfile;
  m_snapshot.lastError.clear();
  m_snapshot.activeRandomConnectHost.reset();

  if (configuredLastRandomConnectHost.has_value())
  {
    SetPreferredRandomConnectHost(*configuredLastRandomConnectHost);
  }
  else
  {
    m_snapshot.preferredRandomConnectHost = "nolimitconnect.net";
  }

  if (configuredDisplayName.has_value())
  {
    const std::string trimmed = TrimWhitespace(*configuredDisplayName);
    if (!trimmed.empty())
    {
      m_snapshot.displayName = trimmed;
      m_snapshot.state = SignOnState::kAwaitUserActivation;
      return;
    }
  }

  m_snapshot.displayName.reset();
  m_snapshot.state = SignOnState::kPromptDisplayName;
}

void SignOnFlow::ProvideDisplayName(const std::string& displayName)
{
  const std::string trimmed = TrimWhitespace(displayName);
  if (trimmed.empty())
  {
    m_snapshot.state = SignOnState::kPromptDisplayName;
    m_snapshot.lastError = "Display name is required";
    return;
  }

  m_snapshot.displayName = trimmed;
  m_snapshot.lastError.clear();
  m_snapshot.state = SignOnState::kPersistDisplayName;

  // Persistence is managed by add-on settings; wait for explicit UI entry before join.
  m_snapshot.state = SignOnState::kAwaitUserActivation;
}

void SignOnFlow::SetPreferredRandomConnectHost(const std::string& host)
{
  const std::string trimmed = TrimWhitespace(host);
  if (trimmed.empty())
  {
    m_snapshot.preferredRandomConnectHost = "nolimitconnect.net";
    return;
  }

  m_snapshot.preferredRandomConnectHost = trimmed;
}

void SignOnFlow::RequestNetworkJoinFromUiEntry()
{
  m_snapshot.lastError.clear();

  if (!m_snapshot.displayName.has_value())
  {
    m_snapshot.state = SignOnState::kPromptDisplayName;
    return;
  }

  m_snapshot.state = SignOnState::kJoinRandomConnectHost;
}

void SignOnFlow::MarkJoinedNetwork(const std::string& joinedHost)
{
  SetPreferredRandomConnectHost(joinedHost);
  m_snapshot.activeRandomConnectHost = m_snapshot.preferredRandomConnectHost;
  m_snapshot.lastError.clear();
  m_snapshot.state = SignOnState::kOnline;
}

void SignOnFlow::MarkLeftRandomConnectHost()
{
  m_snapshot.activeRandomConnectHost.reset();
  m_snapshot.lastError.clear();
  m_snapshot.state = SignOnState::kAwaitUserActivation;
}

void SignOnFlow::MarkFailed(const std::string& error)
{
  m_snapshot.state = SignOnState::kError;
  m_snapshot.lastError = error;
}

bool SignOnFlow::NeedsDisplayNamePrompt() const
{
  return m_snapshot.state == SignOnState::kPromptDisplayName;
}

const SignOnSnapshot& SignOnFlow::GetSnapshot() const
{
  return m_snapshot;
}

std::string SignOnFlow::TrimWhitespace(const std::string& value)
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

} // namespace nlc::core
