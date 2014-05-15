/**
 * @author Edouard DUPIN
 * 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * 
 * @license BSD v3 (see license file)
 */

#include <ewol/widget/Spacer.h>

#include <ewol/compositing/Drawing.h>
#include <ewol/widget/Manager.h>

#undef __class__
#define __class__ "Spacer"

const char* const ewol::Widget::Spacer::configColor = "color";

static ewol::Widget* create() {
	return new ewol::Widget::Spacer();
}

void ewol::Widget::Spacer::init(ewol::object::Shared<ewol::Widget::Manager> _widgetManager) {
	_widgetManager.addWidgetCreator(__class__,&create);
}

ewol::Widget::Spacer::Spacer() {
	addObjectType("ewol::Widget::Spacer");
	m_userMinSize = ewol::Dimension(vec2(10,10));
	setCanHaveFocus(false);
	m_color = etk::color::black;
	m_color.setA(0);
	registerConfig(configColor, "color", NULL, "background of the spacer");
}

ewol::Widget::Spacer::~Spacer() {
	
}

void ewol::Widget::Spacer::onDraw() {
	m_draw.draw();
}

#define BORDER_SIZE_TMP  (4)
void ewol::Widget::Spacer::onRegenerateDisplay() {
	if (false == needRedraw()) {
		return;
	}
	m_draw.clear();
	
	if (m_color.a() == 0) {
		return;
	}
	m_draw.setColor(m_color);
	m_draw.setPos(vec3(0, 0, 0) );
	m_draw.rectangleWidth(vec3(m_size.x(), m_size.y(),0) );
}

bool ewol::Widget::Spacer::onSetConfig(const ewol::object::Config& _conf) {
	if (true == ewol::Widget::onSetConfig(_conf)) {
		return true;
	}
	if (_conf.getConfig() == configColor) {
		m_color = _conf.getData();
		markToRedraw();
		return true;
	}
	return false;
}

bool ewol::Widget::Spacer::onGetConfig(const char* _config, std::string& _result) const {
	if (true == ewol::Widget::onGetConfig(_config, _result)) {
		return true;
	}
	if (_config == configColor) {
		_result = m_color.getString();
		return true;
	}
	return false;
}


