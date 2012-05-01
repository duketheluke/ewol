/**
 *******************************************************************************
 * @file ewol/widgetMeta/FileChooser.cpp
 * @brief ewol File chooser meta widget system (Sources)
 * @author Edouard DUPIN
 * @date 29/12/2011
 * @par Project
 * ewol
 *
 * @par Copyright
 * Copyright 2011 Edouard DUPIN, all right reserved
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY.
 *
 * Licence summary : 
 *    You can modify and redistribute the sources code and binaries.
 *    You can send me the bug-fix
 *
 * Term of the licence in in the file licence.txt.
 *
 *******************************************************************************
 */

#include <ewol/widgetMeta/FileChooser.h>
#include <ewol/widget/SizerHori.h>
#include <ewol/widget/SizerVert.h>
#include <ewol/widget/List.h>
#include <ewol/widget/Spacer.h>
#include <ewol/WidgetManager.h>
//#include <etk/Vector.h>
#include <etk/VectorType.h>

extern "C" {
	// file browsing ...
	#include <dirent.h>
}

#undef __class__
#define __class__	"FileChooser(FolderList)"


#include <ewol/ewol.h>

void SortList(etk::VectorType<etk::UString *> &m_listDirectory)
{
	etk::VectorType<etk::UString *> tmpList = m_listDirectory;
	m_listDirectory.Clear();
	for(int32_t iii=0; iii<tmpList.Size(); iii++) {
		
		int32_t findPos = 0;
		for(int32_t jjj=0; jjj<m_listDirectory.Size(); jjj++) {
			//EWOL_DEBUG("compare : \""<<*tmpList[iii] << "\" and \"" << *m_listDirectory[jjj] << "\"");
			if (*tmpList[iii] > *m_listDirectory[jjj]) {
				findPos = jjj+1;
			}
		}
		//EWOL_DEBUG("position="<<findPos);
		m_listDirectory.Insert(findPos, tmpList[iii]);
	}
}



const char * const ewolEventFileChooserSelectFolder   = "ewol-event-file-chooser-Select-Folder";
//!< EObject name :
extern const char * const TYPE_EOBJECT_WIDGET_FOLDER_LIST = "FileChooserFolderList";

class FileChooserFolderList : public ewol::List
{
	private:
		etk::VectorType<etk::UString *> m_listDirectory;
		int32_t                        m_selectedLine;
	public:
		FileChooserFolderList(void)
		{
			AddEventId(ewolEventFileChooserSelectFolder);
			m_selectedLine = -1;
		};
		~FileChooserFolderList(void)
		{
			ClearElements();
		};
		
		void AddElement(etk::UString element)
		{
			etk::UString* tmpEmement = new etk::UString(element);
			m_listDirectory.PushBack(tmpEmement);
		}
		void EndGenerating(void)
		{
			SortList(m_listDirectory);
			MarkToReedraw();
		}
		
		void ClearElements(void) {
			for (int32_t iii=0; iii<m_listDirectory.Size(); iii++) {
				if (NULL != m_listDirectory[iii]) {
					delete(m_listDirectory[iii]);
					m_listDirectory[iii] = NULL;
				}
			}
			m_listDirectory.Clear();
			MarkToReedraw();
		}
		
		etk::UString GetSelectedLine(void)
		{
			etk::UString tmpVal = "";
			if (m_selectedLine >= 0) {
				tmpVal = *(m_listDirectory[m_selectedLine]);
			}
			return tmpVal;
		}
		
		
		virtual color_ts GetBasicBG(void) {
			color_ts bg;
			bg.red = 0;
			bg.green = 0;
			bg.blue = 0;
			bg.alpha = 0x30;
			return bg;
		}
		
