/**
 * @author Edouard DUPIN
 * 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * 
 * @license APACHE v2.0 (see license file)
 */

#include <ewol/widget/WSlider.h>
#include <ewol/ewol.h>


static const char* l_listsladingMode[ewol::widget::WSlider::sladingTransition_count] = {
	"transition vertical",
	"transition horisantal"
};

std::ostream& operator <<(std::ostream& _os, const enum ewol::widget::WSlider::sladingMode _obj) {
	_os << l_listsladingMode[_obj];
	return _os;
}

#undef __class__
#define __class__ "WSlider"

ewol::widget::WSlider::WSlider() :
  signalStartSlide(*this, "start"),
  signalStopSlide(*this, "stop"),
  m_windowsSources(0),
  m_windowsDestination(0),
  m_windowsRequested(-1),
  m_slidingProgress(1.0f),
  m_selectNewWidget(*this, "select", "", "Select the requested widget to display"),
  m_transitionSpeed(*this, "speed", 1.0f, 0.0f, 200.0f, "Transition speed of the slider"),
  m_transitionSlide(*this, "mode", sladingTransitionHori, "Transition mode of the slider") {
	addObjectType("ewol::widget::WSlider");
	m_transitionSlide.add(sladingTransitionVert, "vert");
	m_transitionSlide.add(sladingTransitionHori, "hori");
}

ewol::widget::WSlider::~WSlider() {
	
}


void ewol::widget::WSlider::calculateSize(const vec2& _availlable) {
	//EWOL_DEBUG("Update size");
	ewol::widget::ContainerN::calculateSize(_availlable);
	
	if (m_windowsDestination == m_windowsSources) {
		auto it = m_subWidget.begin();
		std::advance(it, m_windowsDestination);
		if (    it != m_subWidget.end()
		     && *it != nullptr) {
			(*it)->setOrigin(m_origin+m_offset);
			(*it)->calculateSize(m_size);
		}
	} else {
		float factor = -1.0f;
		if (m_windowsSources < m_windowsDestination) {
			factor = 1.0f;
		}
		auto it = m_subWidget.begin();
		std::advance(it, m_windowsSources);
		if (    it != m_subWidget.end()
		     && *it != nullptr) {
		     if (m_transitionSlide == sladingTransitionHori) {
				(*it)->setOrigin(   vec2(m_origin.x() + factor*(m_size.x()*m_slidingProgress),
				                         m_origin.y())
				                  + m_offset);
			} else {
				(*it)->setOrigin(   vec2(m_origin.x(),
				                         m_origin.y() + factor*(m_size.y()*m_slidingProgress))
				                  + m_offset);
			}
			(*it)->calculateSize(m_size);
		}
		it = m_subWidget.begin();
		std::advance(it, m_windowsDestination);
		if (    it != m_subWidget.end()
		     && *it != nullptr) {
			if (m_transitionSlide == sladingTransitionHori) {
				(*it)->setOrigin(   vec2(m_origin.x() + factor*(m_size.x()*m_slidingProgress - m_size.x()),
				                         m_origin.y())
				                  + m_offset);
			} else {
				(*it)->setOrigin(   vec2(m_origin.x(),
				                         m_origin.y() + factor*(m_size.y()*m_slidingProgress - m_size.y()))
				                  + m_offset);
			}
			(*it)->calculateSize(m_size);
		}
	}
	markToRedraw();
}

void ewol::widget::WSlider::subWidgetSelectSetVectorId(int32_t _id) {
	if (_id<0) {
		EWOL_ERROR("Can not change to a widget not present : vectID=" << _id);
		return;
	}
	if (_id != m_windowsDestination) {
		m_windowsRequested = _id;
		signalStartSlide.emit();
		periodicCallEnable();
		markToRedraw();
	}
}

