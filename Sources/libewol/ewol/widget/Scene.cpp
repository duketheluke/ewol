/**
 *******************************************************************************
 * @file ewol/widget/Scene.cpp
 * @brief ewol Scene widget system (Sources)
 * @author Edouard DUPIN
 * @date 01/04/2012
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

#include <ewol/widget/Scene.h>
#include <math.h>

#include <ewol/OObject.h>
#include <ewol/WidgetManager.h>

/**
 * @brief Initilise the basic widget property ==> due to the android system
 * @note all widget that have template might have this initializer ...
 * @param ---
 * @return ---
 */
void ewol::WIDGET_SceneInit(void)
{
	
}

#undef __class__
#define __class__	"Scene"

ewol::Scene::Scene(void)
{
	SetCanHaveFocus(true);
}


ewol::Scene::~Scene(void)
{
	
}

//!< EObject name :
extern const char * const ewol::TYPE_EOBJECT_WIDGET_SCENE = "Scene";

/**
 * @brief Check if the object has the specific type.
 * @note In Embended platforme, it is many time no -rtti flag, then it is not possible to use dynamic cast ==> this will replace it
 * @param[in] objectType type of the object we want to check
 * @return true if the object is compatible, otherwise false
 */
bool ewol::Scene::CheckObjectType(const char * const objectType)
{
	if (NULL == objectType) {
		EWOL_ERROR("check error : \"" << ewol::TYPE_EOBJECT_WIDGET_SCENE << "\" != NULL(pointer) ");
		return false;
	}
	if (objectType == ewol::TYPE_EOBJECT_WIDGET_SCENE) {
		return true;
	} else {
		if(true == ewol::WidgetScrooled::CheckObjectType(objectType)) {
			return true;
		}
		EWOL_ERROR("check error : \"" << ewol::TYPE_EOBJECT_WIDGET_SCENE << "\" != \"" << objectType << "\"");
		return false;
	}
}

/**
 * @brief Get the current Object type of the EObject
 * @note In Embended platforme, it is many time no -rtti flag, then it is not possible to use dynamic cast ==> this will replace it
 * @param[in] objectType type description
 * @return true if the object is compatible, otherwise false
 */
const char * const ewol::Scene::GetObjectType(void)
{
	return ewol::TYPE_EOBJECT_WIDGET_SCENE;
}



void ewol::Scene::OnRegenerateDisplay(void)
{
	if (true == NeedRedraw()) {
	}
}
