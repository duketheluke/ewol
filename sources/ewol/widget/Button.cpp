/**
 * @author Edouard DUPIN
 * 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * 
 * @license BSD v3 (see license file)
 */


#include <ewol/ewol.h>
#include <ewol/widget/Button.h>
#include <ewol/widget/WidgetManager.h>


extern const char * const ewolEventButtonPressed    = "ewol-button-Pressed";
extern const char * const ewolEventButtonDown       = "ewol-button-down";
extern const char * const ewolEventButtonUp         = "ewol-button-up";
extern const char * const ewolEventButtonEnter      = "ewol-button-enter";
extern const char * const ewolEventButtonLeave      = "ewol-button-leave";
extern const char * const ewolEventButtonValue      = "ewol-button-value";

#undef __class__
#define __class__	"Button"

// DEFINE for the shader display system :
#define STATUS_UP        (0)
#define STATUS_HOVER     (2)
#define STATUS_PRESSED   (1)
#define STATUS_DOWN      (3)


static ewol::Widget* Create(void)
{
	return new widget::Button();
}

void widget::Button::Init(void)
{
	ewol::widgetManager::AddWidgetCreator(__class__,&Create);
}

void widget::Button::UnInit(void)
{
	ewol::widgetManager::AddWidgetCreator(__class__,NULL);
}



widget::Button::Button(const etk::UString& _shaperName) :
	m_shaper(_shaperName),
	m_toggleMode(false),
	m_value(false),
	m_mouseHover(false),
	m_buttonPressed(false),
	m_imageDisplaySize(32),
	m_selectableAreaPos(0,0),
	m_selectableAreaSize(0,0)
{
	// by default set no widget :
	m_subWidget[0] = NULL;
	m_subWidget[1] = NULL;
	// add basic Event generated :
	AddEventId(ewolEventButtonPressed);
	AddEventId(ewolEventButtonDown);
	AddEventId(ewolEventButtonUp);
	AddEventId(ewolEventButtonEnter);
	AddEventId(ewolEventButtonLeave);
	AddEventId(ewolEventButtonValue);
	// set basic status for the shaper :
	m_shaper.ChangeStatusIn(STATUS_UP);
	// This widget can have the focus ...
	SetCanHaveFocus(true);
	// Limit event at 1:
	SetMouseLimit(1);
}


widget::Button::~Button(void)
{
	
}

void widget::Button::SetShaperName(const etk::UString& _shaperName)
{
	m_shaper.SetSource(_shaperName);
}

void widget::Button::SetSubWidget(ewol::Widget* _subWidget)
{
	int32_t idWidget=0;
	if (NULL!=m_subWidget[idWidget]) {
		delete(m_subWidget[idWidget]);
		// the pointer might already set at NULL:
		if (NULL != m_subWidget[idWidget]) {
			EWOL_ERROR("error while removing previous widget...");
			m_subWidget[idWidget]=NULL;
		}
	}
	EWOL_DEBUG("Add button : " << idWidget << " element : " << (int64_t)_subWidget);
	m_subWidget[idWidget] = _subWidget;
	// element change ... We need to recalculate all the subElments :
	ewol::RequestUpdateSize();
	MarkToRedraw();
}

void widget::Button::SetSubWidgetToggle(ewol::Widget* _subWidget)
{
	int32_t idWidget=1;
	if (NULL!=m_subWidget[idWidget]) {
		delete(m_subWidget[idWidget]);
		// the pointer might already set at NULL:
		if (NULL != m_subWidget[idWidget]) {
			EWOL_ERROR("error while removing previous widget...");
			m_subWidget[idWidget]=NULL;
		}
	}
	EWOL_DEBUG("Add button : " << idWidget << " element : " << (int64_t)_subWidget);
	m_subWidget[idWidget] = _subWidget;
	// element change ... We need to recalculate all the subElments :
	ewol::RequestUpdateSize();
}

ewol::Widget* widget::Button::GetSubWidget(void)
{
	return m_subWidget[0];
}

ewol::Widget* widget::Button::GetSubWidgetToggle(void)
{
	return m_subWidget[1];
}


