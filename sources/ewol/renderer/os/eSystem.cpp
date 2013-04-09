/**
 * @author Edouard DUPIN
 * 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * 
 * @license BSD v3 (see license file)
 */

#include <unistd.h>

#include <etk/types.h>
#include <etk/MessageFifo.h>

#include <ewol/ewol.h>
#include <ewol/DisplayConv.h>
#include <ewol/debug.h>

#include <ewol/config.h>

#include <ewol/eObject/EObject.h>
#include <ewol/eObject/EObjectManager.h>

#include <ewol/renderer/os/eSystem.h>
#include <ewol/renderer/os/gui.h>
#include <ewol/renderer/ResourceManager.h>
#include <ewol/renderer/os/eSystemInput.h>
#include <ewol/renderer/openGL.h>
#include <ewol/renderer/os/Fps.h>

#include <ewol/widget/WidgetManager.h>


static bool                             requestEndProcessing = false;
static bool                             isGlobalSystemInit = false;
static ewol::Windows*                   windowsCurrent = NULL;
static int64_t                          previousDisplayTime = 0;  // this is to limit framerate ... in case...
static ivec2                            windowsSize(320, 480);
static ewol::eSystemInput               l_managementInput;
static ewol::Fps                        l_FpsSystemEvent(     "Event     ", false);
static ewol::Fps                        l_FpsSystemContext(   "Context   ", false);
static ewol::Fps                        l_FpsSystem(          "Draw      ", true);
static ewol::Fps                        l_FpsFlush(           "Flush     ", false);



void eSystem::InputEventTransfertWidget(ewol::Widget* source, ewol::Widget* destination)
{
	l_managementInput.TransfertEvent(source, destination);
}


void eSystem::InputEventGrabPointer(ewol::Widget* widget)
{
	l_managementInput.GrabPointer(widget);
}

void eSystem::InputEventUnGrabPointer(void)
{
	l_managementInput.UnGrabPointer();
}

typedef enum {
	THREAD_NONE,
	THREAD_INIT,
	THREAD_RECALCULATE_SIZE,
	THREAD_RESIZE,
	THREAD_HIDE,
	THREAD_SHOW,
	
	THREAD_INPUT_MOTION,
	THREAD_INPUT_STATE,
	
	THREAD_KEYBORAD_KEY,
	THREAD_KEYBORAD_MOVE,
	
	THREAD_CLIPBOARD_ARRIVE,
} theadMessage_te;



class eSystemMessage {
	public :
		// specify the message type
		theadMessage_te TypeMessage;
		// can not set a union ...
		ewol::clipBoard::clipboardListe_te clipboardID;
		// InputId
		ewol::keyEvent::type_te inputType;
		int32_t                 inputId;
		// generic dimentions
		vec2 dimention;
		// keyboard events :
		bool                        repeateKey;  //!< special flag for the repeating key on the PC interface
		bool                        stateIsDown;
		uniChar_t                   keyboardChar;
		ewol::keyEvent::keyboard_te keyboardMove;
		ewol::SpecialKey            keyboardSpecial;
		
		eSystemMessage(void) :
			TypeMessage(THREAD_NONE),
			clipboardID(ewol::clipBoard::clipboardStd),
			inputType(ewol::keyEvent::typeUnknow),
			inputId(-1),
			dimention(0,0),
			repeateKey(false),
			stateIsDown(false),
			keyboardChar(0),
			keyboardMove(ewol::keyEvent::keyboardUnknow)
		{
			
		}
};

static etk::MessageFifo<eSystemMessage> l_msgSystem;