		uint32_t GetNuberOfColomn(void) {
			return 1;
		};
		bool GetTitle(int32_t colomn, etk::UString &myTitle, color_ts &fg, color_ts &bg) {
			myTitle = "title";
			return true;
		};
		uint32_t GetNuberOfRaw(void) {
			return m_listDirectory.Size();
		};
		bool GetElement(int32_t colomn, int32_t raw, etk::UString &myTextToWrite, color_ts &fg, color_ts &bg) {
			if (raw >= 0 && raw < m_listDirectory.Size()) {
				myTextToWrite = *(m_listDirectory[raw]);
			} else {
				myTextToWrite = "ERROR";
			}
			fg = etk::color::color_Black;
			if (raw % 2) {
				bg = etk::color::color_White;
			} else {
				bg.red = 0x7F;
				bg.green = 0x7F;
				bg.blue = 0x7F;
				bg.alpha = 0xFF;
			}
			if (m_selectedLine == raw) {
				bg.red = 0x8F;
				bg.green = 0x8F;
				bg.blue = 0xFF;
				bg.alpha = 0xFF;
			}
			return true;
		};
		
		bool OnItemEvent(int32_t IdInput, ewol::eventInputType_te typeEvent, int32_t colomn, int32_t raw, etkFloat_t x, etkFloat_t y) {
			if (typeEvent == ewol::EVENT_INPUT_TYPE_SINGLE) {
				EWOL_INFO("Event on List : IdInput=" << IdInput << " colomn=" << colomn << " raw=" << raw );
				if (1 == IdInput) {
					if (raw > m_listDirectory.Size() ) {
						m_selectedLine = -1;
					} else {
						m_selectedLine = raw;
					}
					// need to regenerate the display of the list : 
					MarkToReedraw();
					return true;
				}
			}
			if (typeEvent == ewol::EVENT_INPUT_TYPE_DOUBLE) {
				EWOL_INFO("Event Double on List : IdInput=" << IdInput << " colomn=" << colomn << " raw=" << raw );
				if (1 == IdInput) {
					if (m_selectedLine >=0 ) {
						// generate event extern : 
						GenerateEventId(ewolEventFileChooserSelectFolder);
						return true;
					}
				}
			}
			return false;
		}
	
	public:
		/**
		 * @brief Check if the object has the specific type.
		 * @note In Embended platforme, it is many time no -rtti flag, then it is not possible to use dynamic cast ==> this will replace it
		 * @param[in] objectType type of the object we want to check
		 * @return true if the object is compatible, otherwise false
		 */
		bool CheckObjectType(const char * const objectType)
		{
			if (NULL == objectType) {
				EWOL_ERROR("check error : \"" << TYPE_EOBJECT_WIDGET_FOLDER_LIST << "\" != NULL(pointer) ");
				return false;
			}
			if (objectType == TYPE_EOBJECT_WIDGET_FOLDER_LIST) {
				return true;
			} else {
				if(true == ewol::List::CheckObjectType(objectType)) {
					return true;
				}
				EWOL_ERROR("check error : \"" << TYPE_EOBJECT_WIDGET_FOLDER_LIST << "\" != \"" << objectType << "\"");
				return false;
			}
		}
		
		/**
		 * @brief Get the current Object type of the EObject
		 * @note In Embended platforme, it is many time no -rtti flag, then it is not possible to use dynamic cast ==> this will replace it
		 * @param[in] objectType type description
		 * @return true if the object is compatible, otherwise false
		 */
		const char * const GetObjectType(void)
		{
			return TYPE_EOBJECT_WIDGET_FOLDER_LIST;
		}
};
#define EWOL_CAST_WIDGET_FOLDER_LIST(curentPointer) EWOL_CAST(TYPE_EOBJECT_WIDGET_FOLDER_LIST,FileChooserFolderList,curentPointer)

#undef __class__
#define __class__	"FileChooser(FileList)"

const char * const ewolEventFileChooserSelectFile   = "ewol-event-file-chooser-Select-File";
const char * const ewolEventFileChooserValidateFile   = "ewol-event-file-chooser-Validate-File";
//!< EObject name :
extern const char * const TYPE_EOBJECT_WIDGET_FILE_LIST = "FileChooserFileList";

