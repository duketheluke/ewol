/**
 * @author Edouard DUPIN
 * 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * 
 * @license BSD v3 (see license file)
 */

#include <tinyXML/tinyxml.h>

#include <ewol/debug.h>
#include <ewol/compositing/Text.h>
#include <ewol/config.h>

ewol::Text::Text(void) :
	m_position(0.0, 0.0, 0.0),
	m_clippingPosStart(0.0, 0.0, 0.0),
	m_clippingPosStop(0.0, 0.0, 0.0),
	m_clippingEnable(false),
	m_color(draw::color::black),
	m_colorBg(draw::color::none),
	m_colorCursor(draw::color::black),
	m_colorSelection(draw::color::olive),
	m_mode(ewol::font::Regular),
	m_kerning(true),
	m_distanceField(false),
	m_previousCharcode(0),
	m_startTextpos(0),
	m_stopTextPos(0),
	m_alignement(ewol::Text::alignDisable),
	m_GLprogram(NULL),
	m_GLPosition(-1),
	m_GLMatrix(-1),
	m_GLColor(-1),
	m_GLtexture(-1),
	m_GLtexID(-1),
	m_selectionStartPos(-100),
	m_cursorPos(-100),
	m_font(NULL)
{
	SetFont("", -1);
	LoadProgram();
}


ewol::Text::Text(etk::UString fontName, int32_t fontSize) :
	m_position(0.0, 0.0, 0.0),
	m_clippingPosStart(0.0, 0.0, 0.0),
	m_clippingPosStop(0.0, 0.0, 0.0),
	m_clippingEnable(false),
	m_color(draw::color::black),
	m_colorBg(draw::color::none),
	m_colorCursor(draw::color::black),
	m_colorSelection(draw::color::olive),
	m_mode(ewol::font::Regular),
	m_kerning(true),
	m_distanceField(false),
	m_previousCharcode(0),
	m_startTextpos(0),
	m_stopTextPos(0),
	m_alignement(ewol::Text::alignDisable),
	m_GLprogram(NULL),
	m_GLPosition(-1),
	m_GLMatrix(-1),
	m_GLColor(-1),
	m_GLtexture(-1),
	m_GLtexID(-1),
	m_selectionStartPos(-100),
	m_cursorPos(-100),
	m_font(NULL)
{
	SetFont(fontName, fontSize);
	LoadProgram();
}


ewol::Text::~Text(void)
{
	
	if (NULL != m_font) {
		ewol::resource::Release(m_font);
		m_font = NULL;
	}
	ewol::resource::Release(m_GLprogram);
}

void ewol::Text::LoadProgram(void)
{
	etk::UString tmpString("DATA:text.prog");
	// get the shader resource :
	m_GLPosition = 0;
	if (true == ewol::resource::Keep(tmpString, m_GLprogram) ) {
		m_GLPosition = m_GLprogram->GetAttribute("EW_coord2d");
		m_GLColor    = m_GLprogram->GetAttribute("EW_color");
		m_GLtexture  = m_GLprogram->GetAttribute("EW_texture2d");
		m_GLMatrix   = m_GLprogram->GetUniform("EW_MatrixTransformation");
		m_GLtexID    = m_GLprogram->GetUniform("EW_texID");
	}
}

void ewol::Text::Draw(const mat4& transformationMatrix, bool enableDepthTest)
{
	
	// draw BG in any case:
	m_vectorialDraw.Draw();
	
	if (m_coord.Size()<=0 || NULL == m_font) {
		// TODO : a remÃštre ...
		//EWOL_WARNING("Nothink to draw...");
		return;
	}
	if (m_font == NULL) {
		EWOL_WARNING("no font...");
		return;
	}
	if (m_GLprogram==NULL) {
		EWOL_ERROR("No shader ...");
		return;
	}
	if (true==enableDepthTest) {
		ewol::openGL::Enable(ewol::openGL::FLAG_DEPTH_TEST);
	}
	// set Matrix : translation/positionMatrix
	mat4 projMatrix = ewol::openGL::GetMatrix();
	mat4 camMatrix = ewol::openGL::GetCameraMatrix();
	mat4 tmpMatrix = projMatrix * camMatrix * transformationMatrix;
	m_GLprogram->Use(); 
	m_GLprogram->UniformMatrix4fv(m_GLMatrix, 1, tmpMatrix.m_mat);
	// TextureID
	m_GLprogram->SetTexture0(m_GLtexID, m_font->GetId());
	// position :
	m_GLprogram->SendAttribute(m_GLPosition, 2/*x,y*/, &m_coord[0]);
	// Texture :
	m_GLprogram->SendAttribute(m_GLtexture, 2/*u,v*/, &m_coordTex[0]);
	// color :
	m_GLprogram->SendAttribute(m_GLColor, 4/*r,g,b,a*/, &m_coordColor[0]);
	// Request the draw od the elements : 
	ewol::openGL::DrawArrays(GL_TRIANGLES, 0, m_coord.Size());
	m_GLprogram->UnUse();
	if (true==enableDepthTest) {
		ewol::openGL::Disable(ewol::openGL::FLAG_DEPTH_TEST);
	}
}


