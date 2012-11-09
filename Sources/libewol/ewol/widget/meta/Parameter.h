/**
 * @author Edouard DUPIN
 * 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * 
 * @license BSD v3 (see license file)
 */

#ifndef __EWOL_WIDGET_PARAMETER_H__
#define __EWOL_WIDGET_PARAMETER_H__

#include <etk/Types.h>
#include <etk/UString.h>
#include <ewol/Debug.h>
#include <ewol/widget/PopUp.h>
#include <ewol/widget/Button.h>
#include <ewol/widget/Entry.h>
#include <ewol/widget/Label.h>
#include <ewol/widget/CheckBox.h>
#include <ewol/widget/WSlider.h>
#include <ewol/widget/meta/ParameterList.h>

extern const char * const ewolEventParameterClose;

namespace ewol {
	class Parameter : public ewol::PopUp
	{
		public:
			Parameter(void);
			~Parameter(void);
			/**
			 * @brief Get the current Object type of the EObject
			 * @note In Embended platforme, it is many time no -rtti flag, then it is not possible to use dynamic cast ==> this will replace it
			 * @param[in] objectType type description
			 * @return true if the object is compatible, otherwise false
			 */
			virtual const char * const GetObjectType(void) { return "EwolParameter"; };
			/**
			 * @brief Receive a message from an other EObject with a specific eventId and data
			 * @param[in] CallerObject Pointer on the EObject that information came from
			 * @param[in] eventId Message registered by this class
			 * @param[in] data Data registered by this class
			 * @return ---
			 */
			virtual void OnReceiveMessage(ewol::EObject * CallerObject, const char * eventId, etk::UString data);
			/**
			 * @brief Inform object that an other object is removed ...
			 * @param[in] removeObject Pointer on the EObject remeved ==> the user must remove all reference on this EObject
			 * @note : Sub classes must call this class
			 * @return ---
			 */
			virtual void OnObjectRemove(ewol::EObject * removeObject);
			
			void SetTitle(etk::UString label);
			void MenuAdd(etk::UString label, etk::UString image, ewol::Widget* associateWidget);
			void MenuAddGroup(etk::UString label);
			void MenuClear(void);
			void MenuSeparator(void);
		private:
			int32_t               m_currentIdList;
			ewol::Label*          m_widgetTitle;
			ewol::Button*         m_widgetCancel;
			ewol::ParameterList*  m_paramList;
			ewol::WSlider*        m_wSlider;
	};
	
};

#endif