void widget::Button::CalculateSize(const vec2& _availlable)
{
	vec2 padding = m_shaper.GetPadding();
	// set minimal size
	m_size = m_minSize;
	
	vec2 minimumSizeBase(0,0);
	vec2 minimumSizeToggle(0,0);
	// Checking the expand properties :
	if (m_userExpand.x() == true) {
		m_size.setX(_availlable.x());
	}
	if (m_userExpand.y() == true) {
		m_size.setY(_availlable.y());
	}
	// Checkin the filling properties ==> for the subElements:
	vec2 subElementSize = m_minSize;
	if (m_userFill.x() == true) {
		subElementSize.setX(m_size.x());
	}
	if (m_userFill.y() == true) {
		subElementSize.setY(m_size.y());
	}
	vec2 origin = (m_size - subElementSize)/2.0f + padding;
	subElementSize -= padding*2.0f;
	if (NULL!=m_subWidget[0]) {
		m_subWidget[0]->SetOrigin(m_origin+origin);
		m_subWidget[0]->CalculateSize(subElementSize);
	}
	if (NULL!=m_subWidget[1]) {
		m_subWidget[1]->SetOrigin(m_origin+origin);
		m_subWidget[1]->CalculateSize(subElementSize);
	}
	//EWOL_DEBUG(" configuring : origin=" << origin << " size=" << subElementSize << "");
	m_selectableAreaSize = subElementSize + (padding*2.0f);
	m_selectableAreaPos = origin-padding;
	MarkToRedraw();
}


void widget::Button::CalculateMinMaxSize(void)
{
	vec2 padding = m_shaper.GetPadding();
	vec2 minimumSizeBase(0,0);
	vec2 minimumSizeToggle(0,0);
	if (NULL!=m_subWidget[0]) {
		m_subWidget[0]->CalculateMinMaxSize();
		minimumSizeBase = m_subWidget[0]->GetCalculateMinSize();
	}
	if (NULL!=m_subWidget[1]) {
		m_subWidget[1]->CalculateMinMaxSize();
		minimumSizeToggle = m_subWidget[1]->GetCalculateMinSize();
	}
	// get the maxixmum min size of the 2 sub-widget (if they are present indeed):
	m_minSize.setX(etk_max(minimumSizeBase.x(), minimumSizeToggle.x()) );
	m_minSize.setY(etk_max(minimumSizeBase.y(), minimumSizeToggle.y()) );
	
	// add padding :
	m_minSize += padding*2.0f;
	// verify the min max of the min size ...
	CheckMinSize();
	MarkToRedraw();
}

void widget::Button::OnDraw(ewol::DrawProperty& _displayProp)
{
	// draw the shaaper (if needed indeed)
	m_shaper.Draw();
	// draw the widget that need something ...
	if(    false == m_toggleMode
	    || false == m_value) {
		if (NULL!=m_subWidget[0]) {
			m_subWidget[0]->GenDraw(_displayProp);
		}
	} else {
		if (NULL!=m_subWidget[1]) {
			m_subWidget[1]->GenDraw(_displayProp);
		}
	}
}

void widget::Button::OnRegenerateDisplay(void)
{
	if (true == NeedRedraw()) {
		vec2 padding = m_shaper.GetPadding();
		m_shaper.Clear();
		m_shaper.SetOrigin(vec2ClipInt32(m_selectableAreaPos));
		m_shaper.SetSize(vec2ClipInt32(m_selectableAreaSize));
		m_shaper.SetInsidePos(vec2ClipInt32(m_selectableAreaPos+padding));
		m_shaper.SetInsideSize(vec2ClipInt32(m_selectableAreaSize-padding*2.0f));
	}
	if(    false == m_toggleMode
	    || false == m_value) {
		if (NULL!=m_subWidget[0]) {
			m_subWidget[0]->OnRegenerateDisplay();
		}
	} else {
		if (NULL!=m_subWidget[1]) {
			m_subWidget[1]->OnRegenerateDisplay();
		}
	}
}

void widget::Button::SetValue(bool _val)
{
	if (m_value != _val) {
		m_value = _val;
		MarkToRedraw();
	}
}

bool widget::Button::GetValue(void)
{
	return m_value;
}

void widget::Button::SetToggleMode(bool _togg)
{
	if (m_toggleMode != _togg) {
		m_toggleMode = _togg;
		if (m_value == true) {
			m_value = false;
			// TODO : Change display and send event ...
		}
		CheckStatus();
		MarkToRedraw();
	}
}