void ewol::Text::Draw(void)
{
	// draw BG in any case:
	m_vectorialDraw.Draw();
	
	if (m_coord.Size()<=0 || NULL == m_font) {
		// TODO : a remètre ...
		//EWOL_WARNING("Nothink to draw...");
		return;
	}
	if (m_font == NULL) {
		EWOL_WARNING("no font...");
		return;
	}
	if (m_GLprogram==NULL) {
		EWOL_ERROR("No shader ...");
		return;
	}
	// set Matrix : translation/positionMatrix
	mat4 tmpMatrix = ewol::openGL::GetMatrix()*m_matrixApply;
	m_GLprogram->Use(); 
	m_GLprogram->UniformMatrix4fv(m_GLMatrix, 1, tmpMatrix.m_mat);
	// TextureID
	m_GLprogram->SetTexture0(m_GLtexID, m_font->GetId());
	// position :
	m_GLprogram->SendAttribute(m_GLPosition, 2/*x,y*/, &m_coord[0]);
	// Texture :
	m_GLprogram->SendAttribute(m_GLtexture, 2/*u,v*/, &m_coordTex[0]);
	// color :
	m_GLprogram->SendAttribute(m_GLColor, 4/*r,g,b,a*/, &m_coordColor[0]);
	// Request the draw od the elements : 
	ewol::openGL::DrawArrays(GL_TRIANGLES, 0, m_coord.Size());
	m_GLprogram->UnUse();
}

void ewol::Text::Translate(vec3 vect)
{
	ewol::Compositing::Translate(vect);
	m_vectorialDraw.Translate(vect);
}


void ewol::Text::Rotate(vec3 vect, float angle)
{
	ewol::Compositing::Rotate(vect, angle);
	m_vectorialDraw.Rotate(vect, angle);
}


void ewol::Text::Scale(vec3 vect)
{
	ewol::Compositing::Scale(vect);
	m_vectorialDraw.Scale(vect);
}


void ewol::Text::Clear(void)
{
	// call upper class
	ewol::Compositing::Clear();
	// remove sub draw system
	m_vectorialDraw.Clear();
	// Reset Buffer :
	m_coord.Clear();
	m_coordTex.Clear();
	m_coordColor.Clear();
	// Reset temporal variables :
	Reset();
}

void ewol::Text::Reset(void)
{
	m_position = vec3(0.0, 0.0, 0.0);
	m_clippingPosStart = vec3(0.0, 0.0, 0.0);
	m_clippingPosStop = vec3(0.0, 0.0, 0.0);
	m_sizeDisplayStart = m_position;
	m_sizeDisplayStop = m_position;
	m_nbCharDisplayed = 0;
	m_clippingEnable = false;
	m_color = draw::color::black;
	m_colorBg = draw::color::none;
	m_mode = ewol::font::Regular;
	m_previousCharcode = 0;
	m_startTextpos = 0;
	m_stopTextPos = 0;
	m_alignement = ewol::Text::alignDisable;
	m_htmlCurrrentLine = "";
	m_selectionStartPos = -100;
	m_cursorPos = -100;
	m_htmlDecoration.Clear();
	m_needDisplay = true;
	m_nbCharDisplayed = 0;
}


vec3 ewol::Text::GetPos(void)
{
	return m_position;
}


void ewol::Text::SetPos(const vec3& pos)
{
	// check min max for display area
	if (m_nbCharDisplayed != 0) {
		EWOL_VERBOSE("update size 1 " << m_sizeDisplayStart << " " << m_sizeDisplayStop);
		m_sizeDisplayStop.setX(etk_max(m_position.x(), m_sizeDisplayStop.x()));
		m_sizeDisplayStop.setY(etk_max(m_position.y(), m_sizeDisplayStop.y()));
		m_sizeDisplayStart.setX(etk_min(m_position.x(), m_sizeDisplayStart.x()));
		m_sizeDisplayStart.setY(etk_min(m_position.y(), m_sizeDisplayStart.y()));
		EWOL_VERBOSE("update size 2 " << m_sizeDisplayStart << " " << m_sizeDisplayStop);
	}
	// update position
	m_position = pos;
	m_previousCharcode = 0;
	m_vectorialDraw.SetPos(m_position);
	// update min max of the display area:
	if (m_nbCharDisplayed == 0) {
		m_sizeDisplayStart = m_position;
		m_sizeDisplayStop = m_position;
		m_sizeDisplayStop.setY( m_sizeDisplayStop.y()+ m_font->GetHeight(m_mode));
		EWOL_VERBOSE("update size 0 " << m_sizeDisplayStart << " " << m_sizeDisplayStop);
	} else {
		EWOL_VERBOSE("update size 3 " << m_sizeDisplayStart << " " << m_sizeDisplayStop);
		m_sizeDisplayStop.setX(etk_max(m_position.x(), m_sizeDisplayStop.x()));
		m_sizeDisplayStop.setY(etk_max(m_position.y(), m_sizeDisplayStop.y()));
		m_sizeDisplayStart.setX(etk_min(m_position.x(), m_sizeDisplayStart.x()));
		m_sizeDisplayStart.setY(etk_min(m_position.y(), m_sizeDisplayStart.y()));
		EWOL_VERBOSE("update size 4 " << m_sizeDisplayStart << " " << m_sizeDisplayStop);
	}
}


