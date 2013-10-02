/**
 * @author Edouard DUPIN
 * 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * 
 * @license BSD v3 (see license file)
 */

#include <etk/types.h>
#include <etk/os/FSNode.h>
#include <egami/egami.h>

#include <ewol/resources/ResourceManager.h>
#include <ewol/renderer/eContext.h>

#include <ewol/resources/font/FontBase.h>
#include <ewol/resources/TexturedFont.h>
#include <ewol/resources/FontFreeType.h>


etk::CCout& ewol::operator <<(etk::CCout& _os, const ewol::font::mode_te& _obj)
{
	switch(_obj) {
		default :
			_os << "error";
			break;
		case ewol::font::Regular:
			_os << "Regular";
			break;
		case ewol::font::Italic:
			_os << "Italic";
			break;
		case ewol::font::Bold:
			_os << "Bold";
			break;
		case ewol::font::BoldItalic:
			_os << "BoldItalic";
			break;
	}
	return _os;
}

#undef __class__
#define __class__	"TexturedFont"

ewol::TexturedFont::TexturedFont(etk::UString fontName) : 
	ewol::Texture(fontName)
{
	m_font[0] = NULL;
	m_font[1] = NULL;
	m_font[2] = NULL;
	m_font[3] = NULL;
	
	m_modeWraping[0] = ewol::font::Regular;
	m_modeWraping[1] = ewol::font::Regular;
	m_modeWraping[2] = ewol::font::Regular;
	m_modeWraping[3] = ewol::font::Regular;
	
	m_lastGlyphPos[0].setValue(1,1);
	m_lastGlyphPos[1].setValue(1,1);
	m_lastGlyphPos[2].setValue(1,1);
	m_lastGlyphPos[3].setValue(1,1);
	
	m_lastRawHeigh[0] = 0;
	m_lastRawHeigh[1] = 0;
	m_lastRawHeigh[2] = 0;
	m_lastRawHeigh[3] = 0;
	
	int32_t tmpSize = 0;
	// extarct name and size :
	etk::Char tmpChar = fontName.c_str();
	const char * tmpData = tmpChar;
	const char * tmpPos = strchr(tmpData, ':');
	
	if (tmpPos==NULL) {
		m_size = 1;
		EWOL_CRITICAL("Can not parse the font name : \"" << fontName << "\" ??? ':' " );
		return;
	} else {
		if (sscanf(tmpPos+1, "%d", &tmpSize)!=1) {
			m_size = 1;
			EWOL_CRITICAL("Can not parse the font name : \"" << fontName << "\" ==> size ???");
			return;
		}
	}
	m_name = fontName.Extract(0, (tmpPos - tmpData));
	m_size = tmpSize;
	
	etk::Vector<etk::UString> folderList;
	if (true==ewol::GetContext().GetFontDefault().GetUseExternal()) {
		#if defined(__TARGET_OS__Android)
			folderList.PushBack("/system/fonts");
		#elif defined(__TARGET_OS__Linux)
			folderList.PushBack("/usr/share/fonts/truetype");
		#endif
	}
	folderList.PushBack(ewol::GetContext().GetFontDefault().GetFolder());
	for (int32_t folderID=0; folderID<folderList.Size() ; folderID++) {
		etk::FSNode myFolder(folderList[folderID]);
		// find the real Font name :
		etk::Vector<etk::UString> output;
		myFolder.FolderGetRecursiveFiles(output);
		etk::Vector<etk::UString> split = m_name.Split(';');
		EWOL_INFO("try to find font named : '" << split << "' in : '" << myFolder <<"'");
		//EWOL_CRITICAL("parse string : " << split);
		bool hasFindAFont = false;
		for (int32_t jjj=0; jjj<split.Size(); jjj++) {
			EWOL_INFO("    try with : '" << split[jjj] << "'");
			for (int32_t iii=0; iii<output.Size(); iii++) {
				//EWOL_DEBUG(" file : " << output[iii]);
				if(    true == output[iii].EndWith(split[jjj]+"-"+"bold"+".ttf", false)
				    || true == output[iii].EndWith(split[jjj]+"-"+"b"+".ttf", false)
				    || true == output[iii].EndWith(split[jjj]+"-"+"bd"+".ttf", false)
				    || true == output[iii].EndWith(split[jjj]+"bold"+".ttf", false)
				    || true == output[iii].EndWith(split[jjj]+"bd"+".ttf", false)
				    || true == output[iii].EndWith(split[jjj]+"b"+".ttf", false)) {
					EWOL_INFO(" find Font [Bold]        : " << output[iii]);
					m_fileName[ewol::font::Bold] = output[iii];
					hasFindAFont=true;
				} else if(    true == output[iii].EndWith(split[jjj]+"-"+"oblique"+".ttf", false)
				           || true == output[iii].EndWith(split[jjj]+"-"+"italic"+".ttf", false)
				           || true == output[iii].EndWith(split[jjj]+"-"+"Light"+".ttf", false)
				           || true == output[iii].EndWith(split[jjj]+"-"+"i"+".ttf", false)
				           || true == output[iii].EndWith(split[jjj]+"oblique"+".ttf", false)
				           || true == output[iii].EndWith(split[jjj]+"italic"+".ttf", false)
				           || true == output[iii].EndWith(split[jjj]+"light"+".ttf", false)
				           || true == output[iii].EndWith(split[jjj]+"i"+".ttf", false)) {
					EWOL_INFO(" find Font [Italic]      : " << output[iii]);
					m_fileName[ewol::font::Italic] = output[iii];
					hasFindAFont=true;
				} else if(    true == output[iii].EndWith(split[jjj]+"-"+"bolditalic"+".ttf", false)
				           || true == output[iii].EndWith(split[jjj]+"-"+"boldoblique"+".ttf", false)
				           || true == output[iii].EndWith(split[jjj]+"-"+"bi"+".ttf", false)
				           || true == output[iii].EndWith(split[jjj]+"-"+"z"+".ttf", false)
				           || true == output[iii].EndWith(split[jjj]+"bolditalic"+".ttf", false)
				           || true == output[iii].EndWith(split[jjj]+"boldoblique"+".ttf", false)
				           || true == output[iii].EndWith(split[jjj]+"bi"+".ttf", false)
				           || true == output[iii].EndWith(split[jjj]+"z"+".ttf", false)) {
					EWOL_INFO(" find Font [Bold-Italic] : " << output[iii]);
					m_fileName[ewol::font::BoldItalic] = output[iii];
					hasFindAFont=true;
				} else if(    true == output[iii].EndWith(split[jjj]+"-"+"regular"+".ttf", false)
				           || true == output[iii].EndWith(split[jjj]+"-"+"r"+".ttf", false)
				           || true == output[iii].EndWith(split[jjj]+"regular"+".ttf", false)
				           || true == output[iii].EndWith(split[jjj]+"r"+".ttf", false)
				           || true == output[iii].EndWith(split[jjj]+".ttf", false)) {
					EWOL_INFO(" find Font [Regular]     : " << output[iii]);
					m_fileName[ewol::font::Regular] = output[iii];
					hasFindAFont=true;
				}
			}
			if (hasFindAFont==true) {
				EWOL_INFO("    Find this font : '" << split[jjj] << "'");
				break;
			} else if (jjj==split.Size()-1) {
				EWOL_ERROR("Find NO font in the LIST ... " << split);
			}
		}
		if (hasFindAFont==true) {
			EWOL_INFO("    Find this font : '" << folderList[folderID] << "'");
			break;
		} else if (folderID==folderList.Size()-1) {
			EWOL_ERROR("Find NO font in the LIST ... " << folderList);
		}
	}
	// try to find the reference mode :
	ewol::font::mode_te refMode = ewol::font::Regular;
	for(int32_t iii=3; iii>=0; iii--) {
		if (m_fileName[iii].IsEmpty()==false) {
			refMode = (ewol::font::mode_te)iii;
		}
	}
	
	EWOL_DEBUG("         Set reference mode : " << refMode);
	// generate the wrapping on the preventing error
	for(int32_t iii=3; iii>=0; iii--) {
		if (m_fileName[iii].IsEmpty()==false) {
			m_modeWraping[iii] = (ewol::font::mode_te)iii;
		} else {
			m_modeWraping[iii] = refMode;
		}
	}
	
	for (int32_t iiiFontId=0; iiiFontId<4 ; iiiFontId++) {
		if (m_fileName[iiiFontId].IsEmpty()==true) {
			EWOL_DEBUG("can not load FONT [" << iiiFontId << "] name : \"" << m_fileName[iiiFontId] << "\" ==> size=" << m_size );
			m_font[iiiFontId] = NULL;
			continue;
		}
		EWOL_INFO("Load FONT [" << iiiFontId << "] name : \"" << m_fileName[iiiFontId] << "\" ==> size=" << m_size);
		m_font[iiiFontId] = ewol::FontFreeType::Keep(m_fileName[iiiFontId]);
		if (m_font[iiiFontId] == NULL) {
			EWOL_DEBUG("error in loading FONT [" << iiiFontId << "] name : \"" << m_fileName[iiiFontId] << "\" ==> size=" << m_size );
		}
	}
	for (int32_t iiiFontId=0; iiiFontId<4 ; iiiFontId++) {
		// set the bassic charset:
		m_listElement[iiiFontId].Clear();
		if (m_font[iiiFontId] == NULL) {
			continue;
		}
		m_height[iiiFontId] = m_font[iiiFontId]->GetHeight(m_size);
		// TODO : basic font use 512 is better ... ==> maybe estimate it with the dpi ???
		SetImageSize(ivec2(256,32));
		// now we can acces directly on the image
		m_data.Clear(etk::Color<>(0x00000000));
	}
	// Add error glyph
	{
		etk::UniChar tmpchar;
		tmpchar.Set(0);
		AddGlyph(tmpchar);
	}
	// by default we set only the first AINSI char availlable
	for (int32_t iii=0x20; iii<0x7F; iii++) {
		etk::UniChar tmpchar;
		tmpchar.Set(iii);
		AddGlyph(tmpchar);
	}
	Flush();
	EWOL_DEBUG("Wrapping properties : ");
	EWOL_DEBUG("    " << ewol::font::Regular << "==>" << GetWrappingMode(ewol::font::Regular));
	EWOL_DEBUG("    " << ewol::font::Italic << "==>" << GetWrappingMode(ewol::font::Italic));
	EWOL_DEBUG("    " << ewol::font::Bold << "==>" << GetWrappingMode(ewol::font::Bold));
	EWOL_DEBUG("    " << ewol::font::BoldItalic << "==>" << GetWrappingMode(ewol::font::BoldItalic));
}

