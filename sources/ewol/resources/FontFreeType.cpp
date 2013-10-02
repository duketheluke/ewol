/**
 * @author Edouard DUPIN
 * 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * 
 * @license BSD v3 (see license file)
 */

#include <etk/unicode.h>
#include <etk/Vector.h>
#include <etk/os/FSNode.h>


#include <ewol/renderer/openGL.h>

#include <ewol/resources/Texture.h>
#include <ewol/resources/FontFreeType.h>
#include <ewol/resources/font/FontBase.h>
#include <ewol/resources/ResourceManager.h>


#undef __class__
#define __class__	"FontFreeType"


// free Font hnadle of librairies ... entry for acces ...
static int32_t l_countLoaded=0;
static FT_Library library;

void ewol::FreeTypeInit(void)
{
	EWOL_DEBUG("==> Init Font-Manager");
	l_countLoaded++;
	if (l_countLoaded>1) {
		// already loaded ...
		return;
	}
	int32_t error = FT_Init_FreeType( &library );
	if(0 != error) {
		EWOL_CRITICAL(" when loading FreeType Librairy ...");
	}
}

void ewol::FreeTypeUnInit(void)
{
	EWOL_DEBUG("==> Un-Init Font-Manager");
	l_countLoaded--;
	if (l_countLoaded>0) {
		// already needed ...
		return;
	}
	int32_t error = FT_Done_FreeType( library );
	library = NULL;
	if(0 != error) {
		EWOL_CRITICAL(" when Un-loading FreeType Librairy ...");
	}
}



ewol::FontFreeType::FontFreeType(const etk::UString& _fontName) :
	FontBase(_fontName)
{
	m_init = false;
	m_FileBuffer = NULL;
	m_FileSize = 0;
	
	etk::FSNode myfile(_fontName);
	if (false == myfile.Exist()) {
		EWOL_ERROR("File Does not exist : " << myfile);
		return;
	}
	m_FileSize = myfile.FileSize();
	if (0==m_FileSize) {
		EWOL_ERROR("This file is empty : " << myfile);
		return;
	}
	if (false == myfile.FileOpenRead()) {
		EWOL_ERROR("Can not open the file : " << myfile);
		return;
	}
	// allocate data
	m_FileBuffer = new FT_Byte[m_FileSize];
	if (NULL == m_FileBuffer) {
		EWOL_ERROR("Error Memory allocation size=" << _fontName);
		return;
	}
	// load data from the file :
	myfile.FileRead(m_FileBuffer, 1, m_FileSize);
	// close the file:
	myfile.FileClose();
	// load Face ...
	int32_t error = FT_New_Memory_Face( library, m_FileBuffer, m_FileSize, 0, &m_fftFace );
	if( FT_Err_Unknown_File_Format == error) {
		EWOL_ERROR("... the font file could be opened and read, but it appears ... that its font format is unsupported");
	} else if (0 != error) {
		EWOL_ERROR("... another error code means that the font file could not ... be opened or read, or simply that it is broken...");
	} else {
		// all OK
		EWOL_INFO("load font : \"" << _fontName << "\" ");
		//Display();
		m_init = true;
	}
}

ewol::FontFreeType::~FontFreeType(void)
{
	// clean the tmp memory
	if (NULL != m_FileBuffer) {
		delete[] m_FileBuffer;
		m_FileBuffer = NULL;
	}
	// must be deleted fftFace
	FT_Done_Face( m_fftFace );
}


vec2 ewol::FontFreeType::GetSize(int32_t _fontSize, const etk::UString& _unicodeString)
{
	if(false==m_init) {
		return vec2(0,0);
	}
	// TODO : ...
	vec2 outputSize(0,0);
	return outputSize;
}

int32_t ewol::FontFreeType::GetHeight(int32_t _fontSize)
{
	return _fontSize*1.43f; // this is a really "magic" number ...
}