void ewol::Text::SetRelPos(const vec3& pos)
{
	m_position += pos;
	m_previousCharcode = 0;
	m_vectorialDraw.SetPos(m_position);
}


void ewol::Text::SetColor(const draw::Color& color)
{
	m_color = color;
}


void ewol::Text::SetColorBg(const draw::Color& color)
{
	m_colorBg = color;
	m_vectorialDraw.SetColor(color);
}


void ewol::Text::SetClippingWidth(const vec2& pos, const vec2& width)
{
	SetClipping(pos, pos+width);
}

void ewol::Text::SetClippingWidth(const vec3& pos, const vec3& width)
{
	SetClipping(pos, pos+width);
}

void ewol::Text::SetClipping(const vec2& pos, const vec2& posEnd)
{
	SetClipping(vec3(pos.x(),pos.y(),-1), vec3(posEnd.x(),posEnd.y(),1) );
}

void ewol::Text::SetClipping(const vec3& pos, const vec3& posEnd)
{
	// note the internal system all time request to have a bounding all time in the same order
	if (pos.x() <= posEnd.x()) {
		m_clippingPosStart.setX(pos.x());
		m_clippingPosStop.setX(posEnd.x());
	} else {
		m_clippingPosStart.setX(posEnd.x());
		m_clippingPosStop.setX(pos.x());
	}
	if (pos.y() <= posEnd.y()) {
		m_clippingPosStart.setY(pos.y());
		m_clippingPosStop.setY(posEnd.y());
	} else {
		m_clippingPosStart.setY(posEnd.y());
		m_clippingPosStop.setY(pos.y());
	}
	if (pos.z() <= posEnd.z()) {
		m_clippingPosStart.setZ(pos.z());
		m_clippingPosStop.setZ(posEnd.z());
	} else {
		m_clippingPosStart.setZ(posEnd.z());
		m_clippingPosStop.setZ(pos.z());
	}
	m_clippingEnable = true;
	//m_vectorialDraw.SetClipping(m_clippingPosStart, m_clippingPosStop);
}


void ewol::Text::SetClippingMode(bool newMode)
{
	m_clippingEnable = newMode;
	//m_vectorialDraw.SetClippingMode(m_clippingEnable);
}


void ewol::Text::SetFontSize(int32_t fontSize)
{
	// get old size
	etk::UString fontName = "";
	if (NULL != m_font) {
		fontName = m_font->GetName();
	}
	SetFont(fontName, fontSize);
}


void ewol::Text::SetFontName(const etk::UString& fontName)
{
	// get old size
	int32_t fontSize = -1;
	if (NULL != m_font) {
		fontSize = m_font->GetFontSize();
	}
	SetFont(fontName, fontSize);
}


void ewol::Text::SetFont(etk::UString fontName, int32_t fontSize)
{
	Clear();
	// remove old one
	if (NULL != m_font) {
		ewol::resource::Release(m_font);
		m_font = NULL;
	}
	if (fontSize <= 0) {
		fontSize = ewol::config::FontGetDefaultSize();
	}
	if (fontName == "") {
		fontName = ewol::config::FontGetDefaultName();
	}
	fontName += ":";
	fontName += fontSize;
	// link to new One
	if (false == ewol::resource::Keep(fontName, m_font)) {
		EWOL_ERROR("Can not get font resource");
	}
}


void ewol::Text::SetFontMode(ewol::font::mode_te mode)
{
	if (NULL!=m_font) {
		m_mode = m_font->GetWrappingMode(mode);
	}
}


ewol::font::mode_te ewol::Text::GetFontMode(void)
{
	return m_mode;
}


void ewol::Text::SetFontBold(bool status)
{
	if (true == status) {
		// enable
		if (m_mode == ewol::font::Regular) {
			SetFontMode(ewol::font::Bold);
		} else if (m_mode == ewol::font::Italic) {
			SetFontMode(ewol::font::BoldItalic);
		}
	} else {
		// disable
		if (m_mode == ewol::font::Bold) {
			SetFontMode(ewol::font::Regular);
		} else if (m_mode == ewol::font::BoldItalic) {
			SetFontMode(ewol::font::Italic);
		}
	}
}

void ewol::Text::SetFontItalic(bool status)
{
	if (true == status) {
		// enable
		if (m_mode == ewol::font::Regular) {
			SetFontMode(ewol::font::Italic);
		} else if (m_mode == ewol::font::Bold) {
			SetFontMode(ewol::font::BoldItalic);
		}
	} else {
		// disable
		if (m_mode == ewol::font::Italic) {
			SetFontMode(ewol::font::Regular);
		} else if (m_mode == ewol::font::BoldItalic) {
			SetFontMode(ewol::font::Bold);
		}
	}
}


void ewol::Text::SetKerningMode(bool newMode)
{
	m_kerning = newMode;
}


void ewol::Text::SetDistanceFieldMode(bool newMode)
{
	m_distanceField = newMode;
	EWOL_TODO("The Distance field mode is not availlable for now ...");
}


void ewol::Text::Print(const etk::UString& text)
{
	etk::Vector<TextDecoration> decorationEmpty;
	Print(text, decorationEmpty);
}


