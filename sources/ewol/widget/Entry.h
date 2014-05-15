/**
 * @author Edouard DUPIN
 * 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * 
 * @license BSD v3 (see license file)
 */

#ifndef __EWOL_ENTRY_H__
#define __EWOL_ENTRY_H__

#include <etk/types.h>
#include <etk/RegExp.h>
#include <ewol/debug.h>
#include <ewol/compositing/Text.h>
#include <ewol/compositing/Drawing.h>
#include <ewol/compositing/Shaper.h>
#include <ewol/widget/Widget.h>
#include <etk/Color.h>
#include <ewol/widget/Manager.h>

namespace ewol {
	namespace widget {
		/**
		 * @ingroup ewolWidgetGroup
		 * @brief Entry box display :
		 *
		 * ~~~~~~~~~~~~~~~~~~~~~~
		 * 	----------------------------------------------
		 * 	| Editable Text                              |
		 * 	----------------------------------------------
		 * ~~~~~~~~~~~~~~~~~~~~~~
		 */
		class Entry : public ewol::Widget {
			public:
				// Event list of properties
				static const char * const eventClick;
				static const char * const eventEnter;
				static const char * const eventModify; // return in the data the new string inside it ...
				// Config list of properties
				static const char* const configMaxChar;
				static const char* const configRegExp;
				static const char* const configEmptyMessage;
				static const char* const configValue;
			public:
				static void init(ewol::object::Shared<ewol::Widget::Manager> _widgetManager);
			private:
				ewol::compositing::Shaper m_shaper;
				int32_t m_colorIdTextFg; //!< color property of the text foreground
				int32_t m_colorIdTextBg; //!< color property of the text background
				int32_t m_colorIdCursor; //!< color property of the text cursor
				int32_t m_colorIdSelection; //!< color property of the text selection
				ewol::compositing::Text m_text; //!< text display m_text
			public:
				/**
				 * @brief Contuctor
				 * @param[in] _newData The USting that might be set in the Entry box (no event generation!!)
				 */
				Entry(std::string _newData = "");
				/**
				 * @brief Destuctor
				 */
				virtual ~Entry();
			
			private:
				std::string m_data; //!< sting that must be displayed
			protected:
				/**
				 * @brief internal check the value with RegExp checking
				 * @param[in] _newData The new string to display
				 */
				void setInternalValue(const std::string& _newData);
			public:
				/**
				 * @brief set a new value on the entry.
				 * @param[in] _newData the new string to display.
				 */
				void setValue(const std::string& _newData);
				/**
				 * @brief get the current value in the entry
				 * @return The current display value
				 */
				std::string getValue() const {
					return m_data;
				};
			
			private:
				int32_t m_maxCharacter; //!< number max of xharacter in the list
			public:
				/**
				 * @brief Limit the number of Unicode character in the entry
				 * @param[in] _nbMax Number of max character set in the List (0x7FFFFFFF for no limit)
				 */
				void setMaxChar(int32_t _nbMax);
				/**
				 * @brief Limit the number of Unicode character in the entry
				 * @return Number of max character set in the List.
				 */
				int32_t getMaxChar() const {
					return m_maxCharacter;
				};
			private:
				etk::RegExp<std::string> m_regExp; //!< regular expression to limit the input of an entry
			public:
				/**
				 * @brief Limit the input entry at a regular expression... (by default it is "*")
				 * @param _expression New regular expression
				 */
				void setRegExp(const std::string& _expression);
				/**
				 * @brief get the regualar expression limitation
				 * @param The regExp string
				 */
				std::string getRegExp() const {
					return m_regExp.getRegExp();
				};
			private:
				bool m_needUpdateTextPos; //!< text position can have change
				int32_t m_displayStartPosition; //!< ofset in pixel of the display of the UString
				bool m_displayCursor; //!< Cursor must be display only when the widget has the focus
				int32_t m_displayCursorPos; //!< Cursor position in number of Char
				int32_t m_displayCursorPosSelection; //!< Selection position end (can be befor or after cursor and == m_displayCursorPos chan no selection availlable
			protected:
				/**
				 * @brief informe the system thet the text change and the start position change
				 */
				virtual void markToUpdateTextPosition();
				/**
				 * @brief update the display position start  == > depending of the position of the Cursor and the size of the Data inside
				 * @change m_displayStartPosition < ==  updated
				 */
				virtual void updateTextPosition();
				/**
				 * @brief change the cursor position with the curent position requested on the display
				 * @param[in] _pos Absolute position of the event
				 * @note The display is automaticly requested when change apear.
				 */
				virtual void updateCursorPosition(const vec2& _pos, bool _Selection=false);
			
			public:
				/**
				 * @brief Copy the selected data on the specify clipboard
				 * @param[in] _clipboardID Selected clipboard
				 */
				virtual void copySelectionToClipBoard(enum ewol::context::clipBoard::clipboardListe _clipboardID);
				/**
				 * @brief remove the selected area
				 * @note This request a regeneration of the display
				 */
				virtual void removeSelected();
			private:
				std::string m_textWhenNothing; //!< Text to display when nothing in in the entry (decorated text...)
			public:
				/**
				 * @brief set The text displayed when nothing is in the entry.
				 * @param _text Text to display when the entry box is empty (this text can be decorated).
				 */
				void setEmptyText(const std::string& _text);
				/**
				 * @brief get The text displayed when nothing is in the entry.
				 * @return Text display when nothing
				 */
				const std::string& getEmptyText() const {
					return m_textWhenNothing;
				};
			public: // Derived function
				virtual void onRegenerateDisplay();
				virtual bool onEventInput(const ewol::event::Input& _event);
				virtual bool onEventEntry(const ewol::event::Entry& _event);
				virtual void onReceiveMessage(const ewol::object::Message& _msg);
				virtual void onEventClipboard(enum ewol::context::clipBoard::clipboardListe _clipboardID);
				virtual void calculateMinMaxSize();
			protected: // Derived function
				virtual void onDraw();
				virtual void onGetFocus();
				virtual void onLostFocus();
				virtual void changeStatusIn(int32_t _newStatusId);
				virtual void periodicCall(const ewol::event::Time& _event);
				virtual bool onSetConfig(const ewol::object::Config& _conf);
				virtual bool onGetConfig(const char* _config, std::string& _result) const;
		};
	};
};

#endif
