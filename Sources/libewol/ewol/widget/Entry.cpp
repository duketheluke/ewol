/**
 *******************************************************************************
 * @file ewol/widget/Entry.cpp
 * @brief ewol entry widget system (Sources)
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


#include <etk/unicode.h>
#include <ewol/widget/Entry.h>
#include <ewol/oObject/OObject.h>
#include <ewol/widget/WidgetManager.h>
#include <ewol/ewol.h>


extern const char * const ewolEventEntryClick      = "ewol-Entry-click";
extern const char * const ewolEventEntryEnter      = "ewol-Entry-Enter";
extern const char * const ewolEventEntryModify     = "ewol-Entry-Modify";

const char * const ewolEventEntryCut               = "ewol-Entry-Cut";
const char * const ewolEventEntryCopy              = "ewol-Entry-Copy";
const char * const ewolEventEntryPaste             = "ewol-Entry-Paste";
const char * const ewolEventEntryClean             = "ewol-Entry-Clean";
const char * const ewolEventEntrySelect            = "ewol-Entry-Select";


#undef __class__
#define __class__	"Entry"


void ewol::Entry::Init(void)
{
	AddEventId(ewolEventEntryClick);
	AddEventId(ewolEventEntryEnter);
	AddEventId(ewolEventEntryModify);
	m_displayStartPosition = 0;
	m_displayCursorPos = 0;
	m_displayCursorPosSelection = 0;
	m_userSize = 50;
	m_borderSize = 2;
	m_paddingSize = 3;
	m_displayCursor = false;
	m_textColorFg = draw::color::black;
	
	m_textColorBg = draw::color::white;
	m_textColorBg.a = 0xAF;
	SetCanHaveFocus(true);
	ShortCutAdd("ctrl+w",       ewolEventEntryClean);
	ShortCutAdd("ctrl+x",       ewolEventEntryCut);
	ShortCutAdd("ctrl+c",       ewolEventEntryCopy);
	ShortCutAdd("ctrl+v",       ewolEventEntryPaste);
	ShortCutAdd("ctrl+a",       ewolEventEntrySelect, "ALL");
	ShortCutAdd("ctrl+shift+a", ewolEventEntrySelect, "NONE");
}

ewol::Entry::Entry(void)
{
	Init();
	m_data = "";
	UpdateTextPosition();
	MarkToRedraw();
}

ewol::Entry::Entry(etk::UString newData)
{
	Init();
	SetValue(newData);
	UpdateTextPosition();
	MarkToRedraw();
}


ewol::Entry::~Entry(void)
{
	
}


bool ewol::Entry::CalculateMinSize(void)
{
	int32_t minHeight = m_oObjectText.GetHeight();
	m_minSize.x = m_userSize;
	m_minSize.y = minHeight + 2*(m_borderSize + 2*m_paddingSize);
	UpdateTextPosition();
	MarkToRedraw();
	return true;
}


void ewol::Entry::SetValue(etk::UString newData)
{
	m_data = newData;
	m_displayCursorPos = m_data.Size();
	m_displayCursorPosSelection = m_displayCursorPos;
	EWOL_DEBUG("Set ... " << newData);
	MarkToRedraw();
}

etk::UString ewol::Entry::GetValue(void)
{
	return m_data;
}

void ewol::Entry::OnDraw(DrawProperty& displayProp)
{
	m_oObjectDecoration.Draw();
	m_oObjectText.Draw();
}

void ewol::Entry::OnRegenerateDisplay(void)
{
	if (true == NeedRedraw()) {
		m_oObjectDecoration.Clear();
		m_oObjectText.Clear();
		UpdateTextPosition();
		
		int32_t tmpSizeX = m_minSize.x;
		int32_t tmpSizeY = m_minSize.y;
		int32_t tmpOriginX = 0;
		int32_t tmpOriginY = (m_size.y - tmpSizeY) / 2;
		// no change for the text orogin : 
		int32_t tmpTextOriginX = m_borderSize + 2*m_paddingSize;
		int32_t tmpTextOriginY = tmpOriginY + m_borderSize + 2*m_paddingSize;
		
		if (true==m_userFill.x) {
			tmpSizeX = m_size.x;
		}
		if (true==m_userFill.y) {
			//tmpSizeY = m_size.y;
			tmpOriginY = 0;
			tmpTextOriginY = tmpOriginY + m_borderSize + 2*m_paddingSize;
		}
		tmpOriginX += m_paddingSize;
		tmpOriginY += m_paddingSize;
		tmpSizeX -= 2*m_paddingSize;
		tmpSizeY -= 2*m_paddingSize;
		
		
		Vector2D<float> textPos;
		textPos.x = tmpTextOriginX + m_displayStartPosition;
		textPos.y = tmpTextOriginY;
		clipping_ts drawClipping;
		drawClipping.x = 2*m_paddingSize + m_borderSize;
		drawClipping.y = 2*m_paddingSize + m_borderSize;
		drawClipping.w = m_size.x;// - (m_borderSize + 2*m_paddingSize);
		drawClipping.h = m_size.y;
		m_oObjectText.clippingSet(drawClipping);
		m_oObjectText.Text(textPos, m_data);
		m_oObjectText.clippingDisable();
		
		m_oObjectDecoration.SetColor(m_textColorBg);
		m_oObjectDecoration.Rectangle( tmpOriginX, tmpOriginY, tmpSizeX, tmpSizeY);
		m_oObjectDecoration.SetColor(m_textColorFg);
		m_oObjectDecoration.RectangleBorder( tmpOriginX, tmpOriginY, tmpSizeX, tmpSizeY, m_borderSize);
		int32_t pos1 = m_displayCursorPosSelection;
		int32_t pos2 = m_displayCursorPos;
		if(m_displayCursorPosSelection>m_displayCursorPos) {
			pos2 = m_displayCursorPosSelection;
			pos1 = m_displayCursorPos;
		}
		if(pos1!=pos2) {
			m_oObjectDecoration.clippingSet(drawClipping);
			etk::UString tmpDisplay = m_data.Extract(0, pos1);
			Vector2D<int32_t> minSize = m_oObjectText.GetSize(tmpDisplay);
			tmpDisplay = m_data.Extract(0, pos2);
			Vector2D<int32_t> maxSize = m_oObjectText.GetSize(tmpDisplay);
			
			int32_t XPos    = minSize.x + m_borderSize + 2*m_paddingSize + m_displayStartPosition;
			int32_t XPosEnd = maxSize.x + m_borderSize + 2*m_paddingSize + m_displayStartPosition;
			XPos    = etk_avg(m_borderSize + 2*m_paddingSize, XPos,    m_size.x - 2*m_paddingSize );
			XPosEnd = etk_avg(m_borderSize + 2*m_paddingSize, XPosEnd, m_size.x - 2*m_paddingSize );
			m_oObjectDecoration.SetColor(0x4444FFAA);
			m_oObjectDecoration.Rectangle( XPos, tmpTextOriginY, XPosEnd-XPos, maxSize.y);
			m_oObjectDecoration.clippingDisable();
		}
		if (true == m_displayCursor) {
			etk::UString tmpDisplay = m_data.Extract(0, m_displayCursorPos);
			Vector2D<int32_t> minSize = m_oObjectText.GetSize(tmpDisplay);
			int32_t XCursorPos = minSize.x + m_borderSize + 2*m_paddingSize + m_displayStartPosition;
			if (XCursorPos >= m_borderSize + 2*m_paddingSize) {
				m_oObjectDecoration.Line(XCursorPos, tmpTextOriginY, XCursorPos, tmpTextOriginY + minSize.y, 1);
			}
		}
	}
}

/**
 * @brief Change the cursor position with the curent position requested on the display
 * @param[in] pos Absolute position of the event
 * @note The display is automaticly requested when change apear.
 * @return ---
 */
