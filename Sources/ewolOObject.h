/**
 *******************************************************************************
 * @file ewolOObject.h
 * @brief ewol OpenGl Object system (header)
 * @author Edouard DUPIN
 * @date 24/10/2011
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

#ifndef __EWOL_O_OBJECT_H__
#define __EWOL_O_OBJECT_H__

#include <etkTypes.h>
#include <ewolDebug.h>
#include <etkVectorType.h>

namespace ewol {
	extern "C" {
		typedef struct {
			float x;
			float y;
			float z;
		}coord3D_ts;
		typedef struct {
			float x;
			float y;
		}coord2D_ts;
		typedef struct {
			float u;
			float v;
		}texCoord_ts;
		typedef struct {
			float red;
			float green;
			float blue;
			float alpha;
		}color_ts;
		typedef struct {
			int32_t f;
			int32_t s;
			int32_t t;
		}linkCoord_ts;
	};
	
	class OObject
	{
		public:
			OObject(void) { m_name=""; };
			virtual ~OObject(void) {};
		public:
			virtual void Draw(void) = 0;
			void SetName(etk::String & name)
			{
				m_name = name;
			}
			void SetName(const char * name)
			{
				if (NULL != name) {
					m_name = name;
				}
			}
			etk::String GetName(void)
			{
				return m_name;
			}
		private:
			etk::String m_name;
	};
	
	class OObject2DColored :public ewol::OObject
	{
		public:
			OObject2DColored(void) {};
			virtual ~OObject2DColored(void) {};
		public:
			virtual void Draw(void);
		protected:
			etk::VectorType<coord2D_ts>   m_coord;       //!< internal coord of the object
			etk::VectorType<color_ts>     m_coordColor;  //!< internal color of the different point
			//etk::VectorType<linkCoord_ts> m_linkCoord;   //!< internal link between point to generate triangle
		public:
			void Rectangle(float x, float y, float w, float h, float red, float green, float blue, float alpha);
	};
	/*
	class OObjectFile :public ewol::OObject
	{
		public:
			OObjectFile(etk::File fileName) {};
			virtual ~OObject2DColored(void) {};
		public:
			void Draw(void);
			bool Save(etk::File fileName) { return false; };
		protected:
			etk::VectorType<OObject*>   m_listsubObject;   //!< an object file contain more than one object...
			bool                        m_isBinaryFile;    //!< to know th mode of saving the file
	};
	*/
	
	/*
	class OObject2DTextured :public ewol::OObject
	{
		public:
			OObject2DTextured(void) {};
			virtual ~OObject2DTextured(void) {};
		public:
			virtual void Draw(void);
		protected:
			uint32_t                      m_textureId;   //!< texture internal ID
			etk::VectorType<coord2D_ts>   m_coord;       //!< internal coord of the object
			etk::VectorType<texCoord_ts>  m_coordTex;    //!< internal texture coordinate for every point
			etk::VectorType<linkCoord_ts> m_linkCoord;   //!< internal link between point to generate triangle
	};
	*/
};

#endif