class FileChooserFileList : public ewol::List
{
	private:
		etk::VectorType<etk::UString *> m_listFile;
		int32_t                        m_selectedLine;
	public:
		FileChooserFileList(void)
		{
			m_selectedLine = -1;
			AddEventId(ewolEventFileChooserSelectFile);
			AddEventId(ewolEventFileChooserValidateFile);
		};
		~FileChooserFileList(void)
		{
			ClearElements();
		};
		virtual color_ts GetBasicBG(void) {
			color_ts bg;
			bg.red = 0;
			bg.green = 0;
			bg.blue = 0;
			bg.alpha = 0x30;
			return bg;
		}
		void AddElement(etk::UString element)
		{
			etk::UString* tmpEmement = new etk::UString(element);
			m_listFile.PushBack(tmpEmement);
		}
		void EndGenerating(void)
		{
			SortList(m_listFile);
			MarkToReedraw();
		}
		void ClearElements(void) {
			for (int32_t iii=0; iii<m_listFile.Size(); iii++) {
				if (NULL != m_listFile[iii]) {
					delete(m_listFile[iii]);
					m_listFile[iii] = NULL;
				}
			}
			m_listFile.Clear();
			MarkToReedraw();
		}
		
		etk::UString GetSelectedLine(void)
		{
			etk::UString tmpVal = "";
			if (m_selectedLine >= 0) {
				tmpVal = *(m_listFile[m_selectedLine]);
			}
			return tmpVal;
		}
		
		
		uint32_t GetNuberOfColomn(void) {
			return 1;
		};
		bool GetTitle(int32_t colomn, etk::UString &myTitle, color_ts &fg, color_ts &bg) {
			myTitle = "title";
			return true;
		};
		uint32_t GetNuberOfRaw(void) {
			return m_listFile.Size();
		};
		bool GetElement(int32_t colomn, int32_t raw, etk::UString &myTextToWrite, color_ts &fg, color_ts &bg) {
			if (raw >= 0 && raw < m_listFile.Size()) {
				myTextToWrite = *(m_listFile[raw]);
			} else {
				myTextToWrite = "ERROR";
			}
			fg = etk::color::color_Black;
			if (raw % 2) {
				bg.red = 0xCF;
				bg.green = 0xFF;
				bg.blue = 0xFF;
				bg.alpha = 0xFF;
			} else {
				bg.red = 0x9F;
				bg.green = 0x9F;
				bg.blue = 0x9F;
				bg.alpha = 0xFF;
			}
			if (m_selectedLine == raw) {
				bg.red = 0x8F;
				bg.green = 0x8F;
				bg.blue = 0xFF;
				bg.alpha = 0xFF;
			}
			return true;
		};
		
		bool OnItemEvent(int32_t IdInput, ewol::eventInputType_te typeEvent, int32_t colomn, int32_t raw, etkFloat_t x, etkFloat_t y) {
			if (typeEvent == ewol::EVENT_INPUT_TYPE_SINGLE) {
				EWOL_INFO("Event on List : IdInput=" << IdInput << " colomn=" << colomn << " raw=" << raw );
				if (1 == IdInput) {
					if (raw > m_listFile.Size() ) {
						m_selectedLine = -1;
					} else {
						m_selectedLine = raw;
					}
					// need to regenerate the display of the list : 
					MarkToReedraw();
					if (m_selectedLine >=0 ) {
						// generate event extern : 
						GenerateEventId(ewolEventFileChooserSelectFile);
						return true;
					}
					return true;
				}
			}
			if (typeEvent == ewol::EVENT_INPUT_TYPE_DOUBLE) {
				EWOL_INFO("Event Double on List : IdInput=" << IdInput << " colomn=" << colomn << " raw=" << raw );
				if (1 == IdInput) {
					if (m_selectedLine >=0 ) {
						// generate event extern : 
						GenerateEventId(ewolEventFileChooserValidateFile);
						return true;
					}
				}
			}
			return false;
			return false;
		}
	
	
	public:
		/**
		 * @brief Check if the object has the specific type.
		 * @note In Embended platforme, it is many time no -rtti flag, then it is not possible to use dynamic cast ==> this will replace it
		 * @param[in] objectType type of the object we want to check
		 * @return true if the object is compatible, otherwise false
		 */
		bool CheckObjectType(const char * const objectType)
		{
			if (NULL == objectType) {
				EWOL_ERROR("check error : \"" << TYPE_EOBJECT_WIDGET_FILE_LIST << "\" != NULL(pointer) ");
				return false;
			}
			if (objectType == TYPE_EOBJECT_WIDGET_FILE_LIST) {
				return true;
			} else {
				if(true == ewol::List::CheckObjectType(objectType)) {
					return true;
				}
				EWOL_ERROR("check error : \"" << TYPE_EOBJECT_WIDGET_FILE_LIST << "\" != \"" << objectType << "\"");
				return false;
			}
		}
		