void ewol::Text::ParseHtmlNode(void* element2)
{
	// get the static real pointer
	TiXmlNode* element = static_cast<TiXmlNode*>(element2);
	if (NULL != element) {
		for (TiXmlNode * child = element->FirstChild(); NULL != child ; child = child->NextSibling() ) {
			if (child->Type()==TiXmlNode::TINYXML_COMMENT) {
				// nothing to do ...
			} else if (child->Type()==TiXmlNode::TINYXML_TEXT) {
				HtmlAddData(child->Value() );
				EWOL_VERBOSE("XML Add : " << child->Value());
			} else if(    !strcmp(child->Value(), "br")
			           || !strcmp(child->Value(), "BR")) {
				HtmlFlush();
				EWOL_VERBOSE("XML flush & newLine");
				ForceLineReturn();
			} else if (!strcmp(child->Value(), "font")) {
				EWOL_VERBOSE("XML Font ...");
				TextDecoration tmpDeco = m_htmlDecoTmp;
				const char *colorValue = child->ToElement()->Attribute("color");
				if (NULL != colorValue) {
					draw::ParseColor(colorValue, m_htmlDecoTmp.m_colorFg);
				}
				colorValue = child->ToElement()->Attribute("colorBg");
				if (NULL != colorValue) {
					draw::ParseColor(colorValue, m_htmlDecoTmp.m_colorBg);
				}
				ParseHtmlNode(child);
				m_htmlDecoTmp = tmpDeco;
			} else if(    !strcmp(child->Value(), "b")
			           || !strcmp(child->Value(), "bold")) {
				EWOL_VERBOSE("XML bold ...");
				TextDecoration tmpDeco = m_htmlDecoTmp;
				if (m_htmlDecoTmp.m_mode == ewol::font::Regular) {
					m_htmlDecoTmp.m_mode = ewol::font::Bold;
				} else if (m_htmlDecoTmp.m_mode == ewol::font::Italic) {
					m_htmlDecoTmp.m_mode = ewol::font::BoldItalic;
				} 
				ParseHtmlNode(child);
				m_htmlDecoTmp = tmpDeco;
			} else if(    !strcmp(child->Value(), "i")
			           || !strcmp(child->Value(), "italic")) {
				EWOL_VERBOSE("XML italic ...");
				TextDecoration tmpDeco = m_htmlDecoTmp;
				if (m_htmlDecoTmp.m_mode == ewol::font::Regular) {
					m_htmlDecoTmp.m_mode = ewol::font::Italic;
				} else if (m_htmlDecoTmp.m_mode == ewol::font::Bold) {
					m_htmlDecoTmp.m_mode = ewol::font::BoldItalic;
				} 
				ParseHtmlNode(child);
				m_htmlDecoTmp = tmpDeco;
			} else if(    !strcmp(child->Value(), "u")
			           || !strcmp(child->Value(), "underline")) {
				EWOL_VERBOSE("XML underline ...");
				ParseHtmlNode(child);
			} else if(    !strcmp(child->Value(), "p")
			           || !strcmp(child->Value(), "paragraph")) {
				EWOL_VERBOSE("XML paragraph ...");
				HtmlFlush();
				m_alignement = ewol::Text::alignLeft;
				ForceLineReturn();
				ParseHtmlNode(child);
				ForceLineReturn();
			} else if (!strcmp(child->Value(), "center")) {
				EWOL_VERBOSE("XML center ...");
				HtmlFlush();
				m_alignement = ewol::Text::alignCenter;
				ParseHtmlNode(child);
			} else if (!strcmp(child->Value(), "left")) {
				EWOL_VERBOSE("XML left ...");
				HtmlFlush();
				m_alignement = ewol::Text::alignLeft;
				ParseHtmlNode(child);
			} else if (!strcmp(child->Value(), "right")) {
				EWOL_VERBOSE("XML right ...");
				HtmlFlush();
				m_alignement = ewol::Text::alignRight;
				ParseHtmlNode(child);
			} else if (!strcmp(child->Value(), "justify")) {
				EWOL_VERBOSE("XML justify ...");
				HtmlFlush();
				m_alignement = ewol::Text::alignJustify;
				ParseHtmlNode(child);
			} else {
				EWOL_ERROR("(l "<< child->Row() << ") node not suported type : " << child->Type() << " val=\""<< child->Value() << "\"" );
			}
		}
	}
}

void ewol::Text::PrintDecorated(etk::UString& text)
{
	etk::UString tmpData("<html><body>\n");
	tmpData+=text;
	tmpData+="\n</body></html>\n";
	//EWOL_DEBUG("plop : " << tmpData);
	PrintHTML(tmpData);
}

void ewol::Text::PrintHTML(etk::UString text)
{
	TiXmlDocument XmlDocument;
	
	// reset parameter :
	m_htmlDecoTmp.m_colorBg = draw::color::none;
	m_htmlDecoTmp.m_colorFg = draw::color::black;
	m_htmlDecoTmp.m_mode = ewol::font::Regular;
	
	
	// load the XML from the memory
	bool loadError = XmlDocument.Parse((const char*)text.c_str(), 0, TIXML_ENCODING_UTF8);
	if (false == loadError) {
		EWOL_ERROR( "can not load Hightlight XML: PARSING error: Decorated text ");
		return;
	}

	TiXmlElement* root = XmlDocument.FirstChildElement( "html" );
	if (NULL == root) {
		EWOL_ERROR( "can not load Hightlight XML: main node not find: \"html\"");
		return;
	}
	TiXmlElement* bodyNode = root->FirstChildElement( "body" );
	(void)ParseHtmlNode(bodyNode);
	HtmlFlush();
}

