#pragma once

#include <functional>

namespace nlc
{
class NlcAddon;
}

namespace nlc::gui
{

class WindowMain
{
public:
  explicit WindowMain(NlcAddon& addon);

  bool Open();
  bool OpenStartupDebugWindow(const std::function<void()>& startupSequence);
  void Close();

private:
  NlcAddon& m_addon;
  bool m_open{false};
};

} // namespace nlc::gui
