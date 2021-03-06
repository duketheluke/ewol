/**
 * @author Edouard DUPIN
 * 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * 
 * @license APACHE v2.0 (see license file)
 */



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <etk/types.h>
#include <etk/os/FSNode.h>

#include <ewol/debug.h>
#include <ewol/ewol.h>
#include <ewol/key/key.h>
#include <ewol/context/commandLine.h>
#include <ewol/widget/Manager.h>
#include <ewol/resource/Manager.h>
#include <ewol/context/Context.h>
#include <ewol/Dimension.h>


#if defined(__TARGET_OS__Linux)
#include <GL/glx.h>
#elif defined(__TARGET_OS__MacOs)
#include <OpenGL/glx.h>
#endif

#include <X11/Xatom.h>
#include <sys/times.h>

/*
#define GUI_LOCK()          XLockDisplay(m_display)
#define GUI_UNLOCK()        XUnlockDisplay(m_display)
*/
bool hasDisplay = false;
//#define DEBUG_X11_EVENT
#ifdef DEBUG_X11_EVENT
	#define X11_DEBUG      EWOL_DEBUG
	#define X11_VERBOSE    EWOL_VERBOSE
	#define X11_INFO       EWOL_INFO
	#define X11_CRITICAL   EWOL_CRITICAL
#else
	#define X11_DEBUG      EWOL_VERBOSE
	#define X11_VERBOSE    EWOL_VERBOSE
	#define X11_INFO       EWOL_VERBOSE
	#define X11_CRITICAL   EWOL_VERBOSE
#endif

int64_t ewol::getTime() {
	struct timespec now;
	int ret = clock_gettime(CLOCK_REALTIME, &now);
	if (ret != 0) {
		// Error to get the time ...
		now.tv_sec = time(nullptr);
		now.tv_nsec = 0;
	}
	//EWOL_VERBOSE("current time : " << now.tv_sec << "s " << now.tv_usec << "us");
	return (int64_t)((int64_t)now.tv_sec*(int64_t)1000000 + (int64_t)now.tv_nsec/(int64_t)1000);
}

// attributes for a single buffered visual in RGBA format with at least 4 bits per color and a 16 bit depth buffer
static int attrListSgl[] = {
	GLX_RGBA,
	GLX_RED_SIZE, 4,
	GLX_GREEN_SIZE, 4,
	GLX_BLUE_SIZE, 4,
	GLX_DEPTH_SIZE, 16,
	None
};

// attributes for a double buffered visual in RGBA format with at least 4 bits per color and a 16 bit depth buffer
static int attrListDbl[] = {
	GLX_RGBA,
	GLX_DOUBLEBUFFER,
	GLX_RED_SIZE, 4,
	GLX_GREEN_SIZE, 4,
	GLX_BLUE_SIZE, 4,
	GLX_DEPTH_SIZE, 16,
	None
};

extern "C" {
	typedef struct Hints {
		unsigned long   flags;
		unsigned long   functions;
		unsigned long   decorations;
		long            inputMode;
		unsigned long   status;
	} Hints;
}

#include <egami/egami.h>

#include <X11/cursorfont.h>

#undef __class__
#define __class__ "x11Interface"

class X11Interface : public ewol::Context {
	private:
		ewol::key::Special m_guiKeyBoardMode;
		// for double and triple click selection, we need to save the previous click up and down position , and the previous time ...
		Atom m_delAtom;
		Display* m_display;
		Window m_WindowHandle;
		XIM m_xim;
		XIC m_xic;
		int32_t m_originX;
		int32_t m_originY;
		int32_t m_cursorEventX;
		int32_t m_cursorEventY;
		int32_t m_currentHeight;
		int32_t m_currentWidth;
		XVisualInfo* m_visual;
		bool m_doubleBuffered;
		bool m_run;
		//forcing the position
		bool m_grabAllEvent; //!< grab mode enable...
		vec2 m_forcePos; //!< position to reset the cursor
		bool m_positionChangeRequested; //!< the position modifiquation has been requested
		vec2 m_curentGrabDelta; //!< the position in X11 will arrive by pool
		bool m_inputIsPressed[MAX_MANAGE_INPUT];
		// internal copy of the clipBoard ...
		bool m_clipBoardRequestPrimary; //!< if false : request the copy/past buffer, if true : request current selection
		bool m_clipBoardOwnerPrimary; //!< we are the owner of the current selection
		bool m_clipBoardOwnerStd; //!< we are the owner of the current copy buffer
		// Atom access...
		Atom XAtomeSelection;
		Atom XAtomeClipBoard;
		Atom XAtomeTargetString;
		Atom XAtomeTargetStringUTF8;
		Atom XAtomeTargetTarget;
		Atom XAtomeEWOL;
		Atom XAtomeDeleteWindows;
		enum ewol::context::cursorDisplay m_currentCursor; //!< select the current cursor to display :
		char32_t m_lastKeyPressed; //!< The last element key presed...
	public:
		X11Interface(ewol::context::Application* _application, int32_t _argc, const char* _argv[]) :
		  ewol::Context(_application, _argc, _argv),
		  m_display(nullptr),
		  m_originX(0),
		  m_originY(0),
		  m_cursorEventX(0),
		  m_cursorEventY(0),
		  m_currentHeight(0),
		  m_currentWidth(0),
		  m_visual(nullptr),
		  m_doubleBuffered(0),
		  m_run(false),
		  m_grabAllEvent(false),
		  m_forcePos(0,0),
		  m_positionChangeRequested(false),
		  m_curentGrabDelta(0,0),
		  m_clipBoardRequestPrimary(false),
		  m_clipBoardOwnerPrimary(false),
		  m_clipBoardOwnerStd(false),
		  XAtomeSelection(0),
		  XAtomeClipBoard(0),
		  XAtomeTargetString(0),
		  XAtomeTargetStringUTF8(0),
		  XAtomeTargetTarget(0),
		  XAtomeEWOL(0),
		  XAtomeDeleteWindows(0),
		  m_currentCursor(ewol::context::cursorArrow),
		  m_lastKeyPressed(0) {
			X11_INFO("X11:INIT");
			for (int32_t iii=0; iii<MAX_MANAGE_INPUT; iii++) {
				m_inputIsPressed[iii] = false;
			}
			if (m_doubleBuffered) {
				glXSwapBuffers(m_display, m_WindowHandle);
				XSync(m_display,0);
			}
			createX11Context();
			createOGlContext();
			// reset the Atom properties ...
			XAtomeSelection        = XInternAtom(m_display, "PRIMARY", 0);
			XAtomeClipBoard        = XInternAtom(m_display, "CLIPBOARD", 0);
			XAtomeTargetString     = XInternAtom(m_display, "STRING", 0);
			XAtomeTargetStringUTF8 = XInternAtom(m_display, "UTF8_STRING", 0);
			XAtomeTargetTarget     = XInternAtom(m_display, "TARGETS", 0);
			XAtomeEWOL             = XInternAtom(m_display, "EWOL", 0);
			XAtomeDeleteWindows    = XInternAtom(m_display, "WM_DELETE_WINDOW", 0);
			m_run = true;
		}
		
