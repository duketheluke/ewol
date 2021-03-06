/**
 * @author Edouard DUPIN, Kevin BILLONNEAU
 * 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * 
 * @license APACHE v2.0 (see license file)
 */

#include <jni.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <pthread.h>
#include <mutex>
#include <ewol/debug.h>
#include <ewol/context/Context.h>
//#include <ewol/renderer/audio/audio.h>
#include <ewol/Dimension.h>
/* include auto generated file */
#include <ewol/renderer/Android/org_ewol_EwolConstants.h>



int64_t ewol::getTime() {
	struct timeval	now;
	gettimeofday(&now, nullptr);
	//EWOL_VERBOSE("current time : " << now.tv_sec << "s " << now.tv_usec << "us");
	return (int64_t)((int64_t)now.tv_sec*(int64_t)1000000 + (int64_t)now.tv_usec);
}


// jni doc : /usr/lib/jvm/java-1.6.0-openjdk/include

static JavaVM* g_JavaVM=nullptr; // global acces on the unique JVM !!!
std::mutex g_interfaceMutex;
std::mutex g_interfaceAudioMutex;


void java_check_exception(JNIEnv* _env) {
	if (_env->ExceptionOccurred()) {
		EWOL_ERROR("C->java : EXEPTION ...");
		_env->ExceptionDescribe();
		_env->ExceptionClear();
	}
}

