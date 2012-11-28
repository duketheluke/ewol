/**
 * @author Edouard DUPIN
 * 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * 
 * @license BSD v3 (see license file)
 */

#ifndef __EWOL_SCROLLED_WIDGET_H__
#define __EWOL_SCROLLED_WIDGET_H__

#include <etk/types.h>
#include <ewol/debug.h>
#include <ewol/widget/Widget.h>
#include <ewol/compositing/Compositing.h>

namespace widget {
	typedef enum {
		SCROLL_DISABLE,
		SCROLL_INIT,
		SCROLL_ENABLE_FINGER,     // Specific for touchpad
		SCROLL_ENABLE_HORIZONTAL, // Specific for mouse
		SCROLL_ENABLE_VERTICAL,   // Specific for mouse
		SCROLL_GREP_END_EVENT,
	}highSpeedMode_te;
	
	typedef enum {
		SCROLL_MODE_NORMAL, //!< No Zoom , can UP and down, left and right
		SCROLL_MODE_CENTER, //!< Zoom enable, no move left and right
		SCROLL_MODE_GAME, //!< Zoom enable, no move left and right
	} scrollingMode_te;
	class WidgetScrooled : public ewol::Widget
	{
		private:
			etk::Vector<ewol::Compositing*> m_listOObject;   //!< generic element to display...
			void    AddOObject(ewol::Compositing* newObject, int32_t pos=-1);
			void    ClearOObjectList(void);
		protected:
			etk::Vector2D<float>    m_originScrooled;
			etk::Vector2D<float>    m_maxSize;
			float                   m_limitScrolling;
		private:
			scrollingMode_te        m_scroollingMode; //!< mode of management of the scrooling
			float                   m_pixelScrolling;
			etk::Vector2D<float>    m_highSpeedStartPos;
			highSpeedMode_te        m_highSpeedMode;
			int32_t                 m_highSpeedButton;
			ewol::keyEvent::type_te m_highSpeedType;
		public:
			WidgetScrooled(void);
			virtual ~WidgetScrooled(void);
			// Derived function
			virtual const char * const GetObjectType(void) { return "EwolWidgetScrooled"; };
			// Derived function
			virtual void OnRegenerateDisplay(void);
			// Derived function
			virtual void OnDraw(ewol::DrawProperty& displayProp);
			// Derived function
			virtual bool OnEventInput(ewol::keyEvent::type_te type, int32_t IdInput, ewol::keyEvent::status_te typeEvent, etk::Vector2D<float> pos);
			// Derived function
			virtual void GenDraw(ewol::DrawProperty displayProp);
		protected:
			/**
			 * @brief For mouse event when we have a scrolling UP and dows, specify the number of pixel that we scrooled
			 * @param[in] nbPixel number of pixel scrolling
			 * @return ---
			 */
			void SetScrollingSize(float nbPixel) { m_pixelScrolling = nbPixel; };
			/**
			 * @brief Specify the mode of scrolling for this windows
			 * @param[in] newMode the selected mode for the scrolling...
			 * @return ---
			 */
			void ScroolingMode(scrollingMode_te newMode);
			/**
			 * @brief Set the specific mawimum size of the widget
			 * @param[in] localSize new Maximum size
			 * @return ---
			 */
			void SetMaxSize(etk::Vector2D<float>  localSize) { m_maxSize = localSize; };
			/**
			 * @brief Request a specific position for the scrolling of the current windows.
			 * @param[in] borderWidth Size of the border that requested the element might not to be
			 * @param[in] currentPosition Position that is requested to view
			 * @param[in] center True if the position might be at the center of the widget
			 * @return ---
			 */
			void SetScrollingPositionDynamic(etk::Vector2D<float>  borderWidth, etk::Vector2D<float>  currentPosition, bool center = false);
			/**
			 * @brief Set the scrolling limit when arriving at he end of the widget
			 * @param[in] poucentageLimit pourcent of the limit of view nothing in the widget when arriving at the end ...
			 * @return ---
			 */
			void SetLimitScrolling(float poucentageLimit) { m_limitScrolling = etk_avg(0.1, poucentageLimit,0.9); };
	};
	
};

#endif