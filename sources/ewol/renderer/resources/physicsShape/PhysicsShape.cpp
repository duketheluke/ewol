/**
 * @author Edouard DUPIN
 * 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * 
 * @license BSD v3 (see license file)
 */
#include <ewol/debug.h>
#include <ewol/renderer/resources/physicsShape/PhysicsShape.h>
#include <ewol/renderer/resources/physicsShape/PhysicsBox.h>
#include <ewol/renderer/resources/physicsShape/PhysicsCapsule.h>
#include <ewol/renderer/resources/physicsShape/PhysicsCone.h>
#include <ewol/renderer/resources/physicsShape/PhysicsConvexHull.h>
#include <ewol/renderer/resources/physicsShape/PhysicsCylinder.h>
#include <ewol/renderer/resources/physicsShape/PhysicsSphere.h>


ewol::PhysicsShape* ewol::PhysicsShape::Create(const etk::UString& _name)
{
	ewol::PhysicsShape* tmpp = NULL;
	etk::UString name = _name.ToLower();
	if (name == "box") {
		tmpp = new ewol::PhysicsBox();
	} else if (name == "sphere") {
		tmpp = new ewol::PhysicsSphere();
	} else if (name == "cone") {
		tmpp = new ewol::PhysicsCone();
	} else if (name == "cylinder") {
		tmpp = new ewol::PhysicsCylinder();
	} else if (name == "capsule") {
		tmpp = new ewol::PhysicsCapsule();
	} else if (name == "convexhull") {
		tmpp = new ewol::PhysicsConvexHull();
	} else {
		EWOL_ERROR("Create an unknow element : '" << _name << "' availlable : [BOX,SPHERE,CONE,CYLINDER,CAPSULE,CONVEXHULL]");
		return NULL;
	}
	if (tmpp==NULL) {
		EWOL_ERROR("Allocation error for physical element : '" << _name << "'");
	}
	return tmpp;
}


bool ewol::PhysicsShape::Parse(const char* _line)
{
	if(0==strncmp(_line, "origin : ", 9) ) {
		sscanf(&_line[9], "%f %f %f", &m_origin.m_floats[0], &m_origin.m_floats[1], &m_origin.m_floats[2] );
		EWOL_DEBUG("                Origin=" << m_origin);
		return true;
	}
	if(0==strncmp(_line, "rotate : ", 9) ) {
		sscanf(&_line[9], "%f %f %f %f", &m_quaternion.m_floats[0], &m_quaternion.m_floats[1], &m_quaternion.m_floats[2], &m_quaternion.m_floats[3] );
		EWOL_DEBUG("                Rotate=" << m_quaternion);
		return true;
	}
	return false;
}