ewol::TexturedFont::~TexturedFont(void)
{
	for (int32_t iiiFontId=0; iiiFontId<4 ; iiiFontId++) {
		ewol::FontFreeType::Release(m_font[iiiFontId]);
	}
}

bool ewol::TexturedFont::AddGlyph(const etk::UniChar& _val)
{
	bool hasChange = false;
	// for each font :
	for (int32_t iii=0; iii<4 ; iii++) {
		if (m_font[iii] == NULL) {
			continue;
		}
		// add the curent "char"
		GlyphProperty tmpchar;
		tmpchar.m_UVal = _val;
		
		if (true == m_font[iii]->GetGlyphProperty(m_size, tmpchar)) {
			//EWOL_DEBUG("load char : '" << _val << "'=" << _val.Get());
			hasChange = true;
			// change line if needed ...
			if (m_lastGlyphPos[iii].x()+tmpchar.m_sizeTexture.x() > m_data.GetSize().x()) {
				m_lastGlyphPos[iii].setX(1);
				m_lastGlyphPos[iii] += ivec2(0, m_lastRawHeigh[iii]);
				m_lastRawHeigh[iii] = 0;
			}
			while(m_lastGlyphPos[iii].y()+tmpchar.m_sizeTexture.y() > m_data.GetSize().y()) {
				ivec2 size = m_data.GetSize();
				size.setY(size.y()*2);
				m_data.Resize(size, etk::Color<>(0));
				// note : need to rework all the lyer due to the fact that the texture is used by the faur type...
				for (int32_t kkk=0; kkk<4 ; kkk++) {
					// change the coordonate on the element in the texture
					for (int32_t jjj=0 ; jjj<m_listElement[kkk].Size() ; ++jjj) {
						m_listElement[kkk][jjj].m_texturePosStart *= vec2(1.0f, 0.5f);
						m_listElement[kkk][jjj].m_texturePosSize *= vec2(1.0f, 0.5f);
					}
				}
			}
			// draw the glyph
			m_font[iii]->DrawGlyph(m_data, m_size, m_lastGlyphPos[iii], tmpchar, iii);
			// set video position
			tmpchar.m_texturePosStart.setValue( (float)m_lastGlyphPos[iii].x() / (float)m_data.GetSize().x(),
			                                    (float)m_lastGlyphPos[iii].y() / (float)m_data.GetSize().y() );
			tmpchar.m_texturePosSize.setValue(  (float)tmpchar.m_sizeTexture.x() / (float)m_data.GetSize().x(),
			                                    (float)tmpchar.m_sizeTexture.y() / (float)m_data.GetSize().y() );
			
			// update the maximum of the line hight : 
			if (m_lastRawHeigh[iii]<tmpchar.m_sizeTexture.y()) {
				// note : +1 is for the overlapping of the glyph (Part 2)
				m_lastRawHeigh[iii] = tmpchar.m_sizeTexture.y()+1;
			}
			// note : +1 is for the overlapping of the glyph (Part 3)
			// update the Bitmap position drawing : 
			m_lastGlyphPos[iii] += ivec2(tmpchar.m_sizeTexture.x()+1, 0);
		} else {
			EWOL_WARNING("Did not find char : '" << _val << "'=" << _val.Get());
			tmpchar.SetNotExist();
		}
		m_listElement[iii].PushBack(tmpchar);
		//m_font[iii]->Display();
		// generate the kerning for all the characters :
		if (tmpchar.Exist() == true) {
			// TODO : Set the kerning back ...
			//m_font[iii]->GenerateKerning(m_size, m_listElement[iii]);
		}
	}
	if (hasChange==true) {
		Flush();
		egami::Store(m_data, "fileFont.bmp");
	}
	return hasChange;
}


