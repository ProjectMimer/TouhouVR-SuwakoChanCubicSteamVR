#pragma once

#undef UNICODE
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <openvr.h>

struct inputActionGame
{
	vr::VRActionSetHandle_t setHandle;

	vr::VRActionHandle_t movement;
	vr::VRActionHandle_t shoot;
	vr::VRActionHandle_t bomb;
	vr::VRActionHandle_t jump;
	vr::VRActionHandle_t rotateleft;
	vr::VRActionHandle_t rotateright;
	vr::VRActionHandle_t rotateup;
	vr::VRActionHandle_t rotatedown;
	vr::VRActionHandle_t escape;
};


struct inputController
{
	struct inputActionGame game;
};


bool setActiveJSON(std::string relativeFilePath);
bool setActionHandlesGame(inputController* input);
