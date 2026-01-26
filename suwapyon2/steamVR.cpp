#include "steamVR.h"
#include <string>
#include <algorithm>

bool setActiveJSON(std::string relativeFilePath)
{
	vr::EVRInputError iError = vr::VRInputError_None;
	std::string fullManifestPath;
	bool retVal = true;

	//----
	// Gets the current exe path, strips the file name and
	// adds the relative file path
	//----
	char tPath[MAX_PATH];
	int bytes = GetModuleFileName(NULL, tPath, MAX_PATH);
	if (bytes == 0) {
		retVal = false;
	}
	else {
		std::string exePath(tPath);
		std::replace(exePath.begin(), exePath.end(), '\\', '/');
		int pos = exePath.rfind('/') + 1;
		if (pos > -1) {
			exePath.erase(pos);
			fullManifestPath = exePath + relativeFilePath;
			if (GetFileAttributes(fullManifestPath.c_str()) == INVALID_FILE_ATTRIBUTES) {
				retVal = false;
			}
		}
		else {
			retVal = false;
		}
	}

	if (retVal) {
		if (fullManifestPath.length() > 0) {
			iError = vr::VRInput()->SetActionManifestPath(fullManifestPath.c_str());
		}
	}
	return retVal;
}

bool setActionHandlesGame(inputController* input)
{
	vr::EVRInputError iError = vr::VRInputError_None;

	iError = vr::VRInput()->GetActionSetHandle("/actions/game", &input->game.setHandle);
	iError = vr::VRInput()->GetActionHandle("/actions/game/in/movement", &input->game.movement);
	iError = vr::VRInput()->GetActionHandle("/actions/game/in/shoot", &input->game.shoot);
	iError = vr::VRInput()->GetActionHandle("/actions/game/in/bomb", &input->game.bomb);
	iError = vr::VRInput()->GetActionHandle("/actions/game/in/jump", &input->game.jump);
	iError = vr::VRInput()->GetActionHandle("/actions/game/in/rotateleft", &input->game.rotateleft);
	iError = vr::VRInput()->GetActionHandle("/actions/game/in/rotateright", &input->game.rotateright);
	iError = vr::VRInput()->GetActionHandle("/actions/game/in/rotateup", &input->game.rotateup);
	iError = vr::VRInput()->GetActionHandle("/actions/game/in/rotatedown", &input->game.rotatedown);
	iError = vr::VRInput()->GetActionHandle("/actions/game/in/escape", &input->game.escape);

	return true;
}
