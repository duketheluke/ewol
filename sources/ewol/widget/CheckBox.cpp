/**
 * @author Edouard DUPIN
 * 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * 
 * @license APACHE v2.0 (see license file)
 */

#include <ewol/widget/CheckBox.h>
#include <ewol/widget/Manager.h>

// DEFINE for the shader display system :
#define STATUS_UP        (0)
#define STATUS_HOVER     (2)
#define STATUS_PRESSED   (1)

#undef __class__
#define __class__	"CheckBox"


ewol::widget::CheckBox::CheckBox() :
  signalPressed(*this, "pressed", "CheckBox is pressed"),
  signalDown(*this, "down", "CheckBox is DOWN"),
  signalUp(*this, "up", "CheckBox is UP"),
  signalEnter(*this, "enter", "The cursor enter inside the CheckBox"),
  signalValue(*this, "value", "CheckBox value change"),
  m_shaper(*this, "shaper", "The display name for config file"),
  m_mouseHover(false),
  m_buttonPressed(false),
  m_selectableAreaPos(0,0),
  m_selectableAreaSize(0,0),
  m_shaperIdSize(-1),
  m_shaperIdSizeInsize(-1),
  m_value(*this, "value", false, "Basic value of the widget") {
	addObjectType("ewol::widget::CheckBox");
	
	m_shaperIdSize = m_shaper->requestConfig("box-size");
	m_shaperIdSizeInsize = m_shaper->requestConfig("box-inside");
	
	// shaper satatus update:
	CheckStatus();
	// This widget can have the focus ...
	setCanHaveFocus(true);
	// Limit event at 1:
	setMouseLimit(1);
}


void ewol::widget::CheckBox::init(const std::string& _shaperName) {
	ewol::widget::Container2::init();
	m_shaper->setSource(_shaperName);
}

ewol::widget::CheckBox::~CheckBox() {
	
}

void ewol::widget::CheckBox::calculateSize(const vec2& _availlable) {
	ewol::Padding padding = m_shaper->getPadding();
	float boxSize = m_shaper->getConfigNumber(m_shaperIdSize);
	padding.setXLeft(padding.xLeft()*2.0f + boxSize);
	ewol::Padding ret = calculateSizePadded(_availlable, padding);
	//EWOL_DEBUG(" configuring : origin=" << origin << " size=" << subElementSize << "");
	m_selectableAreaPos = vec2(ret.xLeft()/*-boxSize*/, ret.yButtom());
	m_selectableAreaSize = m_size - (m_selectableAreaPos + vec2(ret.xRight(), ret.yTop()));
}

void ewol::widget::CheckBox::calculateMinMaxSize() {
	ewol::Padding padding = m_shaper->getPadding();
	float boxSize = m_shaper->getConfigNumber(m_shaperIdSize);
	padding.setXLeft(padding.xLeft()*2.0f + boxSize);
	calculateMinMaxSizePadded(padding);
	if (m_size.y() < padding.y()+boxSize) {
		m_size.setY(padding.y()+boxSize);
	}
}

void ewol::widget::CheckBox::onDraw() {
	// draw the shaaper (if needed indeed)
	m_shaper->draw();
}

void ewol::widget::CheckBox::onRegenerateDisplay() {
	ewol::widget::Container2::onRegenerateDisplay();
	if (needRedraw() == false) {
		return;
	}
	ewol::Padding padding = m_shaper->getPadding();
	float boxSize = m_shaper->getConfigNumber(m_shaperIdSize);
	float boxInside = m_shaper->getConfigNumber(m_shaperIdSizeInsize);
	m_shaper->clear();
	
	vec2 origin(m_selectableAreaPos + vec2(0, (m_selectableAreaSize.y() - (boxSize+padding.y()))*0.5f));
	vec2 size = vec2(boxSize+padding.x(), boxSize+padding.y());
	
	vec2 origin2 = m_selectableAreaPos + vec2((boxSize-boxInside)*0.5f, (m_selectableAreaSize.y() - (boxInside+padding.y()))*0.5f);
	vec2 size2 = vec2(boxInside+padding.x(), boxInside+padding.y());
	m_shaper->setShape(vec2ClipInt32(origin),
	                   vec2ClipInt32(size),
	                   vec2ClipInt32(origin2+vec2(padding.xLeft(),padding.yButtom()) ),
	                   vec2ClipInt32(size2-vec2(padding.x(),padding.y()) ));
}