void ewol::Entry::UpdateCursorPosition(Vector2D<float>& pos, bool selection)
{
	Vector2D<float> relPos = RelativePosition(pos);
	// try to find the new cursor position :
	etk::UString tmpDisplay = m_data.Extract(0, m_displayStartPosition);
	int32_t displayHidenSize = m_oObjectText.GetSize(tmpDisplay).x;
	//EWOL_DEBUG("hidenSize : " << displayHidenSize);
	int32_t newCursorPosition = -1;
	int32_t tmpTextOriginX = m_borderSize + 2*m_paddingSize;
	for (int32_t iii=0; iii<m_data.Size(); iii++) {
		tmpDisplay = m_data.Extract(0, iii);
		int32_t tmpWidth = m_oObjectText.GetSize(tmpDisplay).x - displayHidenSize;
		if (tmpWidth>=relPos.x-tmpTextOriginX) {
			newCursorPosition = iii;
			break;
		}
	}
	if (newCursorPosition == -1) {
		newCursorPosition = m_data.Size();
	}
	if (false == selection) {
		m_displayCursorPos = newCursorPosition;
		m_displayCursorPosSelection = m_displayCursorPos;
		MarkToRedraw();
	} else {
		if (m_displayCursorPos == m_displayCursorPosSelection) {
			m_displayCursorPosSelection = m_displayCursorPos;
		}
		m_displayCursorPos = newCursorPosition;
		MarkToRedraw();
	}
}