void ewol::Text::Print(const etk::UString& text, const etk::Vector<TextDecoration>& decoration)
{
	if (m_font == NULL) {
		EWOL_ERROR("Font Id is not corectly defined");
		return;
	}
	draw::Color tmpFg(m_color);
	draw::Color tmpBg(m_colorBg);
	if (m_alignement == ewol::Text::alignDisable) {
		if (true == m_needDisplay) {
			if (0==m_cursorPos) {
				m_vectorialDraw.SetPos(m_position);
				SetColorBg(m_colorCursor);
				PrintCursor(false);
			}
		}
		// note this is faster when nothing is requested ...
		for(int32_t iii=0; iii<text.Size(); iii++) {
			if (iii<decoration.Size()) {
				tmpFg = decoration[iii].m_colorFg;
				tmpBg = decoration[iii].m_colorBg;
				SetFontMode(decoration[iii].m_mode);
			}
			if (true == m_needDisplay) {
				if(    (    m_selectionStartPos-1<iii
				         && iii <=m_cursorPos-1)
				    || (    m_selectionStartPos-1>=iii
				         && iii > m_cursorPos-1) ) {
					SetColor(  0x000000FF);
					SetColorBg(m_colorSelection);
				} else {
					SetColor(  tmpFg);
					SetColorBg(tmpBg);
				}
			}
			if(    true == m_needDisplay
			    && m_colorBg.a != 0) {
				vec3 pos = m_position;
				m_vectorialDraw.SetPos(pos);
				Print(text[iii]);
				float fontHeigh = m_font->GetHeight(m_mode);
				m_vectorialDraw.RectangleWidth(vec3(m_position.x()-pos.x(),fontHeigh,0.0f) );
				m_nbCharDisplayed++;
			} else {
				Print(text[iii]);
				m_nbCharDisplayed++;
			}
			if (true == m_needDisplay) {
				if (iii==m_cursorPos-1) {
					m_vectorialDraw.SetPos(m_position);
					SetColorBg(m_colorCursor);
					PrintCursor(false);
				}
			}
		}
	} else {
		// special start case at the right of the endpoint :
		if (m_stopTextPos < m_position.x()) {
			ForceLineReturn();
		}
		float basicSpaceWidth = CalculateSize(etk::UniChar(' ')).x();
		int32_t currentId = 0;
		int32_t stop;
		int32_t space;
		int32_t freeSpace;
		while (currentId < text.Size()) {
			bool needNoJustify = ExtrapolateLastId(text, currentId, stop, space, freeSpace);
			
			float interpolation = basicSpaceWidth;
			switch (m_alignement)
			{
				case ewol::Text::alignJustify:
					if (needNoJustify == false) {
						interpolation += (float)freeSpace / (float)(space-1);
					}
					break;
				case ewol::Text::alignDisable: // must not came from here ...
				case ewol::Text::alignLeft:
					// nothing to do ...
					break;
				case ewol::Text::alignRight:
					if (m_needDisplay == true) {
						// Move the first char at the right :
						SetPos(vec3(m_position.x() + freeSpace,
						            m_position.y(),
						            m_position.z()) );
					}
					break;
				case ewol::Text::alignCenter:
					if (m_needDisplay == true) {
						// Move the first char at the right :
						SetPos(vec3(m_position.x() + freeSpace/2,
						            m_position.y(),
						            m_position.z()) );
					}
					break;
			}
			// display all the elements
			if(    true == m_needDisplay
			    && 0==m_cursorPos) {
				m_vectorialDraw.SetPos(m_position);
				SetColorBg(m_colorCursor);
				PrintCursor(false);
			}
			for(int32_t iii=currentId; iii<stop && iii<text.Size(); iii++) {
				float fontHeigh = m_font->GetHeight(m_mode);
				// Get specific decoration if provided
				if (iii<decoration.Size()) {
					tmpFg = decoration[iii].m_colorFg;
					tmpBg = decoration[iii].m_colorBg;
					SetFontMode(decoration[iii].m_mode);
				}
				if (true == m_needDisplay) {
					if(    (    m_selectionStartPos-1<iii
					         && iii <=m_cursorPos-1)
					    || (    m_selectionStartPos-1>=iii
					         && iii > m_cursorPos-1) ) {
						SetColor(  0x000000FF);
						SetColorBg(m_colorSelection);
					} else {
						SetColor(  tmpFg);
						SetColorBg(tmpBg);
					}
				}
				// special for the justify mode
				if (text[iii] == (uniChar_t)' ') {
					//EWOL_DEBUG(" generateString : \" \"");
					if(    true == m_needDisplay
					    && m_colorBg.a != 0) {
						m_vectorialDraw.SetPos(m_position);
					}
					// Must generate a dynamic space : 
					SetPos(vec3(m_position.x() + interpolation,
					            m_position.y(),
					            m_position.z()) );
					if(    true == m_needDisplay
					    && m_colorBg.a != 0) {
						m_vectorialDraw.RectangleWidth(vec3(interpolation,fontHeigh,0.0f) );
					}
				} else {
					//EWOL_DEBUG(" generateString : \"" << (char)text[iii] << "\"");
					if(    true == m_needDisplay
					    && m_colorBg.a != 0) {
						vec3 pos = m_position;
						m_vectorialDraw.SetPos(pos);
						Print(text[iii]);
						m_vectorialDraw.RectangleWidth(vec3(m_position.x()-pos.x(),fontHeigh,0.0f) );
						m_nbCharDisplayed++;
					} else {
						Print(text[iii]);
						m_nbCharDisplayed++;
					}
				}
				if (m_needDisplay == true) {
					if (iii==m_cursorPos-1) {
						m_vectorialDraw.SetPos(m_position);
						SetColorBg(m_colorCursor);
						PrintCursor(false);
					}
				}
			}
			if (currentId == stop) {
				currentId++;
			} else if(text[stop] == (uniChar_t)' ') {
				currentId = stop+1;
				// Reset position : 
				SetPos(vec3(m_startTextpos,
				            (float)(m_position.y() - m_font->GetHeight(m_mode)),
				            m_position.z()) );
				m_nbCharDisplayed++;
			} else if(text[stop] == (uniChar_t)'\n') {
				currentId = stop+1;
				// Reset position : 
				SetPos(vec3(m_startTextpos,
				            (float)(m_position.y() - m_font->GetHeight(m_mode)),
				            m_position.z()) );
				m_nbCharDisplayed++;
			} else {
				currentId = stop;
			}
		}
	}
}


