#include "core/SignOnFlow.h"

#include <algorithm>
#include <cctype>

namespace nlc::core
{

void SignOnFlow::Reset()
{
  m_snapshot = {};
}

void SignOnFlow::Begin(const std::optional<std::string>& configuredDisplayName)
{
  m_snapshot.state = SignOnState::kLoadProfile;
  m_snapshot.lastError.clear();

  if (configuredDisplayName.has_value())
  {
    const std::string trimmed = TrimWhitespace(*configuredDisplayName);
    if (!trimmed.empty())
    {
      m_snapshot.displayName = trimmed;
      m_snapshot.state = SignOnState::kJoinRandomConnectHost;
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

  // Persistence is stubbed for now; transition immediately to host join.
  m_snapshot.state = SignOnState::kJoinRandomConnectHost;
}

void SignOnFlow::MarkJoinedNetwork()
{
  m_snapshot.lastError.clear();
  m_snapshot.state = SignOnState::kOnline;
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