/**
 * @brief Remove the selected area
 * @note This request a regeneration of the display
 * @return ---
 */
void ewol::Entry::RemoveSelected(void)
{
	if (m_displayCursorPosSelection==m_displayCursorPos) {
		// nothing to cut ...
		return;
	}
	int32_t pos1 = m_displayCursorPosSelection;
	int32_t pos2 = m_displayCursorPos;
	if(m_displayCursorPosSelection>m_displayCursorPos) {
		pos2 = m_displayCursorPosSelection;
		pos1 = m_displayCursorPos;
	}
	// Remove data ...
	m_displayCursorPos = pos1;
	m_displayCursorPosSelection = pos1;
	m_data.Remove(pos1, pos2-pos1);
	MarkToRedraw();
}

/**
 * @brief Copy the selected data on the specify clipboard
 * @param[in] clipboardID Selected clipboard
 * @return ---
 */
void ewol::Entry::CopySelectionToClipBoard(ewol::clipBoard::clipboardListe_te clipboardID)
{
	if (m_displayCursorPosSelection==m_displayCursorPos) {
		// nothing to cut ...
		return;
	}
	int32_t pos1 = m_displayCursorPosSelection;
	int32_t pos2 = m_displayCursorPos;
	if(m_displayCursorPosSelection>m_displayCursorPos) {
		pos2 = m_displayCursorPosSelection;
		pos1 = m_displayCursorPos;
	}
	// Copy
	etk::UString tmpData = m_data.Extract(pos1, pos2);
	ewol::clipBoard::Set(clipboardID, tmpData);
}


/**
 * @brief Event on an input of this Widget
 * @param[in] type Type of the input (ewol::INPUT_TYPE_MOUSE/ewol::INPUT_TYPE_FINGER ...)
 * @param[in] IdInput Id of the current Input (PC : left=1, right=2, middle=3, none=0 / Tactil : first finger=1 , second=2 (only on this widget, no knowledge at ouside finger))
 * @param[in] typeEvent ewol type of event like EVENT_INPUT_TYPE_DOWN/EVENT_INPUT_TYPE_MOVE/EVENT_INPUT_TYPE_UP/EVENT_INPUT_TYPE_SINGLE/EVENT_INPUT_TYPE_DOUBLE/...
 * @param[in] pos Absolute position of the event
 * @return true the event is used
 * @return false the event is not used
 */