bool ewol::widget::CheckBox::onEventInput(const ewol::event::Input& _event) {
	EWOL_VERBOSE("Event on BT : " << _event);
	
	bool previousHoverState = m_mouseHover;
	if(    ewol::key::statusLeave == _event.getStatus()
	    || ewol::key::statusAbort == _event.getStatus()) {
		m_mouseHover = false;
		m_buttonPressed = false;
	} else {
		vec2 relativePos = relativePosition(_event.getPos());
		// prevent error from ouside the button
		if(    relativePos.x() < m_selectableAreaPos.x()
		    || relativePos.y() < m_selectableAreaPos.y()
		    || relativePos.x() > m_selectableAreaPos.x() + m_selectableAreaSize.x()
		    || relativePos.y() > m_selectableAreaPos.y() + m_selectableAreaSize.y() ) {
			m_mouseHover = false;
			m_buttonPressed = false;
		} else {
			m_mouseHover = true;
		}
	}
	bool previousPressed = m_buttonPressed;
	EWOL_VERBOSE("Event on BT ... mouse hover : " << m_mouseHover);
	if (true == m_mouseHover) {
		if (1 == _event.getId()) {
			if(ewol::key::statusDown == _event.getStatus()) {
				EWOL_VERBOSE(getName() << " : Generate event : " << signalDown);
				signalDown.emit();
				m_buttonPressed = true;
				markToRedraw();
			}
			if(ewol::key::statusUp == _event.getStatus()) {
				EWOL_VERBOSE(getName() << " : Generate event : " << signalUp);
				signalUp.emit();
				m_buttonPressed = false;
				markToRedraw();
			}
			if(ewol::key::statusSingle == _event.getStatus()) {
				// inverse value :
				setValue((m_value)?false:true);
				EWOL_VERBOSE(getName() << " : Generate event : " << signalPressed);
				signalPressed.emit();
				EWOL_VERBOSE(getName() << " : Generate event : " << signalValue << " val=" << m_value );
				signalValue.emit(m_value.get());
				markToRedraw();
			}
		}
	}
	if(    m_mouseHover != previousHoverState
	    || m_buttonPressed != previousPressed) {
		CheckStatus();
	}
	return m_mouseHover;
}


bool ewol::widget::CheckBox::onEventEntry(const ewol::event::Entry& _event) {
	//EWOL_DEBUG("BT PRESSED : \"" << UTF8_data << "\" size=" << strlen(UTF8_data));
	if(    _event.getType() == ewol::key::keyboardChar
	    && _event.getStatus() == ewol::key::statusDown
	    && _event.getChar() == '\r') {
		signalEnter.emit();
		return true;
	}
	return false;
}

void ewol::widget::CheckBox::CheckStatus() {
	if (true == m_buttonPressed) {
		changeStatusIn(STATUS_PRESSED);
	} else {
		if (true == m_mouseHover) {
			changeStatusIn(STATUS_HOVER);
		} else {
			changeStatusIn(STATUS_UP);
		}
	}
}

void ewol::widget::CheckBox::changeStatusIn(int32_t _newStatusId) {
	if (true == m_shaper->changeStatusIn(_newStatusId) ) {
		periodicCallEnable();
		markToRedraw();
	}
}


void ewol::widget::CheckBox::periodicCall(const ewol::event::Time& _event) {
	if (false == m_shaper->periodicCall(_event) ) {
		periodicCallDisable();
	}
	markToRedraw();
}

void ewol::widget::CheckBox::onParameterChangeValue(const ewol::object::ParameterRef& _paramPointer) {
	ewol::widget::Container2::onParameterChangeValue(_paramPointer);
	if (_paramPointer == m_shaper) {
		markToRedraw();
	} else if (_paramPointer == m_value) {
		if (m_value.get() == false) {
			m_idWidgetDisplayed = convertId(0);
		} else {
			m_idWidgetDisplayed = convertId(1);
		}
		CheckStatus();
		markToRedraw();
		m_shaper->setActivateState(m_value==true?1:0);
	}
}
