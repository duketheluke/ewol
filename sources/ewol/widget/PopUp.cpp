/**
 * @author Edouard DUPIN
 * 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * 
 * @license BSD v3 (see license file)
 */

#include <ewol/widget/PopUp.h>
#include <ewol/widget/WidgetManager.h>
#include <ewol/compositing/Drawing.h>

#undef __class__
#define __class__	"PopUp"

widget::PopUp::PopUp(void)
{
	m_userExpand.setValue(false, false);
	SetMinSize(ewol::Dimension(vec2(80,80),ewol::Dimension::Pourcent));
	
	m_colorBackGroung = draw::color::white;
	m_colorEmptyArea = draw::color::black;
	m_colorEmptyArea.a = 0x7F;
	m_colorBorder = draw::color::black;
	m_colorBorder.a = 0x7F;
}

widget::PopUp::~PopUp(void)
{
	
}


void widget::PopUp::CalculateSize(const vec2& availlable)
{
	m_size = availlable;
	if (NULL != m_subWidget) {
		vec2 subElementSize = m_minSize;
		vec2 subElementOrigin = m_origin + (m_size-m_minSize)/2.0f;
		
		vec2 subWidgetSize = m_subWidget->GetCalculateMinSize();
		if (true == m_subWidget->CanExpand().x()) {
			subWidgetSize.setX(m_minSize.x());
		}
		if (true == m_subWidget->CanExpand().y()) {
			subWidgetSize.setY(m_minSize.y());
		}
		// limit the size of the element :
		subWidgetSize.setMin(m_minSize);
		// posiition at a int32_t pos :
		subWidgetSize = vec2ClipInt32(subWidgetSize);
		
		// set config to the Sub-widget
		vec2 subWidgetOrigin = m_origin + (m_size-subWidgetSize)/2.0f;
		
		m_subWidget->SetOrigin(subWidgetOrigin);
		m_subWidget->CalculateSize(subWidgetSize);
	}
	MarkToRedraw();
}


void widget::PopUp::CalculateMinMaxSize(void)
{
	// remove expand of sub widget ==> not needed ...
	m_userExpand.setValue(false,false);
	
	if (m_userMinSize.GetType() != ewol::Dimension::Pourcent) {
		EWOL_ERROR(" ==> Please set pourcent size of the popo_up view");
		m_minSize.setValue(50, 50);
	} else {
		m_minSize = m_userMinSize.GetPixel();
	}
	if (NULL != m_subWidget) {
		m_subWidget->CalculateMinMaxSize();
		vec2 tmpSize = m_subWidget->GetCalculateMinSize();
		m_minSize.setMax(tmpSize);
	}
	EWOL_DEBUG("CalculateMinSize=" << m_minSize);
	MarkToRedraw();
}

void widget::PopUp::OnDraw(ewol::DrawProperty& displayProp)
{
	// draw upper classes
	m_compositing.Draw();
	if (NULL != m_subWidget) {
		m_subWidget->GenDraw(displayProp);
	}
}

#define BORDER_SIZE_TMP         (4)
void widget::PopUp::OnRegenerateDisplay(void)
{
	if (true == NeedRedraw()) {
		m_compositing.Clear();
		m_compositing.SetColor(m_colorEmptyArea);
		m_compositing.SetPos(vec3(0,0,0));
		m_compositing.RectangleWidth(vec3(m_size.x(), m_size.y(), 0));
		// set the area in white ...
		if (NULL != m_subWidget) {
			vec2 tmpSize = m_subWidget->GetSize();
			vec2 tmpOrigin = m_subWidget->GetOrigin();
			m_compositing.SetColor(m_colorBorder);
			m_compositing.SetPos(vec3(tmpOrigin.x()-BORDER_SIZE_TMP, tmpOrigin.y()-BORDER_SIZE_TMP,0) );
			m_compositing.RectangleWidth(vec3(tmpSize.x()+2*BORDER_SIZE_TMP, tmpSize.y()+2*BORDER_SIZE_TMP, 0) );
			m_compositing.SetColor(m_colorBackGroung);
			m_compositing.SetPos(vec3(tmpOrigin.x(), tmpOrigin.y(),0) );
			m_compositing.RectangleWidth(vec3(tmpSize.x(), tmpSize.y(), 0) );
		}
	}
	// SUBwIDGET GENERATION ...
	if (NULL != m_subWidget) {
		m_subWidget->OnRegenerateDisplay();
	}
}

ewol::Widget* widget::PopUp::GetWidgetAtPos(const vec2& pos)
{
	ewol::Widget* val = widget::Container::GetWidgetAtPos(pos);
	if (NULL != val) {
		return val;
	}
	return this;
}