void ewol::widget::WSlider::subWidgetSelectSet(int32_t _id) {
	int32_t elementID = 0;
	// search element in the list : 
	for (auto &it : m_subWidget) {
		elementID ++;
		if (it != nullptr) {
			if (it->getId() == _id) {
				if (it->getName() != "") {
					// change the internal event parameter (in case...) ==> no event generation
					m_selectNewWidget.get() = it->getName();
				} else {
					m_selectNewWidget.get() = "";
				}
				break;
			}
		}
	}
	if (elementID < m_subWidget.size()) {
		subWidgetSelectSetVectorId(elementID);
	} else {
		subWidgetSelectSetVectorId(-1);
		// change the internal event parameter (in case...) ==> no event generation
		m_selectNewWidget.get() = "";
	}
}

void ewol::widget::WSlider::subWidgetSelectSet(const std::shared_ptr<ewol::Widget>& _widgetPointer) {
	if (_widgetPointer == nullptr) {
		EWOL_ERROR("Can not change to a widget nullptr");
		return;
	}
	int32_t iii = 0;
	for (auto &it : m_subWidget) {
		if (    it != nullptr
		     && it == _widgetPointer) {
			subWidgetSelectSetVectorId(iii);
			if (_widgetPointer->getName() != "") {
				// change the internal event parameter (in case...) ==> no event generation
				m_selectNewWidget.get() = _widgetPointer->getName();
			} else {
				m_selectNewWidget.get() = "";
			}
			return;
		}
		iii++;
	}
	EWOL_ERROR("Can not change to a widget not present");
}

void ewol::widget::WSlider::subWidgetSelectSet(const std::string& _widgetName) {
	if (_widgetName == "") {
		EWOL_ERROR("Can not change to a widget with no name (input)");
		return;
	}
	int32_t iii = 0;
	for (auto &it : m_subWidget) {
		if (    it != nullptr
		     && it->getName() == _widgetName) {
			subWidgetSelectSetVectorId(iii);
			// change the internal event parameter (in case...) ==> no event generation
			m_selectNewWidget.get() = _widgetName;
			return;
		}
		iii++;
	}
	EWOL_ERROR("Can not change to a widget not present");
}

void ewol::widget::WSlider::periodicCall(const ewol::event::Time& _event) {
	if (m_slidingProgress >= 1.0) {
		m_windowsSources = m_windowsDestination;
		if(    m_windowsRequested != -1
		    && m_windowsRequested != m_windowsSources) {
			m_windowsDestination = m_windowsRequested;
			m_slidingProgress = 0.0;
		} else {
			// end of periodic :
			periodicCallDisable();
			signalStopSlide.emit();
		}
		m_windowsRequested = -1;
	}
	
	if (m_slidingProgress < 1.0) {
		if (m_windowsRequested != -1 && m_slidingProgress<0.5 ) {
			// invert sources with destination
			int32_t tmppp = m_windowsDestination;
			m_windowsDestination = m_windowsSources;
			m_windowsSources = tmppp;
			m_slidingProgress = 1.0f - m_slidingProgress;
			if (m_windowsRequested == m_windowsDestination) {
				m_windowsRequested = -1;
			}
		}
		m_slidingProgress += _event.getDeltaCall()/m_transitionSpeed;
		m_slidingProgress = std::avg(0.0f, m_slidingProgress, 1.0f);
	}
	calculateSize(m_size);
	markToRedraw();
}

void ewol::widget::WSlider::systemDraw(const ewol::DrawProperty& _displayProp) {
	if (true == m_hide){
		// widget is hidden ...
		return;
	}
	// note : do not call the widget container  == > overload this one ...
	ewol::Widget::systemDraw(_displayProp);
	
	// subwidget draw
	ewol::DrawProperty prop = _displayProp;
	prop.limit(m_origin, m_size);
	
	if (m_windowsDestination == m_windowsSources) {
		//EWOL_DEBUG("Draw : " << m_windowsDestination);
		auto it = m_subWidget.begin();
		std::advance(it, m_windowsDestination);
		if (    it != m_subWidget.end()
		     && *it != nullptr) {
			(*it)->systemDraw(prop);
		}
	} else {
		//EWOL_DEBUG("Draw : " << m_windowsSources << "=>" << m_windowsDestination << "progress=" << ((float)m_slidingProgress/1000.) );
		// draw Sources :
		auto it = m_subWidget.begin();
		std::advance(it, m_windowsSources);
		if (    it != m_subWidget.end()
		     && *it != nullptr) {
			(*it)->systemDraw(prop);
		}
		// draw Destination : 
		it = m_subWidget.begin();
		std::advance(it, m_windowsDestination);
		if (    it != m_subWidget.end()
		     && *it != nullptr) {
			(*it)->systemDraw(prop);
		}
	}
}

