/**
 *******************************************************************************
 * @file ewol/oObject/2DTextured.h
 * @brief ewol OpenGl Object system (header)
 * @author Edouard DUPIN
 * @date 09/11/2011
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

#ifndef __EWOL_O_OBJECT_2D_TEXTURED_H__
#define __EWOL_O_OBJECT_2D_TEXTURED_H__

#include <ewol/oObject/OObject.h>
#include <ewol/ResourceManager.h>

namespace ewol {
	class OObject2DTextured :public ewol::OObject
	{
		public:
			OObject2DTextured(etk::UString textureName, float sizeX=-1, float sizeY=-1);
			OObject2DTextured(float sizeX=-1, float sizeY=-1);
			virtual ~OObject2DTextured(void);
		public:
			virtual void Draw(void);
			void Clear(void);
			void Rectangle(float x, float y, float w, float h, float texX=0.0, float texY=0.0, float texSX=1.0, float texSY=1.0, draw::Color tmpColor=draw::color::white);
			void Rectangle(float x, float y, float w, float h, draw::Color tmpColor);
		protected:
			#ifdef __VIDEO__OPENGL_ES_2
				ewol::Program* m_GLprogram;
				int32_t        m_GLPosition;
				int32_t        m_GLMatrix;
				int32_t        m_GLColor;
				int32_t        m_GLtexture;
				int32_t        m_GLtexID;
			#endif
			ewol::Texture*                  m_resource;    //!< texture resources
			etk::Vector<Vector2D<float> >   m_coord;       //!< internal coord of the object
			etk::Vector<texCoord_ts>        m_coordTex;    //!< internal texture coordinate for every point
			#ifdef __VIDEO__OPENGL_ES_2
				etk::Vector<draw::Colorf>   m_coordColor;  //!< internal color of the different point
			#else
				etk::Vector<draw::Color>    m_coordColor;  //!< internal color of the different point
			#endif
		public:
			draw::Image* GetImage(void)
			{
				if (NULL == m_resource) {
					return NULL;
				}
				draw::Image& tmpImage = m_resource->Get();
				return &tmpImage;
			};
			void Flush(void)
			{
				if (NULL != m_resource) {
					m_resource->Flush();
				}
			}
	};
};

#endif