class AndroidContext : public ewol::Context {
	public:
		enum application {
			appl_unknow,
			appl_application,
			appl_wallpaper
		};
	private:
		enum application m_javaApplicationType;
		// get a resources from the java environement : 
		JNIEnv* m_JavaVirtualMachinePointer; //!< the JVM
		jclass m_javaClassEwol; //!< main activity class (android ...)
		jclass m_javaClassEwolCallback;
		jobject m_javaObjectEwolCallback;
		jmethodID m_javaMethodEwolCallbackStop; //!< Stop function identifier
		jmethodID m_javaMethodEwolCallbackEventNotifier; //!< basic methode to call ...
		jmethodID m_javaMethodEwolCallbackKeyboardUpdate; //!< basic methode to call ...
		jmethodID m_javaMethodEwolCallbackOrientationUpdate;
		jmethodID m_javaMethodEwolActivitySetTitle;
		jmethodID m_javaMethodEwolActivityOpenURI;
		jmethodID m_javaMethodEwolActivitySetClipBoardString;
		jmethodID m_javaMethodEwolActivityGetClipBoardString;
		// List of all Audio interface :
		jmethodID m_javaMethodEwolActivityAudioGetDeviceCount;
		jmethodID m_javaMethodEwolActivityAudioGetDeviceProperty;
		jmethodID m_javaMethodEwolActivityAudioOpenDevice;
		jmethodID m_javaMethodEwolActivityAudioCloseDevice;
		jclass m_javaDefaultClassString; //!< default string class
		int32_t m_currentHeight;
		ewol::key::Special m_guiKeyBoardSpecialKeyMode;//!< special key of the android system :
		bool m_clipBoardOwnerStd;
	private:
		bool safeInitMethodID(jmethodID& _mid, jclass& _cls, const char* _name, const char* _sign) {
			_mid = m_JavaVirtualMachinePointer->GetMethodID(_cls, _name, _sign);
			if(_mid == nullptr) {
				EWOL_ERROR("C->java : Can't find the method " << _name);
				/* remove access on the virtual machine : */
				m_JavaVirtualMachinePointer = nullptr;
				return false;
			}
			return true;
		}
	public:
		AndroidContext(ewol::context::Application* _application, JNIEnv* _env, jclass _classBase, jobject _objCallback, enum application _typeAPPL) :
		  ewol::Context(_application),
		  m_javaApplicationType(_typeAPPL),
		  m_JavaVirtualMachinePointer(nullptr),
		  m_javaClassEwol(0),
		  m_javaClassEwolCallback(0),
		  m_javaObjectEwolCallback(0),
		  m_javaMethodEwolCallbackStop(0),
		  m_javaMethodEwolCallbackEventNotifier(0),
		  m_javaMethodEwolCallbackKeyboardUpdate(0),
		  m_javaMethodEwolCallbackOrientationUpdate(0),
		  m_javaMethodEwolActivitySetTitle(0),
		  m_javaMethodEwolActivityOpenURI(0),
		  m_javaMethodEwolActivitySetClipBoardString(0),
		  m_javaMethodEwolActivityGetClipBoardString(0),
		  m_javaMethodEwolActivityAudioGetDeviceCount(0),
		  m_javaMethodEwolActivityAudioGetDeviceProperty(0),
		  m_javaMethodEwolActivityAudioOpenDevice(0),
		  m_javaMethodEwolActivityAudioCloseDevice(0),
		  m_javaDefaultClassString(0),
		  m_currentHeight(0),
		  m_clipBoardOwnerStd(false),
		  m_audioCallBack(nullptr),
		  m_audioCallBackUserData(nullptr) {
			EWOL_DEBUG("*******************************************");
			if (m_javaApplicationType == appl_application) {
				EWOL_DEBUG("** set JVM Pointer (application)         **");
			} else {
				EWOL_DEBUG("** set JVM Pointer (LiveWallpaper)       **");
			}
			EWOL_DEBUG("*******************************************");
			m_JavaVirtualMachinePointer = _env;
			// get default needed all time elements : 
			if (nullptr != m_JavaVirtualMachinePointer) {
				EWOL_DEBUG("C->java : try load org/ewol/Ewol class");
				m_javaClassEwol = m_JavaVirtualMachinePointer->FindClass("org/ewol/Ewol" );
				if (m_javaClassEwol == 0) {
					EWOL_ERROR("C->java : Can't find org/ewol/Ewol class");
					// remove access on the virtual machine : 
					m_JavaVirtualMachinePointer = nullptr;
					return;
				}
				/* The object field extends Activity and implement EwolCallback */
				m_javaClassEwolCallback = m_JavaVirtualMachinePointer->GetObjectClass(_objCallback);
				if(m_javaClassEwolCallback == nullptr) {
					EWOL_ERROR("C->java : Can't find org/ewol/EwolCallback class");
					// remove access on the virtual machine : 
					m_JavaVirtualMachinePointer = nullptr;
					return;
				}
				bool functionCallbackIsMissing = false;
				bool ret= false;
				ret = safeInitMethodID(m_javaMethodEwolActivitySetTitle,
				                       m_javaClassEwolCallback,
				                       "titleSet",
				                       "(Ljava/lang/String;)V");
				if (ret == false) {
					java_check_exception(_env);
					EWOL_ERROR("system can not start without function : titleSet");
					functionCallbackIsMissing = true;
				}
				ret = safeInitMethodID(m_javaMethodEwolActivityOpenURI,
				                       m_javaClassEwolCallback,
				                       "openURI",
				                       "(Ljava/lang/String;)V");
				if (ret == false) {
					java_check_exception(_env);
					EWOL_ERROR("system can not start without function : openURI");
					functionCallbackIsMissing = true;
				}
				
				ret = safeInitMethodID(m_javaMethodEwolCallbackStop,
				                       m_javaClassEwolCallback,
				                       "stop",
				                       "()V");
				if (ret == false) {
					java_check_exception(_env);
					EWOL_ERROR("system can not start without function : stop");
					functionCallbackIsMissing = true;
				}
				
				ret = safeInitMethodID(m_javaMethodEwolCallbackEventNotifier,
				                       m_javaClassEwolCallback,
				                       "eventNotifier",
				                       "([Ljava/lang/String;)V");
				if (ret == false) {
					java_check_exception(_env);
					EWOL_ERROR("system can not start without function : eventNotifier");
					functionCallbackIsMissing = true;
				}
				
				ret = safeInitMethodID(m_javaMethodEwolCallbackKeyboardUpdate,
				                       m_javaClassEwolCallback,
				                       "keyboardUpdate",
				                       "(Z)V");
				if (ret == false) {
					java_check_exception(_env);
					EWOL_ERROR("system can not start without function : keyboardUpdate");
					functionCallbackIsMissing = true;
				}
				
				ret = safeInitMethodID(m_javaMethodEwolCallbackOrientationUpdate,
				                       m_javaClassEwolCallback,
				                       "orientationUpdate",
				                       "(I)V");
				if (ret == false) {
					java_check_exception(_env);
					EWOL_ERROR("system can not start without function : orientationUpdate");
					functionCallbackIsMissing = true;
				}
				
				ret = safeInitMethodID(m_javaMethodEwolActivitySetClipBoardString,
				                       m_javaClassEwolCallback,
				                       "setClipBoardString",
				                       "(Ljava/lang/String;)V");
				if (ret == false) {
					java_check_exception(_env);
					EWOL_ERROR("system can not start without function : setClipBoardString");
					functionCallbackIsMissing = true;
				}
				
				ret = safeInitMethodID(m_javaMethodEwolActivityGetClipBoardString,
				                       m_javaClassEwolCallback,
				                       "getClipBoardString",
				                       "()Ljava/lang/String;");
				if (ret == false) {
					java_check_exception(_env);
					EWOL_ERROR("system can not start without function : getClipBoardString");
					functionCallbackIsMissing = true;
				}
				
				ret = safeInitMethodID(m_javaMethodEwolActivityAudioGetDeviceCount,
				                       m_javaClassEwolCallback,
				                       "audioGetDeviceCount",
				                       "()I");
				if (ret == false) {
					java_check_exception(_env);
					EWOL_ERROR("system can not start without function : audioGetDeviceCount");
					functionCallbackIsMissing = true;
				}
				
				ret = safeInitMethodID(m_javaMethodEwolActivityAudioGetDeviceProperty,
				                       m_javaClassEwolCallback,
				                       "audioGetDeviceProperty",
				                       "(I)Ljava/lang/String;");
				if (ret == false) {
					java_check_exception(_env);
					EWOL_ERROR("system can not start without function : audioGetDeviceProperty");
					functionCallbackIsMissing = true;
				}
				
				ret = safeInitMethodID(m_javaMethodEwolActivityAudioOpenDevice,
				                       m_javaClassEwolCallback,
				                       "audioOpenDevice",
				                       "(IIII)Z");
				if (ret == false) {
					java_check_exception(_env);
					EWOL_ERROR("system can not start without function : audioOpenDevice");
					functionCallbackIsMissing = true;
				}
				ret = safeInitMethodID(m_javaMethodEwolActivityAudioCloseDevice,
				                       m_javaClassEwolCallback,
				                       "audioCloseDevice",
				                       "(I)Z");
				if (ret == false) {
					java_check_exception(_env);
					EWOL_ERROR("system can not start without function : audioCloseDevice");
					functionCallbackIsMissing = true;
				}
				
				m_javaObjectEwolCallback = _env->NewGlobalRef(_objCallback);
				//javaObjectEwolCallbackAndActivity = objCallback;
				if (m_javaObjectEwolCallback == nullptr) {
					functionCallbackIsMissing = true;
				}
				
				m_javaDefaultClassString = m_JavaVirtualMachinePointer->FindClass("java/lang/String" );
				if (m_javaDefaultClassString == 0) {
					EWOL_ERROR("C->java : Can't find java/lang/String" );
					// remove access on the virtual machine : 
					m_JavaVirtualMachinePointer = nullptr;
					functionCallbackIsMissing = true;
				}
				if (functionCallbackIsMissing == true) {
					EWOL_CRITICAL(" mission one function ==> system can not work withut it...");
				}
			}
		}
		
		~AndroidContext() {
			// TODO ...
		}
		
		void unInit(JNIEnv* _env) {
			_env->DeleteGlobalRef(m_javaObjectEwolCallback);
			m_javaObjectEwolCallback = nullptr;
		}
		
		int32_t run() {
			// might never be called !!!
			return -1;
		}
		
		void stop() {
			EWOL_DEBUG("C->java : send message to the java : STOP REQUESTED");
			int status;
			if(!java_attach_current_thread(&status)) {
				return;
			}
			//Call java ...
			m_JavaVirtualMachinePointer->CallVoidMethod(m_javaObjectEwolCallback, m_javaMethodEwolCallbackStop);
			// manage execption : 
			java_check_exception(m_JavaVirtualMachinePointer);
			java_detach_current_thread(status);
		}
		