		/**
		 * @brief Get the current Object type of the EObject
		 * @note In Embended platforme, it is many time no -rtti flag, then it is not possible to use dynamic cast ==> this will replace it
		 * @param[in] objectType type description
		 * @return true if the object is compatible, otherwise false
		 */
		const char * const GetObjectType(void)
		{
			return TYPE_EOBJECT_WIDGET_FILE_LIST;
		}
};
#define EWOL_CAST_WIDGET_FILE_LIST(curentPointer) EWOL_CAST(TYPE_EOBJECT_WIDGET_FILE_LIST,FileChooserFileList,curentPointer)


#undef __class__
#define __class__	"FileChooser"


extern const char * const ewolEventFileChooserCancel          = "ewol-event-file-chooser-cancel";
extern const char * const ewolEventFileChooserValidate        = "ewol-event-file-chooser-validate";
extern const char * const ewolEventFileChooserHidenFileChange = "ewol-event-file-chooser-Show/Hide-hiden-Files";
extern const char * const ewolEventFileChooserEntryFolder     = "ewol-event-file-chooser-modify-entry-folder";
extern const char * const ewolEventFileChooserEntryFile       = "ewol-event-file-chooser-modify-entry-file";


ewol::FileChooser::FileChooser(void)
{
	AddEventId(ewolEventFileChooserCancel);
	AddEventId(ewolEventFileChooserValidate);
	
	m_hasSelectedFile = false;
	
	m_widgetTitle = NULL;
	m_widgetValidate = NULL;
	m_widgetCancel = NULL;
	m_widgetCurrentFolder = NULL;
	m_widgetCurrentFileName = NULL;
	m_widgetListFolder = NULL;
	m_widgetListFile = NULL;
	m_widgetCheckBox = NULL;
	
	ewol::SizerVert * mySizerVert = NULL;
	ewol::SizerHori * mySizerHori = NULL;
	ewol::Spacer * mySpacer = NULL;
	FileChooserFileList * myListFile = NULL;
	FileChooserFolderList * myListFolder = NULL;
	ewol::Label * myLabel = NULL;
	#ifdef __PLATFORM__Android
		m_folder = "/mnt/sdcard/";
		SetDisplayRatio(0.90);
	#else
		m_folder = "/home/";
		SetDisplayRatio(0.80);
	#endif
	m_file = "";
	
	mySizerVert = new ewol::SizerVert();
		mySizerVert->LockExpendContamination(true);
		// set it in the pop-up-system : 
		SubWidgetSet(mySizerVert);
		
		m_widgetTitle = new ewol::Label("File chooser ...");
			mySizerVert->SubWidgetAdd(m_widgetTitle);
		
		mySizerHori = new ewol::SizerHori();
			mySizerVert->SubWidgetAdd(mySizerHori);
			myLabel = new ewol::Label("Folder : ");
				myLabel->SetFillY(true);
				mySizerHori->SubWidgetAdd(myLabel);
			m_widgetCurrentFolder = new ewol::Entry(m_folder);
				m_widgetCurrentFolder->RegisterOnEvent(this, ewolEventEntryModify, ewolEventFileChooserEntryFolder);
				m_widgetCurrentFolder->SetExpendX(true);
				m_widgetCurrentFolder->SetFillX(true);
				m_widgetCurrentFolder->SetWidth(200);
				mySizerHori->SubWidgetAdd(m_widgetCurrentFolder);
		
		mySizerHori = new ewol::SizerHori();
			mySizerVert->SubWidgetAdd(mySizerHori);
			myLabel = new ewol::Label("File Name : ");
				myLabel->SetFillY(true);
				mySizerHori->SubWidgetAdd(myLabel);
			m_widgetCurrentFileName = new ewol::Entry(m_file);
				m_widgetCurrentFileName->RegisterOnEvent(this, ewolEventEntryModify, ewolEventFileChooserEntryFile);
				m_widgetCurrentFileName->SetExpendX(true);
				m_widgetCurrentFileName->SetFillX(true);
				m_widgetCurrentFileName->SetWidth(200);
				mySizerHori->SubWidgetAdd(m_widgetCurrentFileName);
		
		mySizerHori = new ewol::SizerHori();
			mySizerVert->SubWidgetAdd(mySizerHori);
			mySpacer = new ewol::Spacer();
				mySpacer->SetSize(2);
				mySizerHori->SubWidgetAdd(mySpacer);
			myListFolder = new FileChooserFolderList();
				m_widgetListFolder = myListFolder;
				myListFolder->RegisterOnEvent(this, ewolEventFileChooserSelectFolder, ewolEventFileChooserSelectFolder);
				myListFolder->SetExpendY(true);
				myListFolder->SetFillY(true);
				mySizerHori->SubWidgetAdd(myListFolder);
			mySpacer = new ewol::Spacer();
				mySpacer->SetSize(2);
				mySizerHori->SubWidgetAdd(mySpacer);
			myListFile = new FileChooserFileList();
				m_widgetListFile = myListFile;
				myListFile->RegisterOnEvent(this, ewolEventFileChooserSelectFile, ewolEventFileChooserSelectFile);
				myListFile->RegisterOnEvent(this, ewolEventFileChooserValidateFile, ewolEventFileChooserValidateFile);
				myListFile->SetExpendX(true);
				myListFile->SetFillX(true);
				myListFile->SetExpendY(true);
				myListFile->SetFillY(true);
				mySizerHori->SubWidgetAdd(myListFile);
			mySpacer = new ewol::Spacer();
				mySpacer->SetSize(2);
				mySizerHori->SubWidgetAdd(mySpacer);
			
		mySizerHori = new ewol::SizerHori();
			mySizerVert->SubWidgetAdd(mySizerHori);
			m_widgetCheckBox = new ewol::CheckBox("Show hiden files");
				m_widgetCheckBox->RegisterOnEvent(this, ewolEventCheckBoxClicked, ewolEventFileChooserHidenFileChange);
				m_widgetCheckBox->SetValue(false);
				mySizerHori->SubWidgetAdd(m_widgetCheckBox);
			mySpacer = new ewol::Spacer();
				mySpacer->SetExpendX(true);
				mySizerHori->SubWidgetAdd(mySpacer);
			m_widgetValidate = new ewol::Button("Validate");
				m_widgetValidate->RegisterOnEvent(this, ewolEventButtonPressed, ewolEventFileChooserValidate);
				mySizerHori->SubWidgetAdd(m_widgetValidate);
			m_widgetCancel = new ewol::Button("Cancel");
				m_widgetCancel->RegisterOnEvent(this, ewolEventButtonPressed, ewolEventFileChooserCancel);
				mySizerHori->SubWidgetAdd(m_widgetCancel);
	
	// set the default Folder properties:
	UpdateCurrentFolder();
}


