/**
 * @author Edouard DUPIN
 * 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * 
 * @license BSD v3 (see license file)
 */


#include <ewol/oObject/2DTextured.h>
#include <ewol/ResourceManager.h>
#include <ewol/openGL/openGL.h>

#undef __class__
#define __class__	"ewol::OObject2DTextured"


ewol::OObject2DTextured::OObject2DTextured(etk::UString textureName, float sizeX, float sizeY)
{
	EWOL_VERBOSE("Create OObject textured : \"" << textureName << "\"");
	ewol::TextureFile* resourceFile = NULL;
	if (false == ewol::resource::Keep(textureName, resourceFile, etk::Vector2D<int32_t>(sizeX,sizeY)) ) {
		EWOL_CRITICAL("can not get a resource Texture");
	}
	m_resource = resourceFile;
	etk::UString tmpString("DATA:textured.prog");
	// get the shader resource :
	m_GLPosition = 0;
	if (true == ewol::resource::Keep(tmpString, m_GLprogram) ) {
		m_GLPosition = m_GLprogram->GetAttribute("EW_coord2d");
		m_GLColor    = m_GLprogram->GetAttribute("EW_color");
		m_GLtexture  = m_GLprogram->GetAttribute("EW_texture2d");
		m_GLMatrix   = m_GLprogram->GetUniform("EW_MatrixTransformation");
		m_GLtexID    = m_GLprogram->GetUniform("EW_texID");
	}
}

ewol::OObject2DTextured::OObject2DTextured( float sizeX, float sizeY)
{
	if (false == ewol::resource::Keep(m_resource) ) {
		EWOL_CRITICAL("can not get a resource Texture");
	}
	if (NULL!=m_resource) {
		m_resource->SetImageSize(etk::Vector2D<int32_t>(sizeX,sizeY));
		draw::Image& tmpImage = m_resource->Get();
		tmpImage.SetFillColor(draw::color::black);
		tmpImage.Clear();
		m_resource->Flush();
	}
	etk::UString tmpString("DATA:textured.prog");
	// get the shader resource :
	m_GLPosition = 0;
	if (true == ewol::resource::Keep(tmpString, m_GLprogram) ) {
		m_GLPosition = m_GLprogram->GetAttribute("EW_coord2d");
		m_GLColor    = m_GLprogram->GetAttribute("EW_color");
		m_GLtexture  = m_GLprogram->GetAttribute("EW_texture2d");
		m_GLMatrix   = m_GLprogram->GetUniform("EW_MatrixTransformation");
		m_GLtexID    = m_GLprogram->GetUniform("EW_texID");
	}
}


ewol::OObject2DTextured::~OObject2DTextured(void)
{
	if (NULL != m_resource) {
		ewol::resource::Release(m_resource);
	}
	ewol::resource::Release(m_GLprogram);
}

void ewol::OObject2DTextured::Draw(void)
{
	if (m_coord.Size()<=0) {
		return;
	}
	if (NULL == m_resource) {
		EWOL_WARNING("Texture does not exist ...");
		return;
	}
	if (m_GLprogram==NULL) {
		EWOL_ERROR("No shader ...");
		return;
	}
	m_GLprogram->Use();
	// set Matrix : translation/positionMatrix
	etk::Matrix4 tmpMatrix = ewol::openGL::GetMatrix();
	m_GLprogram->UniformMatrix4fv(m_GLMatrix, 1, tmpMatrix.m_mat);
	// TextureID
	m_GLprogram->SetTexture0(m_GLtexID, m_resource->GetId());
	// position :
	m_GLprogram->SendAttribute(m_GLPosition, 2/*x,y*/, &m_coord[0]);
	// Texture :
	m_GLprogram->SendAttribute(m_GLtexture, 2/*u,v*/, &m_coordTex[0]);
	// color :
	m_GLprogram->SendAttribute(m_GLColor, 4/*r,g,b,a*/, &m_coordColor[0]);
	// Request the draw od the elements : 
	glDrawArrays(GL_TRIANGLES, 0, m_coord.Size());
	m_GLprogram->UnUse();
}

void ewol::OObject2DTextured::Clear(void)
{
	m_coord.Clear();
	m_coordTex.Clear();
	m_coordColor.Clear();
}

void ewol::OObject2DTextured::Rectangle(float x, float y, float w, float h, draw::Color tmpColor)
{
	Rectangle(x, y, w, h, 0.0, 0.0, 1.0, 1.0, tmpColor);
}

void ewol::OObject2DTextured::Rectangle(float x, float y, float w, float h, float texX, float texY, float texSX, float texSY, draw::Color tmpColor)
{
	/*
	x += 3;
	y += 3;
	w -= 6;
	h -= 6;
	*/
	//EWOL_DEBUG("Add rectangle : ...");
	etk::Vector2D<float> point;
	texCoord_ts tex;

	tex.u = texX;
	tex.v = texSY;
	point.x = x;
	point.y = y;
	m_coord.PushBack(point);
	m_coordTex.PushBack(tex);
	m_coordColor.PushBack(tmpColor);


	tex.u = texSX;
	tex.v = texSY;
	point.x = x + w;
	point.y = y;
	m_coord.PushBack(point);
	m_coordTex.PushBack(tex);
	m_coordColor.PushBack(tmpColor);


	tex.u = texSX;
	tex.v = texY;
	point.x = x + w;
	point.y = y + h;
	m_coord.PushBack(point);
	m_coordTex.PushBack(tex);
	m_coordColor.PushBack(tmpColor);

	m_coord.PushBack(point);
	m_coordTex.PushBack(tex);
	m_coordColor.PushBack(tmpColor);

	tex.u = texX;
	tex.v = texY;
	point.x = x;
	point.y = y + h;
	m_coord.PushBack(point);
	m_coordTex.PushBack(tex);
	m_coordColor.PushBack(tmpColor);

	tex.u = texX;
	tex.v = texSY;
	point.x = x;
	point.y = y;
	m_coord.PushBack(point);
	m_coordTex.PushBack(tex);
	m_coordColor.PushBack(tmpColor);
}
