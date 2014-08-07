/**
 * @author Edouard DUPIN
 * 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * 
 * @license BSD v3 (see license file)
 */

#include <ewol/widget/Slider.h>

#include <ewol/widget/Manager.h>

const char * const ewol::widget::Slider::eventChange = "change";

#undef __class__
#define __class__ "Slider"

const int32_t dotRadius = 6;

ewol::widget::Slider::Slider() {
	addObjectType("ewol::widget::Slider");
	addEventId(eventChange);
	
	m_value = 0;
	m_min = 0;
	m_max = 10;
	
	m_textColorFg = etk::color::black;
	
	m_textColorBg = etk::color::black;
	m_textColorBg.setA(0x3F);
	setCanHaveFocus(true);
	// Limit event at 1:
	setMouseLimit(1);
}

void ewol::widget::Slider::init() {
	ewol::Widget::init();
}

ewol::widget::Slider::~Slider() {
	
}

void ewol::widget::Slider::calculateMinMaxSize() {
	vec2 minTmp = m_userMinSize.getPixel();
	m_minSize.setValue(std::max(minTmp.x(), 40.0f),
	                   std::max(minTmp.y(), dotRadius*2.0f) );
	markToRedraw();
}

void ewol::widget::Slider::setValue(int32_t _val) {
	m_value = std::max(std::min(_val, m_max), m_min);
	markToRedraw();
}

int32_t ewol::widget::Slider::getValue() {
	return m_value;
}

void ewol::widget::Slider::setMin(int32_t _val) {
	m_min = _val;
	m_value = std::max(std::min(m_value, m_max), m_min);
	markToRedraw();
}

void ewol::widget::Slider::setMax(int32_t _val) {
	m_max = _val;
	m_value = std::max(std::min(m_value, m_max), m_min);
	markToRedraw();
}

void ewol::widget::Slider::onDraw() {
	m_draw.draw();
}

void ewol::widget::Slider::onRegenerateDisplay() {
	if (needRedraw() == false) {
		return;
	}
	// clean the object list ...
	m_draw.clear();
	m_draw.setColor(m_textColorFg);
	// draw a line :
	m_draw.setThickness(1);
	m_draw.setPos(vec3(dotRadius, m_size.y()/2, 0) );
	m_draw.lineTo(vec3(m_size.x()-dotRadius, m_size.y()/2, 0) );
	m_draw.setThickness(0);
	
	etk::Color<> borderDot = m_textColorFg;
	borderDot.setA(borderDot.a()/2);
	m_draw.setPos(vec3(4+((float)(m_value-m_min)/(float)(m_max-m_min))*(float)(m_size.x()-2*dotRadius), m_size.y()/2, 0) );
	m_draw.setColorBg(borderDot);
	m_draw.circle(dotRadius);
	m_draw.setColorBg(m_textColorFg);
	m_draw.circle(dotRadius/1.6);
}

bool ewol::widget::Slider::onEventInput(const ewol::event::Input& _event) {
	vec2 relativePos = relativePosition(_event.getPos());
	//EWOL_DEBUG("Event on Slider ...");
	if (1 == _event.getId()) {
		if(    ewol::key::statusSingle == _event.getStatus()
		    || ewol::key::statusMove   == _event.getStatus()) {
			// get the new position :
			EWOL_VERBOSE("Event on Slider (" << relativePos.x() << "," << relativePos.y() << ")");
			int32_t oldValue = m_value;
			m_value = m_min + (float)(relativePos.x() - dotRadius) / (float)(m_size.x()-2*dotRadius) * (float)(m_max-m_min);
			m_value = std::max(std::min(m_value, m_max), m_min);
			if (oldValue != m_value) {
				EWOL_DEBUG(" new value : " << m_value << " in [" << m_min << ".." << m_max << "]");
				generateEventId(eventChange, std::to_string(m_value));
				markToRedraw();
			}
			return true;
		}
	}
	return false;
}


