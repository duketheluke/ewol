/**
 *******************************************************************************
 * @file ewol/widget/ButtonColor.h
 * @brief ewol Button select color widget system (header)
 * @author Edouard DUPIN
 * @date 04/03/2012
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

#ifndef __EWOL_BUTTON_COLOR_H__
#define __EWOL_BUTTON_COLOR_H__

#include <etk/Types.h>
#include <ewol/Debug.h>
#include <ewol/widget/Drawable.h>
#include <ewol/widget/Button.h>
#include <ewol/widget/ContextMenu.h>

extern const char * const ewolEventButtonColorChange;

namespace ewol {
	class ButtonColor :public ewol::Drawable
	{
		public:
			ButtonColor(void);
			ButtonColor(etk::UString newLabel);
			/**
			 * @brief Get the current Object type of the EObject
			 * @note In Embended platforme, it is many time no -rtti flag, then it is not possible to use dynamic cast ==> this will replace it
			 * @param[in] objectType type description
			 * @return true if the object is compatible, otherwise false
			 */
			virtual const char * const GetObjectType(void) { return "EwolButtonColor"; };
			void Init(void);
			virtual ~ButtonColor(void);
			virtual bool   CalculateMinSize(void);
			void           SetLabel(etk::UString newLabel);
			etk::UString   GetLabel(void) {return m_label;};
			void           SetValue(bool val);
			bool           GetValue(void);
			void           SetAlignement(textAlignement_te typeAlign);
			void           SetPadding(Vector2D<float>  newPadding);
		private:
			textAlignement_te    m_alignement;
			Vector2D<float>            m_padding;
			etk::UString         m_label;
			color_ts             m_textColorFg;    //!< Text color
			color_ts             m_textColorBg;    //!< Background color
			color_ts             m_selectedColor;  //!< user current selected Color
			ewol::ContextMenu*   m_widgetContextMenu;
		public:
			virtual void   OnRegenerateDisplay(void);
		public:
			/**
			 * @brief Event on an input of this Widget
			 * @param[in] type Type of the input (ewol::INPUT_TYPE_MOUSE/ewol::INPUT_TYPE_FINGER ...)
			 * @param[in] IdInput Id of the current Input (PC : left=1, right=2, middle=3, none=0 / Tactil : first finger=1 , second=2 (only on this widget, no knowledge at ouside finger))
			 * @param[in] typeEvent ewol type of event like EVENT_INPUT_TYPE_DOWN/EVENT_INPUT_TYPE_MOVE/EVENT_INPUT_TYPE_UP/EVENT_INPUT_TYPE_SINGLE/EVENT_INPUT_TYPE_DOUBLE/...
			 * @param[in] pos Absolute position of the event
			 * @return true the event is used
			 * @return false the event is not used
			 */
			virtual bool OnEventInput(ewol::inputType_te type, int32_t IdInput, eventInputType_te typeEvent, Vector2D<float>  pos);
			color_ts GetCurrentColor(void) { return m_selectedColor; };
			void     SetCurrentColor(color_ts color);
			/**
			 * @brief Receive a message from an other EObject with a specific eventId and data
			 * @param[in] CallerObject Pointer on the EObject that information came from
			 * @param[in] eventId Message registered by this class
			 * @param[in] data Data registered by this class
			 * @return ---
			 */
			virtual void OnReceiveMessage(ewol::EObject * CallerObject, const char * eventId, etk::UString data);
	};
	
	/**
	 * @brief Initilise the basic widget property ==> due to the android system
	 * @note all widget that have template might have this initializer ...
	 * @param ---
	 * @return ---
	 */
	void WIDGET_ButtonColorInit(void);
	
};

#endif
