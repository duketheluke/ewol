/**
 * @author Edouard DUPIN
 * 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * 
 * @license BSD v3 (see license file)
 */

#include <ewol/widget/Scene.h>
#include <math.h>

#include <ewol/oObject/OObject.h>
#include <ewol/widget/WidgetManager.h>
#include <ewol/openGL/openGL.h>

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
	m_isRunning = true;
	SetCanHaveFocus(true);
	PeriodicCallSet(true);
	m_lastCallTime = -1;
	m_zoom = 1.0;
}


ewol::Scene::~Scene(void)
{
	
}


void ewol::Scene::OnRegenerateDisplay(void)
{
	if (true == NeedRedraw()) {
		// clean elements
		for (int32_t iii=0; iii<m_sceneElement.animated.Size(); iii++) {
			if (NULL != m_sceneElement.animated[iii]) {
				m_sceneElement.animated[iii]->Clear();
			}
		}
		for (int32_t jjj=0; jjj<MAX_GROUP_NUMBER; jjj++) {
			for (int32_t iii=0; iii<m_sceneElement.listAnimatedElements[jjj].Size(); iii++) {
				if (NULL != m_sceneElement.listAnimatedElements[jjj][iii]) {
					// find an empty slot ...
					m_sceneElement.listAnimatedElements[jjj][iii]->Draw();
				}
			}
		}
	}
}

/**
 * @brief Common widget drawing function (called by the drawing thread [Android, X11, ...])
 * @param ---
 * @return ---
 */
//TODO : Il y a un bug : seg fault ... je ne sais pas trop ou ...
void ewol::Scene::OnDraw(DrawProperty& displayProp)
{
	//EWOL_ERROR(" On draw : " << m_currentDrawId);
	// draw background :
	// TODO : ...
	if (NULL != m_sceneElement.background) {
		m_sceneElement.background->Draw();
	}
	//background
	// draw elements
	for (int32_t iii=0; iii<m_sceneElement.animated.Size(); iii++) {
		if (NULL != m_sceneElement.animated[iii]) {
			m_sceneElement.animated[iii]->Draw();
		}
	}
}


void ewol::Scene::PeriodicCall(int64_t localTime)
{
	// First time : 
	if (-1 == m_lastCallTime) {
		m_lastCallTime = localTime;
	}
	// check if the processing is availlable
	if (false == m_isRunning) {
		m_lastCallTime = localTime;
		MarkToRedraw();
		return;
	}
	// cut the processing in small slot of time to prevent error in the real-time Display (Android call us between 30 to 60 fps)
	int32_t deltaTime = (int32_t) (localTime - m_lastCallTime);
	//EWOL_DEBUG(" currentTime = " << localTime << " last=" << m_lastCallTime << "  delta=" << deltaTime);
	while (deltaTime >= CYCLIC_CALL_PERIODE_US) {
		//EWOL_DEBUG(" process = " << CYCLIC_CALL_PERIODE_US);
		m_lastCallTime += CYCLIC_CALL_PERIODE_US;
		deltaTime -= CYCLIC_CALL_PERIODE_US;
		ScenePeriodicCall(m_lastCallTime, CYCLIC_CALL_PERIODE_US);
		//EWOL_ERROR("Periodic Call ... " << localTime);
		for (int32_t jjj=0; jjj<MAX_GROUP_NUMBER; jjj++) {
			for (int32_t iii=0; iii<m_sceneElement.listAnimatedElements[jjj].Size(); iii++) {
				if (NULL != m_sceneElement.listAnimatedElements[jjj][iii]) {
					if(true == m_sceneElement.listAnimatedElements[jjj][iii]->IsEnable() ) {
						// check if the element request an auto Kill ...
						if (true == m_sceneElement.listAnimatedElements[jjj][iii]->Process(m_lastCallTime, CYCLIC_CALL_PERIODE_US) ) {
							m_sceneElement.RmElement(jjj, iii);
						}
					}
				}
			}
		}
	}
	MarkToRedraw();
}


void ewol::Scene::GenDraw(DrawProperty displayProp)
{

	ewol::openGL::Push();
	// here we invert the reference of the standard OpenGl view because the reference in the common display is Top left and not buttom left
	glViewport( m_origin.x,
	            m_origin.y,
	            m_size.x,
	            m_size.y);
	float ratio = m_size.x / m_size.y;
		m_zoom = 1.0/1000.0;
	//EWOL_INFO("ratio : " << ratio);
		etk::Matrix4 tmpProjection;
		
		if (ratio >= 1.0) {
			tmpProjection = etk::matrix::Perspective(-ratio, ratio, -1, 1, -1, 1);
		} else {
			ratio = 1.0/ratio;
			tmpProjection = etk::matrix::Perspective(-1, 1, -ratio, ratio, -1, 1);
		}
		etk::Matrix4 tmpScale = etk::matrix::Scale(m_zoom, m_zoom, m_zoom);
		etk::Matrix4 tmpMat = tmpProjection * tmpScale;
		// set internal matrix system :
		ewol::openGL::SetMatrix(tmpMat);
	// Clear the screen with transparency ...
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Call the widget drawing methode
	OnDraw(displayProp);
	
	ewol::openGL::Pop();
}


etk::Vector2D<float> ewol::Scene::RelativePosition(etk::Vector2D<float>  pos)
{
	// Remove origin of the widget
	pos.x -= m_origin.x;
	pos.y -= m_origin.y;
	// move the position at the center (openGl system
	pos.x -= m_size.x/2;
	pos.y -= m_size.y/2;
	// scale the position with the ratio display of the screen
	float ratio = m_size.x / m_size.y;
	if (ratio >= 1.0) {
		pos.x /= m_size.x;
		pos.x *= ratio;
		pos.y /= m_size.y;
	} else {
		ratio = 1.0/ratio;
		pos.x /= m_size.x;
		pos.y /= m_size.y;
		pos.y *= ratio;
	}
	// integrate zoom
	pos.x /= m_zoom;
	pos.y /= m_zoom;
	// all the position are half the size due to the fact -1 --> 1
	pos.x *= 2;
	pos.y *= 2;
	
	return pos;
};