		void clipBoardGet(enum ewol::context::clipBoard::clipboardListe _clipboardID) {
			// this is to force the local system to think we have the buffer
			// TODO : remove this 2 line when code will be writen
			m_clipBoardOwnerStd = true;
			switch (_clipboardID) {
				case ewol::context::clipBoard::clipboardSelection:
					// NOTE : Windows does not support the middle button the we do it internaly
					// just transmit an event , we have the data in the system
					OS_ClipBoardArrive(_clipboardID);
					break;
				case ewol::context::clipBoard::clipboardStd:
					if (false == m_clipBoardOwnerStd) {
						// generate a request TO the OS
						// TODO : Send the message to the OS "We disire to receive the copy buffer ...
					} else {
						// just transmit an event , we have the data in the system
						OS_ClipBoardArrive(_clipboardID);
					}
					break;
				default:
					EWOL_ERROR("Request an unknow ClipBoard ...");
					break;
			}
		}
		
		void clipBoardSet(enum ewol::context::clipBoard::clipboardListe _clipboardID) {
			switch (_clipboardID) {
				case ewol::context::clipBoard::clipboardSelection:
					// NOTE : nothing to do : Windows deas ot supported Middle button
					break;
				case ewol::context::clipBoard::clipboardStd:
					
					// Request the clipBoard :
					EWOL_DEBUG("C->java : set clipboard");
					if (m_javaApplicationType == appl_application) {
						int status;
						if(!java_attach_current_thread(&status)) {
							return;
						}
						//Call java ...
						jstring data = m_JavaVirtualMachinePointer->NewStringUTF(ewol::context::clipBoard::get(_clipboardID).c_str());
						m_JavaVirtualMachinePointer->CallVoidMethod(m_javaObjectEwolCallback, m_javaMethodEwolActivityGetClipBoardString, data);
						m_JavaVirtualMachinePointer->DeleteLocalRef(data);
						// manage execption : 
						java_check_exception(m_JavaVirtualMachinePointer);
						java_detach_current_thread(status);
					} else {
						EWOL_ERROR("C->java : can not set clipboard");
					}
					break;
				default:
					EWOL_ERROR("Request an unknow ClipBoard ...");
					break;
			}
		}
		int32_t audioGetDeviceCount() {
			// Request the clipBoard :
			EWOL_DEBUG("C->java : audio get device count");
			if (m_javaApplicationType == appl_application) {
				int status;
				if(!java_attach_current_thread(&status)) {
					return 0;
				}
				EWOL_DEBUG("Call CallIntMethod ...");
				//Call java ...
				jint ret = m_JavaVirtualMachinePointer->CallIntMethod(m_javaObjectEwolCallback, m_javaMethodEwolActivityAudioGetDeviceCount);
				// manage execption : 
				java_check_exception(m_JavaVirtualMachinePointer);
				java_detach_current_thread(status);
				return (int32_t)ret;
			} else {
				EWOL_ERROR("C->java : can not get audio device count");
			}
			return 0;
		}
		std::string audioGetDeviceProperty(int32_t _idDevice) {
			// Request the clipBoard :
			EWOL_DEBUG("C->java : audio get device count");
			if (m_javaApplicationType == appl_application) {
				int status;
				if(!java_attach_current_thread(&status)) {
					return "";
				}
				//Call java ...
				jstring returnString = (jstring) m_JavaVirtualMachinePointer->CallObjectMethod(m_javaObjectEwolCallback, m_javaMethodEwolActivityAudioGetDeviceProperty, _idDevice);
				const char *js = m_JavaVirtualMachinePointer->GetStringUTFChars(returnString, nullptr);
				std::string retString(js);
				m_JavaVirtualMachinePointer->ReleaseStringUTFChars(returnString, js);
				//m_JavaVirtualMachinePointer->DeleteLocalRef(returnString);
				// manage execption : 
				java_check_exception(m_JavaVirtualMachinePointer);
				java_detach_current_thread(status);
				return retString;
			} else {
				EWOL_ERROR("C->java : can not get audio device count");
			}
			return "";
		}
	private:
		AndroidAudioCallback m_audioCallBack;
		void* m_audioCallBackUserData;
	public:
		bool audioOpenDevice(int32_t _idDevice,
		                     int32_t _freq,
		                     int32_t _nbChannel,
		                     int32_t _format,
		                     AndroidAudioCallback _callback,
		                     void* _userData) {
			if (m_audioCallBack != nullptr) {
				EWOL_ERROR("AudioCallback already started ...");
				return false;
			}
			// Request the clipBoard :
			EWOL_DEBUG("C->java : audio get device count");
			if (m_javaApplicationType == appl_application) {
				int status;
				if(!java_attach_current_thread(&status)) {
					return false;
				}
				//Call java ...
				jboolean ret = m_JavaVirtualMachinePointer->CallBooleanMethod(m_javaObjectEwolCallback, m_javaMethodEwolActivityAudioOpenDevice, _idDevice, _freq, _nbChannel, _format);
				// manage execption : 
				java_check_exception(m_JavaVirtualMachinePointer);
				java_detach_current_thread(status);
				if (ret == true) {
					m_audioCallBack = _callback;
					m_audioCallBackUserData = _userData;
				}
				return (bool)ret;
			} else {
				EWOL_ERROR("C->java : can not get audio device count");
			}
			return false;
		}
		bool audioCloseDevice(int32_t _idDevice) {
			if (m_audioCallBack == nullptr) {
				EWOL_ERROR("AudioCallback Not started ...");
				return false;
			}
			// Request the clipBoard :
			EWOL_DEBUG("C->java : audio get device count");
			if (m_javaApplicationType == appl_application) {
				int status;
				if(!java_attach_current_thread(&status)) {
					return false;
				}
				//Call java ...
				jboolean ret = m_JavaVirtualMachinePointer->CallBooleanMethod(m_javaObjectEwolCallback, m_javaMethodEwolActivityAudioCloseDevice, _idDevice);
				// manage execption : 
				java_check_exception(m_JavaVirtualMachinePointer);
				java_detach_current_thread(status);
				m_audioCallBack = nullptr;
				m_audioCallBackUserData = nullptr;
				return (bool)ret;
			} else {
				EWOL_ERROR("C->java : can not get audio device count");
			}
			return false;
		}
	private:
		bool java_attach_current_thread(int *_rstatus) {
			EWOL_DEBUG("C->java : call java");
			if (g_JavaVM == nullptr) {
				EWOL_ERROR("C->java : JVM not initialised");
				m_JavaVirtualMachinePointer = nullptr;
				return false;
			}
			*_rstatus = g_JavaVM->GetEnv((void **) &m_JavaVirtualMachinePointer, JNI_VERSION_1_6);
			if (*_rstatus == JNI_EDETACHED) {
				JavaVMAttachArgs lJavaVMAttachArgs;
				lJavaVMAttachArgs.version = JNI_VERSION_1_6;
				lJavaVMAttachArgs.name = "EwolNativeThread";
				lJavaVMAttachArgs.group = nullptr; 
				int status = g_JavaVM->AttachCurrentThread(&m_JavaVirtualMachinePointer, &lJavaVMAttachArgs);
				java_check_exception(m_JavaVirtualMachinePointer);
				if (status != JNI_OK) {
					EWOL_ERROR("C->java : AttachCurrentThread failed : " << status);
					m_JavaVirtualMachinePointer = nullptr;
					return false;
				}
			}
			return true;
		}
		