bool ewol::TexturedFont::HasName(const etk::UString& fileName)
{
	etk::UString tmpName = m_name;
	tmpName += ":";
	tmpName += m_size;
	EWOL_VERBOSE("S : check : " << fileName << " ?= " << tmpName << " = " << (fileName==tmpName) );
	return (fileName==tmpName);
}


int32_t ewol::TexturedFont::GetIndex(const uniChar_t& charcode, const ewol::font::mode_te displayMode)
{
	if (charcode.Get() < 0x20) {
		return 0;
	} else if (charcode.Get() < 0x80) {
		return charcode.Get() - 0x1F;
	} else {
		for (int32_t iii=0x80-0x20; iii < m_listElement[displayMode].Size(); iii++) {
			//EWOL_DEBUG("search : '" << charcode << "' =?= '" << (m_listElement[displayMode])[iii].m_UVal << "'");
			if (charcode == (m_listElement[displayMode])[iii].m_UVal) {
				//EWOL_DEBUG("search : '" << charcode << "'");
				if ((m_listElement[displayMode])[iii].Exist()) {
					//EWOL_DEBUG("return " << iii);
					return iii;
				} else {
					return 0;
				}
			}
		}
	}
	if (AddGlyph(charcode) == true) {
		// TODO : This does not work due to the fact that the update of open GL is not done in the context main cycle !!!
		ewol::GetContext().ForceRedrawAll();
	}
	return 0;
}