bool ewol::FontFreeType::GetGlyphProperty(int32_t _fontSize, ewol::GlyphProperty& _property)
{
	if(false==m_init) {
		return false;
	}
	// 300dpi (hight quality) 96 dpi (normal quality)
	int32_t fontQuality = 96;
	// Select Size ...
	// note tha <<6==*64 corespond with the 1/64th of points calculation of freetype
	int32_t error = FT_Set_Char_Size(m_fftFace, _fontSize<<6, _fontSize<<6, fontQuality, fontQuality);
	if (0!=error ) {
		EWOL_ERROR("FT_Set_Char_Size ==> error in settings ...");
		return false;
	}
	// a small shortcut
	FT_GlyphSlot slot = m_fftFace->glyph;
	// retrieve glyph index from character code 
	int32_t glyph_index = FT_Get_Char_Index(m_fftFace, _property.m_UVal.Get());
	// load glyph image into the slot (erase previous one)
	error = FT_Load_Glyph(m_fftFace, // handle to face object
	                      glyph_index, // glyph index
	                      FT_LOAD_DEFAULT );
	if (0!=error ) {
		EWOL_ERROR("FT_Load_Glyph specify Glyph");
		return false;
	}
	// convert to an anti-aliased bitmap
	error = FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL );
	if (0!=error) {
		EWOL_ERROR("FT_Render_Glyph");
		return false;
	}
	// set properties :
	_property.m_glyphIndex = glyph_index;
	_property.m_sizeTexture.setValue(slot->bitmap.width, slot->bitmap.rows);
	_property.m_bearing.setValue( slot->metrics.horiBearingX>>6 , slot->metrics.horiBearingY>>6 );
	_property.m_advance.setValue( slot->metrics.horiAdvance>>6 , slot->metrics.vertAdvance>>6 );
	
	return true;
}

bool ewol::FontFreeType::DrawGlyph(egami::Image& _imageOut,
                                   int32_t _fontSize,
                                   ivec2 _glyphPosition,
                                   ewol::GlyphProperty& _property,
                                   int8_t _posInImage)
{

	if(false==m_init) {
		return false;
	}
	// 300dpi (hight quality) 96 dpi (normal quality)
	int32_t fontQuality = 96;
	// Select Size ...
	// note tha <<6==*64 corespond with the 1/64th of points calculation of freetype
	int32_t error = FT_Set_Char_Size(m_fftFace, _fontSize<<6, _fontSize<<6, fontQuality, fontQuality);
	if (0!=error ) {
		EWOL_ERROR("FT_Set_Char_Size ==> error in settings ...");
		return false;
	}
	// a small shortcut
	FT_GlyphSlot slot = m_fftFace->glyph;
	// load glyph image into the slot (erase previous one)
	error = FT_Load_Glyph(m_fftFace, // handle to face object
	                      _property.m_glyphIndex, // glyph index
	                      FT_LOAD_DEFAULT );
	if (0!=error ) {
		EWOL_ERROR("FT_Load_Glyph specify Glyph");
		return false;
	}
	// convert to an anti-aliased bitmap
	error = FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL );
	if (0!=error) {
		EWOL_ERROR("FT_Render_Glyph");
		return false;
	}
	// draw it on the output Image :
	etk::Color<> tlpppp(0xFFFFFF00);
	for(int32_t jjj=0; jjj < slot->bitmap.rows;jjj++) {
		for(int32_t iii=0; iii < slot->bitmap.width; iii++){
			tlpppp = _imageOut.Get(ivec2(_glyphPosition.x()+iii, _glyphPosition.y()+jjj));
			uint8_t valueColor = slot->bitmap.buffer[iii + slot->bitmap.width*jjj];
			// set only alpha :
			switch(_posInImage)
			{
				default:
				case 0:
					tlpppp.SetA(valueColor);
					break;
				case 1:
					tlpppp.SetR(valueColor);
					break;
				case 2:
					tlpppp.SetG(valueColor);
					break;
				case 3:
					tlpppp.SetB(valueColor);
					break;
			}
			// real set of color
			_imageOut.Set(ivec2(_glyphPosition.x()+iii, _glyphPosition.y()+jjj), tlpppp );
		}
	}
	return true;
}