ewol::FileChooser::~FileChooser(void)
{
	
}


//!< EObject name :
extern const char * const ewol::TYPE_EOBJECT_WIDGET_FILE_CHOOSER = "FileChooser";

/**
 * @brief Check if the object has the specific type.
 * @note In Embended platforme, it is many time no -rtti flag, then it is not possible to use dynamic cast ==> this will replace it
 * @param[in] objectType type of the object we want to check
 * @return true if the object is compatible, otherwise false
 */
bool ewol::FileChooser::CheckObjectType(const char * const objectType)
{
	if (NULL == objectType) {
		EWOL_ERROR("check error : \"" << ewol::TYPE_EOBJECT_WIDGET_FILE_CHOOSER << "\" != NULL(pointer) ");
		return false;
	}
	if (objectType == ewol::TYPE_EOBJECT_WIDGET_FILE_CHOOSER) {
		return true;
	} else {
		if(true == ewol::PopUp::CheckObjectType(objectType)) {
			return true;
		}
		EWOL_ERROR("check error : \"" << ewol::TYPE_EOBJECT_WIDGET_FILE_CHOOSER << "\" != \"" << objectType << "\"");
		return false;
	}
}

/**
 * @brief Get the current Object type of the EObject
 * @note In Embended platforme, it is many time no -rtti flag, then it is not possible to use dynamic cast ==> this will replace it
 * @param[in] objectType type description
 * @return true if the object is compatible, otherwise false
 */
