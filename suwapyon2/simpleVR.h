#pragma once
#define WIN32_LEAN_AND_MEAN
#define _USE_MATH_DEFINES
// Windows Header Files
#include <windows.h>
#include <d3d11_4.h>
#include <DirectXMath.h>
#include <iostream>
#include <sstream>

#include <openvr.h>

union uMatrix
{
	float matrix[4][4];
	float _m[16];
};

enum poseType
{
	None = 0,
	Projection = 1,
	EyeOffset = 5,
	hmdPosition = 10,
	LeftHand = 20,
	LeftHandPalm = 21,
	RightHand = 30,
	RightHandPalm = 31,
};

class simpleVR
{
	vr::IVRSystem* openVRSession = nullptr;
	vr::IVRChaperone* openVRChaperone = nullptr;
	vr::IVRRenderModels* openVRModels = nullptr;
	vr::TrackedDevicePose_t rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
	bool _isConnected = false;
	POINT bufferSize;
	POINT resolution;
	uMatrix projMatrixRaw[2];
	uMatrix eyeViewMatrixRaw[2];
	uMatrix eyeViewMatrix[2];
	uMatrix identMatrix;
	uMatrix hmdMatrix;
	uMatrix controllerMatrix[4];
	uMatrix genericMatrix[3];
	int gTrackCount;
	float currentIPD;
	bool printLogs = false;
	std::stringstream logError;
	vr::HmdVector2_t depthRange;
	vr::VRTextureBounds_t textureBounds[2];
	bool asymmetricProjection = true;
	//fingerHandLayout layoutFinger[2];

private:
	void InitalizeVR();

public:
	simpleVR(bool showLogs = false);
	~simpleVR();

	bool PreloadVR(vr::EVRApplicationType appType);
	bool StartVR(vr::EVRApplicationType appType);
	void TanProjection(int eye, float* up, float* down, float* left, float* right);
	bool StopVR();
	bool isEnabled();
	void Recenter();
	POINT GetBufferSize();
	void SetActionPose(vr::HmdMatrix34_t matPose, poseType poseType);
	void SetSkeletalPose(vr::VRBoneTransform_t* boneArray, int boneCount, poseType poseType);
	void SetFramePose();
	uMatrix GetFramePose(poseType poseType, int eye);
	//fingerHandLayout GetSkeletalPose(poseType poseType);
	void Render(ID3D11Texture2D* leftEye, ID3D11Texture2D* leftDepth, ID3D11Texture2D* rightEye, ID3D11Texture2D* rightDepth);
	void WaitGetPoses();
	void MakeIPDOffset();
	bool HasErrors();
	std::string GetErrors();
};