		~X11Interface() {
			// TODO : ...
		}
		
		int32_t run() {
			bool specialEventThatNeedARedraw = false;
			// main cycle
			while(true == m_run) {
				//EWOL_ERROR("plop1");
				XEvent event;
				XEvent respond;
				// main X boucle :
				while (XPending(m_display)) {
					XNextEvent(m_display, &event);
					switch (event.type) {
						case ClientMessage: {
							X11_INFO("Receive : ClientMessage");
							if(XAtomeDeleteWindows == (uint64_t)event.xclient.data.l[0]) {
								EWOL_INFO("     == > Kill Requested ...");
								OS_Stop();
								m_run = false;
							}
							break;
						}
						///////////////////////////////////////////////////////////////////////////////////////////////////////////////
						//                               Selection AREA                                                              //
						///////////////////////////////////////////////////////////////////////////////////////////////////////////////
						case SelectionClear: {
							// Selection has been done on an other program  == > clear ours ...
							X11_INFO("X11 event SelectionClear");
							#ifdef DEBUG_X11_EVENT
							{
								XSelectionRequestEvent *req=&(event.xselectionrequest);
								if (req->property == 0) {
									EWOL_ERROR("Get nullptr ATOM ... property");
									break;
								}
								if (req->target == 0) {
									EWOL_ERROR("Get nullptr ATOM ... target");
									break;
								}
								char * atomNameProperty = XGetAtomName(m_display, req->property);
								char * atomNameTarget = XGetAtomName(m_display, req->target);
								EWOL_INFO("X11    property: \"" << atomNameProperty << "\"");
								EWOL_INFO("X11    target:   \"" << atomNameTarget << "\"");
								if (atomNameProperty != nullptr) {
									XFree(atomNameProperty);
								}
								if (atomNameTarget != nullptr) {
									XFree(atomNameTarget);
								}
							}
							#endif
							if (true == m_clipBoardOwnerPrimary) {
								m_clipBoardOwnerPrimary = false;
							} else if (true == m_clipBoardOwnerStd) {
								m_clipBoardOwnerStd = false;
							} else {
								EWOL_ERROR("X11 event SelectionClear  == > but no selection requested anymore ...");
							}
							break;
						}
						case SelectionNotify:
							X11_INFO("X11 event SelectionNotify");
							if (event.xselection.property == None) {
								EWOL_VERBOSE("     == > no data ...");
							} else {
								unsigned char *buf = 0;
								Atom type;
								int format;
								unsigned long nitems, bytes;
								XGetWindowProperty(m_display,
								                   m_WindowHandle,
								                   event.xselection.property,
								                   0, // offset
								                   (~0L), // length
								                   False, // delete
								                   AnyPropertyType, // reg_type
								                   &type,// *actual_type_return,
								                   &format,// *actual_format_return
								                   &nitems,// *nitems_return
								                   &bytes, // *bytes_after_return
								                   &buf// **prop_return);
								                   );
								if (true == m_clipBoardRequestPrimary) {
									std::string tmpppp((char*)buf);
									ewol::context::clipBoard::setSystem(ewol::context::clipBoard::clipboardSelection, tmpppp);
									// just transmit an event , we have the data in the system
									OS_ClipBoardArrive(ewol::context::clipBoard::clipboardSelection);
								} else {
									std::string tmpppp((char*)buf);
									ewol::context::clipBoard::setSystem(ewol::context::clipBoard::clipboardStd, tmpppp);
									// just transmit an event , we have the data in the system
									OS_ClipBoardArrive(ewol::context::clipBoard::clipboardStd);
								}
							}
							break;
						case SelectionRequest: {
							X11_INFO("X11 event SelectionRequest");
							XSelectionRequestEvent *req=&(event.xselectionrequest);
							#ifdef DEBUG_X11_EVENT
							{
								if (req->property == 0) {
									EWOL_ERROR("Get nullptr ATOM ... property");
									break;
								}
								if (req->selection == 0) {
									EWOL_ERROR("Get nullptr ATOM ... selection");
									break;
								}
								if (req->target == 0) {
									EWOL_ERROR("Get nullptr ATOM ... target");
									break;
								}
								char * atomNameProperty = XGetAtomName(m_display, req->property);
								char * atomNameSelection = XGetAtomName(m_display, req->selection);
								char * atomNameTarget = XGetAtomName(m_display, req->target);
								EWOL_INFO("    from: " << atomNameProperty << "  request=" << atomNameSelection << " in " << atomNameTarget);
								if (nullptr != atomNameProperty) { XFree(atomNameProperty); }
								if (nullptr != atomNameSelection) { XFree(atomNameSelection); }
								if (nullptr != atomNameTarget) { XFree(atomNameTarget); }
							}
							#endif
							
							std::string tmpData = "";
							if (req->selection == XAtomeSelection) {
								tmpData = ewol::context::clipBoard::get(ewol::context::clipBoard::clipboardSelection);
							} else if (req->selection == XAtomeClipBoard) {
								tmpData = ewol::context::clipBoard::get(ewol::context::clipBoard::clipboardStd);
							}
							const char * magatTextToSend = tmpData.c_str();
							Atom listOfAtom[4];
							if(strlen(magatTextToSend) == 0 ) {
								respond.xselection.property= None;
							} else if(XAtomeTargetTarget == req->target) {
								// We need to generate the list of the possibles target element of atom
								int32_t nbAtomSupported = 0;
								listOfAtom[nbAtomSupported++] = XAtomeTargetTarget;
								listOfAtom[nbAtomSupported++] = XAtomeTargetString;
								listOfAtom[nbAtomSupported++] = XAtomeTargetStringUTF8;
								listOfAtom[nbAtomSupported++] = None;
								XChangeProperty( m_display,
								                 req->requestor,
								                 req->property,
								                 XA_ATOM,
								                 32,
								                 PropModeReplace,
								                 (unsigned char*)listOfAtom,
								                 nbAtomSupported );
								respond.xselection.property=req->property;
								EWOL_INFO("             == > Respond ... (test)");
							} else if(XAtomeTargetString == req->target) {
								XChangeProperty( m_display,
								                 req->requestor,
								                 req->property,
								                 req->target,
								                 8,
								                 PropModeReplace,
								                 (unsigned char*)magatTextToSend,
								                 strlen(magatTextToSend));
								respond.xselection.property=req->property;
								EWOL_INFO("             == > Respond ...");
							} else if (XAtomeTargetStringUTF8 == req->target) {
								XChangeProperty( m_display,
								                 req->requestor,
								                 req->property,
								                 req->target,
								                 8,
								                 PropModeReplace,
								                 (unsigned char*)magatTextToSend,
								                 strlen(magatTextToSend));
								respond.xselection.property=req->property;
								EWOL_INFO("             == > Respond ...");
							} else {
								respond.xselection.property= None;
							}
							respond.xselection.type= SelectionNotify;
							respond.xselection.display= req->display;
							respond.xselection.requestor= req->requestor;
							respond.xselection.selection=req->selection;
							respond.xselection.target= req->target;
							respond.xselection.time = req->time;
							XSendEvent (m_display, req->requestor,0,0,&respond);
							// flush the message on the pipe ...
							XFlush (m_display);
							break;
						}
						///////////////////////////////////////////////////////////////////////////////////////////////////////////////
						case Expose:
							X11_INFO("X11 event Expose");
							specialEventThatNeedARedraw=true;
							break;
						case GraphicsExpose:
							X11_INFO("X11 event GraphicsExpose");
							specialEventThatNeedARedraw=true;
							break;
						case NoExpose:
							X11_INFO("X11 event NoExpose");
							break;
						case CreateNotify:
							X11_INFO("X11 event CreateNotify");
							break;
						case DestroyNotify:
							X11_INFO("X11 event DestroyNotify");
							break;
						case GravityNotify:
							X11_INFO("X11 event GravityNotify");
							break;
						case VisibilityNotify:
							X11_INFO("X11 event VisibilityNotify");
							break;
						case CirculateNotify:
							X11_INFO("X11 event CirculateNotify");
							break;
						case ReparentNotify:
							X11_INFO("X11 event ReparentNotify");
							break;
						case PropertyNotify:
							specialEventThatNeedARedraw=true;
							X11_INFO("X11 event PropertyNotify");
							break;
						case ConfigureNotify:
							specialEventThatNeedARedraw=true;
							X11_INFO("X11 event ConfigureNotify");
							if (m_display == event.xconfigure.display) {
								//EWOL_INFO("X11 event ConfigureNotify event=" << (int32_t)event.xconfigure.event << "  Window=" << (int32_t)event.xconfigure.window << "  above=" << (int32_t)event.xconfigure.above << "  border_width=" << (int32_t)event.xconfigure.border_width );
								m_originX = event.xconfigure.x;
								m_originY = event.xconfigure.y;
								X11_INFO("X11 configure windows position : (" << m_originX << "," << m_originY << ")");
								m_currentHeight = event.xconfigure.height;
								m_currentWidth = event.xconfigure.width;
								X11_INFO("X11 configure windows size : (" << m_currentHeight << "," << m_currentWidth << ")");
								OS_Resize(vec2(event.xconfigure.width, event.xconfigure.height));
							}
							break;
						case ButtonPress:
							X11_INFO("X11 event ButtonPress");
							m_cursorEventX = event.xbutton.x;
							m_cursorEventY = (m_currentHeight-event.xbutton.y);
							if (event.xbutton.button < MAX_MANAGE_INPUT) {
								m_inputIsPressed[event.xbutton.button] = true;
							}
							OS_SetMouseState(event.xbutton.button, true, vec2(event.xbutton.x, m_cursorEventY));
							break;
						case ButtonRelease:
							X11_INFO("X11 event ButtonRelease");
							m_cursorEventX = event.xbutton.x;
							m_cursorEventY = (m_currentHeight-event.xbutton.y);
							if (event.xbutton.button < MAX_MANAGE_INPUT) {
								m_inputIsPressed[event.xbutton.button] = false;
							}
							OS_SetMouseState(event.xbutton.button, false, vec2(event.xbutton.x, m_cursorEventY));
							break;
						case EnterNotify:
							X11_INFO("X11 event EnterNotify");
							m_cursorEventX = event.xcrossing.x;
							m_cursorEventY = (m_currentHeight-event.xcrossing.y);
							//EWOL_DEBUG("X11 event : " << event.type << " = \"EnterNotify\" (" << (float)event.xcrossing.x << "," << (float)event.xcrossing.y << ")");
							//gui_uniqueWindows->GenEventInput(0, ewol::EVENT_INPUT_TYPE_ENTER, (float)event.xcrossing.x, (float)event.xcrossing.y);
							m_curentGrabDelta -= vec2(m_originX, -m_originY);
							EWOL_VERBOSE("change grab delta of : " << vec2(m_originX, m_originY) );
							break;
						case LeaveNotify:
							X11_INFO("X11 event LeaveNotify");
							m_cursorEventX = event.xcrossing.x;
							m_cursorEventY = (m_currentHeight-event.xcrossing.y);
							//EWOL_DEBUG("X11 event : " << event.type << " = \"LeaveNotify\" (" << (float)event.xcrossing.x << "," << (float)event.xcrossing.y << ")");
							m_curentGrabDelta += vec2(m_originX, -m_originY);
							EWOL_VERBOSE("change grab delta of : " << vec2(m_originX, m_originY) );
							break;
						case MotionNotify:
							X11_INFO("X11 event MotionNotify");
							if(    true == m_grabAllEvent
							    && event.xmotion.x == (int32_t)m_forcePos.x()
							    && event.xmotion.y == (int32_t)m_forcePos.y()) {
								X11_VERBOSE("X11 reject mouse move (grab mode)");
								// we get our requested position...
								m_positionChangeRequested = false;
								m_curentGrabDelta = vec2(0,0);
							} else {
								m_cursorEventX = event.xmotion.x;
								m_cursorEventY = (m_currentHeight-event.xmotion.y);
								if(true == m_grabAllEvent) {
									m_cursorEventX -= m_forcePos.x();
									m_cursorEventY -= (m_currentHeight-m_forcePos.y());
								}
								vec2 newDelta = vec2(m_cursorEventX, m_cursorEventY);
								if(true == m_grabAllEvent) {
									m_cursorEventX -= m_curentGrabDelta.x();
									m_cursorEventY -= m_curentGrabDelta.y();
								}
								m_curentGrabDelta = newDelta;
								// For compatibility of the Android system : 
								bool findOne = false;
								for (int32_t iii=0; iii<MAX_MANAGE_INPUT ; iii++) {
									if (true == m_inputIsPressed[iii]) {
										X11_DEBUG("X11 event: bt=" << iii << " " << event.type << " = \"MotionNotify\" (" << m_cursorEventX << "," << m_cursorEventY << ")");
										OS_SetMouseMotion(iii, vec2(m_cursorEventX, m_cursorEventY));
										findOne = true;
									}
								}
								if (false == findOne) {
									X11_DEBUG("X11 event: bt=" << 0 << " " << event.type << " = \"MotionNotify\" (" << m_cursorEventX << "," << m_cursorEventY << ")");
									OS_SetMouseMotion(0, vec2(m_cursorEventX, m_cursorEventY));
								}
								if (true == m_grabAllEvent) {
									if (m_positionChangeRequested == false) {
										X11_DEBUG("X11 set pointer position : " << m_forcePos);
										XWarpPointer(m_display, None, m_WindowHandle, 0,0, 0, 0, m_forcePos.x(), m_forcePos.y());
										XFlush(m_display);
										m_positionChangeRequested = true;
									}
								}
							}
							break;
						case FocusIn:
							X11_INFO("X11 event focusIn");
							specialEventThatNeedARedraw=true;
							break;
						case FocusOut:
							X11_INFO("X11 event : focusOut");
							specialEventThatNeedARedraw=true;
							break;
						case KeyPress:
						case KeyRelease: {
							bool thisIsAReapeateKey = false;
							// filter repeate key : 
							if(    event.type == KeyRelease
							    && XEventsQueued(m_display, QueuedAfterReading)) {
								XEvent nev;
								XPeekEvent(m_display, &nev);
								if(    nev.type == KeyPress
								    && nev.xkey.time == event.xkey.time
								    && nev.xkey.keycode == event.xkey.keycode) {
									// Key was not actually released
									// remove next event too ...
									
									//This remove repeate key
									XNextEvent(m_display, &nev);
									//break;
									
									thisIsAReapeateKey = true;
								}
							}
							X11_INFO("X11 event : " << event.type << " = \"KeyPress/KeyRelease\" ");
							{
								X11_DEBUG("eventKey : " << event.xkey.keycode << " state : " << event.xkey.state);
								if (event.xkey.state & (1<<0) ) {
									//EWOL_DEBUG("    Special Key : SHIFT");
									m_guiKeyBoardMode.setShift(true);
								} else {
									m_guiKeyBoardMode.setShift(false);
								}
								if (event.xkey.state & (1<<1) ) {
									//EWOL_DEBUG("    Special Key : CAPS_LOCK");
									m_guiKeyBoardMode.setCapsLock(true);
								} else {
									m_guiKeyBoardMode.setCapsLock(false);
								}
								if (event.xkey.state & (1<<2) ) {
									//EWOL_DEBUG("    Special Key : Ctrl");
									m_guiKeyBoardMode.setCtrl(true);
								} else {
									m_guiKeyBoardMode.setCtrl(false);
								}
								if (event.xkey.state & (1<<3) ) {
									//EWOL_DEBUG("    Special Key : Alt");
									m_guiKeyBoardMode.setAlt(true);
								} else {
									m_guiKeyBoardMode.setAlt(false);
								}
								if (event.xkey.state & (1<<4) ) {
									//EWOL_DEBUG("    Special Key : VER_num");
									m_guiKeyBoardMode.setNumLock(true);
								} else {
									m_guiKeyBoardMode.setNumLock(false);
								}
								if (event.xkey.state & (1<<5) ) {
									EWOL_DEBUG("    Special Key : MOD");
								}
								if (event.xkey.state & (1<<6) ) {
									//EWOL_DEBUG("    Special Key : META");
									m_guiKeyBoardMode.setMeta(true);
								} else {
									m_guiKeyBoardMode.setMeta(false);
								}
								if (event.xkey.state & (1<<7) ) {
									//EWOL_DEBUG("    Special Key : ALT_GR");
									m_guiKeyBoardMode.setAltGr(true);
								} else {
									m_guiKeyBoardMode.setAltGr(false);
								}
								bool find = true;
								enum ewol::key::keyboard keyInput;
								switch (event.xkey.keycode) {
									//case 80: // keypad
									case 111:	keyInput = ewol::key::keyboardUp;            break;
									//case 83: // keypad
									case 113:	keyInput = ewol::key::keyboardLeft;          break;
									//case 85: // keypad
									case 114:	keyInput = ewol::key::keyboardRight;         break;
									//case 88: // keypad
									case 116:	keyInput = ewol::key::keyboardDown;          break;
									//case 81: // keypad
									case 112:	keyInput = ewol::key::keyboardPageUp;        break;
									//case 89: // keypad
									case 117:	keyInput = ewol::key::keyboardPageDown;      break;
									//case 79: // keypad
									case 110:	keyInput = ewol::key::keyboardStart;         break;
									//case 87: // keypad
									case 115:	keyInput = ewol::key::keyboardEnd;           break;
									case 78:	keyInput = ewol::key::keyboardStopDefil;     break;
									case 127:	keyInput = ewol::key::keyboardWait;          break;
									//case 90: // keypad
									case 118:
										keyInput = ewol::key::keyboardInsert;
										if(event.type == KeyRelease) {
											if (true == m_guiKeyBoardMode.getInsert()) {
												m_guiKeyBoardMode.setInsert(false);
											} else {
												m_guiKeyBoardMode.setInsert(true);
											}
										}
										break;
									//case 84:  keyInput = ewol::key::keyboardCenter; break; // Keypad
									case 67:    keyInput = ewol::key::keyboardF1; break;
									case 68:    keyInput = ewol::key::keyboardF2; break;
									case 69:    keyInput = ewol::key::keyboardF3; break;
									case 70:    keyInput = ewol::key::keyboardF4; break;
									case 71:    keyInput = ewol::key::keyboardF5; break;
									case 72:    keyInput = ewol::key::keyboardF6; break;
									case 73:    keyInput = ewol::key::keyboardF7; break;
									case 74:    keyInput = ewol::key::keyboardF8; break;
									case 75:    keyInput = ewol::key::keyboardF9; break;
									case 76:    keyInput = ewol::key::keyboardF10; break;
									case 95:    keyInput = ewol::key::keyboardF11; break;
									case 96:    keyInput = ewol::key::keyboardF12; break;
									case 66:    keyInput = ewol::key::keyboardCapLock;     m_guiKeyBoardMode.setCapsLock( (event.type == KeyPress) ? true : false); break;
									case 50:    keyInput = ewol::key::keyboardShiftLeft;   m_guiKeyBoardMode.setShift   ( (event.type == KeyPress) ? true : false); break;
									case 62:    keyInput = ewol::key::keyboardShiftRight;  m_guiKeyBoardMode.setShift   ( (event.type == KeyPress) ? true : false); break;
									case 37:    keyInput = ewol::key::keyboardCtrlLeft;    m_guiKeyBoardMode.setCtrl    ( (event.type == KeyPress) ? true : false); break;
									case 105:   keyInput = ewol::key::keyboardCtrlRight;   m_guiKeyBoardMode.setCtrl    ( (event.type == KeyPress) ? true : false); break;
									case 133:   keyInput = ewol::key::keyboardMetaLeft;    m_guiKeyBoardMode.setMeta    ( (event.type == KeyPress) ? true : false); break;
									case 134:   keyInput = ewol::key::keyboardMetaRight;   m_guiKeyBoardMode.setMeta    ( (event.type == KeyPress) ? true : false); break;
									case 64:    keyInput = ewol::key::keyboardAlt;         m_guiKeyBoardMode.setAlt     ( (event.type == KeyPress) ? true : false); break;
									case 108:   keyInput = ewol::key::keyboardAltGr;       m_guiKeyBoardMode.setAltGr   ( (event.type == KeyPress) ? true : false); break;
									case 135:   keyInput = ewol::key::keyboardContextMenu; break;
									case 77:    keyInput = ewol::key::keyboardNumLock;     m_guiKeyBoardMode.setNumLock ( (event.type == KeyPress) ? true : false); break;
									case 91: // Suppr on keypad
										find = false;
										if(m_guiKeyBoardMode.getNumLock() == true){
											OS_SetKeyboard(m_guiKeyBoardMode, '.', (event.type == KeyPress), thisIsAReapeateKey);
											if (true == thisIsAReapeateKey) {
												OS_SetKeyboard(m_guiKeyBoardMode, '.', !(event.type == KeyPress), thisIsAReapeateKey);
											}
										} else {
											OS_SetKeyboard(m_guiKeyBoardMode, 0x7F, (event.type == KeyPress), thisIsAReapeateKey);
											if (true == thisIsAReapeateKey) {
												OS_SetKeyboard(m_guiKeyBoardMode, 0x7F, !(event.type == KeyPress), thisIsAReapeateKey);
											}
										}
										break;
									case 23: // special case for TAB
										find = false;
										OS_SetKeyboard(m_guiKeyBoardMode, 0x09, (event.type == KeyPress), thisIsAReapeateKey);
										if (true == thisIsAReapeateKey) {
											OS_SetKeyboard(m_guiKeyBoardMode, 0x09, !(event.type == KeyPress), thisIsAReapeateKey);
										}
										break;
									default:
										find = false;
										{
											char buf[11];
											//EWOL_DEBUG("Keycode: " << event.xkey.keycode);
											// change keystate for simple reson of the ctrl error...
											int32_t keyStateSave = event.xkey.state;
											if (event.xkey.state & (1<<2) ) {
												event.xkey.state = event.xkey.state & 0xFFFFFFFB;
											}
											KeySym keysym;
											Status status = 0;
											//int count = Xutf8LookupString(m_xic, (XKeyPressedEvent*)&event, buf, 10, &keysym, &status);
											int count = Xutf8LookupString(m_xic, &event.xkey, buf, 10, &keysym, &status);
											// retreave real keystate
											event.xkey.state = keyStateSave;
											buf[count] = '\0';
											// Replace \r error ...
											if (buf[0] == '\r') {
												buf[0] = '\n';
												buf[1] = '\0';
											}
											if (count >= 0) {
												// repeated kay from previous element :
												if (count>0) {
													// transform it in unicode
													m_lastKeyPressed = utf8::convertChar32(buf);
												}
												X11_INFO("event Key : " << event.xkey.keycode << " char=\"" << buf << "\"'len=" << strlen(buf) << " unicode=" << m_lastKeyPressed);
												OS_SetKeyboard(m_guiKeyBoardMode, m_lastKeyPressed, (event.type == KeyPress), thisIsAReapeateKey);
												if (true == thisIsAReapeateKey) {
													OS_SetKeyboard(m_guiKeyBoardMode, m_lastKeyPressed, !(event.type == KeyPress), thisIsAReapeateKey);
												}
											} else {
												EWOL_WARNING("Unknow event Key : " << event.xkey.keycode << " res='" << buf << "' repeate=" << thisIsAReapeateKey);
											}
										}
										break;
								}
								if (true == find) {
									//EWOL_DEBUG("eventKey Move type : " << getCharTypeMoveEvent(keyInput) );
									OS_SetKeyboardMove(m_guiKeyBoardMode, keyInput, (event.type == KeyPress), thisIsAReapeateKey);
									if (true == thisIsAReapeateKey) {
										OS_SetKeyboardMove(m_guiKeyBoardMode, keyInput, !(event.type == KeyPress), thisIsAReapeateKey);
									}
								}
							}
							break;
						}
						//case DestroyNotify:
						//	break;
						case MapNotify:
							X11_INFO("X11 event : MapNotify");
							specialEventThatNeedARedraw=true;
							OS_Show();
							break;
						case UnmapNotify:
							X11_INFO("X11 event : UnmapNotify");
							specialEventThatNeedARedraw=true;
							OS_Hide();
							break;
						default:
							X11_INFO("X11 event : " << event.type << " = \"???\"");
							break;
					}
				}
				if(true == m_run) {
					if (m_doubleBuffered && hasDisplay) {
						glXSwapBuffers(m_display, m_WindowHandle);
						XSync(m_display,0);
					}
					// draw after switch the previous windows ...
					//EWOL_DEBUG("specialEventThatNeedARedraw"<<specialEventThatNeedARedraw);
					hasDisplay = OS_Draw(specialEventThatNeedARedraw);
					specialEventThatNeedARedraw=false;
				}
			}
			return 0;
		}
		/****************************************************************************************/
		virtual void stop() {
			X11_INFO("X11-API: Stop");
			m_run = false;
		}
		/****************************************************************************************/
		virtual void setSize(const vec2& _size) {
			X11_INFO("X11-API: changeSize=" << _size);
			m_currentHeight = _size.y();
			m_currentWidth = _size.x();
			XResizeWindow(m_display, m_WindowHandle, _size.x(), _size.y());
		}
		/****************************************************************************************/
		virtual void setPos(const vec2& _pos) {
			X11_INFO("X11-API: changePos=" << _pos);
			XMoveWindow(m_display, m_WindowHandle, _pos.x(), _pos.y());
			m_originX = _pos.x();
			m_originY = _pos.y();
		}
		/****************************************************************************************/
		/*
		virtual void getAbsPos(ivec2& pos) {
			X11_INFO("X11-API: getAbsPos");
			int tmp;
			unsigned int tmp2;
			Window fromroot, tmpwin;
			XQueryPointer(m_display, m_WindowHandle, &fromroot, &tmpwin, &pos.m_floats[0], &pos.m_floats[1], &tmp, &tmp, &tmp2);
		}
		*/
		/****************************************************************************************/
		virtual void setCursor(enum ewol::context::cursorDisplay _newCursor) {
			if (_newCursor != m_currentCursor) {
				X11_DEBUG("X11-API: set New Cursor : " << _newCursor);
				// undefine previous cursors ...
				XUndefineCursor(m_display, m_WindowHandle);
				// set the new one :
				m_currentCursor = _newCursor;
				Cursor myCursor = None;
				switch (m_currentCursor) {
					case ewol::context::cursorNone:
						{
							Pixmap bitmapNoData;
							XColor black;
							static char noData[] = { 0,0,0,0,0,0,0,0 };
							black.red = 0;
							black.green = 0;
							black.blue = 0;
							bitmapNoData = XCreateBitmapFromData(m_display, m_WindowHandle, noData, 8, 8);
							myCursor = XCreatePixmapCursor(m_display, bitmapNoData, bitmapNoData, 
							                               &black, &black, 0, 0);
						}
						break;
					case ewol::context::cursorLeftArrow:
						myCursor = XCreateFontCursor(m_display, XC_top_left_arrow);
						break;
					case ewol::context::cursorInfo:
						myCursor = XCreateFontCursor(m_display, XC_hand1);
						break;
					case ewol::context::cursorDestroy:
						myCursor = XCreateFontCursor(m_display, XC_pirate);
						break;
					case ewol::context::cursorHelp:
						myCursor = XCreateFontCursor(m_display, XC_question_arrow);
						break;
					case ewol::context::cursorCycle:
						myCursor = XCreateFontCursor(m_display, XC_exchange);
						break;
					case ewol::context::cursorSpray:
						myCursor = XCreateFontCursor(m_display, XC_spraycan);
						break;
					case ewol::context::cursorWait:
						myCursor = XCreateFontCursor(m_display, XC_watch);
						break;
					case ewol::context::cursorText:
						myCursor = XCreateFontCursor(m_display, XC_xterm);
						break;
					case ewol::context::cursorCrossHair:
						myCursor = XCreateFontCursor(m_display, XC_crosshair);
						break;
					case ewol::context::cursorSlideUpDown:
						myCursor = XCreateFontCursor(m_display, XC_sb_v_double_arrow);
						break;
					case ewol::context::cursorSlideLeftRight:
						myCursor = XCreateFontCursor(m_display, XC_sb_h_double_arrow);
						break;
					case ewol::context::cursorResizeUp:
						myCursor = XCreateFontCursor(m_display, XC_top_side);
						break;
					case ewol::context::cursorResizeDown:
						myCursor = XCreateFontCursor(m_display, XC_bottom_side);
						break;
					case ewol::context::cursorResizeLeft:
						myCursor = XCreateFontCursor(m_display, XC_left_side);
						break;
					case ewol::context::cursorResizeRight:
						myCursor = XCreateFontCursor(m_display, XC_right_side);
						break;
					case ewol::context::cursorCornerTopLeft:
						myCursor = XCreateFontCursor(m_display, XC_top_left_corner);
						break;
					case ewol::context::cursorCornerTopRight:
						myCursor = XCreateFontCursor(m_display, XC_top_right_corner);
						break;
					case ewol::context::cursorCornerButtomLeft:
						myCursor = XCreateFontCursor(m_display, XC_bottom_right_corner);
						break;
					case ewol::context::cursorCornerButtomRight:
						myCursor = XCreateFontCursor(m_display, XC_bottom_left_corner);
						break;
					default :
						// nothing to do ... basic pointer ...
						break;
				}
				if (myCursor != None) {
					XDefineCursor(m_display, m_WindowHandle, myCursor);
					XFreeCursor(m_display, myCursor);
				}
			}
		}
		/****************************************************************************************/
		void grabPointerEvents(bool _isGrabbed, const vec2& _forcedPosition) {
			if (true == _isGrabbed) {
				X11_DEBUG("X11-API: Grab Events");
				int32_t test = XGrabPointer(m_display,RootWindow(m_display, DefaultScreen(m_display)), True,
				                            ButtonPressMask |
				                            ButtonReleaseMask |
				                            PointerMotionMask |
				                            FocusChangeMask |
				                            EnterWindowMask |
				                            LeaveWindowMask,
				                            GrabModeAsync,
				                            GrabModeAsync,
				                            RootWindow(m_display, DefaultScreen(m_display)),
				                            None,
				                            CurrentTime);
				
				if (GrabSuccess != test)
				{
					EWOL_CRITICAL("Display error " << test);
					switch (test)
					{
						case BadCursor:
							EWOL_CRITICAL(" BadCursor");
							break;
						case BadValue:
							EWOL_CRITICAL(" BadValue");
							break;
						case BadWindow:
							EWOL_CRITICAL(" BadWindow");
							break;
					}
				}
				m_forcePos = _forcedPosition;
				m_forcePos.setY(m_currentHeight - m_forcePos.y());
				m_grabAllEvent = true;
				// change the pointer position to generate a good mouving at the start ...
				X11_DEBUG("X11-API: set pointer position : " << m_forcePos);
				XWarpPointer(m_display, None, m_WindowHandle, 0,0, 0, 0, m_forcePos.x(), m_forcePos.y());
				XFlush(m_display);
				m_positionChangeRequested = true;
				m_curentGrabDelta = vec2(0,0);
			} else {
				X11_DEBUG("X11-API: Un-Grab Events");
				XUngrabPointer(m_display, CurrentTime);
				m_grabAllEvent = false;
				m_forcePos = vec2(0,0);
				m_curentGrabDelta = vec2(0,0);
			}
		}
		/****************************************************************************************/
		bool createX11Context() {
			X11_INFO("X11: CreateX11Context");
			int x,y, attr_mask;
			XSizeHints hints;
			XWMHints *StartupState;
			XTextProperty textprop;
			XSetWindowAttributes attr;
			// basic title of the windows ...
			static char *title = (char*)"Ewol";
			
			// Connect to the X server
			m_display = XOpenDisplay(nullptr);
			if(nullptr == m_display) {
				EWOL_CRITICAL("Could not open display X.");
				exit(-1);
			} else {
				EWOL_INFO("Display opened.");
			}
			int Xscreen = DefaultScreen(m_display);
			// set the DPI for the current screen :
			ewol::Dimension::setPixelRatio(vec2((float)DisplayWidth(m_display, Xscreen)/(float)DisplayWidthMM(m_display, Xscreen),
			                                    (float)DisplayHeight(m_display, Xscreen)/(float)DisplayHeightMM(m_display, Xscreen)),
			                               ewol::Dimension::Millimeter);
			// get an appropriate visual
			m_visual = glXChooseVisual(m_display, Xscreen, attrListDbl);
			if (nullptr == m_visual) {
				m_visual = glXChooseVisual(m_display, Xscreen, attrListSgl);
				m_doubleBuffered = false;
				EWOL_INFO("GL-X singlebuffered rendering will be used, no doublebuffering available");
			} else {
				m_doubleBuffered = true;
				EWOL_INFO("GL-X doublebuffered rendering available");
			}
			{
				int32_t glxMajor, glxMinor;
				glXQueryVersion(m_display, &glxMajor, &glxMinor);
				EWOL_INFO("GLX-Version " << glxMajor << "." << glxMinor);
			}
			// Create a colormap - only needed on some X clients, eg. IRIX
			Window Xroot = RootWindow(m_display, Xscreen);
			attr.colormap = XCreateColormap(m_display, Xroot, m_visual->visual, AllocNone);
			
			attr.border_pixel = 0;
			attr.event_mask =   StructureNotifyMask
			                  | SubstructureNotifyMask
			                  | EnterWindowMask
			                  | LeaveWindowMask
			                  | ExposureMask
			                  | ButtonPressMask
			                  | ButtonReleaseMask
			                  | OwnerGrabButtonMask
			                  | KeyPressMask
			                  | KeyReleaseMask
			                  | PointerMotionMask
			                  | FocusChangeMask
			                  | SubstructureRedirectMask;
			
			// set no background at the gui
			attr.background_pixmap = None;
			
			// select internal attribute
			attr_mask = CWBackPixmap | CWColormap | CWBorderPixel | CWEventMask;
			// Create the window
			int32_t tmp_width = 640;//DisplayWidth(m_display, DefaultScreen(m_display))/2;
			int32_t tmp_height = 480;//DisplayHeight(m_display, DefaultScreen(m_display))/2;
			OS_Resize(vec2(tmp_width, tmp_height));
			x=20;
			y=20;
			m_originX = x;
			m_originY = y;
			EWOL_INFO("X11 request creating windows at pos=(" << m_originX << "," << m_originY << ") size=(" << tmp_width << "," << tmp_height << ")" );
			// Real create of the window
			m_WindowHandle = XCreateWindow(m_display,
			                             Xroot,
			                             x, y, tmp_width, tmp_height,
			                             1,
			                             m_visual->depth,
			                             InputOutput,
			                             m_visual->visual,
			                             attr_mask, &attr);
			
			if( !m_WindowHandle ) {
				EWOL_CRITICAL("Couldn't create the window");
				exit(-1);
			}
			
			/* Configure it...  (ok, ok, this next bit isn't "minimal") */
			textprop.value = (unsigned char*)title;
			textprop.encoding = XA_STRING;
			textprop.format = 8;
			textprop.nitems = strlen(title);
			
			hints.x = x;
			hints.y = y;
			hints.width = tmp_width;
			hints.height = tmp_height;
			hints.flags = USPosition|USSize;
			
			StartupState = XAllocWMHints();
			StartupState->initial_state = NormalState;
			StartupState->flags = StateHint;
			
			XSetWMProperties(m_display, m_WindowHandle,&textprop, &textprop,/* Window title/icon title*/
				nullptr, 0,/* Argv[], argc for program*/
				&hints, /* Start position/size*/
				StartupState,/* Iconised/not flag   */
				nullptr);
			
			XFree(StartupState);
			
			/* open it, wait for it to appear */
			XMapWindow(m_display, m_WindowHandle);
			//XIfEvent(m_display, &event, WaitForMapNotify, (char*)&m_WindowHandle);
			
			
			m_xim = XOpenIM(m_display, nullptr, NULL, NULL);
			if (m_xim == nullptr) {
				EWOL_ERROR("Could not open input method");
				return false;
			}
			/*
			XIMStyles *styles=nullptr;
			char* failed_arg = XGetIMValues(m_xim, XNQueryInputStyle, &styles, nullptr);
			if (failed_arg != nullptr) {
				EWOL_ERROR("XIM Can't get styles");
				return false;
			}
			for (int32_t iii=0; iii<styles->count_styles; ++iii) {
				EWOL_INFO("style " << styles->supported_styles[iii]);
			}
			*/
			m_xic = XCreateIC(m_xim, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, m_WindowHandle, nullptr);
			if (m_xic == nullptr) {
				EWOL_ERROR("Could not open IC");
				return false;
			}
			
			XSetICFocus(m_xic);
			
			// set the kill atom so we get a message when the user tries to close the window
			if ((m_delAtom = XInternAtom(m_display, "WM_DELETE_WINDOW", 0)) != None) {
				XSetWMProtocols(m_display, m_WindowHandle, &m_delAtom, 1);
			}
			
			vec2 tmpSize(400, 300);
			OS_Resize(tmpSize);
			
			return true;
		}
		/****************************************************************************************/
		void setIcon(const std::string& _inputFile) {
			egami::Image dataImage;
			// load data
			if (false == egami::load(dataImage, _inputFile)) {
				EWOL_ERROR("Error when loading Icon");
				return;
			}
			int32_t depth = DefaultDepth(m_display, DefaultScreen(m_display) );
			EWOL_DEBUG("X11 Create icon size=(" << dataImage.getWidth() << "," << dataImage.getHeight() << ") depth=" << depth);
			switch(depth) {
				case 8:
					EWOL_CRITICAL("Not manage pixmap in 8 bit...  == > no icon ...");
					return;
				case 16:
					break;
				case 24:
					break;
				case 32:
					break;
				default:
					EWOL_CRITICAL("Unknow thys type of bitDepth : " << depth);
					return;
			}
			char* tmpVal = new char[4*dataImage.getWidth()*dataImage.getHeight()];
			if (nullptr == tmpVal) {
				EWOL_CRITICAL("Allocation error ...");
				return;
			}
			char* tmpPointer = tmpVal;
			switch(depth) {
				case 16:
					for(ivec2 pos(0,0); pos.y()<dataImage.getHeight(); pos.setY(pos.y()+1)) {
						for(pos.setX(0); pos.x()<dataImage.getHeight();  pos.setX(pos.x()+1)) {
							etk::Color<> tmpColor = dataImage.get(pos);
							int16_t tmpVal =   (((uint16_t)((uint16_t)tmpColor.r()>>3))<<11)
							                 + (((uint16_t)((uint16_t)tmpColor.g()>>2))<<5)
							                 +  ((uint16_t)((uint16_t)tmpColor.b()>>3));
							*tmpPointer++ = (uint8_t)(tmpVal>>8);
							*tmpPointer++ = (uint8_t)(tmpVal&0x00FF);
						}
					}
					break;
				case 24:
					for(ivec2 pos(0,0); pos.y()<dataImage.getHeight(); pos.setY(pos.y()+1)) {
						for(pos.setX(0); pos.x()<dataImage.getHeight();  pos.setX(pos.x()+1)) {
							etk::Color<> tmpColor = dataImage.get(pos);
							*tmpPointer++ = tmpColor.b();
							*tmpPointer++ = tmpColor.g();
							*tmpPointer++ = tmpColor.r();
							tmpPointer++;
						}
					}
					break;
				case 32:
					for(ivec2 pos(0,0); pos.y()<dataImage.getHeight(); pos.setY(pos.y()+1)) {
						for(pos.setX(0); pos.x()<dataImage.getHeight();  pos.setX(pos.x()+1)) {
							etk::Color<> tmpColor = dataImage.get(pos);
							*tmpPointer++ = tmpColor.a();
							*tmpPointer++ = tmpColor.b();
							*tmpPointer++ = tmpColor.g();
							*tmpPointer++ = tmpColor.r();
						}
					}
					break;
				default:
					return;
			}
			
			XImage* myImage = XCreateImage(m_display,
			                              m_visual->visual,
			                              depth,
			                              ZPixmap,
			                              0,
			                              (char*)tmpVal,
			                              dataImage.getWidth(),
			                              dataImage.getHeight(),
			                              32,
			                              0);
			
			Pixmap tmpPixmap = XCreatePixmap(m_display, m_WindowHandle, dataImage.getWidth(), dataImage.getHeight(), depth);
			switch(tmpPixmap) {
				case BadAlloc:
					EWOL_ERROR("X11: BadAlloc");
					break;
				case BadDrawable:
					EWOL_ERROR("X11: BadDrawable");
					break;
				case BadValue:
					EWOL_ERROR("X11: BadValue");
					break;
				default:
					EWOL_DEBUG("Create Pixmap OK");
					break;
			}
			GC tmpGC = DefaultGC(m_display, DefaultScreen(m_display) );
			int error = XPutImage(m_display, tmpPixmap, tmpGC, myImage, 0, 0, 0, 0, dataImage.getWidth(), dataImage.getHeight());
			switch(error) {
				case BadDrawable:
					EWOL_ERROR("X11: BadDrawable");
					break;
				case BadGC:
					EWOL_ERROR("X11: BadGC");
					break;
				case BadMatch:
					EWOL_ERROR("X11: BadMatch");
					break;
				case BadValue:
					EWOL_ERROR("X11: BadValue");
					break;
				default:
					EWOL_DEBUG("insert image OK");
					break;
			}
			// allocate a WM hints structure.
			XWMHints* win_hints = XAllocWMHints();
			if (!win_hints) {
				EWOL_ERROR("XAllocWMHints - out of memory");
				return;
			}
			// initialize the structure appropriately. first, specify which size hints we want to fill in. in our case - setting the icon's pixmap.
			win_hints->flags = IconPixmapHint;
			// next, specify the desired hints data. in our case - supply the icon's desired pixmap.
			win_hints->icon_pixmap = tmpPixmap;
			// pass the hints to the window manager.
			XSetWMHints(m_display, m_WindowHandle, win_hints);
			EWOL_INFO("     == > might be done ");
			// finally, we can free the WM hints structure.
			XFree(win_hints);
			
			// Note when we free the pixmap ... the icon is removed ...  == > this is a real memory leek ...
			//XFreePixmap(m_display, tmpPixmap);
			
			myImage->data = nullptr;
			XDestroyImage(myImage);
			delete[] tmpVal;
			
		}
		/****************************************************************************************/
		static void setVSync(bool _sync) {
			// Function pointer for the wgl extention function we need to enable/disable
			typedef int32_t (APIENTRY *PFNWGLSWAPINTERVALPROC)( int );
			PFNWGLSWAPINTERVALPROC wglSwapIntervalEXT = 0;
			const char *extensions = (char*)glGetString( GL_EXTENSIONS );
			if( strstr( extensions, "WGL_EXT_swap_control" ) == 0 ) {
				EWOL_ERROR("Can not set the vertical synchronisation status" << _sync << "   (1)");
				return;
			} else {
				wglSwapIntervalEXT = (PFNWGLSWAPINTERVALPROC)glXGetProcAddress( (const GLubyte *)"wglSwapIntervalEXT" );
				if(wglSwapIntervalEXT) {
					wglSwapIntervalEXT(_sync);
				} else {
					EWOL_ERROR("Can not set the vertical synchronisation status" << _sync << "   (2)");
				}
			}
		}
		/****************************************************************************************/
		bool createOGlContext() {
			X11_INFO("X11:CreateOGlContext");
			/* create a GLX context */
			GLXContext RenderContext = glXCreateContext(m_display, m_visual, 0, GL_TRUE);
			/* connect the glx-context to the window */
			glXMakeCurrent(m_display, m_WindowHandle, RenderContext);
			if (glXIsDirect(m_display, RenderContext)) {
				EWOL_INFO("XF86 DRI enabled\n");
			} else {
				EWOL_INFO("XF86 DRI NOT available\n");
			}
			
			// enable vertical synchronisation : (some computer has synchronisation disable)
			setVSync(true);
			
			return true;
		}
		/****************************************************************************************/
		void setTitle(const std::string& _title) {
			X11_INFO("X11: set Title (START)");
			XTextProperty tp;
			tp.value = (unsigned char *)_title.c_str();
			tp.encoding = XA_WM_NAME;
			tp.format = 8;
			tp.nitems = strlen((const char*)tp.value);
			XSetWMName(m_display, m_WindowHandle, &tp);
			XStoreName(m_display, m_WindowHandle, (const char*)tp.value);
			XSetIconName(m_display, m_WindowHandle, (const char*)tp.value);
			XSetWMIconName(m_display, m_WindowHandle, &tp);
			X11_INFO("X11: set Title (END)");
		}
		