bool widget::Button::OnEventInput(const ewol::EventInput& _event)
{
	bool previousHoverState = m_mouseHover;
	if(    ewol::keyEvent::statusLeave == _event.GetStatus()
	    || ewol::keyEvent::statusAbort == _event.GetStatus()) {
		m_mouseHover = false;
		m_buttonPressed = false;
	} else {
		vec2 relativePos = RelativePosition(_event.GetPos());
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
	//EWOL_DEBUG("Event on BT ... mouse position : " << m_mouseHover);
	if (true == m_mouseHover) {
		if (1 == _event.GetId()) {
			if(ewol::keyEvent::statusDown == _event.GetStatus()) {
				//EWOL_DEBUG("Generate event : " << ewolEventButtonDown);
				GenerateEventId(ewolEventButtonDown);
				m_buttonPressed = true;
				MarkToRedraw();
			}
			if(ewol::keyEvent::statusUp == _event.GetStatus()) {
				//EWOL_DEBUG("Generate event : " << ewolEventButtonUp);
				GenerateEventId(ewolEventButtonUp);
				m_buttonPressed = false;
				MarkToRedraw();
			}
			if(ewol::keyEvent::statusSingle == _event.GetStatus()) {
				// inverse value :
				m_value = (m_value)?false:true;
				//EWOL_DEBUG("Generate event : " << ewolEventButtonPressed);
				GenerateEventId(ewolEventButtonPressed);
				//EWOL_DEBUG("Generate event : " << ewolEventButtonValue << " val=" << m_value);
				GenerateEventId(ewolEventButtonValue, m_value);
				if(    false == m_toggleMode
				    && true == m_value) {
					m_value = false;
					//EWOL_DEBUG("Generate event : " << ewolEventButtonValue << " val=" << m_value);
					GenerateEventId(ewolEventButtonValue, m_value);
				}
				MarkToRedraw();
			}
		}
	}
	if(    m_mouseHover != previousHoverState
	    || m_buttonPressed != previousPressed) {
		CheckStatus();
	}
	return m_mouseHover;
}


bool widget::Button::OnEventEntry(const ewol::EventEntry& _event)
{
	//EWOL_DEBUG("BT PRESSED : \"" << UTF8_data << "\" size=" << strlen(UTF8_data));
	if(    _event.GetType() == ewol::keyEvent::keyboardChar
	    && _event.GetStatus() == ewol::keyEvent::statusDown
	    && _event.GetChar() == '\r') {
		GenerateEventId(ewolEventButtonEnter);
		return true;
	}
	return false;
}

void widget::Button::CheckStatus(void)
{
	if (true==m_buttonPressed) {
		ChangeStatusIn(STATUS_PRESSED);
	} else {
		if (true==m_mouseHover) {
			ChangeStatusIn(STATUS_HOVER);
		} else {
			if (true == m_value) {
				ChangeStatusIn(STATUS_DOWN);
			} else {
				ChangeStatusIn(STATUS_UP);
			}
		}
	}
}

void widget::Button::ChangeStatusIn(int32_t _newStatusId)
{
	if (true == m_shaper.ChangeStatusIn(_newStatusId) ) {
		PeriodicCallSet(true);
		MarkToRedraw();
	}
}


void widget::Button::PeriodicCall(int64_t _localTime)
{
	if (false == m_shaper.PeriodicCall(_localTime) ) {
		PeriodicCallSet(false);
	}
	MarkToRedraw();
}


bool widget::Button::LoadXML(TiXmlNode* _node)
{
	if (NULL==_node) {
		return false;
	}
	// parse generic properties :
	ewol::Widget::LoadXML(_node);
	// remove previous element :
	SetSubWidget(NULL);
	SetSubWidgetToggle(NULL);
	
	// parse all the elements :
	for(TiXmlNode * pNode = _node->FirstChild() ;
	    NULL != pNode ;
	    pNode = pNode->NextSibling() ) {
		if (pNode->Type()==TiXmlNode::TINYXML_COMMENT) {
			// nothing to do, just proceed to next step
			continue;
		}
		etk::UString widgetName = pNode->Value();
		if (ewol::widgetManager::Exist(widgetName) == false) {
			EWOL_ERROR("(l "<<pNode->Row()<<") Unknown basic node=\"" << widgetName << "\" not in : [" << ewol::widgetManager::List() << "]" );
			continue;
		}
		bool toogleMode=false;
		if (NULL != GetSubWidget()) {
			toogleMode=true;
			if (NULL != GetSubWidgetToggle()) {
				EWOL_ERROR("(l "<<pNode->Row()<<") " << __class__ << " Can only have one subWidget ??? node=\"" << widgetName << "\"" );
				continue;
			}
		}
		EWOL_DEBUG("try to create subwidget : '" << widgetName << "'");
		ewol::Widget* tmpWidget = ewol::widgetManager::Create(widgetName);
		if (tmpWidget == NULL) {
			EWOL_ERROR ("(l "<<pNode->Row()<<") Can not create the widget : \"" << widgetName << "\"");
			continue;
		}
		// add widget :
		if (toogleMode==false) {
			SetSubWidget(tmpWidget);
		} else {
			SetToggleMode(true);
			SetSubWidgetToggle(tmpWidget);
		}
		if (false == tmpWidget->LoadXML(pNode)) {
			EWOL_ERROR ("(l "<<pNode->Row()<<") can not load widget properties : \"" << widgetName << "\"");
			return false;
		}
	}
	return true;
}