const char * const ewol::FileChooser::GetObjectType(void)
{
	return ewol::TYPE_EOBJECT_WIDGET_FILE_CHOOSER;
}



void ewol::FileChooser::SetTitle(etk::UString label)
{
	if (NULL == m_widgetTitle) {
		return;
	}
	m_widgetTitle->SetLabel(label);
}

void ewol::FileChooser::SetValidateLabel(etk::UString label)
{
	if (NULL == m_widgetValidate) {
		return;
	}
	m_widgetValidate->SetLabel(label);
}

void ewol::FileChooser::SetCancelLabel(etk::UString label)
{
	if (NULL == m_widgetCancel) {
		return;
	}
	m_widgetCancel->SetLabel(label);
}

void ewol::FileChooser::SetFolder(etk::UString folder)
{
	m_folder = folder + "/";
	UpdateCurrentFolder();
}

void ewol::FileChooser::SetFileName(etk::UString filename)
{
	m_file = filename;
	if (NULL == m_widgetCurrentFileName) {
		return;
	}
	m_widgetCurrentFileName->SetValue(filename);
}


/**
 * @brief Receive a message from an other EObject with a specific eventId and data
 * @param[in] CallerObject Pointer on the EObject that information came from
 * @param[in] eventId Message registered by this class
 * @param[in] data Data registered by this class
 * @return ---
 */
void ewol::FileChooser::OnReceiveMessage(ewol::EObject * CallerObject, const char * eventId, etk::UString data)
{
	EWOL_INFO("Receive Event from the LIST ... : widgetPointer=" << CallerObject << "\"" << eventId << "\" ==> data=\"" << data << "\"" );
	if (ewolEventFileChooserEntryFolder == eventId) {
		//==> change the folder name
		// TODO : Change the folder, if it exit ...
		return;
	} else if (ewolEventFileChooserEntryFile == eventId) {
		//==> change the file name
		if (NULL != m_widgetCurrentFileName) {
			m_file = m_widgetCurrentFileName->GetValue();
		}
		// TODO : Remove file selection
		return;
	} else if (ewolEventFileChooserCancel == eventId) {
		//==> Auto remove ...
		GenerateEventId(eventId);
		MarkToRemove();
		return;
	} else if (ewolEventFileChooserHidenFileChange == eventId) {
		// regenerate the display ...
		UpdateCurrentFolder();
		return;
	} else if (ewolEventFileChooserSelectFolder == eventId) {
		//==> this is an internal event ...
		FileChooserFolderList * myListFolder = EWOL_CAST_WIDGET_FOLDER_LIST(m_widgetListFolder);
		etk::UString tmpString = myListFolder->GetSelectedLine();
		EWOL_VERBOSE(" old PATH : \"" << m_folder << "\" + \"" << tmpString << "\"");
		m_folder = m_folder + tmpString;
		char buf[MAX_FILE_NAME];
		memset(buf, 0, MAX_FILE_NAME);
		char * ok;
		EWOL_DEBUG("new PATH : \"" << m_folder << "\"");
		
		ok = realpath(m_folder.Utf8Data(), buf);
		if (!ok) {
			EWOL_ERROR("Error to get the real path");
			m_folder = "/";
		} else {
			m_folder = buf;
		}
		if (m_folder != "/" ) {
			m_folder +=  "/";
		}
		SetFileName("");
		UpdateCurrentFolder();
		m_hasSelectedFile = false;
		return;
	} else if (ewolEventFileChooserSelectFile == eventId) {
		m_hasSelectedFile = true;
		FileChooserFileList * myListFile = EWOL_CAST_WIDGET_FILE_LIST(m_widgetListFile);
		etk::UString file = myListFile->GetSelectedLine();
		SetFileName(file);
		GenerateEventId(eventId);
	} else if(    ewolEventFileChooserValidateFile == eventId
	           || (ewolEventFileChooserValidate == eventId && true == m_hasSelectedFile) ) {
		// select the File ==> generate a validate
		GenerateEventId(ewolEventFileChooserValidate);
		MarkToRemove();
		return;
	}
	return;
};



