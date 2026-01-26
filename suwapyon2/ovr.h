#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

typedef enum
{
    ovrHmd_None = 0,
    ovrHmd_DK1 = 3,
    ovrHmd_DKHD = 4,
    ovrHmd_DK2 = 6,
    ovrHmd_Other
} ovrHmdType;

typedef enum
{
    ovrEye_Left = 0,
    ovrEye_Right = 1,
    ovrEye_Count = 2
} ovrEyeType;

struct ovrFovPort
{
    float UpTan;
    float DownTan;
    float LeftTan;
    float RightTan;
};

struct ovrVector2i
{
    int x, y;
};

struct ovrSizei
{
    int w, h;
};

struct ovrRecti
{
    ovrVector2i Pos;
    ovrSizei    Size;
};

struct ovrVector2f
{
    float x, y;
};

struct ovrVector3f
{
    float x, y, z;
};

struct ovrQuatf
{
    float x, y, z, w;
};

struct ovrPosef
{
    ovrQuatf     Orientation;
    ovrVector3f  Position;
};

struct ovrHmdDesc;
struct ovrHmdStruct
{
    byte b1[0x008 - 0x000];
    ovrHmdDesc* ovrDescription;
    HWND hwnd;
    byte b2[0x030 - 0x010];
    int width1;
    int height1;
    byte b3[0x064 - 0x038];
    int screenWidth;
    byte b4[0x074 - 0x068];
    int width2;
    int height2;
    byte b6[0x11C - 0x07C];
    int width3;
    int height3;
    byte b7[0x25C - 0x124];
};

struct ovrHmdDesc
{
    ovrHmdStruct* Handle;
    ovrHmdType  Type;
    const char* ProductName;
    const char* Manufacturer;
    short       VendorId;
    short       ProductId;
    char        SerialNumber[24];
    short       FirmwareMajor;
    short       FirmwareMinor;
    float       CameraFrustumHFovInRadians;
    float       CameraFrustumVFovInRadians;
    float       CameraFrustumNearZInMeters;
    float       CameraFrustumFarZInMeters;
    unsigned int HmdCaps;
    unsigned int TrackingCaps;
    unsigned int DistortionCaps;
    ovrFovPort  DefaultEyeFov[ovrEye_Count];
    ovrFovPort  MaxEyeFov[ovrEye_Count];
    ovrEyeType  EyeRenderOrder[ovrEye_Count];
    ovrSizei    Resolution;
    ovrVector2i WindowsPos;
    const char* DisplayDeviceName;
    int         DisplayId;
};

struct ovrEyeRenderDesc
{
    ovrEyeType  Eye;
    ovrFovPort  Fov;
    ovrRecti	DistortedViewport; 	        /// Distortion viewport.
    ovrVector2f PixelsPerTanAngleAtCenter;  /// How many display pixels will fit in tan(angle) = 1.
    ovrVector3f ViewAdjust;  		        /// Translation to be applied to view matrix.
};

struct Vector4f
{
    float x, y, z, w;
};