void ewolProcessEvents(void)
{
	int32_t nbEvent = 0;
	//EWOL_DEBUG(" ********  Event");
	eSystemMessage data;
	while (l_msgSystem.Count()>0) 
	{
		nbEvent++;
		l_msgSystem.Wait(data);
		//EWOL_DEBUG("EVENT");
		switch (data.TypeMessage) {
			case THREAD_INIT:
				// this is due to the openGL context
				APP_Init();
				break;
			case THREAD_RECALCULATE_SIZE:
				eSystem::ForceRedrawAll();
				break;
			case THREAD_RESIZE:
				//EWOL_DEBUG("Receive MSG : THREAD_RESIZE");
				windowsSize = data.dimention;
				ewol::SetPixelWindowsSize(vec2(windowsSize.x(),windowsSize.y()));
				eSystem::ForceRedrawAll();
				break;
			case THREAD_INPUT_MOTION:
				//EWOL_DEBUG("Receive MSG : THREAD_INPUT_MOTION");
				l_managementInput.Motion(data.inputType, data.inputId, data.dimention);
				break;
			case THREAD_INPUT_STATE:
				//EWOL_DEBUG("Receive MSG : THREAD_INPUT_STATE");
				l_managementInput.State(data.inputType, data.inputId, data.stateIsDown, data.dimention);
				break;
			case THREAD_KEYBORAD_KEY:
			case THREAD_KEYBORAD_MOVE:
				//EWOL_DEBUG("Receive MSG : THREAD_KEYBORAD_KEY");
				{
					ewol::SpecialKey& specialCurrentKey = ewol::GetCurrentSpecialKeyStatus();
					specialCurrentKey = data.keyboardSpecial;
					//EWOL_DEBUG("newStatus Key" << specialCurrentKey);
				}
				if (NULL != windowsCurrent) {
					if (false==windowsCurrent->OnEventShortCut(data.keyboardSpecial,
					                                           data.keyboardChar,
					                                           data.keyboardMove,
					                                           data.stateIsDown) ) {
						// Get the current Focused Widget :
						ewol::Widget * tmpWidget = ewol::widgetManager::FocusGet();
						if (NULL != tmpWidget) {
							// check if the widget allow repeating key events.
							//EWOL_DEBUG("repeating test :" << data.repeateKey << " widget=" << tmpWidget->GetKeyboardRepeate() << " state=" << data.stateIsDown);
							if(    false==data.repeateKey
							    || (    true==data.repeateKey
							         && true==tmpWidget->GetKeyboardRepeate()) ) {
								// check Widget shortcut
								if (false==tmpWidget->OnEventShortCut(data.keyboardSpecial,
								                                      data.keyboardChar,
								                                      data.keyboardMove,
								                                      data.stateIsDown) ) {
									// generate the direct event ...
									if (data.TypeMessage == THREAD_KEYBORAD_KEY) {
										if(true == data.stateIsDown) {
											tmpWidget->OnEventKb(ewol::keyEvent::statusDown, data.keyboardChar);
										} else {
											tmpWidget->OnEventKb(ewol::keyEvent::statusUp, data.keyboardChar);
										}
									} else { // THREAD_KEYBORAD_MOVE
										EWOL_DEBUG("THREAD_KEYBORAD_MOVE" << data.keyboardMove << " " << data.stateIsDown);
										if(true == data.stateIsDown) {
											tmpWidget->OnEventKbMove(ewol::keyEvent::statusDown, data.keyboardMove);
										} else {
											tmpWidget->OnEventKbMove(ewol::keyEvent::statusUp, data.keyboardMove);
										}
									}
								} else {
									EWOL_DEBUG("remove Repeate key ...");
								}
							}
						}
					}
				}
				break;
			case THREAD_CLIPBOARD_ARRIVE:
				{
					ewol::Widget * tmpWidget = ewol::widgetManager::FocusGet();
					if (tmpWidget != NULL) {
						tmpWidget->OnEventClipboard(data.clipboardID);
					}
				}
				break;
			case THREAD_HIDE:
				EWOL_DEBUG("Receive MSG : THREAD_HIDE");
				//guiAbstraction::SendKeyboardEventMove(tmpData->isDown, tmpData->move);
				//gui_uniqueWindows->SysOnHide();
				break;
			case THREAD_SHOW:
				EWOL_DEBUG("Receive MSG : THREAD_SHOW");
				//guiAbstraction::SendKeyboardEventMove(tmpData->isDown, tmpData->move);
				//gui_uniqueWindows->SysOnShow();
				break;
			default:
				EWOL_DEBUG("Receive MSG : UNKNOW");
				break;
		}
	}
}



void eSystem::SetArchiveDir(int mode, const char* str)
{
	switch(mode)
	{
		case 0:
			EWOL_DEBUG("Directory APK : path=" << str);
			etk::SetBaseFolderData(str);
			break;
		case 1:
			EWOL_DEBUG("Directory mode=FILE path=" << str);
			etk::SetBaseFolderDataUser(str);
			break;
		case 2:
			EWOL_DEBUG("Directory mode=CACHE path=" << str);
			etk::SetBaseFolderCache(str);
			break;
		case 3:
			EWOL_DEBUG("Directory mode=EXTERNAL_CACHE path=" << str);
			break;
		default:
			EWOL_DEBUG("Directory mode=???? path=" << str);
			break;
	}
}






void RequestInit(void)
{
	if (true == isGlobalSystemInit) {
		eSystemMessage data;
		data.TypeMessage = THREAD_INIT;
		l_msgSystem.Post(data);
	}
}