		void java_detach_current_thread(int _status) {
			if(_status == JNI_EDETACHED) {
				g_JavaVM->DetachCurrentThread();
				m_JavaVirtualMachinePointer = nullptr;
			}
		}
		
		void sendJavaKeyboardUpdate(jboolean _showIt) {
			int status;
			if(!java_attach_current_thread(&status)) {
				return;
			}
			//Call java ...
			m_JavaVirtualMachinePointer->CallVoidMethod(m_javaObjectEwolCallback, m_javaMethodEwolCallbackKeyboardUpdate, _showIt);
			// manage execption : 
			java_check_exception(m_JavaVirtualMachinePointer);
			java_detach_current_thread(status);
		}
		void keyboardShow() {
			sendJavaKeyboardUpdate(JNI_TRUE);
		};
		void keyboardHide() {
			sendJavaKeyboardUpdate(JNI_FALSE);
		};
		
		// mode 0 : auto; 1 landscape, 2 portrait
		void forceOrientation(enum ewol::orientation _orientation) {
			int status;
			if(!java_attach_current_thread(&status)) {
				return;
			}
			jint param = (jint)_orientation;
			
			//Call java ...
			m_JavaVirtualMachinePointer->CallVoidMethod(m_javaObjectEwolCallback, m_javaMethodEwolCallbackOrientationUpdate, param);
			
			// manage execption : 
			java_check_exception(m_JavaVirtualMachinePointer);
			java_detach_current_thread(status);
		}
		
		void setTitle(const std::string& _title) {
			EWOL_DEBUG("C->java : send message to the java : \"" << _title << "\"");
			if (m_javaApplicationType == appl_application) {
				int status;
				if(!java_attach_current_thread(&status)) {
					return;
				}
				//Call java ...
				jstring title = m_JavaVirtualMachinePointer->NewStringUTF(_title.c_str());
				m_JavaVirtualMachinePointer->CallVoidMethod(m_javaObjectEwolCallback, m_javaMethodEwolActivitySetTitle, title);
				m_JavaVirtualMachinePointer->DeleteLocalRef(title);
				// manage execption : 
				java_check_exception(m_JavaVirtualMachinePointer);
				java_detach_current_thread(status);
			} else {
				EWOL_ERROR("C->java : can not set title on appliation that is not real application");
			}
		}
		
		void openURL(const std::string& _url) {
			EWOL_DEBUG("C->java : send message to the java : open URL'" << _url << "'");
			int status;
			if(!java_attach_current_thread(&status)) {
				return;
			}
			//Call java ...
			jstring url = m_JavaVirtualMachinePointer->NewStringUTF(_url.c_str());
			m_JavaVirtualMachinePointer->CallVoidMethod(m_javaObjectEwolCallback, m_javaMethodEwolActivityOpenURI, url);
			m_JavaVirtualMachinePointer->DeleteLocalRef(url);
			// manage execption :
			java_check_exception(m_JavaVirtualMachinePointer);
			java_detach_current_thread(status);
		}
		
		void sendSystemMessage(const char* _dataString) {
			EWOL_DEBUG("C->java : send message to the java : \"" << _dataString << "\"");
			int status;
			if(!java_attach_current_thread(&status)) {
				return;
			}
			EWOL_DEBUG("C->java : 222");
			if (nullptr == _dataString) {
				EWOL_ERROR("C->java : No data to send ...");
				return;
			}
			EWOL_DEBUG("C->java : 333");
			// create the string to the java
			jstring jstr = m_JavaVirtualMachinePointer->NewStringUTF(_dataString);
			if (jstr == 0) {
				EWOL_ERROR("C->java : Out of memory" );
				return;
			}
			EWOL_DEBUG("C->java : 444");
			// create argument list
			jobjectArray args = m_JavaVirtualMachinePointer->NewObjectArray(1, m_javaDefaultClassString, jstr);
			if (args == 0) {
				EWOL_ERROR("C->java : Out of memory" );
				return;
			}
			EWOL_DEBUG("C->java : 555");
			//Call java ...
			m_JavaVirtualMachinePointer->CallVoidMethod(m_javaObjectEwolCallback, m_javaMethodEwolCallbackEventNotifier, args);
			
			EWOL_DEBUG("C->java : 666");
			java_check_exception(m_JavaVirtualMachinePointer);
			java_detach_current_thread(status);
		}
	public:
		void OS_SetInputMotion(int _pointerID, const vec2& _pos) {
			ewol::Context::OS_SetInputMotion(_pointerID, vec2(_pos.x(),m_currentHeight-_pos.y()) );
		}
		
		void OS_SetInputState(int _pointerID, bool _isDown, const vec2& _pos) {
			ewol::Context::OS_SetInputState(_pointerID, _isDown, vec2(_pos.x(),m_currentHeight-_pos.y()) );
		}
		