void ewol::Text::Print(const uniChar_t charcode)
{
	if (NULL==m_font) {
		EWOL_ERROR("Font Id is not corectly defined");
		return;
	}
	// get a pointer on the glyph property : 
	ewol::GlyphProperty * myGlyph = m_font->GetGlyphPointer(charcode, m_mode);
	if (NULL==myGlyph) {
		EWOL_ERROR(" font does not really existed ...");
		return;
	}
	int32_t fontSize = m_font->GetFontSize();
	int32_t fontHeigh = m_font->GetHeight(m_mode);
	
	// Get the kerning ofset :
	float kerningOffset = 0.0;
	if (true==m_kerning) {
		kerningOffset = myGlyph->KerningGet(m_previousCharcode);
		if (kerningOffset != 0) {
			//EWOL_DEBUG("Kerning between : '" << (char)m_previousCharcode << "'&'" << (char)myGlyph->m_UVal << "' value : " << kerningOffset);
		}
	}
	// 0x01 == 0x20 == ' ';
	if (charcode != 0x01) {
		/* Bitmap position
		 *      xA     xB
		 *   yC *------*
		 *      |      |
		 *      |      |
		 *   yD *------*
		 */
		float dxA = m_position.x() + myGlyph->m_bearing.x() + kerningOffset;
		float dxB = dxA + myGlyph->m_sizeTexture.x();
		float dyC = m_position.y() + myGlyph->m_bearing.y() + fontHeigh - fontSize;
		float dyD = dyC - myGlyph->m_sizeTexture.y();
		
		float tuA = myGlyph->m_texturePosStart.x();
		float tuB = myGlyph->m_texturePosStop.x();
		float tvC = myGlyph->m_texturePosStart.y();
		float tvD = myGlyph->m_texturePosStop.y();
		
		
		// Clipping and drawing area
		if(     true == m_clippingEnable
		    && (    dxB < m_clippingPosStart.x()
		         || dxA > m_clippingPosStop.x()
		         || dyC < m_clippingPosStart.y()
		         || dyD > m_clippingPosStop.y() ) ) {
			// Nothing to diplay ...
		} else {
			if (true == m_clippingEnable) {
				// generata positions...
				float TexSizeX = tuB - tuA;
				if (dxA < m_clippingPosStart.x()) {
					// clip display
					float drawSize = m_clippingPosStart.x() - dxA;
					// Update element start display
					dxA = m_clippingPosStart.x();
					float addElement = TexSizeX * drawSize / (float)myGlyph->m_sizeTexture.x();
					// update texture start X Pos
					tuA += addElement;
				}
				if (dxB > m_clippingPosStop.x()) {
					// clip display
					float drawSize = dxB - m_clippingPosStop.x();
					// Update element start display
					dxB = m_clippingPosStop.x();
					float addElement = TexSizeX * drawSize / (float)myGlyph->m_sizeTexture.x();
					// update texture start X Pos
					tuB -= addElement;
				}
				float TexSizeY = tvC - tvD;
				if (dyC > m_clippingPosStop.y()) {
					// clip display
					float drawSize = dyC - m_clippingPosStop.y();
					// Update element start display
					dyC = m_clippingPosStop.y();
					float addElement = TexSizeY * drawSize / (float)myGlyph->m_sizeTexture.y();
					// update texture start X Pos
					tvC -= addElement;
				}
				if (dyD < m_clippingPosStart.y()) {
					// clip display
					float drawSize = m_clippingPosStart.y() - dyD;
					// Update element start display
					dyD = m_clippingPosStart.y();
					float addElement = TexSizeY * drawSize / (float)myGlyph->m_sizeTexture.y();
					// update texture start X Pos
					tvD += addElement;
				}
			}
			if(    dxB <= dxA
			    || dyD >= dyC) {
				// nothing to do ...
			} else {
				/* Bitmap position
				 *   0------1
				 *   |      |
				 *   |      |
				 *   3------2
				 */
				if (m_needDisplay == true) {
					ivec2 bitmapDrawPos[4];
					bitmapDrawPos[0].setValue((int32_t)dxA, (int32_t)dyC);
					bitmapDrawPos[1].setValue((int32_t)dxB, (int32_t)dyC);
					bitmapDrawPos[2].setValue((int32_t)dxB, (int32_t)dyD);
					bitmapDrawPos[3].setValue((int32_t)dxA, (int32_t)dyD);
					/* texture Position : 
					 *   0------1
					 *   |      |
					 *   |      |
					 *   3------2
					 */
					vec2 texturePos[4];
					texturePos[0].setValue(tuA+m_mode, tvC);
					texturePos[1].setValue(tuB+m_mode, tvC);
					texturePos[2].setValue(tuB+m_mode, tvD);
					texturePos[3].setValue(tuA+m_mode, tvD);
					
					// NOTE : Android does not support the Quads elements ...
					/* Step 1 : 
					 *   ********     
					 *     ******     
					 *       ****     
					 *         **     
					 *                
					 */
					// set texture coordonates :
					m_coordTex.PushBack(texturePos[0]);
					m_coordTex.PushBack(texturePos[1]);
					m_coordTex.PushBack(texturePos[2]);
					// set display positions :
					m_coord.PushBack(bitmapDrawPos[0]);
					m_coord.PushBack(bitmapDrawPos[1]);
					m_coord.PushBack(bitmapDrawPos[2]);
					// set the color
					m_coordColor.PushBack(m_color);
					m_coordColor.PushBack(m_color);
					m_coordColor.PushBack(m_color);
					/* Step 2 : 
					 *              
					 *   **         
					 *   ****       
					 *   ******     
					 *   ********   
					 */
					// set texture coordonates :
					m_coordTex.PushBack(texturePos[0]);
					m_coordTex.PushBack(texturePos[2]);
					m_coordTex.PushBack(texturePos[3]);
					// set display positions :
					m_coord.PushBack(bitmapDrawPos[0]);
					m_coord.PushBack(bitmapDrawPos[2]);
					m_coord.PushBack(bitmapDrawPos[3]);
					// set the color
					m_coordColor.PushBack(m_color);
					m_coordColor.PushBack(m_color);
					m_coordColor.PushBack(m_color);
				}
			}
		}
	}
	// move the position :
	m_position.setX(m_position.x() + myGlyph->m_advance.x() + kerningOffset);
	// Register the previous character
	m_previousCharcode = charcode;
	return;
}