ewol::GlyphProperty* ewol::TexturedFont::GetGlyphPointer(const uniChar_t& _charcode, const ewol::font::mode_te _displayMode)
{
	//EWOL_DEBUG("Get glyph property for mode: " << _displayMode << " ==> wrapping index : " << m_modeWraping[_displayMode]);
	int32_t index = GetIndex(_charcode, _displayMode);
	if(    index < 0
	    || index >= m_listElement[_displayMode].Size() ) {
		EWOL_ERROR(" Try to get glyph index inexistant ... ==> return the index 0 ... id=" << index);
		if (m_listElement[_displayMode].Size()>=0) {
			return &((m_listElement[_displayMode])[0]);
		}
		return NULL;
	}
	//EWOL_ERROR("      index=" << index);
	//EWOL_ERROR("      m_UVal=" << m_listElement[_displayMode][index].m_UVal);
	//EWOL_ERROR("      m_glyphIndex=" << m_listElement[_displayMode][index].m_glyphIndex);
	//EWOL_ERROR("      m_advance=" << m_listElement[_displayMode][index].m_advance);
	//EWOL_ERROR("      m_bearing=" << m_listElement[_displayMode][index].m_bearing);
	return &((m_listElement[_displayMode])[index]);
}



ewol::TexturedFont* ewol::TexturedFont::Keep(const etk::UString& _filename)
{
	EWOL_VERBOSE("KEEP : TexturedFont : file : \"" << _filename << "\"");
	ewol::TexturedFont* object = static_cast<ewol::TexturedFont*>(GetManager().LocalKeep(_filename));
	if (NULL != object) {
		return object;
	}
	// need to crate a new one ...
	object = new ewol::TexturedFont(_filename);
	if (NULL == object) {
		EWOL_ERROR("allocation error of a resource : " << _filename);
		return NULL;
	}
	GetManager().LocalAdd(object);
	return object;
}

void ewol::TexturedFont::Release(ewol::TexturedFont*& _object)
{
	if (NULL == _object) {
		return;
	}
	ewol::Resource* object2 = static_cast<ewol::Resource*>(_object);
	GetManager().Release(object2);
	_object = NULL;
}