/**
 * @author Edouard DUPIN
 * 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * 
 * @license APACHE v2.0 (see license file)
 */

#include <ewol/widget/Spacer.h>

#include <ewol/compositing/Drawing.h>
#include <ewol/widget/Manager.h>

#undef __class__
#define __class__ "Spacer"

ewol::widget::Spacer::Spacer() :
  m_color(*this, "color", etk::color::none, "background of the spacer") {
	addObjectType("ewol::widget::Spacer");
	m_userMinSize = ewol::Dimension(vec2(10,10));
	setCanHaveFocus(false);
}

void ewol::widget::Spacer::init() {
	ewol::Widget::init();
}

ewol::widget::Spacer::~Spacer() {
	
}

void ewol::widget::Spacer::onDraw() {
	m_draw.draw();
}

#define BORDER_SIZE_TMP  (4)
void ewol::widget::Spacer::onRegenerateDisplay() {
	if (false == needRedraw()) {
		return;
	}
	m_draw.clear();
	
	if (m_color->a() == 0) {
		return;
	}
	m_draw.setColor(m_color);
	m_draw.setPos(vec3(0, 0, 0) );
	m_draw.rectangleWidth(vec3(m_size.x(), m_size.y(),0) );
}

void ewol::widget::Spacer::onParameterChangeValue(const ewol::object::ParameterRef& _paramPointer) {
	ewol::Widget::onParameterChangeValue(_paramPointer);
	if (_paramPointer == m_color) {
		markToRedraw();
	}
}

