/**
 * @author Edouard DUPIN
 * 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * 
 * @license APACHE v2.0 (see license file)
 */

#ifndef __EWOL_KEY_TYPE_H__
#define __EWOL_KEY_TYPE_H__

#include <ewol/debug.h>

namespace ewol {
	namespace key {
		/**
		 * @brief type of input : Note that the keyboard is not prevent due to the fact that data is too different
		 */
		enum type {
			typeUnknow = 0, //!< Unknow input Type
			typeMouse,      //!< Mouse type
			typeFinger,     //!< Finger type
			typeStylet,     //!< Stylet type
			typeCount       //!< number of types
		};
	};
	/**
	 * @brief Debug operator To display the curent element in a Human redeable information
	 */
	std::ostream& operator <<(std::ostream& _os, const enum ewol::key::type _obj);
};



#endif


