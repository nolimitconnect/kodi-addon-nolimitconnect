#include "gui/WindowMain.h"

#include "addon/NlcAddon.h"

namespace nlc::gui
{

WindowMain::WindowMain(NlcAddon& addon)
  : m_addon(addon)
{
}

bool WindowMain::Open()
{
  const bool entered = m_addon.OnNlcUiEntered();
  m_open = entered;
  return entered;
}

void WindowMain::Close()
{
  if (!m_open)
  {
    return;
  }

  m_addon.OnNlcUiExited();
  m_open = false;
}

} // namespace nlc::gui