void ewol::FontFreeType::GenerateKerning(int32_t fontSize, etk::Vector<ewol::GlyphProperty>& listGlyph)
{
	if(false==m_init) {
		return;
	}
	if ((FT_FACE_FLAG_KERNING & m_fftFace->face_flags) == 0) {
		EWOL_INFO("No kerning generation (Disable) in the font");
	}
	// 300dpi (hight quality) 96 dpi (normal quality)
	int32_t fontQuality = 96;
	// Select Size ...
	// note tha <<6==*64 corespond with the 1/64th of points calculation of freetype
	int32_t error = FT_Set_Char_Size(m_fftFace, fontSize<<6, fontSize<<6, fontQuality, fontQuality);
	if (0!=error ) {
		EWOL_ERROR("FT_Set_Char_Size ==> error in settings ...");
		return;
	}
	// For all the kerning element we get the kerning value :
	for(int32_t iii=0; iii<listGlyph.Size(); iii++) {
		listGlyph[iii].KerningClear();
		for(int32_t kkk=0; kkk<listGlyph.Size(); kkk++) {
			FT_Vector kerning;
			FT_Get_Kerning(m_fftFace, listGlyph[kkk].m_glyphIndex, listGlyph[iii].m_glyphIndex, FT_KERNING_UNFITTED, &kerning );
			// add the kerning only if != 0 ... 
			if (kerning.x != 0) {
				listGlyph[iii].KerningAdd(listGlyph[kkk].m_UVal,
				                          kerning.x/32.0f );
				//EWOL_DEBUG("Kerning between : '" << (char)listGlyph[iii].m_UVal << "'&'" << (char)listGlyph[kkk].m_UVal << "' value : " << kerning.x << " => " << (kerning.x/64.0f));
			}
		}
	}
}