bool ewol::Entry::OnEventInput(ewol::inputType_te type, int32_t IdInput, eventInputType_te typeEvent, Vector2D<float> pos)
{
	//EWOL_DEBUG("Event on Entry ... type=" << (int32_t)type << " id=" << IdInput);
	if (1 == IdInput) {
		if (ewol::EVENT_INPUT_TYPE_SINGLE == typeEvent) {
			KeepFocus();
			GenerateEventId(ewolEventEntryClick);
			//nothing to do ...
			return true;
		} else if (ewol::EVENT_INPUT_TYPE_DOUBLE == typeEvent) {
			KeepFocus();
			// select word
			m_displayCursorPosSelection = m_displayCursorPos-1;
			// search forward
			for (int32_t iii=m_displayCursorPos; iii<=m_data.Size(); iii++) {
				if(iii==m_data.Size()) {
					m_displayCursorPos = iii;
					break;
				}
				if(!(    (    m_data[iii] >= 'a'
				           && m_data[iii] <= 'z')
				      || (    m_data[iii] >= 'A'
				           && m_data[iii] <= 'Z')
				      || (    m_data[iii] >= '0'
				           && m_data[iii] <= '9')
				      || m_data[iii] == '_'
				      || m_data[iii] == '-'
				  ) ) {
					m_displayCursorPos = iii;
					break;
				}
			}
			// search backward
			for (int32_t iii=m_displayCursorPosSelection; iii>=-1; iii--) {
				if(iii==-1) {
					m_displayCursorPosSelection = 0;
					break;
				}
				if(!(    (    m_data[iii] >= 'a'
				           && m_data[iii] <= 'z')
				      || (    m_data[iii] >= 'A'
				           && m_data[iii] <= 'Z')
				      || (    m_data[iii] >= '0'
				           && m_data[iii] <= '9')
				      || m_data[iii] == '_'
				      || m_data[iii] == '-'
				  ) ) {
					m_displayCursorPosSelection = iii+1;
					break;
				}
			}
			// Copy to clipboard Middle ...
			CopySelectionToClipBoard(ewol::clipBoard::CLIPBOARD_SELECTION);
			MarkToRedraw();
		} else if (ewol::EVENT_INPUT_TYPE_TRIPLE == typeEvent) {
			KeepFocus();
			m_displayCursorPosSelection = 0;
			m_displayCursorPos = m_data.Size();
		} else if (ewol::EVENT_INPUT_TYPE_DOWN == typeEvent) {
			KeepFocus();
			UpdateCursorPosition(pos);
			MarkToRedraw();
		} else if (ewol::EVENT_INPUT_TYPE_MOVE == typeEvent) {
			KeepFocus();
			UpdateCursorPosition(pos, true);
			MarkToRedraw();
		} else if (ewol::EVENT_INPUT_TYPE_UP == typeEvent) {
			KeepFocus();
			UpdateCursorPosition(pos, true);
			// Copy to clipboard Middle ...
			CopySelectionToClipBoard(ewol::clipBoard::CLIPBOARD_SELECTION);
			MarkToRedraw();
		}
	}
	else if(    ewol::INPUT_TYPE_MOUSE == type
	         && 2 == IdInput) {
		if(    typeEvent == ewol::EVENT_INPUT_TYPE_DOWN
		    || typeEvent == ewol::EVENT_INPUT_TYPE_MOVE
		    || typeEvent == ewol::EVENT_INPUT_TYPE_UP) {
			KeepFocus();
			// updatethe cursor position : 
			UpdateCursorPosition(pos);
		}
		// Paste current selection only when up button
		if (typeEvent == ewol::EVENT_INPUT_TYPE_UP) {
			KeepFocus();
			// middle button => past data...
			ewol::clipBoard::Request(ewol::clipBoard::CLIPBOARD_SELECTION);
		}
	}
	return false;
}


/**
 * @brief Event on the keybord (if no shortcut has been detected before).
 * @param[in] type of the event (ewol::EVENT_KB_TYPE_DOWN or ewol::EVENT_KB_TYPE_UP)
 * @param[in] unicodeValue key pressed by the user
 * @return true if the event has been used
 * @return false if the event has not been used
 */
bool ewol::Entry::OnEventKb(eventKbType_te typeEvent, uniChar_t unicodeData)
{
	if( typeEvent == ewol::EVENT_KB_TYPE_DOWN) {
		//EWOL_DEBUG("Entry input data ... : \"" << unicodeData << "\" " );
		//return GenEventInputExternal(ewolEventEntryEnter, -1, -1);
		// remove curent selected data ...
		RemoveSelected();
		if(    '\n' == unicodeData
		    || '\r' == unicodeData) {
			GenerateEventId(ewolEventEntryEnter, m_data);
			return true;
		} else if (0x7F == unicodeData) {
			// SUPPR :
			if (m_data.Size() > 0 && m_displayCursorPos<m_data.Size()) {
				m_data.Remove(m_displayCursorPos, 1);
				m_displayCursorPos = etk_max(m_displayCursorPos, 0);
				m_displayCursorPosSelection = m_displayCursorPos;
			}
		} else if (0x08 == unicodeData) {
			// DEL :
			if (m_data.Size() > 0 && m_displayCursorPos != 0) {
				m_data.Remove(m_displayCursorPos-1, 1);
				m_displayCursorPos--;
				m_displayCursorPos = etk_max(m_displayCursorPos, 0);
				m_displayCursorPosSelection = m_displayCursorPos;
			}
		} else if(unicodeData >= 20) {
			m_data.Add(m_displayCursorPos, unicodeData);
			m_displayCursorPos++;
			m_displayCursorPosSelection = m_displayCursorPos;
		}
		GenerateEventId(ewolEventEntryModify, m_data);
		MarkToRedraw();
		return true;
	}
	return false;
}