		void openURL(const std::string& _url) {
			std::string req = "xdg-open ";
			req += _url;
			system(req.c_str());
			return;
		}
		/****************************************************************************************/
		void clipBoardGet(enum ewol::context::clipBoard::clipboardListe _clipboardID) {
			switch (_clipboardID) {
				case ewol::context::clipBoard::clipboardSelection:
					if (false == m_clipBoardOwnerPrimary) {
						m_clipBoardRequestPrimary = true;
						// generate a request on X11
						XConvertSelection(m_display,
						                  XAtomeSelection,
						                  XAtomeTargetStringUTF8,
						                  XAtomeEWOL,
						                  m_WindowHandle,
						                  CurrentTime);
					} else {
						// just transmit an event , we have the data in the system
						OS_ClipBoardArrive(_clipboardID);
					}
					break;
				case ewol::context::clipBoard::clipboardStd:
					if (false == m_clipBoardOwnerStd) {
						m_clipBoardRequestPrimary = false;
						// generate a request on X11
						XConvertSelection(m_display,
						                  XAtomeClipBoard,
						                  XAtomeTargetStringUTF8,
						                  XAtomeEWOL,
						                  m_WindowHandle,
						                  CurrentTime);
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
		/****************************************************************************************/
		void clipBoardSet(enum ewol::context::clipBoard::clipboardListe _clipboardID) {
			switch (_clipboardID)
			{
				case ewol::context::clipBoard::clipboardSelection:
					// Request the selection :
					if (false == m_clipBoardOwnerPrimary) {
						XSetSelectionOwner(m_display, XAtomeSelection, m_WindowHandle, CurrentTime);
						m_clipBoardOwnerPrimary = true;
					}
					break;
				case ewol::context::clipBoard::clipboardStd:
					// Request the clipBoard :
					if (false == m_clipBoardOwnerStd) {
						XSetSelectionOwner(m_display, XAtomeClipBoard, m_WindowHandle, CurrentTime);
						m_clipBoardOwnerStd = true;
					}
					break;
				default:
					EWOL_ERROR("Request an unknow ClipBoard ...");
					break;
			}
		}
};


/**
 * @brief Main of the program
 * @param std IO
 * @return std IO
 */
int ewol::run(ewol::context::Application* _application, int _argc, const char *_argv[]) {
	etk::setArgZero(_argv[0]);
	X11Interface* interface = new X11Interface(_application, _argc, _argv);
	if (interface == nullptr) {
		EWOL_CRITICAL("Can not create the X11 interface ... MEMORY allocation error");
		return -2;
	}
	int32_t retValue = interface->run();
	delete(interface);
	interface = nullptr;
	return retValue;
}
