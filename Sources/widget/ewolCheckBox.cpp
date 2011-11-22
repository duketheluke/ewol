/**
 *******************************************************************************
 * @file ewolCheckBox.cpp
 * @brief ewol check-box widget system (Sources)
 * @author Edouard DUPIN
 * @date 07/11/2011
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


#include <widget/ewolCheckBox.h>

#include <ewolOObject.h>
#include <ewolWidgetManager.h>


const char * ewolEventCheckBoxClicked    = "ewol CheckBox Clicked";

#undef __class__
#define __class__	"ewol::CheckBox"


void ewol::CheckBox::Init(void)
{
	m_textColorFg.red   = 0.0;
	m_textColorFg.green = 0.0;
	m_textColorFg.blue  = 0.0;
	m_textColorFg.alpha = 1.0;
	
	m_textColorBg.red   = 1.0;
	m_textColorBg.green = 1.0;
	m_textColorBg.blue  = 1.0;
	m_textColorBg.alpha = 1.0;
	m_value = false;
	SetCanHaveFocus(true);
}

ewol::CheckBox::CheckBox(void)
{
	m_label = "No Label";
	Init();
}

ewol::CheckBox::CheckBox(etk::String newLabel)
{
	m_label = newLabel;
	Init();
}


ewol::CheckBox::~CheckBox(void)
{
	
}

bool ewol::CheckBox::CalculateMinSize(void)
{
	int32_t fontId = GetDefaultFontId();
	int32_t minWidth = ewol::GetWidth(fontId, m_label.c_str());
	int32_t minHeight = ewol::GetHeight(fontId);
	m_minSize.x = 25+minWidth;
	m_minSize.y = etk_max(25, minHeight);
	return true;
}


void ewol::CheckBox::SetLabel(etk::String newLabel)
{
	m_label = newLabel;
}

void ewol::CheckBox::SetValue(bool val)
{
	if (m_value == val) {
		return;
	}
	m_value = val;
	OnRegenerateDisplay();
}

bool ewol::CheckBox::GetValue(void)
{
	return m_value;
}


void ewol::CheckBox::OnRegenerateDisplay(void)
{
	// clean the object list ...
	ClearOObjectList();
	
	int32_t borderWidth = 2;
	
	
	ewol::OObject2DText * tmpText = new ewol::OObject2DText("", -1, m_textColorFg);
	int32_t fontId = GetDefaultFontId();
	int32_t fontHeight = ewol::GetHeight(fontId);
	//int32_t fontWidth = ewol::GetWidth(fontId, m_label.c_str());
	int32_t posy = (m_size.y - fontHeight - 6)/2 + 3;
	//int32_t posx = (m_size.x - fontWidth - 6)/2 + 25;
	tmpText->Text(25, posy+3, m_label.c_str());
	
	
	ewol::OObject2DColored * tmpOObjects = new ewol::OObject2DColored;
	tmpOObjects->SetColor(m_textColorBg);
	tmpOObjects->Rectangle( 2.5, 2.5, 20, 20);
	tmpOObjects->SetColor(m_textColorFg);
	tmpOObjects->RectangleBorder( 2.5, 2.5, 20, 20, borderWidth);
	if (m_value) {
		tmpOObjects->Line( 2.5, 2.5, 22.5, 22.5, borderWidth);
		tmpOObjects->Line( 2.5, 22.5, 22.5, 2.5, borderWidth);
	}
	
	AddOObject(tmpOObjects, "Decoration");
	AddOObject(tmpText, "Text");

	// Regenerate the event Area:
	EventAreaRemoveAll();
	coord origin;
	coord size;
	origin.x = 3.0;
	origin.y = 3.0;
	size.x = m_size.x-6;
	size.y = m_size.y-6;
	AddEventArea(origin, size, FLAG_EVENT_INPUT_1 | FLAG_EVENT_INPUT_CLICKED_ALL, ewolEventCheckBoxClicked);

}

/*
bool ewol::CheckBox::OnEventInput(int32_t IdInput, eventInputType_te typeEvent, etkFloat_t x, etkFloat_t y)
{
	EWOL_DEBUG("Event on BT ...");
	return true;
}
*/

bool ewol::CheckBox::OnEventArea(const char * generateEventId, etkFloat_t x, etkFloat_t y)
{
	bool eventIsOK = false;
	//EWOL_DEBUG("Receive event : \"" << generateEventId << "\"");
	if(ewolEventCheckBoxClicked == generateEventId) {
		ewol::widgetManager::FocusKeep(this);
		EWOL_INFO("CB pressed ... " << m_label);
		if(true == m_value) {
			m_value = false;
		} else {
			m_value = true;
		}
		OnRegenerateDisplay();
		eventIsOK = true;
	}
	return eventIsOK;
}

bool ewol::CheckBox::OnEventKb(eventKbType_te typeEvent, char UTF8_data[UTF8_MAX_SIZE])
{
	//EWOL_DEBUG("BT PRESSED : \"" << UTF8_data << "\" size=" << strlen(UTF8_data));
	if(    UTF8_data != NULL
	    && typeEvent == ewol::EVENT_KB_TYPE_DOWN
	    && (    UTF8_data[0] == '\r'
	         || UTF8_data[0] == ' ')
	       ) {
		return OnEventArea(ewolEventCheckBoxClicked, -1, -1);
	}
	return false;
}