/**
 * @brief Event on the keyboard that is not a printable key (if no shortcut has been detected before).
 * @return true if the event has been used
 * @return false if the event has not been used
 */
bool ewol::Entry::OnEventKbMove(eventKbType_te typeEvent, eventKbMoveType_te moveTypeEvent)
{
	if(typeEvent == ewol::EVENT_KB_TYPE_DOWN) {
		switch (moveTypeEvent)
		{
			case EVENT_KB_MOVE_TYPE_LEFT:
				m_displayCursorPos--;
				break;
			case EVENT_KB_MOVE_TYPE_RIGHT:
				m_displayCursorPos++;
				break;
			case EVENT_KB_MOVE_TYPE_START:
				m_displayCursorPos = 0;
				break;
			case EVENT_KB_MOVE_TYPE_END:
				m_displayCursorPos = m_data.Size();
				break;
			default:
				return false;
		}
		m_displayCursorPos = etk_avg(0, m_displayCursorPos, m_data.Size());
		m_displayCursorPosSelection = m_displayCursorPos;
		MarkToRedraw();
		return true;
	}
	return false;
}

/**
 * @brief Event on a past event ==> this event is asynchronous due to all system does not support direct getting datas
 * @note : need to have focus ...
 * @param[in] mode Mode of data requested
 * @return ---
 */
void ewol::Entry::OnEventClipboard(ewol::clipBoard::clipboardListe_te clipboardID)
{
	// remove curent selected data ...
	RemoveSelected();
	// get current selection / Copy :
	etk::UString tmpData = Get(clipboardID);
	// add it on the current display :
	if (tmpData.Size() >= 0) {
		m_data.Add(m_displayCursorPos, &tmpData[0]);
		if (m_data.Size() == tmpData.Size()) {
			m_displayCursorPos = tmpData.Size();
		} else {
			m_displayCursorPos += tmpData.Size();
		}
		m_displayCursorPosSelection = m_displayCursorPos;
		MarkToRedraw();
	}
	GenerateEventId(ewolEventEntryModify, m_data);
}

/**
 * @brief Receive a message from an other EObject with a specific eventId and data
 * @param[in] CallerObject Pointer on the EObject that information came from
 * @param[in] eventId Message registered by this class
 * @param[in] data Data registered by this class
 * @return ---
 */
void ewol::Entry::OnReceiveMessage(ewol::EObject * CallerObject, const char * eventId, etk::UString data)
{
	ewol::Widget::OnReceiveMessage(CallerObject, eventId, data);
	if(eventId == ewolEventEntryClean) {
		m_data = "";
		m_displayStartPosition = 0;
		m_displayCursorPos = 0;
		m_displayCursorPosSelection = m_displayCursorPos;
		MarkToRedraw();
	} else if(eventId == ewolEventEntryCut) {
		CopySelectionToClipBoard(ewol::clipBoard::CLIPBOARD_STD);
		RemoveSelected();
		GenerateEventId(ewolEventEntryModify, m_data);
	} else if(eventId == ewolEventEntryCopy) {
		CopySelectionToClipBoard(ewol::clipBoard::CLIPBOARD_STD);
	} else if(eventId == ewolEventEntryPaste) {
		ewol::clipBoard::Request(ewol::clipBoard::CLIPBOARD_STD);
	} else if(eventId == ewolEventEntrySelect) {
		if(data == "ALL") {
			m_displayCursorPosSelection = 0;
			m_displayCursorPos = m_data.Size();
		} else {
			m_displayCursorPosSelection = m_displayCursorPos;
		}
		MarkToRedraw();
	}
}

void ewol::Entry::UpdateTextPosition(void)
{
	int32_t tmpSizeX = m_minSize.x;
	if (true==m_userFill.x) {
		tmpSizeX = m_size.x;
	}
	int32_t tmpUserSize = tmpSizeX - 2*(m_borderSize + 2*m_paddingSize);
	int32_t totalWidth = m_oObjectText.GetSize(m_data).x;
	if (totalWidth < tmpUserSize) {
		m_displayStartPosition = 0;
	} else {
		m_displayStartPosition = -totalWidth + tmpUserSize;
	}
}


void ewol::Entry::OnGetFocus(void)
{
	m_displayCursor = true;
	ewol::KeyboardShow();
	MarkToRedraw();
}

void ewol::Entry::OnLostFocus(void)
{
	m_displayCursor = false;
	ewol::KeyboardHide();
	MarkToRedraw();
}
