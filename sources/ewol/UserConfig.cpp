/**
 * @author Edouard DUPIN
 * 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * 
 * @license BSD v3 (see license file)
 */


#include <ewol/UserConfig.h>
#include <ewol/eObject/EObjectManager.h>
#include <etk/os/FSNode.h>
#include <ewol/debug.h>
#include <exml/Declaration.h>

class UserConfig : public ewol::EObject
{
	private:
		etk::Vector<ewol::EObject*> m_list;
		etk::UString m_fileName;
	public:
		UserConfig(void) : m_fileName("USERDATA:generalConfig.xml")
		{
			m_static = true;  // Note : Set the object static notification( Must be set or assert at the end of process)
		};
		~UserConfig(void) { m_list.Clear(); };
		etk::Vector<ewol::EObject*>& List(void) { return m_list; };
		etk::UString& FileName(void) { return m_fileName; };
		
		void OnObjectRemove(ewol::EObject * _removeObject)
		{
			for( int32_t iii=m_list.Size()-1 ; iii>=0 ; iii--) {
				if (m_list[iii] == _removeObject) {
					m_list.Erase(iii);
				}
			}
		};
};


static UserConfig& l_obj(void)
{
	static UserConfig s_obj;
	return s_obj;
}


void ewol::userConfig::Init(void)
{
	
}

void ewol::userConfig::UnInit(void)
{
	
}


const ewol::EObject* ewol::userConfig::GetUserConfig(const etk::UString& _name)
{
	if (_name == "") {
		return NULL;
	}
	for (int32_t iii=0; iii<l_obj().List().Size() ; iii++) {
		if (l_obj().List()[iii] != NULL) {
			if (l_obj().List()[iii]->GetName() == _name) {
				return l_obj().List()[iii];
			}
		}
	}
	return NULL;
}

void ewol::userConfig::AddUserConfig(ewol::EObject* _newConfig)
{
	if (_newConfig == NULL) {
		return;
	}
	for (int32_t iii=0; iii<l_obj().List().Size() ; iii++) {
		if (l_obj().List()[iii] != NULL) {
			if (l_obj().List()[iii] == _newConfig) {
				return ;
			}
		}
	}
	l_obj().List().PushBack(_newConfig);
}



void ewol::userConfig::SetConfigName(const etk::UString& _fileName)
{
	l_obj().FileName() = _fileName;
}


bool ewol::userConfig::Load(void)
{
	// allocate the document in the stack
	exml::Document doc;
	if (false == doc.Load(l_obj().FileName())) {
		EWOL_ERROR("Error occured when loading XML : " << l_obj().FileName());
		return false;
	}
	if (0 == doc.Size() ) {
		EWOL_ERROR("(l ?) No nodes in the xml file ... \"" << l_obj().FileName() << "\"");
		return false;
	}
	exml::Element* root = (exml::Element*)doc.GetNamed("config");
	if (NULL == root ) {
		EWOL_ERROR("(l ?) main node not find: \"config\" in \"" << l_obj().FileName() << "\"");
		return false;
	}
	for(int32_t iii=0; iii< root->Size(); iii++) {
		exml::Element* child = root->GetElement(iii);
		if (child==NULL) {
			// other than element is trash here
			continue;
		}
		bool elementFound = false;
		for (int32_t iii=0; iii<l_obj().List().Size() ; iii++) {
			if (l_obj().List()[iii] != NULL) {
				if (l_obj().List()[iii]->GetName() == child->GetValue()) {
					l_obj().List()[iii]->LoadXML(child);
					elementFound = true;
					break;
				}
			}
		}
		if (elementFound==false) {
			EWOL_ERROR("(l "<<child->Pos()<<") node not suported : \""<<child->GetValue());
		}
	}
	return true;
}

#define MAX_SAVING_FILE_HISTORY (9)

bool ewol::userConfig::Save(void)
{
	// step 1 : Move the file to prevent writing error
	//Get the first oldest save :
	for (int32_t iii=MAX_SAVING_FILE_HISTORY-1; iii>0 ; iii--) {
		if (true==etk::FSNodeExist(l_obj().FileName()+"-"+iii) ) {
			etk::FSNodeMove(l_obj().FileName()+"-"+iii,l_obj().FileName()+"-"+(iii+1));
		}
	}
	if (true==etk::FSNodeExist(l_obj().FileName()) ) {
		etk::FSNodeMove(l_obj().FileName(),l_obj().FileName()+"-1");
	}
	exml::Document doc;
	doc.Append(new exml::Declaration("1.0", "UTF-8", ""));
	exml::Element * ElementBase = new exml::Element("config");
	doc.Append(ElementBase);
	for (int32_t iii=0; iii<l_obj().List().Size() ; iii++) {
		if (l_obj().List()[iii] == NULL) {
			continue;
		}
		if (l_obj().List()[iii]->GetName().Size() == 0) {
			continue;
		}
		exml::Element* element = new exml::Element(l_obj().List()[iii]->GetName());
		if (NULL != element) {
			l_obj().List()[iii]->StoreXML(element);
			ElementBase->Append(element);
		}
	}
	//Save Document
	doc.Store(l_obj().FileName());
	EWOL_DEBUG("Save in file : " << l_obj().FileName());
	// step 3 : Remove oldest save
	return true;
}


