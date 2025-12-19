	#include "InputWrapper.h"

#include <SDL_events.h>

InputWrapper::InputWrapper()
{
	event = new SDL_Event();
}

InputWrapper::~InputWrapper()
{
	delete event;
	event = nullptr;
}

SDL_Event* InputWrapper::getEvent()
{
	return event;
}