void ewol::FileChooser::UpdateCurrentFolder(void)
{
	if (NULL == m_widgetListFile) {
		return;
	}
	if (NULL == m_widgetListFolder) {
		return;
	}
	FileChooserFileList * myListFile     = EWOL_CAST_WIDGET_FILE_LIST(m_widgetListFile);
	FileChooserFolderList * myListFolder = EWOL_CAST_WIDGET_FOLDER_LIST(m_widgetListFolder);
	
	myListFile->ClearElements();
	myListFolder->ClearElements();
	bool ShowHidenFile = true;
	if (NULL != m_widgetCheckBox) {
		ShowHidenFile = m_widgetCheckBox->GetValue();
	} else {
		EWOL_ERROR("Can not get the hiden property of the file choozer...");
	}
	
	if (NULL != m_widgetCurrentFolder) {
		m_widgetCurrentFolder->SetValue(m_folder);
	}
	myListFolder->AddElement(etk::UString("."));
	if (m_folder != "/" ) {
		myListFolder->AddElement(etk::UString(".."));
	}
	DIR *dir;
	struct dirent *ent;
	dir = opendir(m_folder.Utf8Data());
	if (dir != NULL) {
		// for each element in the drectory...
		while ((ent = readdir(dir)) != NULL) {
			etk::UString tmpString(ent->d_name);
			if (DT_REG == ent->d_type) {
				if (false == tmpString.StartWith(".") || true==ShowHidenFile) {
					myListFile->AddElement(tmpString);
				}
			} else if (DT_DIR == ent->d_type) {
				//EWOL_DEBUG("    find Folder : \"" << tmpString << "\"(" << tmpString.Size() << ") ?= \"" << ent->d_name << "\"(" << strlen(ent->d_name) );
				if (tmpString != "." && tmpString != "..") {
					if (false == tmpString.StartWith(".") || true==ShowHidenFile) {
						myListFolder->AddElement(tmpString);
					}
				}
			}
		}
		closedir(dir);
	} else {
		EWOL_ERROR("could not open directory : \"" << m_folder << "\"");
	}
	myListFile->EndGenerating();
	myListFolder->EndGenerating();
	MarkToReedraw();
}


etk::UString ewol::FileChooser::GetCompleateFileName(void)
{
	etk::UString tmpString = m_folder;
	tmpString += "/";
	tmpString += m_file;
	return tmpString;
}


/**
 * @brief Inform object that an other object is removed ...
 * @param[in] removeObject Pointer on the EObject remeved ==> the user must remove all reference on this EObject
 * @note : Sub classes must call this class
 * @return ---
 */
void ewol::FileChooser::OnObjectRemove(ewol::EObject * removeObject)
{
	// First step call parrent : 
	ewol::PopUp::OnObjectRemove(removeObject);
	// second step find if in all the elements ...
	if(removeObject == m_widgetTitle) {
		m_widgetTitle = NULL;
		m_needFlipFlop = true;
	}
	if(removeObject == m_widgetValidate) {
		m_widgetValidate = NULL;
		m_needFlipFlop = true;
	}
	if(removeObject == m_widgetCancel) {
		m_widgetCancel = NULL;
		m_needFlipFlop = true;
	}
	if(removeObject == m_widgetCurrentFolder) {
		m_widgetCurrentFolder = NULL;
		m_needFlipFlop = true;
	}
	if(removeObject == m_widgetCurrentFileName) {
		m_widgetCurrentFileName = NULL;
		m_needFlipFlop = true;
	}
	if(removeObject == m_widgetListFolder) {
		m_widgetListFolder = NULL;
		m_needFlipFlop = true;
	}
	if(removeObject == m_widgetListFile) {
		m_widgetListFile = NULL;
		m_needFlipFlop = true;
	}
	if(removeObject == m_widgetCheckBox) {
		m_widgetCheckBox = NULL;
		m_needFlipFlop = true;
	}
}

