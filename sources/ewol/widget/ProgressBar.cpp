/**
 * @author Edouard DUPIN
 * 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * 
 * @license BSD v3 (see license file)
 */

#include <ewol/widget/ProgressBar.h>

#include <ewol/compositing/Drawing.h>
#include <ewol/widget/Manager.h>

#undef __class__
#define __class__	"ProgressBar"

static ewol::Widget* create() {
	return new ewol::Widget::ProgressBar();
}

void ewol::Widget::ProgressBar::init(ewol::object::Shared<ewol::widget::Manager> _widgetManager) {
	_widgetManager->addWidgetCreator(__class__,&create);
}

const char* const ewol::Widget::ProgressBar::configColorBg = "color-bg";
const char* const ewol::Widget::ProgressBar::configColorFgOn = "color-on";
const char* const ewol::Widget::ProgressBar::configColorFgOff = "color-off";
const char* const ewol::Widget::ProgressBar::configValue = "value";

const int32_t dotRadius = 6;

ewol::Widget::ProgressBar::ProgressBar() {
	addObjectType("ewol::Widget::ProgressBar");
	m_value = 0.0;
	
	m_textColorFg = etk::color::black;
	
	m_textColorBgOn = 0x00FF00FF;
	
	m_textColorBgOff = etk::color::black;
	m_textColorBgOff.setA(0x3F);
	setCanHaveFocus(true);
	registerConfig(configColorBg, "color", NULL, "Background color");
	registerConfig(configColorFgOn, "color", NULL, "Corlor of the true value");
	registerConfig(configColorFgOff, "color", NULL, "Corlor of the false value");
	registerConfig(configValue, "integer", NULL, "Value of the progress bar");
	
}

ewol::Widget::ProgressBar::~ProgressBar() {
	
}

void ewol::Widget::ProgressBar::calculateMinMaxSize() {
	vec2 tmpMin = m_userMinSize.getPixel();
	m_minSize.setValue( etk_max(tmpMin.x(), 40),
	                    etk_max(tmpMin.y(), dotRadius*2) );
	markToRedraw();
}

void ewol::Widget::ProgressBar::setValue(float _val) {
	m_value = etk_avg(0, _val, 1);
	markToRedraw();
}

void ewol::Widget::ProgressBar::onDraw() {
	m_draw.draw();
}

void ewol::Widget::ProgressBar::onRegenerateDisplay() {
	if (true == needRedraw()) {
		// clean the object list ...
		m_draw.clear();
		
		m_draw.setColor(m_textColorFg);
		
		int32_t tmpSizeX = m_size.x() - 10;
		int32_t tmpSizeY = m_size.y() - 10;
		int32_t tmpOriginX = 5;
		int32_t tmpOriginY = 5;
		m_draw.setColor(m_textColorBgOn);
		m_draw.setPos(vec3(tmpOriginX, tmpOriginY, 0) );
		m_draw.rectangleWidth(vec3(tmpSizeX*m_value, tmpSizeY, 0) );
		m_draw.setColor(m_textColorBgOff);
		m_draw.setPos(vec3(tmpOriginX+tmpSizeX*m_value, tmpOriginY, 0) );
		m_draw.rectangleWidth(vec3(tmpSizeX*(1.0-m_value), tmpSizeY, 0) );
		
		// TODO : Create a better progress Bar ...
		//m_draw.setColor(m_textColorFg);
		//m_draw.rectangleBorder( tmpOriginX, tmpOriginY, tmpSizeX, tmpSizeY, 1);
	}
}



bool ewol::Widget::ProgressBar::onSetConfig(const ewol::object::Config& _conf) {
	if (true == ewol::Widget::onSetConfig(_conf)) {
		return true;
	}
	if (_conf.getConfig() == configColorBg) {
		m_textColorFg = _conf.getData();
		markToRedraw();
		return true;
	}
	if (_conf.getConfig() == configColorFgOn) {
		m_textColorBgOn = _conf.getData();
		markToRedraw();
		return true;
	}
	if (_conf.getConfig() == configColorFgOff) {
		m_textColorBgOff = _conf.getData();
		markToRedraw();
		return true;
	}
	if (_conf.getConfig() == configValue) {
		m_value = stof(_conf.getData());
		markToRedraw();
		return true;
	}
	return false;
}

bool ewol::Widget::ProgressBar::onGetConfig(const char* _config, std::string& _result) const {
	if (true == ewol::Widget::onGetConfig(_config, _result)) {
		return true;
	}
	if (_config == configColorBg) {
		_result = m_textColorFg.getString();
		return true;
	}
	if (_config == configColorFgOn) {
		_result = m_textColorBgOn.getString();
		return true;
	}
	if (_config == configColorFgOff) {
		_result = m_textColorBgOff.getString();
		return true;
	}
	if (_config == configValue) {
		_result = m_value;
		return true;
	}
	return false;
}


