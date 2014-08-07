/**
 * @author Edouard DUPIN
 * 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * 
 * @license BSD v3 (see license file)
 */

#include <ewol/widget/meta/Parameter.h>
#include <ewol/widget/Sizer.h>
#include <ewol/widget/List.h>
#include <ewol/widget/Spacer.h>
#include <ewol/widget/Image.h>
#include <ewol/widget/Manager.h>
#include <ewol/widget/Composer.h>
#include <vector>


#include <ewol/ewol.h>

#undef __class__
#define __class__ "Parameter"

const char * const ewol::widget::Parameter::eventClose = "close";

static const char * const ewolEventParameterValidate = "ewol-event-parameter-validate";
static const char * const ewolEventParameterSave     = "ewol-event-parameter-save";
static const char * const l_eventMenuSelected = "local-event-menu-selected";

ewol::widget::Parameter::Parameter() :
  m_currentIdList(0),
  m_widgetTitle(),
  m_paramList() {
	addObjectType("ewol::widget::Parameter");
	addEventId(eventClose);
}

void ewol::widget::Parameter::init() {
	ewol::widget::PopUp::init();
	
	std::shared_ptr<ewol::widget::Sizer> mySizerVert = nullptr;
	std::shared_ptr<ewol::widget::Sizer> mySizerHori = nullptr;
	std::shared_ptr<ewol::widget::Spacer> mySpacer = nullptr;
	#ifdef __TARGET_OS__Android
		setMinSize(ewol::Dimension(vec2(90, 90), ewol::Dimension::Pourcent));
	#else
		setMinSize(ewol::Dimension(vec2(80, 80), ewol::Dimension::Pourcent));
	#endif
	
	mySizerVert = ewol::widget::Sizer::create(widget::Sizer::modeVert);
	if (nullptr == mySizerVert) {
		EWOL_ERROR("Can not allocate widget  == > display might be in error");
	} else {
		EWOL_INFO("add widget");
		mySizerVert->lockExpand(bvec2(true,true));
		mySizerVert->setExpand(bvec2(true,true));
		// set it in the pop-up-system :
		setSubWidget(mySizerVert);
		
		mySizerHori = ewol::widget::Sizer::create(widget::Sizer::modeHori);
		if (nullptr == mySizerHori) {
			EWOL_ERROR("Can not allocate widget  == > display might be in error");
		} else {
			mySizerVert->subWidgetAdd(mySizerHori);
			
			mySpacer = ewol::widget::Spacer::create();
			if (nullptr == mySpacer) {
				EWOL_ERROR("Can not allocate widget  == > display might be in error");
			} else {
				mySpacer->setExpand(bvec2(true,false));
				mySizerHori->subWidgetAdd(mySpacer);
			}
			
			std::shared_ptr<ewol::widget::Button> tmpButton = widget::Button::create();
			if (nullptr == tmpButton) {
				EWOL_ERROR("Can not allocate widget  == > display might be in error");
			} else {
				tmpButton->setSubWidget(ewol::widget::Composer::create(widget::Composer::String,
				        "<composer>\n"
				        "	<sizer mode=\"hori\">\n"
				        "		<image src=\"THEME:GUI:Save.svg\" expand=\"true\" size=\"8,8mm\"/>\n"
				        "		<label>Save</label>\n"
				        "	</sizer>\n"
				        "</composer>\n"));
				tmpButton->registerOnEvent(shared_from_this(), "pressed", ewolEventParameterSave);
				mySizerHori->subWidgetAdd(tmpButton);
			}
			
			mySpacer = ewol::widget::Spacer::create();
			if (nullptr == mySpacer) {
				EWOL_ERROR("Can not allocate widget  == > display might be in error");
			} else {
				mySpacer->setExpand(bvec2(false,false));
				mySpacer->setMinSize(ewol::Dimension(vec2(10,0)));
				mySizerHori->subWidgetAdd(mySpacer);
			}
			
			tmpButton = ewol::widget::Button::create();
			if (nullptr == tmpButton) {
				EWOL_ERROR("Can not allocate widget  == > display might be in error");
			} else {
				tmpButton->setSubWidget(ewol::widget::Composer::create(widget::Composer::String,
				        "<composer>\n"
				        "	<sizer mode=\"hori\">\n"
				        "		<image src=\"THEME:GUI:Remove.svg\" expand=\"true\" size=\"8,8mm\"/>\n"
				        "		<label>Close</label>\n"
				        "	</sizer>\n"
				        "</composer>\n"));
				tmpButton->registerOnEvent(shared_from_this(), "pressed", eventClose);
				mySizerHori->subWidgetAdd(tmpButton);
			}
		}
		
		mySizerHori = ewol::widget::Sizer::create(widget::Sizer::modeHori);
		if (nullptr == mySizerHori) {
			EWOL_ERROR("Can not allocate widget  == > display might be in error");
		} else {
			mySizerVert->subWidgetAdd(mySizerHori);
			
			m_paramList = ewol::widget::ParameterList::create();
			if (nullptr == m_paramList) {
				EWOL_ERROR("Can not allocate widget  == > display might be in error");
			} else {
			
				m_paramList->registerOnEvent(shared_from_this(), "select", l_eventMenuSelected);
				m_paramList->setFill(bvec2(false,true));
				m_paramList->setExpand(bvec2(false,true));
				mySizerHori->subWidgetAdd(m_paramList);
			}
			mySpacer = ewol::widget::Spacer::create();
			if (nullptr == mySpacer) {
				EWOL_ERROR("Can not allocate widget  == > display might be in error");
			} else {
				mySpacer->setFill(bvec2(false,true));
				mySpacer->setMinSize(vec2(5,5));
				mySpacer->setColor(0x000000BF);
				mySizerHori->subWidgetAdd(mySpacer);
			}
			
			std::shared_ptr<ewol::widget::Sizer> mySizerVert2 = widget::Sizer::create(widget::Sizer::modeVert);
			if (nullptr == mySizerVert2) {
				EWOL_ERROR("Can not allocate widget  == > display might be in error");
			} else {
				mySizerHori->subWidgetAdd(mySizerVert2);
				
				mySpacer = ewol::widget::Spacer::create();
				if (nullptr == mySpacer) {
					EWOL_ERROR("Can not allocate widget  == > display might be in error");
				} else {
					mySpacer->setExpand(bvec2(true,false));
					mySpacer->setMinSize(vec2(5,5));
					mySpacer->setColor(0x000000BF);
					mySizerVert2->subWidgetAdd(mySpacer);
				}
				
				m_wSlider = ewol::widget::WSlider::create();
				if (nullptr == m_wSlider) {
					EWOL_ERROR("Can not allocate widget  == > display might be in error");
				} else {
					m_wSlider->setTransitionSpeed(0.5);
					m_wSlider->setTransitionMode(ewol::widget::WSlider::sladingTransitionVert);
					m_wSlider->setExpand(bvec2(true,true));
					mySizerVert2->subWidgetAdd(m_wSlider);
				}
			}
		}
		
		mySpacer = ewol::widget::Spacer::create();
		if (nullptr == mySpacer) {
			EWOL_ERROR("Can not allocate widget  == > display might be in error");
		} else {
			mySpacer->setExpand(bvec2(true,false));
			mySpacer->setMinSize(vec2(5,5));
			mySpacer->setColor(0x000000BF);
			mySizerVert->subWidgetAdd(mySpacer);
		}
		
		m_widgetTitle = ewol::widget::Label::create("File chooser ...");
		if (nullptr == m_widgetTitle) {
			EWOL_ERROR("Can not allocate widget  == > display might be in error");
		} else {
			m_widgetTitle->setExpand(bvec2(true,false));
			mySizerVert->subWidgetAdd(m_widgetTitle);
		}
	}
	markToRedraw();
}