void ewol::widget::WSlider::onRegenerateDisplay() {
	if (m_windowsDestination == m_windowsSources) {
		auto it = m_subWidget.begin();
		std::advance(it, m_windowsDestination);
		if (    it != m_subWidget.end()
		     && *it != nullptr) {
			(*it)->onRegenerateDisplay();
		}
	} else {
		auto it = m_subWidget.begin();
		std::advance(it, m_windowsSources);
		if (    it != m_subWidget.end()
		     && *it != nullptr) {
			(*it)->onRegenerateDisplay();
		}
		it = m_subWidget.begin();
		std::advance(it, m_windowsDestination);
		if (    it != m_subWidget.end()
		     && *it != nullptr) {
			(*it)->onRegenerateDisplay();
		}
	}
}
void ewol::widget::WSlider::onParameterChangeValue(const ewol::object::ParameterRef& _paramPointer) {
	ewol::widget::ContainerN::onParameterChangeValue(_paramPointer);
	if (_paramPointer == m_selectNewWidget) {
		if (m_selectNewWidget.get() != "") {
			subWidgetSelectSet(m_selectNewWidget);
		}
	} else if (_paramPointer == m_transitionSpeed) {
		// nothing to do ...
	} else if (_paramPointer == m_transitionSlide) {
		markToRedraw();
	}
}

std::shared_ptr<ewol::Widget> ewol::widget::WSlider::getWidgetAtPos(const vec2& _pos) {
	if (true == isHide()) {
		return nullptr;
	}
	if (m_windowsDestination == m_windowsSources) {
		auto it = m_subWidget.begin();
		std::advance(it, m_windowsDestination);
		if (    it != m_subWidget.end()
		     && *it != nullptr) {
			vec2 tmpSize = (*it)->getSize();
			vec2 tmpOrigin = (*it)->getOrigin();
			if(    (tmpOrigin.x() <= _pos.x() && tmpOrigin.x() + tmpSize.x() >= _pos.x())
			    && (tmpOrigin.y() <= _pos.y() && tmpOrigin.y() + tmpSize.y() >= _pos.y()) )
			{
				std::shared_ptr<ewol::Widget> tmpWidget = (*it)->getWidgetAtPos(_pos);
				if (nullptr != tmpWidget) {
					return tmpWidget;
				}
				return nullptr;
			}
		}
	} else {
		auto it = m_subWidget.begin();
		std::advance(it, m_windowsDestination);
		if (    it != m_subWidget.end()
		     && *it != nullptr) {
			vec2 tmpSize = (*it)->getSize();
			vec2 tmpOrigin = (*it)->getOrigin();
			if(    (tmpOrigin.x() <= _pos.x() && tmpOrigin.x() + tmpSize.x() >= _pos.x())
			    && (tmpOrigin.y() <= _pos.y() && tmpOrigin.y() + tmpSize.y() >= _pos.y()) )
			{
				std::shared_ptr<ewol::Widget> tmpWidget = (*it)->getWidgetAtPos(_pos);
				if (nullptr != tmpWidget) {
					return tmpWidget;
				}
				return nullptr;
			}
		}
		it = m_subWidget.begin();
		std::advance(it, m_windowsSources);
		if (    it != m_subWidget.end()
		     && *it != nullptr) {
			vec2 tmpSize = (*it)->getSize();
			vec2 tmpOrigin = (*it)->getOrigin();
			if(    (tmpOrigin.x() <= _pos.x() && tmpOrigin.x() + tmpSize.x() >= _pos.x())
			    && (tmpOrigin.y() <= _pos.y() && tmpOrigin.y() + tmpSize.y() >= _pos.y()) )
			{
				std::shared_ptr<ewol::Widget> tmpWidget = (*it)->getWidgetAtPos(_pos);
				if (nullptr != tmpWidget) {
					return tmpWidget;
				}
				return nullptr;
			}
		}
	}
	return nullptr;
}

