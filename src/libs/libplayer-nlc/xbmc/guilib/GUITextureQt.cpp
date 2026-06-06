/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://kodi.tv
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */
#include "config_components_kodi.h"
#if defined(HAVE_QT_GUI)
#if defined(TARGET_OS_WINDOWS)
# include <libglew/include/GL/glew.h>
#endif

#include "GUITextureQt.h"
#include "Texture.h"
#include "ServiceBroker.h"
#include "utils/log.h"
#include "utils/GLUtils.h"
#include "utils/MathUtils.h"
#include "rendering/qt/RenderSystemQt.h"
#include "windowing/GraphicContext.h"
#include "windowing/WinSystem.h"

#include <cstddef>

#include <GuiInterface/INlcRender.h>


CGUITextureQt::CGUITextureQt( float posX, float posY, float width, float height, const CTextureInfo &texture )
    : CGUITextureBase( posX, posY, width, height, texture )
    , m_INlc( INlcRender::getINlcRender() )
{
    m_renderSystem = dynamic_cast< CRenderSystemQt* >( CServiceBroker::GetRenderSystem() );
}

void CGUITextureQt::Begin( UTILS::COLOR::Color color )
{
    m_INlc.beginGuiTexture( this, (NlcColor) color );
}

void CGUITextureQt::End()
{
    m_INlc.endGuiTexture( this );

    m_renderSystem->DisableGUIShader();
}

void CGUITextureQt::Draw( float *x, float *y, float *z, const CRect &texture, const CRect &diffuse, int orientation )
{
    m_INlc.drawGuiTexture( this, x, y, z, (const NlcRect &)texture, (const NlcRect &)diffuse, orientation );
}

void CGUITextureQt::DrawQuad( const CRect &rect, UTILS::COLOR::Color color, CTextureBase *texture, const CRect *texCoords )
{
    INlcRender::getINlcRender().drawQuad( ( const NlcRect & )rect, ( NlcColor )color, texture, ( const NlcRect * )texCoords );

    CRenderSystemQt *renderSystem = dynamic_cast< CRenderSystemQt* >( CServiceBroker::GetRenderSystem() );
    renderSystem->DisableGUIShader();
}


CGUITextureQt* CGUITextureQt::Clone() const
{
    return new CGUITextureQt(*this);
}

#endif // defined(HAVE_QT_GUI)