void eSystem::Init(void)
{
	EWOL_INFO("==> Ewol System Init (BEGIN)");
	if (false == isGlobalSystemInit) {
		l_msgSystem.Clean();
		requestEndProcessing = false;
		EWOL_INFO("v" EWOL_VERSION_TAG_NAME);
		EWOL_INFO("Build Date: " BUILD_TIME);
		etk::InitDefaultFolder("ewolApplNoName");
		ewol::openGL::Init();
		ewol::EObjectManager::Init();
		ewol::EObjectMessageMultiCast::Init();
		l_managementInput.Reset();
		ewol::resource::Init();
		ewol::widgetManager::Init();
		ewol::config::Init();
		isGlobalSystemInit = true;
		// request the init of the application in the main context of openGL ...
		RequestInit();
		// force a recalculation
		ewol::RequestUpdateSize();
		#if defined(__EWOL_ANDROID_ORIENTATION_LANDSCAPE__)
			ewol::ForceOrientation(ewol::SCREEN_ORIENTATION_LANDSCAPE);
		#elif defined(__EWOL_ANDROID_ORIENTATION_PORTRAIT__)
			ewol::ForceOrientation(ewol::SCREEN_ORIENTATION_PORTRAIT);
		#else
			ewol::ForceOrientation(ewol::SCREEN_ORIENTATION_AUTO);
		#endif
	}
	EWOL_INFO("==> Ewol System Init (END)");
}

void eSystem::UnInit(void)
{
	EWOL_INFO("==> Ewol System Un-Init (BEGIN)");
	if (true == isGlobalSystemInit) {
		isGlobalSystemInit = false;
		requestEndProcessing = true;
		// unset all windows
		ewol::WindowsSet(NULL);
		// call application to uninit
		APP_UnInit();
		ewol::widgetManager::UnInit();
		ewol::config::UnInit();
		ewol::EObjectMessageMultiCast::UnInit();
		ewol::EObjectManager::UnInit();
		ewol::resource::UnInit();
		ewol::openGL::UnInit();
		l_managementInput.Reset();
		l_msgSystem.Clean();
	}
	EWOL_INFO("==> Ewol System Un-Init (END)");
}


void eSystem::RequestUpdateSize(void)
{
	if (true == isGlobalSystemInit) {
		eSystemMessage data;
		data.TypeMessage = THREAD_RECALCULATE_SIZE;
		l_msgSystem.Post(data);
	}
}

void eSystem::Resize(int w, int h )
{
	// TODO : Better in the thread ... ==> but generate some init error ...
	ewol::SetPixelWindowsSize(vec2(w,h));
	if (true == isGlobalSystemInit) {
		eSystemMessage data;
		data.TypeMessage = THREAD_RESIZE;
		data.dimention.setValue(w,h);
		l_msgSystem.Post(data);
	}
}
void eSystem::Move(int w, int h )
{
	if (true == isGlobalSystemInit) {
		/*
		eSystemMessage data;
		data.TypeMessage = THREAD_RESIZE;
		data.resize.w = w;
		data.resize.h = h;
		l_msgSystem.Post(data);
		*/
	}
}

void eSystem::SetInputMotion(int pointerID, float x, float y )
{
	if (true == isGlobalSystemInit) {
		eSystemMessage data;
		data.TypeMessage = THREAD_INPUT_MOTION;
		data.inputType = ewol::keyEvent::typeFinger;
		data.inputId = pointerID;
		data.dimention.setValue(x,y);
		l_msgSystem.Post(data);
	}
}


void eSystem::SetInputState(int pointerID, bool isDown, float x, float y )
{
	if (true == isGlobalSystemInit) {
		eSystemMessage data;
		data.TypeMessage = THREAD_INPUT_STATE;
		data.inputType = ewol::keyEvent::typeFinger;
		data.inputId = pointerID;
		data.stateIsDown = isDown;
		data.dimention.setValue(x,y);
		l_msgSystem.Post(data);
	}
}


void eSystem::SetMouseMotion(int pointerID, float x, float y )
{
	if (true == isGlobalSystemInit) {
		eSystemMessage data;
		data.TypeMessage = THREAD_INPUT_MOTION;
		data.inputType = ewol::keyEvent::typeMouse;
		data.inputId = pointerID;
		data.dimention.setValue(x,y);
		l_msgSystem.Post(data);
	}
}


void eSystem::SetMouseState(int pointerID, bool isDown, float x, float y )
{
	if (true == isGlobalSystemInit) {
		eSystemMessage data;
		data.TypeMessage = THREAD_INPUT_STATE;
		data.inputType = ewol::keyEvent::typeMouse;
		data.inputId = pointerID;
		data.stateIsDown = isDown;
		data.dimention.setValue(x,y);
		l_msgSystem.Post(data);
	}
}


void eSystem::SetKeyboard(ewol::SpecialKey& special,
                          uniChar_t myChar,
                          bool isDown,
                          bool isARepeateKey)
{
	if (true == isGlobalSystemInit) {
		eSystemMessage data;
		data.TypeMessage = THREAD_KEYBORAD_KEY;
		data.stateIsDown = isDown;
		data.keyboardChar = myChar;
		data.keyboardSpecial = special;
		data.repeateKey = isARepeateKey;
		l_msgSystem.Post(data);
	}
}