void ewol::FontFreeType::Display(void)
{
	if(false==m_init) {
		return;
	}
	EWOL_INFO("    nuber of glyph       = " << (int)m_fftFace->num_glyphs);
	if ((FT_FACE_FLAG_SCALABLE & m_fftFace->face_flags) != 0) {
		EWOL_INFO("    flags                = FT_FACE_FLAG_SCALABLE (enable)");
	} else {
		EWOL_DEBUG("    flags                = FT_FACE_FLAG_SCALABLE (disable)");
	}
	if ((FT_FACE_FLAG_FIXED_SIZES & m_fftFace->face_flags) != 0) {
			EWOL_INFO("    flags                = FT_FACE_FLAG_FIXED_SIZES (enable)");
	} else {
		EWOL_DEBUG("    flags                = FT_FACE_FLAG_FIXED_SIZES (disable)");
	}
	if ((FT_FACE_FLAG_FIXED_WIDTH & m_fftFace->face_flags) != 0) {
		EWOL_INFO("    flags                = FT_FACE_FLAG_FIXED_WIDTH (enable)");
	} else {
		EWOL_DEBUG("    flags                = FT_FACE_FLAG_FIXED_WIDTH (disable)");
	}
	if ((FT_FACE_FLAG_SFNT & m_fftFace->face_flags) != 0) {
		EWOL_INFO("    flags                = FT_FACE_FLAG_SFNT (enable)");
	} else {
		EWOL_DEBUG("    flags                = FT_FACE_FLAG_SFNT (disable)");
	}
	if ((FT_FACE_FLAG_HORIZONTAL & m_fftFace->face_flags) != 0) {
		EWOL_INFO("    flags                = FT_FACE_FLAG_HORIZONTAL (enable)");
	} else {
		EWOL_DEBUG("    flags                = FT_FACE_FLAG_HORIZONTAL (disable)");
	}
	if ((FT_FACE_FLAG_VERTICAL & m_fftFace->face_flags) != 0) {
		EWOL_INFO("    flags                = FT_FACE_FLAG_VERTICAL (enable)");
	} else {
		EWOL_DEBUG("    flags                = FT_FACE_FLAG_VERTICAL (disable)");
	}
	if ((FT_FACE_FLAG_KERNING & m_fftFace->face_flags) != 0) {
		EWOL_INFO("    flags                = FT_FACE_FLAG_KERNING (enable)");
	} else {
		EWOL_DEBUG("    flags                = FT_FACE_FLAG_KERNING (disable)");
	}
	/* Deprecated flag
	if ((FT_FACE_FLAG_FAST_GLYPHS & face->face_flags) != 0) {
		EWOL_INFO("    flags                = FT_FACE_FLAG_FAST_GLYPHS (enable)");
	} else {
		EWOL_DEBUG("    flags                = FT_FACE_FLAG_FAST_GLYPHS (disable)");
	}
	*/
	if ((FT_FACE_FLAG_MULTIPLE_MASTERS & m_fftFace->face_flags) != 0) {
		EWOL_INFO("    flags                = FT_FACE_FLAG_MULTIPLE_MASTERS (enable)");
	} else {
		EWOL_DEBUG("    flags                = FT_FACE_FLAG_MULTIPLE_MASTERS (disable)");
	}
	if ((FT_FACE_FLAG_GLYPH_NAMES & m_fftFace->face_flags) != 0) {
		EWOL_INFO("    flags                = FT_FACE_FLAG_GLYPH_NAMES (enable)");
	} else {
		EWOL_DEBUG("    flags                = FT_FACE_FLAG_GLYPH_NAMES (disable)");
	}
	if ((FT_FACE_FLAG_EXTERNAL_STREAM & m_fftFace->face_flags) != 0) {
		EWOL_INFO("    flags                = FT_FACE_FLAG_EXTERNAL_STREAM (enable)");
	} else {
		EWOL_DEBUG("    flags                = FT_FACE_FLAG_EXTERNAL_STREAM (disable)");
	}
	if ((FT_FACE_FLAG_HINTER & m_fftFace->face_flags) != 0) {
		EWOL_INFO("    flags                = FT_FACE_FLAG_HINTER (enable)");
	} else {
		EWOL_DEBUG("    flags                = FT_FACE_FLAG_HINTER (disable)");
	}
	if ((FT_FACE_FLAG_CID_KEYED & m_fftFace->face_flags) != 0) {
		EWOL_INFO("    flags                = FT_FACE_FLAG_CID_KEYED (enable)");
	} else {
		EWOL_DEBUG("    flags                = FT_FACE_FLAG_CID_KEYED (disable)");
	}
	/*
	if ((FT_FACE_FLAG_TRICKY & m_fftFace->face_flags) != 0) {
		EWOL_INFO("    flags                = FT_FACE_FLAG_TRICKY (enable)");
	} else {
		EWOL_DEBUG("    flags                = FT_FACE_FLAG_TRICKY (disable)");
	}
	*/
	EWOL_INFO("    unit per EM          = " << m_fftFace->units_per_EM);
	EWOL_INFO("    num of fixed sizes   = " << m_fftFace->num_fixed_sizes);
	//EWOL_INFO("    Availlable sizes     = " << (int)m_fftFace->available_sizes);
	
	//EWOL_INFO("    Current size         = " << (int)m_fftFace->size);
}



ewol::FontBase* ewol::FontFreeType::Keep(const etk::UString& _filename)
{
	EWOL_VERBOSE("KEEP : Font : file : \"" << _filename << "\"");
	ewol::FontBase* object = static_cast<ewol::FontBase*>(GetManager().LocalKeep(_filename));
	if (NULL != object) {
		return object;
	}
	// need to crate a new one ...
	object = new ewol::FontFreeType(_filename);
	if (NULL == object) {
		EWOL_ERROR("allocation error of a resource : " << _filename);
		return NULL;
	}
	GetManager().LocalAdd(object);
	return object;
}

void ewol::FontFreeType::Release(ewol::FontBase*& _object)
{
	if (NULL == _object) {
		return;
	}
	ewol::Resource* object2 = static_cast<ewol::Resource*>(_object);
	GetManager().Release(object2);
	_object = NULL;
}