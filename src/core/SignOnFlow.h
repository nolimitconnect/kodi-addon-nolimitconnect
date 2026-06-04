#pragma once

#include <optional>
#include <string>

namespace nlc::core
{

enum class SignOnState
{
  kUninitialized,
  kLoadProfile,
  kPromptDisplayName,
  kPersistDisplayName,
  kJoinRandomConnectHost,
  kOnline,
  kError
};

struct SignOnSnapshot
{
  SignOnState state{SignOnState::kUninitialized};
  std::string networkHostUrl{"nolimitconnect.net"};
  std::optional<std::string> displayName;
  std::string lastError;
};

class SignOnFlow
{
public:
  void Reset();
  void Begin(const std::optional<std::string>& configuredDisplayName);
  void ProvideDisplayName(const std::string& displayName);
  void MarkJoinedNetwork();
  void MarkFailed(const std::string& error);

  bool NeedsDisplayNamePrompt() const;
  const SignOnSnapshot& GetSnapshot() const;

private:
  static std::string TrimWhitespace(const std::string& value);

  SignOnSnapshot m_snapshot;
};

} // namespace nlc::core
