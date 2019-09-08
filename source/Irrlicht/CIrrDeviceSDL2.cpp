// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_SDL2_DEVICE_

#include "CIrrDeviceSDL2.h"
#include "IEventReceiver.h"
#include "irrList.h"
#include "os.h"
#include "CTimer.h"
#include "irrString.h"
#include "Keycodes.h"
#include "COSOperator.h"
#include <stdio.h>
#include <stdlib.h>
#include "SIrrCreationParameters.h"
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_video.h>


namespace irr
{
	
//! constructor
CIrrDeviceSDL::CIrrDeviceSDL(const SIrrlichtCreationParameters& param)
	: CIrrDeviceStub(param),
	ScreenWindow(0), ScreenTexture(0),
	MouseX(0), MouseY(0), MouseXRel(0), MouseYRel(0), MouseButtonStates(0),
	Width(param.WindowSize.Width), Height(param.WindowSize.Height),
	Resizable(false), WindowHasFocus(true)
{

	if (SDL_Init( SDL_INIT_TIMER|SDL_INIT_VIDEO| SDL_INIT_NOPARACHUTE ) <0)
	{
		os::Printer::log( "Unable to initialize SDL!", SDL_GetError());
		Close = true;
	}

	SDL_VERSION(&Info.version);

	core::stringc sdlversion = "SDL Version ";
	sdlversion += Info.version.major;
	sdlversion += ".";
	sdlversion += Info.version.minor;
	sdlversion += ".";
	sdlversion += Info.version.patch;

	Operator = new COSOperator(sdlversion);
	os::Printer::log(sdlversion.c_str(), ELL_INFORMATION);

	createWindow();

	// create cursor control
	CursorControl = new CCursorControl(this);

	// create driver
	createDriver();

	if (VideoDriver)
		createGUIAndScene();
	
}


//! destructor
CIrrDeviceSDL::~CIrrDeviceSDL()
{	
	if (ScreenTexture) 
    { 
        SDL_DestroyTexture(ScreenTexture); 
        ScreenTexture = NULL; 
    } 
	SDL_DestroyRenderer(ScreenRenderer);
	SDL_DestroyWindow(ScreenWindow);

	SDL_Quit();

	os::Printer::log("Quit SDL", ELL_INFORMATION);
}

bool CIrrDeviceSDL::createWindow()
{
	if ( Close )
		return false;

	int flags = 0;//SDL_WINDOW_FULLSCREEN;
	ScreenWindow = SDL_CreateWindow("Untitled", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, Width, Height, flags);
	ScreenRenderer = SDL_CreateRenderer(ScreenWindow, -1, 0);//SDL_RENDERER_ACCELERATED);
	ScreenTexture = SDL_CreateTexture(ScreenRenderer, SDL_PIXELFORMAT_ARGB1555, SDL_TEXTUREACCESS_STREAMING, Width, Height);
	


	return true;
}


//! create the driver
void CIrrDeviceSDL::createDriver()
{
	switch(CreationParams.DriverType)
	{
	case video::EDT_SOFTWARE:
		#ifdef _IRR_COMPILE_WITH_SOFTWARE_
		VideoDriver = video::createSoftwareDriver(CreationParams.WindowSize, CreationParams.Fullscreen, FileSystem, this);
		#else
		os::Printer::log("No Software driver support compiled in.", ELL_ERROR);
		#endif
		break;

	default:
		os::Printer::log("Unable to create video driver of unknown type.", ELL_ERROR);
		break;
	}

}


//! runs the device. Returns false if device wants to be deleted
bool CIrrDeviceSDL::run()
{
	os::Timer::tick();

	SEvent irrevent;
	SDL_Event SDL_event;

	while ( !Close && SDL_PollEvent( &SDL_event ) )
	{
		switch ( SDL_event.type )
		{
		case SDL_MOUSEMOTION:
			irrevent.EventType = irr::EET_MOUSE_INPUT_EVENT;
			irrevent.MouseInput.Event = irr::EMIE_MOUSE_MOVED;
			MouseX = irrevent.MouseInput.X = SDL_event.motion.x;
			MouseY = irrevent.MouseInput.Y = SDL_event.motion.y;
			irrevent.MouseInput.ButtonStates = MouseButtonStates;

			postEventFromUser(irrevent);
			break;
		case SDL_WINDOWEVENT:
            switch (SDL_event.window.event) 
            { 

            case SDL_WINDOWEVENT_ENTER: 
            case SDL_WINDOWEVENT_FOCUS_GAINED: 
                WindowHasFocus = true; 
                break; 
                 
            case SDL_WINDOWEVENT_LEAVE: 
            case SDL_WINDOWEVENT_FOCUS_LOST: 
                WindowHasFocus = false; 
                break; 
            } 
            break; 
			
		case SDL_QUIT:
			Close = true;
			break;


		case SDL_KEYDOWN: // if key pressed
			switch(SDL_event.key.keysym.sym){ // check what one
					case SDLK_ESCAPE: // if "esc"
						Close = true; // then quit
						break;
			}
		break;


		default:
			break;
		} // end switch

	} // end while

	return !Close;
}


//! pause execution temporarily
void CIrrDeviceSDL::yield()
{
	SDL_Delay(0);
}


//! pause execution for a specified time
void CIrrDeviceSDL::sleep(u32 timeMs, bool pauseTimer)
{
	const bool wasStopped = Timer ? Timer->isStopped() : true;
	if (pauseTimer && !wasStopped)
		Timer->stop();

	SDL_Delay(timeMs);

	if (pauseTimer && !wasStopped)
		Timer->start();
}


//! sets the caption of the window
void CIrrDeviceSDL::setWindowCaption(const wchar_t* text)
{
	core::stringc textc = text;
	SDL_SetWindowTitle(ScreenWindow, textc.c_str());
}


//! presents a surface in the client area
bool CIrrDeviceSDL::present(video::IImage* surface, void* windowId, core::rect<s32>* srcClip)
{

	if (SDL_UpdateTexture(ScreenTexture, NULL /* update whole texture */, surface->lock(), surface->getPitch()) != 0) { 
		// SDL_GetError 
	} 
     
	if (SDL_RenderCopy(ScreenRenderer, ScreenTexture, NULL, NULL) != 0) { 
		// SDL_GetError 
	} 
	SDL_RenderPresent(ScreenRenderer); 

				
	return true;
}


//! notifies the device that it should close itself
void CIrrDeviceSDL::closeDevice()
{
	Close = true;
}


//! \return Pointer to a list with all video modes supported
video::IVideoModeList* CIrrDeviceSDL::getVideoModeList()
{
}
 
 


//! Sets if the window should be resizable in windowed mode.
void CIrrDeviceSDL::setResizable(bool resize)
{
}


//! Minimizes window if possible
void CIrrDeviceSDL::minimizeWindow()
{
	if (ScreenWindow) {
		SDL_MinimizeWindow(ScreenWindow);
	}
}


//! Maximize window
void CIrrDeviceSDL::maximizeWindow()
{
	if (ScreenWindow) {
		SDL_MaximizeWindow(ScreenWindow);
	}
}

//! Restore original window size
void CIrrDeviceSDL::restoreWindow()
{
	// do nothing
}

//! returns if window is active. if not, nothing need to be drawn
bool CIrrDeviceSDL::isWindowActive() const
{
	return (WindowHasFocus && !WindowMinimized);
}


//! returns if window has focus.
bool CIrrDeviceSDL::isWindowFocused() const
{
	return WindowHasFocus;
}


//! returns if window is minimized.
bool CIrrDeviceSDL::isWindowMinimized() const
{
	return WindowMinimized;
}


//! Set the current Gamma Value for the Display
bool CIrrDeviceSDL::setGammaRamp( f32 red, f32 green, f32 blue, f32 brightness, f32 contrast )
{
	return false;
}

//! Get the current Gamma Value for the Display
bool CIrrDeviceSDL::getGammaRamp( f32 &red, f32 &green, f32 &blue, f32 &brightness, f32 &contrast )
{
	return false;
}


//! returns color format of the window.
video::ECOLOR_FORMAT CIrrDeviceSDL::getColorFormat() const
{
		return CIrrDeviceStub::getColorFormat();
}

} // end namespace irr

#endif // _IRR_COMPILE_WITH_SDL1_DEVICE_

