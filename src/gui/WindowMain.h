#pragma once

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
  void Close();

private:
  NlcAddon& m_addon;
  bool m_open{false};
};

} // namespace nlc::gui
