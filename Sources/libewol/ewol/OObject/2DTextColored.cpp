/**
 *******************************************************************************
 * @file ewol/OObject/2DTextColored.cpp
 * @brief ewol OpenGl Object system (Sources)
 * @author Edouard DUPIN
 * @date 16/01/2012
 * @par Project
 * ewol
 *
 * @par Copyright
 * Copyright 2011 Edouard DUPIN, all right reserved
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY.
 *
 * Licence summary : 
 *    You can modify and redistribute the sources code and binaries.
 *    You can send me the bug-fix
 *
 * Term of the licence in in the file licence.txt.
 *
 *******************************************************************************
 */

#include <ewol/OObject/2DTextColored.h>
#include <ewol/importgl.h>

#undef __class__
#define __class__	"ewol::OObject2DTextColored"

ewol::OObject2DTextColored::OObject2DTextColored(etk::String FontName, int32_t size)
{
	m_color.red = 0.0;
	m_color.green = 0.0;
	m_color.blue = 0.0;
	m_color.alpha = 1.0;
	if (FontName == "") {
		m_FontId = GetDefaultFontId();
	} else {
		EWOL_TODO("pas encore fait...");
		//m_FontId = GetFontIdWithName(FontName);
		m_FontId = -1;
		return;
	}
}
// open with default font ...
ewol::OObject2DTextColored::OObject2DTextColored(void)
{
	m_color.red = 0.0;
	m_color.green = 0.0;
	m_color.blue = 0.0;
	m_color.alpha = 1.0;
	m_FontId = GetDefaultFontId();
}

ewol::OObject2DTextColored::~OObject2DTextColored(void)
{
	
}

void ewol::OObject2DTextColored::Draw(void)
{
	if (m_coord.Size()<=0) {
		// TODO : a rem�tre ...
		//EWOL_WARNING("Nothink to draw...");
		return;
	}
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_FontTextureId);
	glEnableClientState( GL_VERTEX_ARRAY );                     // Enable Vertex Arrays
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );              // Enable Texture Coord Arrays
	glEnableClientState( GL_COLOR_ARRAY );                      // Enable Color Arrays
	glVertexPointer(   2, oglTypeFloat_t, 0, &m_coord[0] );
	glTexCoordPointer( 2, oglTypeFloat_t, 0, &m_coordTex[0] );
	glColorPointer(    4, oglTypeFloat_t, 0, &m_coordColor[0] );
	glDrawArrays( GL_TRIANGLES, 0, m_coord.Size());
	//EWOL_DEBUG("request draw of " << m_coord.Size() << " elements");
	glDisableClientState( GL_COLOR_ARRAY );                     // Disable Color Arrays
	glDisableClientState( GL_VERTEX_ARRAY );                    // Disable Vertex Arrays
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );             // Disable Texture Coord Arrays
	glDisable(GL_TEXTURE_2D);
}

int32_t ewol::OObject2DTextColored::Text(etkFloat_t x, etkFloat_t y, const char* utf8String, int32_t clippingPositionX)
{
	m_coord.Clear();
	m_coordTex.Clear();
	m_coordColor.Clear();
	// normal adding text : 
	return TextAdd(x, y, utf8String, clippingPositionX);
}

int32_t ewol::OObject2DTextColored::TextAdd(etkFloat_t x, etkFloat_t y, const char* utf8String, int32_t clippingPositionX)
{
	m_FontTextureId = 0;
	if (m_FontId == -1) {
		EWOL_ERROR("Font Id is not corectly defined");
		return 0;
	}
	coord2D_ts drawPosition;
	drawPosition.x = x;
	drawPosition.y = y;
	coord2D_ts clipSize;
	clipSize.x = clippingPositionX;
	clipSize.y = -1;
	int32_t nbElementInTheArray = m_coord.Size();
	int32_t size = ewol::DrawText(m_FontId, drawPosition, clipSize, utf8String, m_FontTextureId, m_coord, m_coordTex);
	for (int32_t iii=nbElementInTheArray; iii<m_coord.Size(); iii++) {
		m_coordColor.PushBack(m_color);
	}
	return size;
}

void ewol::OObject2DTextColored::UpdateOrigin(etkFloat_t x, etkFloat_t y)
{
	for (int32_t iii=0; iii<m_coord.Size(); iii++) {
		m_coord[iii].x += x;
		m_coord[iii].y += y;
	}
}


void ewol::OObject2DTextColored::SetColor(color_ts color)
{
	m_color = color;
}


void ewol::OObject2DTextColored::SetColor(etkFloat_t red, etkFloat_t green, etkFloat_t blue, etkFloat_t alpha)
{
	m_color.red = red;
	m_color.green = green;
	m_color.blue = blue;
	m_color.alpha = alpha;
}

