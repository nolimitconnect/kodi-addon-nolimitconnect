/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "SlideShowPicture.h"

class CTexture;

class CSlideShowPicQt : public CSlideShowPic
{
public:
  CSlideShowPicQt() = default;
  ~CSlideShowPicQt() override = default;

protected:
  void Render(float* x, float* y, CTextureBase* pTexture, UTILS::COLOR::Color color) override;
};
