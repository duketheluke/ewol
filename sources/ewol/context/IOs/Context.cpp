/**
 * @author Edouard DUPIN
 *
 * @copyright 2011, Edouard DUPIN, all right reserved
 *
 * @license APACHE v2.0 (see license file)
 */


#include <ewol/debug.h>
#include <ewol/ewol.h>
#include <ewol/key/key.h>
#include <ewol/context/commandLine.h>
#include <etk/types.h>
#include <etk/os/FSNode.h>
#include <ewol/widget/Manager.h>

#include <ewol/resource/Manager.h>
#include <ewol/context/Context.h>

#include <ewol/context/IOs/Interface.h>
#include <ewol/context/IOs/Context.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include <sys/times.h>
#include <mach/clock.h>
#include <mach/mach.h>


int64_t ewol::getTime() {
	struct timespec now;
	clock_serv_t cclock;
	mach_timespec_t mts;
	host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
	clock_get_time(cclock, &mts);
	mach_port_deallocate(mach_task_self(), cclock);
	now.tv_sec = mts.tv_sec;
	now.tv_nsec = mts.tv_nsec;
	//EWOL_VERBOSE("current time : " << now.tv_sec << "s " << now.tv_usec << "us");
	return (int64_t)((int64_t)now.tv_sec*(int64_t)1000000 + (int64_t)now.tv_nsec/(int64_t)1000);
}

#undef __class__
#define __class__	"MacOSInterface"



class MacOSInterface : public ewol::Context {
private:
	ewol::key::Special m_guiKeyBoardMode;
public:
	MacOSInterface(ewol::context::Application* _application, int32_t _argc, const char* _argv[]) :
	ewol::Context(_application, _argc, _argv) {
		// nothing to do ...
	}
	
	int32_t Run() {
		
		return 0;
	}
	virtual void stop() {
		mm_exit();
	}
public:
	//interface MacOS :
	
	bool MAC_Draw(bool _displayEveryTime) {
		return OS_Draw(_displayEveryTime);
	}
	void MAC_Resize(float _x, float _y) {
		OS_Resize(vec2(_x,_y));
	}
	void MAC_SetMouseState(int32_t _id, bool _isDown, float _x, float _y) {
		OS_SetMouseState(_id, _isDown, vec2(_x, _y));
	}
	void MAC_SetMouseMotion(int32_t _id, float _x, float _y) {
		OS_SetMouseMotion(_id, vec2(_x, _y));
	}
	void MAC_SetInputState(int32_t _id, bool _isDown, float _x, float _y) {
		OS_SetInputState(_id, _isDown, vec2(_x, _y));
	}
	void MAC_SetInputMotion(int32_t _id, float _x, float _y) {
		OS_SetInputMotion(_id, vec2(_x, _y));
	}
	void MAC_SetKeyboard(ewol::key::Special _keyboardMode, int32_t _unichar, bool _isDown, bool _isAReapeateKey) {
		if (_unichar == u32char::Delete) {
			_unichar = u32char::Suppress;
		} else if (_unichar == u32char::Suppress) {
			_unichar = u32char::Delete;
		}
		if (_unichar == u32char::CarrierReturn) {
			_unichar = u32char::Return;
		}
		//EWOL_DEBUG("key: " << _unichar << " up=" << !_isDown);
		if (_unichar <= 4) {
			enum ewol::key::keyboard move;
			switch(_unichar) {
				case 0:
					move = ewol::key::keyboardUp;
					break;
				case 1:
					move = ewol::key::keyboardDown;
					break;
				case 2:
					move = ewol::key::keyboardLeft;
					break;
				case 3:
					move = ewol::key::keyboardRight;
					break;
			}
			OS_SetKeyboardMove(_keyboardMode, move, !_isDown, _isAReapeateKey);
		} else {
			OS_SetKeyboard(_keyboardMode, _unichar, !_isDown, _isAReapeateKey);
		}
	}
	void MAC_SetKeyboardMove(ewol::key::Special& _special,
							 enum ewol::key::keyboard _move,
							 bool _isDown) {
		OS_SetKeyboardMove(_special, _move, _isDown);
	}
	void openURL(const std::string& _url) {
		mm_openURL(_url.c_str());
	}
};


MacOSInterface* interface = nullptr;



bool IOs::draw(bool _displayEveryTime) {
	if (interface == nullptr) {
		return false;
	}
	return interface->MAC_Draw(_displayEveryTime);
}

void IOs::resize(float _x, float _y) {
	if (interface == nullptr) {
		return;
	}
	interface->MAC_Resize(_x, _y);
}


void IOs::setMouseState(int32_t _id, bool _isDown, float _x, float _y) {
	if (interface == nullptr) {
		return;
	}
	interface->MAC_SetMouseState(_id, _isDown, _x, _y);
}

void IOs::setMouseMotion(int32_t _id, float _x, float _y) {
	if (interface == nullptr) {
		return;
	}
	interface->MAC_SetMouseMotion(_id, _x, _y);
}

void IOs::setInputState(int32_t _id, bool _isDown, float _x, float _y) {
	if (interface == nullptr) {
		return;
	}
	interface->MAC_SetInputState(_id, _isDown, _x, _y);
}

void IOs::setInputMotion(int32_t _id, float _x, float _y) {
	if (interface == nullptr) {
		return;
	}
	interface->MAC_SetInputMotion(_id, _x, _y);
}

void IOs::setKeyboard(ewol::key::Special _keyboardMode, int32_t _unichar, bool _isDown, bool _isAReapeateKey) {
	if (interface == nullptr) {
		return;
	}
	interface->MAC_SetKeyboard(_keyboardMode, _unichar, _isDown, _isAReapeateKey);
}

void IOs::setKeyboardMove(ewol::key::Special& _keyboardMode, enum ewol::key::keyboard _move, bool _isDown) {
	if (interface == nullptr) {
		return;
	}
	interface->MAC_SetKeyboardMove(_keyboardMode, _move, _isDown);
}

void IOs::start() {
	if (interface == nullptr) {
		return;
	}
	//interface->OS_Start();
}

void IOs::resume() {
	if (interface == nullptr) {
		return;
	}
	interface->OS_Resume();
}

void IOs::suspend() {
	if (interface == nullptr) {
		return;
	}
	interface->OS_Suspend();
}

void IOs::stop() {
	if (interface == nullptr) {
		return;
	}
	interface->OS_Stop();
}
void IOs::background() {
	if (interface == nullptr) {
		return;
	}
	interface->OS_Background();
}
void IOs::foreground() {
	if (interface == nullptr) {
		return;
	}
	interface->OS_Foreground();
}


static int l_argc = 0;
static const char **l_argv = nullptr;
/**
 * @brief Main of the program
 * @param std IO
 * @return std IO
 */
int ewol::run(ewol::context::Application* _application, int _argc, const char *_argv[]) {
	l_argc = _argc;
	l_argv = _argv;
	return mm_main(_application, _argc, _argv);
}

// Creat and relaese ewol::Context interface:
void IOs::createInterface() {
	etk::setArgZero(l_argv[0]);
	EWOL_INFO("Create new interface");
	interface = new MacOSInterface(l_argc, l_argv);
	if (nullptr == interface) {
		EWOL_CRITICAL("Can not create the X11 interface ... MEMORY allocation error");
		return;
	}
}

void IOs::releaseInterface() {
	if (interface == nullptr) {
		return;
	}
	EWOL_INFO("Remove interface");
	delete(interface);
	interface = nullptr;
}