		void OS_SetMouseMotion(int _pointerID, const vec2& _pos) {
			ewol::Context::OS_SetMouseMotion(_pointerID, vec2(_pos.x(),m_currentHeight-_pos.y()) );
		}
		
		void OS_SetMouseState(int _pointerID, bool _isDown, const vec2& _pos) {
			ewol::Context::OS_SetMouseState(_pointerID, _isDown, vec2(_pos.x(),m_currentHeight-_pos.y()) );
		}
		
		void ANDROID_SetKeyboard(char32_t _myChar, bool _isDown, bool _isARepeateKey=false) {
			OS_SetKeyboard(m_guiKeyBoardSpecialKeyMode, _myChar, _isDown, _isARepeateKey);
		}
		
		bool ANDROID_systemKeyboradEvent(enum ewol::key::keyboardSystem _key, bool _down) {
			return systemKeyboradEvent(_key, _down);
		}
		void ANDROID_SetKeyboardMove(int _move, bool _isDown, bool _isARepeateKey=false) {
			// direct wrapping :
			enum ewol::key::keyboard move = (enum ewol::key::keyboard)_move;
			m_guiKeyBoardSpecialKeyMode.update(move, _isDown);
			OS_SetKeyboardMove(m_guiKeyBoardSpecialKeyMode, move, _isDown, _isARepeateKey);
		}
		
		void OS_Resize(const vec2& _size) {
			m_currentHeight = _size.y();
			ewol::Context::OS_Resize(_size);
		}
		
		void audioPlayback(void* _dataOutput, int32_t _frameRate) {
			if (m_audioCallBack != nullptr) {
				//EWOL_DEBUG("IO Audio event request: Frames=" << _frameRate);
				m_audioCallBack(_dataOutput, _frameRate, m_audioCallBackUserData);
			}
		}
};

static std::vector<AndroidContext*> s_listInstance;
ewol::context::Application* s_applicationInit = NULL;

