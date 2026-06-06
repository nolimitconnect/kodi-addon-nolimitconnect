#pragma once

#include <optional>
#include <string>

namespace nlc::core
{

enum class SignOnState
{
  kUninitialized,
  kLoadProfile,
  kAwaitUserActivation,
  kPromptDisplayName,
  kPersistDisplayName,
  kJoinRandomConnectHost,
  kOnline,
  kError
};

struct SignOnSnapshot
{
  SignOnState state{SignOnState::kUninitialized};
  std::string preferredRandomConnectHost{"nolimitconnect.net"};
  std::optional<std::string> activeRandomConnectHost;
  std::optional<std::string> displayName;
  std::string lastError;
};

class SignOnFlow
{
public:
  void Reset();
  void Begin(const std::optional<std::string>& configuredDisplayName,
             const std::optional<std::string>& configuredLastRandomConnectHost);
  void ProvideDisplayName(const std::string& displayName);
  void SetPreferredRandomConnectHost(const std::string& host);
  void RequestNetworkJoinFromUiEntry();
  void MarkJoinedNetwork(const std::string& joinedHost);
  void MarkLeftRandomConnectHost();
  void MarkFailed(const std::string& error);

  bool NeedsDisplayNamePrompt() const;
  const SignOnSnapshot& GetSnapshot() const;

private:
  static std::string TrimWhitespace(const std::string& value);

  SignOnSnapshot m_snapshot;
};

} // namespace nlc::core
