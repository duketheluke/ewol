/**
 * @author Edouard DUPIN
 * 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * 
 * @license BSD v3 (see license file)
 */

#ifndef __EWOL_LAYER_H__
#define __EWOL_LAYER_H__

#include <etk/types.h>
#include <ewol/debug.h>
#include <ewol/widget/ContainerN.h>

namespace widget {
	class Layer : public widget::ContainerN
	{
		public:
			/**
			 * @brief Main call of recording the widget on the List of "widget named creator"
			 */
			static void Init(void);
			/**
			 * @brief Main call to unrecord the widget from the list of "widget named creator"
			 */
			static void UnInit(void);
		public:
			/**
			 * @brief Constructor
			 */
			Layer(void);
			/**
			 * @brief Desstructor
			 */
			virtual ~Layer(void);
		public:
			virtual const char * const GetObjectType(void) { return "Ewol::Layer"; };
	};
	
};

#endif
