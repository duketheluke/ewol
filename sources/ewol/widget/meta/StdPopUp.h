/**
 * @author Edouard DUPIN
 * 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * 
 * @license BSD v3 (see license file)
 */

#ifndef __EWOL_STD_POP_UP_H__
#define __EWOL_STD_POP_UP_H__

#include <ewol/widget/PopUp.h>
#include <ewol/widget/Label.h>
#include <ewol/widget/Button.h>
#include <ewol/widget/Sizer.h>
namespace ewol {
	namespace widget {
		/**
		 * @brief The std pop up widget is a siple message widget to notify user of some simple things, like:
		 *
		 * [pre]
		 * 	+---------------------------------+---+---+---+
		 * 	| Windows name...                 | _ | O | X |
		 * 	+---------------------------------+---+---+---+
		 * 	|                                             |
		 * 	|                                             |
		 * 	|                                             |
		 * 	|            +-------------------+            |
		 * 	|            | Erreur:           |            |
		 * 	|            |                   |            |
		 * 	|            | Message to diplay |            |
		 * 	|            | to user           |            |
		 * 	|            |                   |            |
		 * 	|            |             Close |            |
		 * 	|            +-------------------+            |
		 * 	|                                             |
		 * 	|                                             |
		 * 	|                                             |
		 * 	+---------------------------------------------+
		 * [/pre]
		 */
		class StdPopUp : public ewol::Widget::PopUp {
			public:
				/**
				 * @brief std-pop-up constructor.
				 */
				StdPopUp();
				/**
				 * @brief std-pop-up destructor.
				 */
				~StdPopUp();
			protected:
				ewol::object::Shared<ewol::Widget::Label> m_title; //!< Title Label widget
			public:
				/**
				 * @brief Set the title string.
				 * @param[in] _text Decorated text to diplay in title.
				 */
				void setTitle(const std::string& _text);
			protected:
				ewol::object::Shared<ewol::Widget::Label> m_comment; //!< Comment label widget
			public:
				/**
				 * @brief Set the commentary string.
				 * @param[in] _text Decorated text to diplay in Comment.
				 */
				void setComment(const std::string& _text);
			protected:
				ewol::object::Shared<ewol::Widget::Sizer> m_subBar; //!< subwidget bar containing all the button.
			public:
				/**
				 * @brief Add a buttom button.
				 * @param[in] _text Decorated text to diplay in button.
				 */
				ewol::object::Shared<ewol::Widget::Button> addButton(const std::string& _text, bool _autoExit=false);
			public: // Derived function
				virtual void onObjectRemove(ewol::object::Shared<ewol::Object> _removeObject);
				virtual void onReceiveMessage(const ewol::object::Message& _msg);
		};
	};
};

#endif