extern "C" {
	// JNI onLoad
	JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* _jvm, void* _reserved) {
		// get the java virtual machine handle ...
		std::unique_lock<std::mutex> lock(g_interfaceMutex);
		std::unique_lock<std::mutex> lock(g_interfaceAudioMutex);
		g_JavaVM = _jvm;
		EWOL_DEBUG("JNI-> load the jvm ..." );
		return JNI_VERSION_1_6;
	}
	// JNI onUnLoad
	JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* _vm, void *_reserved) {
		std::unique_lock<std::mutex> lock(g_interfaceMutex);
		std::unique_lock<std::mutex> lock(g_interfaceAudioMutex);
		g_JavaVM = nullptr;
		EWOL_DEBUG("JNI-> Un-load the jvm ..." );
	}
	
	/* Call to initialize the graphics state */
	void Java_org_ewol_Ewol_EWparamSetArchiveDir(JNIEnv* _env,
	                                             jclass _cls,
	                                             jint _id,
	                                             jint _mode,
	                                             jstring _myString) {
		std::unique_lock<std::mutex> lock(g_interfaceMutex);
		if(    _id >= (int32_t)s_listInstance.size()
		    || _id<0
		    || nullptr == s_listInstance[_id] ) {
			EWOL_ERROR("Call C With an incorrect instance _id=" << (int32_t)_id);
			// TODO : generate error in java to stop the current instance
			return;
		}
		//EWOL_CRITICAL(" call with ID : " << _id);
		// direct setting of the date in the string system ...
		jboolean isCopy;
		const char* str = _env->GetStringUTFChars(_myString, &isCopy);
		s_listInstance[_id]->setArchiveDir(_mode, str);
		if (isCopy == JNI_TRUE) {
			// from here str is reset ...
			_env->ReleaseStringUTFChars(_myString, str);
			str = nullptr;
		}
	}
	
	// declare main application instance like an application:
	int main(int argc, char**argv);
	jint Java_org_ewol_Ewol_EWsetJavaVirtualMachineStart(JNIEnv* _env,
	                                                     jclass _classBase,
	                                                     jobject _objCallback,
	                                                     int _typeApplication) {
		std::unique_lock<std::mutex> lock(g_interfaceMutex);
		EWOL_DEBUG("*******************************************");
		EWOL_DEBUG("** Creating EWOL context                 **");
		EWOL_DEBUG("*******************************************");
		AndroidContext* tmpContext = nullptr;
		s_applicationInit = NULL;
		ewol::context::Application* localApplication = NULL;
		// call the basic init of all application (that call us ...)
		main(0,NULL);
		localApplication = s_applicationInit;
		s_applicationInit = NULL;
		if (org_ewol_EwolConstants_EWOL_APPL_TYPE_ACTIVITY == _typeApplication) {
			tmpContext = new AndroidContext(localApplication, _env, _classBase, _objCallback, AndroidContext::appl_application);
		} else if (org_ewol_EwolConstants_EWOL_APPL_TYPE_WALLPAPER == _typeApplication) {
			tmpContext = new AndroidContext(localApplication, _env, _classBase, _objCallback, AndroidContext::appl_wallpaper);
		} else {
			EWOL_CRITICAL(" try to create an instance with no apply type: " << _typeApplication);
			return -1;
		}
		if (nullptr == tmpContext) {
			EWOL_ERROR("Can not allocate the main context instance _id=" << (s_listInstance.size()-1));
			return -1;
		}
		// for future case : all time this ...
		s_listInstance.push_back(tmpContext);
		int32_t newID = s_listInstance.size()-1;
		return newID;
	}
	
	void Java_org_ewol_Ewol_EWsetJavaVirtualMachineStop(JNIEnv* _env, jclass _cls, jint _id) {
		std::unique_lock<std::mutex> lock(g_interfaceMutex);
		EWOL_DEBUG("*******************************************");
		EWOL_DEBUG("** remove JVM Pointer                    **");
		EWOL_DEBUG("*******************************************");
		if(    _id >= (int32_t)s_listInstance.size()
		    || _id<0) {
			EWOL_ERROR("Call C With an incorrect instance _id=" << (int32_t)_id);
			return;
		}
		if (nullptr == s_listInstance[_id]) {
			EWOL_ERROR("the requested instance _id=" << (int32_t)_id << " is already removed ...");
			return;
		}
		s_listInstance[_id]->unInit(_env);
		delete(s_listInstance[_id]);
		s_listInstance[_id]=nullptr;
	}
	void Java_org_ewol_Ewol_EWtouchEvent(JNIEnv* _env, jobject _thiz, jint _id) {
		std::unique_lock<std::mutex> lock(g_interfaceMutex);
		EWOL_DEBUG("  == > Touch Event");
		if(    _id >= (int32_t)s_listInstance.size()
		    || _id<0
		    || nullptr == s_listInstance[_id] ) {
			EWOL_ERROR("Call C With an incorrect instance _id=" << (int32_t)_id);
			// TODO : generate error in java to stop the current instance
			return;
		}
		java_check_exception(_env);
	}
	
	void Java_org_ewol_Ewol_EWonCreate(JNIEnv* _env, jobject _thiz, jint _id) {
		std::unique_lock<std::mutex> lock(g_interfaceMutex);
		EWOL_DEBUG("*******************************************");
		EWOL_DEBUG("** Activity on Create                    **");
		EWOL_DEBUG("*******************************************");
		if(    _id >= (int32_t)s_listInstance.size()
		    || _id<0
		    || nullptr == s_listInstance[_id] ) {
			EWOL_ERROR("Call C With an incorrect instance _id=" << (int32_t)_id);
			// TODO : generate error in java to stop the current instance
			return;
		}
		//s_listInstance[_id]->init();
	}
	
	void Java_org_ewol_Ewol_EWonStart(JNIEnv* _env, jobject _thiz, jint _id) {
		std::unique_lock<std::mutex> lock(g_interfaceMutex);
		EWOL_DEBUG("*******************************************");
		EWOL_DEBUG("** Activity on Start                     **");
		EWOL_DEBUG("*******************************************");
		if(    _id >= (int32_t)s_listInstance.size()
		    || _id<0
		    || nullptr == s_listInstance[_id] ) {
			EWOL_ERROR("Call C With an incorrect instance _id=" << (int32_t)_id);
			// TODO : generate error in java to stop the current instance
			return;
		}
		//SendSystemMessage(" testmessages ... ");
	}
	void Java_org_ewol_Ewol_EWonReStart(JNIEnv* _env, jobject _thiz, jint _id) {
		std::unique_lock<std::mutex> lock(g_interfaceMutex);
		EWOL_DEBUG("*******************************************");
		EWOL_DEBUG("** Activity on Re-Start                  **");
		EWOL_DEBUG("*******************************************");
		if(    _id >= (int32_t)s_listInstance.size()
		    || _id<0
		    || nullptr == s_listInstance[_id] ) {
			EWOL_ERROR("Call C With an incorrect instance _id=" << (int32_t)_id);
			// TODO : generate error in java to stop the current instance
			return;
		}
	}
	void Java_org_ewol_Ewol_EWonResume(JNIEnv* _env, jobject _thiz, jint _id) {
		std::unique_lock<std::mutex> lock(g_interfaceMutex);
		EWOL_DEBUG("*******************************************");
		EWOL_DEBUG("** Activity on resume                    **");
		EWOL_DEBUG("*******************************************");
		if(    _id >= (int32_t)s_listInstance.size()
		    || _id<0
		    || nullptr == s_listInstance[_id] ) {
			EWOL_ERROR("Call C With an incorrect instance _id=" << (int32_t)_id);
			// TODO : generate error in java to stop the current instance
			return;
		}
		s_listInstance[_id]->OS_Resume();
	}
	void Java_org_ewol_Ewol_EWonPause(JNIEnv* _env, jobject _thiz, jint _id) {
		std::unique_lock<std::mutex> lock(g_interfaceMutex);
		EWOL_DEBUG("*******************************************");
		EWOL_DEBUG("** Activity on pause                     **");
		EWOL_DEBUG("*******************************************");
		if(    _id >= (int32_t)s_listInstance.size()
		    || _id<0
		    || nullptr == s_listInstance[_id] ) {
			EWOL_ERROR("Call C With an incorrect instance _id=" << (int32_t)_id);
			// TODO : generate error in java to stop the current instance
			return;
		}
		// All the openGl has been destroyed ...
		s_listInstance[_id]->getResourcesManager().contextHasBeenDestroyed();
		s_listInstance[_id]->OS_Suspend();
	}
	void Java_org_ewol_Ewol_EWonStop(JNIEnv* _env, jobject _thiz, jint _id) {
		std::unique_lock<std::mutex> lock(g_interfaceMutex);
		EWOL_DEBUG("*******************************************");
		EWOL_DEBUG("** Activity on Stop                      **");
		EWOL_DEBUG("*******************************************");
		if(    _id >= (int32_t)s_listInstance.size()
		    || _id<0
		    || nullptr == s_listInstance[_id] ) {
			EWOL_ERROR("Call C With an incorrect instance _id=" << (int32_t)_id);
			// TODO : generate error in java to stop the current instance
			return;
		}
		s_listInstance[_id]->OS_Stop();
	}
	void Java_org_ewol_Ewol_EWonDestroy(JNIEnv* _env, jobject _thiz, jint _id) {
		std::unique_lock<std::mutex> lock(g_interfaceMutex);
		EWOL_DEBUG("*******************************************");
		EWOL_DEBUG("** Activity on Destroy                   **");
		EWOL_DEBUG("*******************************************");
		if(    _id >= (int32_t)s_listInstance.size()
		    || _id<0
		    || nullptr == s_listInstance[_id] ) {
			EWOL_ERROR("Call C With an incorrect instance _id=" << (int32_t)_id);
			// TODO : generate error in java to stop the current instance
			return;
		}
		//s_listInstance[_id]->UnInit();
	}
	
	
	
	/* **********************************************************************************************
	 * ** IO section :
	 * ********************************************************************************************** */
	void Java_org_ewol_Ewol_EWinputEventMotion(JNIEnv* _env,
	                                           jobject _thiz,
	                                           jint _id,
	                                           jint _pointerID,
	                                           jfloat _x,
	                                           jfloat _y) {
		std::unique_lock<std::mutex> lock(g_interfaceMutex);
		if(    _id >= (int32_t)s_listInstance.size()
		    || _id<0
		    || nullptr == s_listInstance[_id] ) {
			EWOL_ERROR("Call C With an incorrect instance _id=" << (int32_t)_id);
			// TODO : generate error in java to stop the current instance
			return;
		}
		s_listInstance[_id]->OS_SetInputMotion(_pointerID+1, vec2(_x,_y));
	}
	
	void Java_org_ewol_Ewol_EWinputEventState(JNIEnv* _env,
	                                          jobject _thiz,
	                                          jint _id,
	                                          jint _pointerID,
	                                          jboolean _isUp,
	                                          jfloat _x,
	                                          jfloat _y) {
		std::unique_lock<std::mutex> lock(g_interfaceMutex);
		if(    _id >= (int32_t)s_listInstance.size()
		    || _id<0
		    || nullptr == s_listInstance[_id] ) {
			EWOL_ERROR("Call C With an incorrect instance _id=" << (int32_t)_id);
			// TODO : generate error in java to stop the current instance
			return;
		}
		s_listInstance[_id]->OS_SetInputState(_pointerID+1, _isUp, vec2(_x,_y));
	}
	
	void Java_org_ewol_Ewol_EWmouseEventMotion(JNIEnv* _env,
	                                           jobject _thiz,
	                                           jint _id,
	                                           jint _pointerID,
	                                           jfloat _x,
	                                           jfloat _y) {
		std::unique_lock<std::mutex> lock(g_interfaceMutex);
		if(    _id >= (int32_t)s_listInstance.size()
		    || _id<0
		    || nullptr == s_listInstance[_id] ) {
			EWOL_ERROR("Call C With an incorrect instance _id=" << (int32_t)_id);
			// TODO : generate error in java to stop the current instance
			return;
		}
		s_listInstance[_id]->OS_SetMouseMotion(_pointerID+1, vec2(_x,_y));
	}
	
	void Java_org_ewol_Ewol_EWmouseEventState(JNIEnv* _env,
	                                          jobject _thiz,
	                                          jint _id,
	                                          jint _pointerID,
	                                          jboolean _isUp,
	                                          jfloat _x,
	                                          jfloat _y) {
		std::unique_lock<std::mutex> lock(g_interfaceMutex);
		if(    _id >= (int32_t)s_listInstance.size()
		    || _id<0
		    || nullptr == s_listInstance[_id] ) {
			EWOL_ERROR("Call C With an incorrect instance _id=" << (int32_t)_id);
			// TODO : generate error in java to stop the current instance
			return;
		}
		s_listInstance[_id]->OS_SetMouseState(_pointerID+1, _isUp, vec2(_x,_y));
	}
	
	void Java_org_ewol_Ewol_EWunknowEvent(JNIEnv* _env,
	                                      jobject _thiz,
	                                      jint _id,
	                                      jint _pointerID) {
		std::unique_lock<std::mutex> lock(g_interfaceMutex);
		if(    _id >= (int32_t)s_listInstance.size()
		    || _id<0
		    || nullptr == s_listInstance[_id] ) {
			EWOL_ERROR("Call C With an incorrect instance _id=" << (int32_t)_id);
			// TODO : generate error in java to stop the current instance
			return;
		}
		EWOL_DEBUG("Unknown IO event : " << _pointerID << " ???");
	}
	
	void Java_org_ewol_Ewol_EWkeyboardEventMove(JNIEnv* _env,
	                                            jobject _thiz,
	                                            jint _id,
	                                            jint _type,
	                                            jboolean _isdown) {
		std::unique_lock<std::mutex> lock(g_interfaceMutex);
		if(    _id >= (int32_t)s_listInstance.size()
		    || _id<0
		    || nullptr == s_listInstance[_id] ) {
			EWOL_ERROR("Call C With an incorrect instance _id=" << (int32_t)_id);
			// TODO : generate error in java to stop the current instance
			return;
		}
		EWOL_DEBUG("IO keyboard Move event : \"" << _type << "\" is down=" << _isdown);
		s_listInstance[_id]->ANDROID_SetKeyboardMove(_type, _isdown);
	}
	
	void Java_org_ewol_Ewol_EWkeyboardEventKey(JNIEnv* _env,
	                                           jobject _thiz,
	                                           jint _id,
	                                           jint _uniChar,
	                                           jboolean _isdown) {
		std::unique_lock<std::mutex> lock(g_interfaceMutex);
		if(    _id >= (int32_t)s_listInstance.size()
		    || _id<0
		    || nullptr == s_listInstance[_id] ) {
			EWOL_ERROR("Call C With an incorrect instance _id=" << (int32_t)_id);
			// TODO : generate error in java to stop the current instance
			return;
		}
		EWOL_DEBUG("IO keyboard Key event : \"" << _uniChar << "\" is down=" << _isdown);
		s_listInstance[_id]->ANDROID_SetKeyboard(_uniChar, _isdown);
	}
	
	void Java_org_ewol_Ewol_EWdisplayPropertyMetrics(JNIEnv* _env,
	                                                 jobject _thiz,
	                                                 jint _id,
	                                                 jfloat _ratioX,
	                                                 jfloat _ratioY) {
		std::unique_lock<std::mutex> lock(g_interfaceMutex);
		if(    _id >= (int32_t)s_listInstance.size()
		    || _id<0
		    || nullptr == s_listInstance[_id] ) {
			EWOL_ERROR("Call C With an incorrect instance _id=" << (int32_t)_id);
			// TODO : generate error in java to stop the current instance
			return;
		}
		// set the internal system ratio properties ...
		ewol::Dimension::setPixelRatio(vec2(_ratioX,_ratioY), ewol::Dimension::Inch);
	}
	
	// TODO : set a return true or false if we want to grep this event ...
	bool Java_org_ewol_Ewol_EWkeyboardEventKeySystem(JNIEnv* _env,
	                                                 jobject _thiz,
	                                                 jint _id,
	                                                 jint _keyVal,
	                                                 jboolean _isdown) {
		std::unique_lock<std::mutex> lock(g_interfaceMutex);
		if(    _id >= (int32_t)s_listInstance.size()
		    || _id<0
		    || nullptr == s_listInstance[_id] ) {
			EWOL_ERROR("Call C With an incorrect instance _id=" << (int32_t)_id);
			// TODO : generate error in java to stop the current instance
			return false;
		}
		switch (_keyVal) {
			case org_ewol_EwolConstants_EWOL_SYSTEM_KEY_VOLUME_UP:
				EWOL_VERBOSE("IO keyboard Key system \"VOLUME_UP\" is down=" << _isdown);
				return s_listInstance[_id]->ANDROID_systemKeyboradEvent(ewol::key::keyboardSystemVolumeUp, _isdown);
			case org_ewol_EwolConstants_EWOL_SYSTEM_KEY_VOLUME_DOWN:
				EWOL_DEBUG("IO keyboard Key system \"VOLUME_DOWN\" is down=" << _isdown);
				return s_listInstance[_id]->ANDROID_systemKeyboradEvent(ewol::key::keyboardSystemVolumeDown, _isdown);
			case org_ewol_EwolConstants_EWOL_SYSTEM_KEY_MENU:
				EWOL_DEBUG("IO keyboard Key system \"MENU\" is down=" << _isdown);
				return s_listInstance[_id]->ANDROID_systemKeyboradEvent(ewol::key::keyboardSystemMenu, _isdown);
			case org_ewol_EwolConstants_EWOL_SYSTEM_KEY_CAMERA:
				EWOL_DEBUG("IO keyboard Key system \"CAMERA\" is down=" << _isdown);
				return s_listInstance[_id]->ANDROID_systemKeyboradEvent(ewol::key::keyboardSystemCamera, _isdown);
			case org_ewol_EwolConstants_EWOL_SYSTEM_KEY_HOME:
				EWOL_DEBUG("IO keyboard Key system \"HOME\" is down=" << _isdown);
				return s_listInstance[_id]->ANDROID_systemKeyboradEvent(ewol::key::keyboardSystemHome, _isdown);
			case org_ewol_EwolConstants_EWOL_SYSTEM_KEY_POWER:
				EWOL_DEBUG("IO keyboard Key system \"POWER\" is down=" << _isdown);
				return s_listInstance[_id]->ANDROID_systemKeyboradEvent(ewol::key::keyboardSystemPower, _isdown);
			case org_ewol_EwolConstants_EWOL_SYSTEM_KEY_BACK:
				EWOL_DEBUG("IO keyboard Key system \"BACK\" is down=" << _isdown);
				return s_listInstance[_id]->ANDROID_systemKeyboradEvent(ewol::key::keyboardSystemBack, _isdown);
			default:
				EWOL_ERROR("IO keyboard Key system event : \"" << _keyVal << "\" is down=" << _isdown);
				break;
		}
		return false;
	}
	
	
	/* **********************************************************************************************
	 * **	Renderer section :
	 * ********************************************************************************************** */
	void Java_org_ewol_Ewol_EWrenderInit(JNIEnv* _env,
	                                     jobject _thiz,
	                                     jint _id) {
		std::unique_lock<std::mutex> lock(g_interfaceMutex);
		if(    _id >= (int32_t)s_listInstance.size()
		    || _id<0
		    || nullptr == s_listInstance[_id] ) {
			EWOL_ERROR("Call C With an incorrect instance _id=" << (int32_t)_id);
			// TODO : generate error in java to stop the current instance
			return;
		}
	}
	
	void Java_org_ewol_Ewol_EWrenderResize(JNIEnv* _env,
	                                       jobject _thiz,
	                                       jint _id,
	                                       jint _w,
	                                       jint _h) {
		std::unique_lock<std::mutex> lock(g_interfaceMutex);
		if(    _id >= (int32_t)s_listInstance.size()
		    || _id<0
		    || nullptr == s_listInstance[_id] ) {
			EWOL_ERROR("Call C With an incorrect instance _id=" << (int32_t)_id);
			// TODO : generate error in java to stop the current instance
			return;
		}
		s_listInstance[_id]->OS_Resize(vec2(_w, _h));
	}
	
	// TODO : Return true or false to not redraw when the under draw has not be done (processing gain of time)
	void Java_org_ewol_Ewol_EWrenderDraw(JNIEnv* _env,
	                                     jobject _thiz,
	                                     jint _id) {
		std::unique_lock<std::mutex> lock(g_interfaceMutex);
		if(    _id >= (int32_t)s_listInstance.size()
		    || _id<0
		    || nullptr == s_listInstance[_id] ) {
			EWOL_ERROR("Call C With an incorrect instance _id=" << (int32_t)_id);
			// TODO : generate error in java to stop the current instance
			return;
		}
		s_listInstance[_id]->OS_Draw(true);
	}
	
	void Java_org_ewol_Ewol_EWaudioPlayback(JNIEnv* _env,
	                                        void* _reserved,
	                                        jint _id,
	                                        jshortArray _location,
	                                        jint _frameRate,
	                                        jint _nbChannels) {
		std::unique_lock<std::mutex> lock(g_interfaceAudioMutex);
		if(    _id >= (int32_t)s_listInstance.size()
		    || _id<0
		    || nullptr == s_listInstance[_id] ) {
			EWOL_ERROR("Call C With an incorrect instance _id=" << (int32_t)_id);
			// TODO : generate error in java to stop the current instance
			return;
		}
		// get the short* pointer from the Java array
		jboolean isCopy;
		jshort* dst = _env->GetShortArrayElements(_location, &isCopy);
		if (nullptr != dst) {
			memset(dst, sizeof(jshort), _frameRate*_nbChannels);
			//EWOL_DEBUG("IO Audio event request: Frames=" << _frameRate << " channels=" << _nbChannels);
			s_listInstance[_id]->audioPlayback(dst, _frameRate);
		}
		// TODO : Understand why it did not work corectly ...
		//if (isCopy == JNI_TRUE) {
		// release the short* pointer
		_env->ReleaseShortArrayElements(_location, dst, 0);
		//}
	}
};


int ewol::run(ewol::context::Application* _application, int _argc, const char *_argv[]) {
	s_applicationInit = _application;
	return 0;
}