void eSystem::SetKeyboardMove(ewol::SpecialKey& special,
                              ewol::keyEvent::keyboard_te move,
                              bool isDown,
                              bool isARepeateKey)
{
	if (true == isGlobalSystemInit) {
		eSystemMessage data;
		data.TypeMessage = THREAD_KEYBORAD_MOVE;
		data.stateIsDown = isDown;
		data.keyboardMove = move;
		data.keyboardSpecial = special;
		data.repeateKey = isARepeateKey;
		l_msgSystem.Post(data);
	}
}


void eSystem::Hide(void)
{
	if (true == isGlobalSystemInit) {
		eSystemMessage data;
		data.TypeMessage = THREAD_HIDE;
		l_msgSystem.Post(data);
	}
}

void eSystem::Show(void)
{
	if (true == isGlobalSystemInit) {
		eSystemMessage data;
		data.TypeMessage = THREAD_SHOW;
		l_msgSystem.Post(data);
	}
}


void eSystem::ClipBoardArrive(ewol::clipBoard::clipboardListe_te clipboardID)
{
	if (true == isGlobalSystemInit) {
		eSystemMessage data;
		data.TypeMessage = THREAD_CLIPBOARD_ARRIVE;
		data.clipboardID = clipboardID;
		l_msgSystem.Post(data);
	}
}

bool eSystem::Draw(bool displayEveryTime)
{
	int64_t currentTime = ewol::GetTime();
	// this is to prevent the multiple display at the a high frequency ...
	#ifndef __PLATFORM__Android
	if(currentTime - previousDisplayTime < 1000000/120) {
		usleep(1000);
		return false;
	}
	#endif
	previousDisplayTime = currentTime;
	
	if (true == isGlobalSystemInit) {
		// process the events
l_FpsSystemEvent.Tic();
		ewolProcessEvents();
		// call all the widget that neded to do something periodicly
		ewol::widgetManager::PeriodicCall(currentTime);
		// Remove all widget that they are no more usefull (these who decided to destroy themself)
		ewol::EObjectManager::RemoveAllAutoRemove();
		ewol::Windows* tmpWindows = eSystem::GetCurrentWindows();
		// check if the user selected a windows
		if (NULL != tmpWindows) {
			// Redraw all needed elements
			tmpWindows->OnRegenerateDisplay();
		}
l_FpsSystemEvent.IncrementCounter();
l_FpsSystemEvent.Toc();
		bool needRedraw = ewol::widgetManager::IsDrawingNeeded();
		
l_FpsSystemContext.Tic();
		if (NULL != tmpWindows) {
			if(    true == needRedraw
			    || true == displayEveryTime) {
				ewol::resource::UpdateContext();
l_FpsSystemContext.IncrementCounter();
			}
		}
l_FpsSystemContext.Toc();
		bool hasDisplayDone = false;
l_FpsSystem.Tic();
		if (NULL != tmpWindows) {
			if(    true == needRedraw
			    || true == displayEveryTime) {
l_FpsSystem.IncrementCounter();
				tmpWindows->SysDraw();
				hasDisplayDone = true;
			}
		}
l_FpsSystem.Toc();

l_FpsFlush.Tic();
l_FpsFlush.IncrementCounter();
		glFlush();
		//glFinish();
l_FpsFlush.Toc();

l_FpsSystemEvent.Draw();
l_FpsSystemContext.Draw();
l_FpsSystem.Draw();
l_FpsFlush.Draw();
		return hasDisplayDone;
	}
	return false;
}


void eSystem::OnObjectRemove(ewol::EObject * removeObject)
{
	l_managementInput.OnObjectRemove(removeObject);
}


void eSystem::ResetIOEvent(void)
{
	l_managementInput.NewLayerSet();
}


void eSystem::OpenGlContextDestroy(void)
{
	ewol::resource::ContextHasBeenDestroyed();
}


void eSystem::SetCurrentWindows(ewol::Windows * windows)
{
	// set the new pointer as windows system
	windowsCurrent = windows;
	// request all the widget redrawing
	eSystem::ForceRedrawAll();
}


ewol::Windows* eSystem::GetCurrentWindows(void)
{
	return windowsCurrent;
}


ivec2 eSystem::GetSize(void)
{
	return windowsSize;
}


void eSystem::ForceRedrawAll(void)
{
	ewol::Windows* tmpWindows = eSystem::GetCurrentWindows();
	if (NULL != tmpWindows) {
		ivec2 systemSize = eSystem::GetSize();
		tmpWindows->CalculateSize(vec2(systemSize.x(), systemSize.y()));
	}
}


void eSystem::OnKill(void)
{
	ewol::Windows* tmpWindows = eSystem::GetCurrentWindows();
	if (NULL != tmpWindows) {
		tmpWindows->SysOnKill();
	}
}