void ewol::Text::ForceLineReturn(void)
{
	// Reset position : 
	SetPos(vec3(m_startTextpos, m_position.y() - m_font->GetHeight(m_mode), 0) );
}


void ewol::Text::SetTextAlignement(float startTextpos, float stopTextPos, ewol::Text::aligneMode_te alignement)
{
	m_startTextpos = startTextpos;
	m_stopTextPos = stopTextPos+1;
	m_alignement = alignement;
	if (m_startTextpos >= m_stopTextPos) {
		EWOL_ERROR("Request allignement with Borne position error : " << startTextpos << " => " << stopTextPos);
	}
}


ewol::Text::aligneMode_te ewol::Text::GetAlignement(void)
{
	return m_alignement;
}


void ewol::Text::DisableAlignement(void)
{
	m_alignement = ewol::Text::alignDisable;
}


vec3 ewol::Text::CalculateSizeHTML(const etk::UString& text)
{
	// remove intermediate result 
	Reset();
	
	// disable display system
	m_needDisplay = false;
	
	SetPos(vec3(0,0,0) );
	// same as print without the end display ...
	PrintHTML(text);
	// get the last elements
	m_sizeDisplayStop.setValue(etk_max(m_position.x(), m_sizeDisplayStop.x()) ,
	                           etk_max(m_position.y(), m_sizeDisplayStop.y()) ,
	                           0);
	m_sizeDisplayStart.setValue(etk_min(m_position.x(), m_sizeDisplayStart.x()) ,
	                            etk_min(m_position.y(), m_sizeDisplayStart.y()) ,
	                            0);
	
	// set back the display system
	m_needDisplay = true;
	
	return vec3( m_sizeDisplayStop.x()-m_sizeDisplayStart.x(),
	             m_sizeDisplayStop.y()-m_sizeDisplayStart.y(),
	             m_sizeDisplayStop.z()-m_sizeDisplayStart.z());
}

vec3 ewol::Text::CalculateSizeDecorated(const etk::UString& text)
{
	etk::UString tmpData("<html><body>\n");
	tmpData+=text;
	tmpData+="\n</body></html>\n";
	vec3 tmpVal = CalculateSizeHTML(tmpData);
	return tmpVal;
}