ewol::widget::Parameter::~Parameter() {
	
}

void ewol::widget::Parameter::setTitle(std::string _label) {
	if (nullptr == m_widgetTitle) {
		return;
	}
	m_widgetTitle->setLabel(_label);
}

void ewol::widget::Parameter::onReceiveMessage(const ewol::object::Message& _msg) {
	ewol::widget::PopUp::onReceiveMessage(_msg);
	EWOL_DEBUG("event on the parameter : " << _msg);
	if (_msg.getMessage() == eventClose) {
		// inform that the parameter windows is closed
		generateEventId(eventClose);
		// close this widget ...
		autoDestroy();
	} else if (_msg.getMessage() == ewolEventParameterSave) {
		//ewol::userConfig::Save();
		EWOL_TODO("Save Parameter !!! ");
	} else if (_msg.getMessage() == l_eventMenuSelected) {
		if (nullptr != m_wSlider) {
			int32_t value = 0;
			sscanf(_msg.getData().c_str(), "%d", &value);
			EWOL_DEBUG("event on the parameter : " << _msg.getMessage() << " select ID=" << value << "");
			m_wSlider->subWidgetSelectSet(value);
		}
	}
	return;
}

void ewol::widget::Parameter::onObjectRemove(const std::shared_ptr<ewol::Object>& _removeObject) {
	// First step call parrent : 
	ewol::widget::PopUp::onObjectRemove(_removeObject);
	// second step find if in all the elements ...
	if(_removeObject == m_widgetTitle) {
		m_widgetTitle = nullptr;
	}
	if(_removeObject == m_paramList) {
		m_paramList = nullptr;
	}
	if(_removeObject == m_wSlider) {
		m_wSlider = nullptr;
	}
}

void ewol::widget::Parameter::menuAdd(std::string _label, std::string _image, std::shared_ptr<ewol::Widget> _associateWidget) {
	if (nullptr != m_paramList) {
		m_paramList->menuAdd(_label, m_currentIdList, _image);
		if (nullptr != m_wSlider) {
			if (nullptr != _associateWidget) {
				m_wSlider->subWidgetAdd(_associateWidget);
			} else { 
				EWOL_DEBUG("Associate an empty widget on it ...");
				std::shared_ptr<ewol::widget::Label> myLabel = widget::Label::create(std::string("No widget for : ") + _label);
				if (nullptr == myLabel) {
					EWOL_ERROR("Can not allocate widget  == > display might be in error");
				} else {
					myLabel->setExpand(bvec2(true,true));
					m_wSlider->subWidgetAdd(myLabel);
				}
			}
			if (m_currentIdList == 0) {
				m_wSlider->subWidgetSelectSet(0);
			}
		}
		m_currentIdList++;
	}
}
void ewol::widget::Parameter::menuAddGroup(std::string _label) {
	if (nullptr != m_paramList) {
		m_paramList->menuSeparator();
		m_paramList->menuAddGroup(_label);
	}
}

void ewol::widget::Parameter::menuClear() {
	if (nullptr != m_paramList) {
		m_paramList->menuClear();
		m_currentIdList = 0;
	}
}

void ewol::widget::Parameter::menuSeparator() {
	if (nullptr != m_paramList) {
		m_paramList->menuSeparator();
	}
}

