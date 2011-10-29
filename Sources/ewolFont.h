/**
 *******************************************************************************
 * @file ewolFont.h
 * @brief ewol Font system (header)
 * @author Edouard DUPIN
 * @date 29/10/2011
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

#ifndef __EWOL_FONT_H__
#define __EWOL_FONT_H__

#include <etkTypes.h>
#include <ewolDebug.h>
#include <etkFile.h>


namespace ewol
{
	int32_t LoadFont(etk::File fontFileName);
	void    DrawText(double x, double y, const char * myString);
};

#endif