vec3 ewol::Text::CalculateSize(const etk::UString& text)
{
	if (m_font == NULL) {
		EWOL_ERROR("Font Id is not corectly defined");
		return vec3(0,0,0);
	}
	vec3 outputSize(0, 0, 0);
	for(int32_t iii=0; iii<text.Size(); iii++) {
		vec3 tmpp = CalculateSize(text[iii]);
		if (outputSize.y() == 0) {
			outputSize.setY(tmpp.y());
		}
		outputSize.setX( outputSize.x() + tmpp.x());
	}
	return outputSize;
}

vec3 ewol::Text::CalculateSize(const uniChar_t charcode)
{
	if (m_font == NULL) {
		EWOL_ERROR("Font Id is not corectly defined");
		return vec3(0,0,0);
	}
	// get a pointer on the glyph property : 
	ewol::GlyphProperty * myGlyph = m_font->GetGlyphPointer(charcode, m_mode);
	int32_t fontHeigh = m_font->GetHeight(m_mode);
	
	// Get the kerning ofset :
	float kerningOffset = 0.0;
	if (true==m_kerning) {
		kerningOffset = myGlyph->KerningGet(m_previousCharcode);
	}
	
	vec3 outputSize((float)(myGlyph->m_advance.x() + kerningOffset),
	                (float)(fontHeigh),
	                (float)(0.0));
	// Register the previous character
	m_previousCharcode = charcode;
	return outputSize;
}


void ewol::Text::PrintCursor(bool isInsertMode)
{
	int32_t fontHeigh = m_font->GetHeight(m_mode);
	if (true == isInsertMode) {
		m_vectorialDraw.RectangleWidth(vec3(20, fontHeigh, 0) );
	} else {
		m_vectorialDraw.SetThickness(2);
		m_vectorialDraw.LineRel( vec3(0, fontHeigh, 0) );
		m_vectorialDraw.SetThickness(0);
	}
}


bool ewol::Text::ExtrapolateLastId(const etk::UString& text, const int32_t start, int32_t& stop, int32_t& space, int32_t& freeSpace)
{
	// store previous :
	uniChar_t storePrevious = m_previousCharcode;
	
	stop = text.Size();
	space = 0;
	
	int32_t lastSpacePosition = start;
	int32_t lastSpacefreeSize;
	
	float endPos = m_position.x();
	bool endOfLine = false;
	
	float stopPosition = m_stopTextPos;
	if(    false == m_needDisplay
	    || m_stopTextPos == m_startTextpos) {
		stopPosition = m_startTextpos + 3999999999.0;
	}
	
	for (int32_t iii=start; iii<text.Size(); iii++) {
		vec3 tmpSize = CalculateSize(text[iii]);
		// check oveflow :
		if (endPos + tmpSize.x() > stopPosition) {
			stop = iii;
			break;
		}
		// save number of space :
		if (text[iii] == (uniChar_t)' ') {
			space++;
			lastSpacePosition = iii;
			lastSpacefreeSize = stopPosition - endPos;
		} else if (text[iii] == (uniChar_t)'\n') {
			stop = iii;
			endOfLine = true;
			break;
		}
		// update local size :
		endPos += tmpSize.x();
	}
	freeSpace = stopPosition - endPos;
	// retore previous :
	m_previousCharcode = storePrevious;
	// need to align left or right ...
	if(stop == text.Size()) {
		return true;
	} else {
		if (endOfLine) {
			return true;
		} else {
			if (space == 0) {
				return true;
			}
			stop = lastSpacePosition;
			freeSpace = lastSpacefreeSize;
			return false;
		}
	}
}

void ewol::Text::HtmlAddData(etk::UString data)
{
	if(    m_htmlCurrrentLine.Size()>0
	    && m_htmlCurrrentLine[m_htmlCurrrentLine.Size()-1] != ' ') {
		m_htmlCurrrentLine+=" ";
		if(m_htmlDecoration.Size()>0) {
			TextDecoration tmp = m_htmlDecoration[m_htmlDecoration.Size()-1];
			m_htmlDecoration.PushBack(tmp);
		} else {
			m_htmlDecoration.PushBack(m_htmlDecoTmp);
		}
	}
	m_htmlCurrrentLine += data;
	for(int32_t iii=0; iii<data.Size() ; iii++) {
		m_htmlDecoration.PushBack(m_htmlDecoTmp);
	}
}


void ewol::Text::HtmlFlush(void)
{
	if (m_htmlCurrrentLine.Size()>0) {
		Print(m_htmlCurrrentLine, m_htmlDecoration);
	}
	m_htmlCurrrentLine = "";
	m_htmlDecoration.Clear();
}


void ewol::Text::DisableCursor(void)
{
	m_selectionStartPos = -100;
	m_cursorPos = -100;
}

void ewol::Text::SetCursorPos(int32_t cursorPos)
{
	m_selectionStartPos = cursorPos;
	m_cursorPos = cursorPos;
}

void ewol::Text::SetCursorSelection(int32_t cursorPos, int32_t selectionStartPos)
{
	m_selectionStartPos = selectionStartPos;
	m_cursorPos = cursorPos;
}

void ewol::Text::SetSelectionColor(draw::Color color)
{
	m_colorSelection = color;
}

void ewol::Text::SetCursorColor(draw::Color color)
{
	m_colorCursor = color;
}
