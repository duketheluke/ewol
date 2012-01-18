/**
 *******************************************************************************
 * @file ewol/theme/EolBaseRect.cpp
 * @brief basic ewol theme eol file basic element type=rectangle (Sources)
 * @author Edouard DUPIN
 * @date 23/11/2011
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

#include <ewol/theme/Theme.h>
#include <ewol/theme/EolBaseRect.h>

#undef __class__
#define __class__	"ewol::theme::EolBaseRect"



ewol::theme::EolBaseRect::EolBaseRect(void)
{
	//EWOL_DEBUG("new element Base : Rectangle ...");
}

ewol::theme::EolBaseRect::~EolBaseRect(void)
{
	
}


void ewol::theme::EolBaseRect::Parse(TiXmlNode * pNode)
{
	const char * tmp = pNode->ToElement()->Attribute("position");
	if (NULL == tmp) {
		m_position.x=0;
		m_position.y=0;
	} else {
		double xxx,yyy;
		// optimize for multiple type input ...
		sscanf(tmp, "%lf;%lf", &xxx,&yyy);
		m_position.x=xxx;
		m_position.y=yyy;
	}
	tmp = pNode->ToElement()->Attribute("size");
	if (NULL == tmp) {
		m_size.x=0;
		m_size.y=0;
	} else {
		double xxx,yyy;
		// optimize for multiple type input ...
		sscanf(tmp, "%lf;%lf", &xxx,&yyy);
		m_size.x=xxx;
		m_size.y=yyy;
	}
	tmp = pNode->ToElement()->Attribute("thickness");
	if (NULL == tmp) {
		m_thickness=0.01;
	} else {
		double tmpVal;
		// optimize for multiple type input ...
		sscanf(tmp, "%lf", &tmpVal);
		m_thickness=tmpVal;
	}
	m_colorBG     = pNode->ToElement()->Attribute("colorBG");
	m_colorBorder = pNode->ToElement()->Attribute("colorBorder");
	EWOL_DEBUG("(l " << pNode->Row() << ")     Parse Base Element : \"rect\" : pos(" << m_position.x << "," << m_position.y << ") "
	           << "size(" << m_size.x << "," << m_size.y << ") colorBG=\"" << m_colorBG << "\" colorBorder=\"" << m_colorBorder << "\" thickness=" << m_thickness);
}

void ewol::theme::EolBaseRect::Generate(const ewol::theme::Theme * myTheme, const ewol::theme::EolElement * myElement, ewol::OObject2DColored & newObject, etkFloat_t posX, etkFloat_t posY, etkFloat_t sizeX, etkFloat_t sizeY)
{
	if (m_colorBG != "") {
		color_ts selectedColorBG = {1.0, 1.0, 1.0, 1.0};
		bool res = false;
		if (NULL != myElement) {
			res = myElement->GetColor(m_colorBG, selectedColorBG);
		}
		// try from theme if not existed
		if(    false == res
		    && NULL != myTheme ) {
			myElement->GetColor(m_colorBG, selectedColorBG);
		}
		newObject.SetColor(selectedColorBG);
		newObject.Rectangle(posX + m_position.x*sizeX, posY + m_position.y*sizeY, m_size.x*sizeX, m_size.y*sizeY);
	}
	if (m_colorBorder != "") {
		color_ts selectedColorBorder = {0.0, 0.0, 0.0, 1.0};
		bool res = false;
		// try get color for current element
		if (NULL != myElement) {
			res = myElement->GetColor(m_colorBorder, selectedColorBorder);
		}
		// try from theme if not existed
		if(    false == res
		    && NULL != myTheme ) {
			myElement->GetColor(m_colorBorder, selectedColorBorder);
		}
		newObject.SetColor(selectedColorBorder);
		newObject.RectangleBorder(posX + m_position.x*sizeX, posY + m_position.y*sizeY, m_size.x*sizeX, m_size.y*sizeY, m_thickness*(sizeX+sizeY)/2);
	}
}


