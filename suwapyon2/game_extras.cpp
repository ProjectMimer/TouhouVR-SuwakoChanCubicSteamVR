#include "game_extras.h"
#include "stBasicTexture.h"
#include "clsMemManager.h"
#include "clsKeyboard.h"
#include "simpleVR.h"
#include "steamVR.h"
#include "ovr.h"

extern bool doLog;
extern std::stringstream logError;
bool UseThread = false;

using namespace DirectX;

simpleVR* svr = new simpleVR(true);
POINT hmdBufferSize = { 0, 0 };

XMMATRIX matProjection[2] = { XMMatrixIdentity(), XMMatrixIdentity() };
XMMATRIX matEyeOffset[2] = { XMMatrixIdentity(), XMMatrixIdentity() };
XMMATRIX matHMDPos = XMMatrixIdentity();

XMMATRIX matSwapHanded = XMMatrixScaling(1, 1,-1);

inputController input = {}; //{ { 0, 0, 0, 0, 0, 0, 0, 0, 0 } };

stBasicTexture BackBuffer[2] = { stBasicTexture(), stBasicTexture() };
stBasicTexture DepthBuffer[2] = { stBasicTexture(), stBasicTexture() };

ovrEyeRenderDesc eyeRender[2] = { ovrEyeRenderDesc(), ovrEyeRenderDesc() };

struct stMatrixBuffer
{
    DirectX::XMMATRIX worldMatrix;
    DirectX::XMMATRIX viewMatrix;
    DirectX::XMMATRIX projectionMatrix;
} MatrixBuffer;

struct stTextureBlock
{
    int DirectX;
    ID3D11Texture2D* texture;
    ID3D11ShaderResourceView* shaderView;
    int type;
    int a;
    int b;
    int c;
    int width;
    int height;
    ID3D11Texture2D* depth;
    ID3D11RenderTargetView* renderView;
    ID3D11DepthStencilView* depthView;
};


const std::string g_VR_PATH = "./";
const std::string g_CONFIG_FILE = g_VR_PATH + "config.txt";


const int USED_KEYS_LEFT  = 0x001;
const int USED_KEYS_UP    = 0x002;
const int USED_KEYS_RIGHT = 0x004;
const int USED_KEYS_DOWN  = 0x008;
const int USED_KEYS_Z     = 0x010;
const int USED_KEYS_X     = 0x020;
const int USED_KEYS_SHIFT = 0x040;
const int USED_KEYS_A     = 0x080;
const int USED_KEYS_D     = 0x100;
const int USED_KEYS_W     = 0x200;
const int USED_KEYS_S     = 0x400;
const int USED_KEYS_ESC   = 0x800;


wchar_t GAME_PROCESS_NAME[] = L"suwapyon2Oculus.exe";
DWORD GAME_PROCESS_OFFSET = 0x0;

ovrHmdDesc* ovrHMD = nullptr;

uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName)
{
    uintptr_t modBaseAddr = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
    if (hSnap != INVALID_HANDLE_VALUE) {
        MODULEENTRY32 modEntry;
        modEntry.dwSize = sizeof(modEntry);
        if (Module32First(hSnap, &modEntry)) {
            do {
                if (!_wcsicmp(modEntry.szModule, modName)) {
                    modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
                    break;
                }
            } while (Module32Next(hSnap, &modEntry));
        }
    }
    CloseHandle(hSnap);
    return modBaseAddr;
}

bool CreateTextures(ID3D11Device* devDX11, POINT textureSize)
{
    int bufferCount = sizeof(BackBuffer) / sizeof(BackBuffer[0]);
    for (int i = 0; i < bufferCount; i++)
    {
        BackBuffer[i].SetWidthHeight(textureSize.x, textureSize.y);
        BackBuffer[i].textureDesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;
        if (!BackBuffer[i].Create(devDX11, true, true, false, true))
            logError << BackBuffer[i].GetErrors();

        DepthBuffer[i].SetWidthHeight(textureSize.x, textureSize.y);
        DepthBuffer[i].textureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        DepthBuffer[i].textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        DepthBuffer[i].textureDesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;
        if (!DepthBuffer[i].Create(devDX11, false, false, true, true))
            logError << DepthBuffer[i].GetErrors();
    }
    return true;
}

void DestroyTextures()
{
    int bufferCount = sizeof(BackBuffer) / sizeof(BackBuffer[0]);
    for (int i = 0; i < bufferCount; i++)
    {
        BackBuffer[i].Release();
        DepthBuffer[i].Release();
    }
}

// Create Texture 2D
void(__thiscall *sub_041990)(int, int, int, int) = NULL;


// Find_HMD
ovrHmdDesc* (*sub_099E60)(int) = NULL;
ovrHmdDesc* (msub_099E60)(int a)
{
    //ovrHmdDesc* retVal = sub_099E60(a);
    return ovrHMD;
}

// ovrSetupRender
void(__thiscall* sub_01AFA0)(void*, int, ovrHmdDesc*) = NULL;
void(__fastcall msub_01AFA0)(void* ecx, void* edx, int a, ovrHmdDesc* b)
{
    if (doLog) logError << "-- ovrSetupRender Start" << std::endl;

    if (doLog) logError << "-- ovrSetupRender Preload" << std::endl;
    svr->PreloadVR(vr::VRApplication_Scene);

    hmdBufferSize = svr->GetBufferSize();
    if (hmdBufferSize.x == 0) {
        hmdBufferSize.x = 1844;
        hmdBufferSize.y = 1844;
    }

    bool retVal = true;
    if (doLog) logError << "-- ovrSetupRender StartVR" << std::endl;
    svr->StartVR(vr::VRApplication_Scene);
    if (svr->HasErrors())
        logError << svr->GetErrors();

    if (svr->isEnabled())
    {
        matProjection[0] = XMMatrixTranspose(matSwapHanded * (XMMATRIX)(svr->GetFramePose(poseType::Projection, 0)._m));
        matProjection[1] = XMMatrixTranspose(matSwapHanded * (XMMATRIX)(svr->GetFramePose(poseType::Projection, 1)._m));
        matEyeOffset[0] = (XMMATRIX)(svr->GetFramePose(poseType::EyeOffset, 0)._m);
        matEyeOffset[1] = (XMMATRIX)(svr->GetFramePose(poseType::EyeOffset, 1)._m);
    }

    //----
    // Update hmdDesc details with current headset data
    //-----
    eyeRender[0].Eye = ovrEyeType::ovrEye_Left;
    eyeRender[0].Fov.UpTan = 1;
    eyeRender[0].Fov.DownTan = 1;
    eyeRender[0].Fov.LeftTan = 1;
    eyeRender[0].Fov.RightTan = 1;
    eyeRender[0].DistortedViewport.Pos.x = 0;
    eyeRender[0].DistortedViewport.Pos.y = 0;
    eyeRender[0].DistortedViewport.Size.w = hmdBufferSize.x;
    eyeRender[0].DistortedViewport.Size.h = hmdBufferSize.y;
    eyeRender[0].PixelsPerTanAngleAtCenter.x = 0.f;
    eyeRender[0].PixelsPerTanAngleAtCenter.y = 0.f;
    eyeRender[0].ViewAdjust.x = matEyeOffset[0].r[3].m128_f32[0];
    eyeRender[0].ViewAdjust.y = 0;
    eyeRender[0].ViewAdjust.z = 0;

    eyeRender[1].Eye = ovrEyeType::ovrEye_Right;
    eyeRender[1].Fov.UpTan = 1;
    eyeRender[1].Fov.DownTan = 1;
    eyeRender[1].Fov.LeftTan = -1;
    eyeRender[1].Fov.RightTan = -1;
    eyeRender[1].DistortedViewport.Pos.x = hmdBufferSize.x;
    eyeRender[1].DistortedViewport.Pos.y = 0;
    eyeRender[1].DistortedViewport.Size.w = hmdBufferSize.x;
    eyeRender[1].DistortedViewport.Size.h = hmdBufferSize.y;
    eyeRender[1].PixelsPerTanAngleAtCenter.x = 0.f;
    eyeRender[1].PixelsPerTanAngleAtCenter.y = 0.f;
    eyeRender[1].ViewAdjust.x = matEyeOffset[1].r[3].m128_f32[0];
    eyeRender[1].ViewAdjust.y = 0;
    eyeRender[1].ViewAdjust.z = 0;

    ovrHMD->Handle->width1 = hmdBufferSize.x;
    ovrHMD->Handle->width2 = hmdBufferSize.x;
    ovrHMD->Handle->width3 = hmdBufferSize.x;
    ovrHMD->Handle->height1 = hmdBufferSize.y;
    ovrHMD->Handle->height2 = hmdBufferSize.y;
    ovrHMD->Handle->height3 = hmdBufferSize.y;


    //----
    // Run function
    //----
    sub_01AFA0(ecx, a, b);


    if (doLog) logError << "-- ovrSetupRender Mid" << std::endl;

    *(int*)((int)ecx + 0x24) = hmdBufferSize.x * 2;
    *(int*)((int)ecx + 0x28) = hmdBufferSize.y;

    Vector4f* eyeOffsetL = (Vector4f*)((int)ecx + 0x22C);
    eyeOffsetL->x = 0;
    eyeOffsetL->y = 0;
    eyeOffsetL->z = (float)hmdBufferSize.x;
    eyeOffsetL->w = (float)hmdBufferSize.y;

    Vector4f* eyeOffsetR = (Vector4f*)((int)ecx + 0x23C);
    eyeOffsetR->x = (float)hmdBufferSize.x;
    eyeOffsetR->y = 0;
    eyeOffsetR->z = (float)hmdBufferSize.x;
    eyeOffsetR->w = (float)hmdBufferSize.y;

    // TextureBlock, DirectX, width, height
    sub_041990(*(int*)((int)ecx + 0xC0), (int)ecx, hmdBufferSize.x * 2, hmdBufferSize.y);

    DWORD dxOffset = *(DWORD*)(0x002A7354 + GAME_PROCESS_OFFSET);
    ID3D11Device* devDX11 = *(ID3D11Device**)(dxOffset + 0x3C);
    ID3D11DeviceContext* devConDX11 = *(ID3D11DeviceContext**)(dxOffset + 0x40);

    if (retVal) retVal = CreateTextures(devDX11, hmdBufferSize);

    if (retVal) retVal = setActiveJSON(g_VR_PATH + "actions.json");
    if (retVal) retVal = setActionHandlesGame(&input);
}

// Release OVR
void(*sub_099FA0)(ovrHmdDesc*) = NULL;
void(msub_099FA0)(ovrHmdDesc* a)
{
    //sub_09A620(a);
}

// Release OVR
void(*sub_09A620)() = NULL;
void(msub_09A620)()
{
    if (doLog) logError << "-- 005C20 DestroyDX - DestroyTextures" << std::endl;
    DestroyTextures();

    if (svr->isEnabled())
        svr->StopVR();
    if (doLog) logError << "-- 005C20 DestroyDX - Destory DX" << std::endl;

    sub_09A620();
}

// DiGetKey
int(__thiscall *sub_01CC00)(void*, int) = NULL;
int(__fastcall msub_01CC00)(void* a, void* b, int c)
{
    int retVal = sub_01CC00(a, c);
    return retVal | RunControllerGame();
}

// CreateProjection
void(*sub_09A4F0)(XMMATRIX*, float, float, float, float, float, float, bool) = NULL;
XMMATRIX*(msub_09A4F0)(XMMATRIX* a, float b, float c, float d, float e, float f, float g, bool h)
{
    sub_09A4F0(a, b, c, d, e, f, g, h);
    int eye = (d > 0) ? 0 : 1;
    return &matProjection[eye];
}

// GetTrackingData
void(*sub_09A050)(ovrPosef*, ovrHmdDesc*, int) = NULL;
ovrPosef*(msub_09A050)(ovrPosef* a, ovrHmdDesc* b, int c)
{
    sub_09A050(a, b, c);
    
    XMMATRIX tmpHMD = matHMDPos;
    XMVECTOR position = tmpHMD.r[3];
    tmpHMD.r[3] = { 0.f, 0.f, 0.f, 1.f };
    XMVECTOR rotation = XMQuaternionRotationMatrix(tmpHMD);
    
    a->Orientation.x = rotation.m128_f32[0];
    a->Orientation.y = rotation.m128_f32[1];
    a->Orientation.z = rotation.m128_f32[2];
    a->Orientation.w = rotation.m128_f32[3];

    a->Position.x = position.m128_f32[0];
    a->Position.y = position.m128_f32[1];
    a->Position.z = position.m128_f32[2];

    return a;
}

// calcSizeFromFOV
void(*sub_09A190)(ovrEyeRenderDesc*, ovrHmdDesc*, ovrEyeType, float, float, float, float) = NULL;
void(msub_09A190)(ovrEyeRenderDesc* a, ovrHmdDesc* b, ovrEyeType c, float d, float e, float f, float g)
{
    sub_09A190(a, b, c, d, e, f, g);
    memcpy_s((void*)a, sizeof(ovrEyeRenderDesc), &eyeRender[(int)c], sizeof(ovrEyeRenderDesc));

    if (c == ovrEyeType::ovrEye_Left)
    {
        a->DistortedViewport.Pos.x = 0;
        a->DistortedViewport.Pos.x = 0;
        a->DistortedViewport.Size.w = 640;
        a->DistortedViewport.Size.h = 800;
    }
    else if (c == ovrEyeType::ovrEye_Right)
    {
        a->DistortedViewport.Pos.x = 640;
        a->DistortedViewport.Pos.x = 0;
        a->DistortedViewport.Size.w = 640;
        a->DistortedViewport.Size.h = 800;
    }
}

void(*sub_09A3E0)(ovrHmdDesc*, unsigned int) = NULL;
void(msub_09A3E0)(ovrHmdDesc* a, unsigned int b)
{
    //sub_09A3E0(a, b);
}

void(__fastcall* sub_09F270)(void*, void*, int) = NULL;
void(__fastcall msub_09F270)(void* ecx, void* edx, int a)
{
    //sub_09F270(ecx, edx, a);
}

void(*sub_09A020)(ovrHmdDesc*) = NULL;
void(msub_09A020)(ovrHmdDesc* a)
{
    //sub_09A020(a);
}

// Present
void(*sub_01C030)() = NULL;
void(msub_01C030)()
{
    DWORD dxOffset = *(DWORD*)(0x002A7354 + GAME_PROCESS_OFFSET);
    ID3D11DeviceContext* devConDX11 = *(ID3D11DeviceContext**)(dxOffset + 0x40);
    IDXGISwapChain1* swapchain = *(IDXGISwapChain1**)(dxOffset + 0x44);
    ID3D11RenderTargetView* bbRenderTargetView = *(ID3D11RenderTargetView**)(dxOffset + 0x48);
    stTextureBlock* textureBlock = *(stTextureBlock**)(dxOffset + 0xC0);

    D3D11_BOX srcBox[] = {
    {0, 0, 0, hmdBufferSize.x, hmdBufferSize.y, 1},
    {hmdBufferSize.x, 0, 0, hmdBufferSize.x + hmdBufferSize.x, hmdBufferSize.y, 1}
    };

    devConDX11->CopySubresourceRegion(BackBuffer[0].pTexture, 0, 0, 0, 0, textureBlock->texture, 0, &srcBox[0]);
    devConDX11->CopySubresourceRegion(BackBuffer[1].pTexture, 0, 0, 0, 0, textureBlock->texture, 0, &srcBox[1]);

    //----
    // calibration - rotation
    //----
    //*(float*)(dxOffset + 0x310) = 0; // lX
    //*(float*)(dxOffset + 0x314) = 0; // rX
    //*(float*)(dxOffset + 0x318) = 0; // lY
    //*(float*)(dxOffset + 0x31C) = 0; // rY
    *(float*)(dxOffset + 0x320) = 0; // lZ
    *(float*)(dxOffset + 0x324) = 0; // rZ

    //----
    // calibration - position
    //----
    //*(float*)(dxOffset + 0x284) = 0; // lX
    //*(float*)(dxOffset + 0x288) = 0; // lY
    //*(float*)(dxOffset + 0x28C) = 0; // lZ
    //*(float*)(dxOffset + 0x290) = 0; // rX
    //*(float*)(dxOffset + 0x294) = 0; // rY
    //*(float*)(dxOffset + 0x298) = 0; // rZ


    sub_01C030();

    //----
    // Render the current texture to the headset
    //----
    if (svr->isEnabled())
    {
        svr->Render(BackBuffer[0].pTexture, DepthBuffer[0].pTexture, BackBuffer[1].pTexture, DepthBuffer[1].pTexture);
        if (svr->HasErrors())
            logError << svr->GetErrors();

        svr->WaitGetPoses();
        svr->SetFramePose();

        float tmpFloat = 0;
        matProjection[0] = XMMatrixTranspose(matSwapHanded * (XMMATRIX)(svr->GetFramePose(poseType::Projection, 0)._m));
        matProjection[1] = XMMatrixTranspose(matSwapHanded * (XMMATRIX)(svr->GetFramePose(poseType::Projection, 1)._m));
        matEyeOffset[0] = (XMMATRIX)(svr->GetFramePose(poseType::EyeOffset, 0)._m);
        matEyeOffset[1] = (XMMATRIX)(svr->GetFramePose(poseType::EyeOffset, 1)._m);
        matHMDPos = (XMMATRIX)(svr->GetFramePose(poseType::hmdPosition, -1)._m);
    }
}

void InitFakeDK1()
{
    int hmdWidth = 1280;
    int hmdHeight = 800;
    int screenWidth = 1080;

    ovrHMD = new ovrHmdDesc();
    ovrHMD->Handle = new ovrHmdStruct();
    ovrHMD->Handle->ovrDescription = ovrHMD;
    ovrHMD->Handle->hwnd = 0;
    ovrHMD->Handle->width1 = hmdWidth;
    ovrHMD->Handle->height1 = hmdHeight;
    ovrHMD->Handle->screenWidth = screenWidth;
    ovrHMD->Handle->width2 = hmdWidth;
    ovrHMD->Handle->height2 = hmdHeight;
    ovrHMD->Handle->width3 = hmdWidth;
    ovrHMD->Handle->height3 = hmdHeight;

    ovrHMD->Type = ovrHmdType::ovrHmd_DK1;
    ovrHMD->ProductName = "Oculus Rift DK1";
    ovrHMD->Manufacturer = "Oculus VR";
    ovrHMD->VendorId = 10291;
    ovrHMD->ProductId = 1;
    memcpy_s(ovrHMD->SerialNumber, 24, L"1234567890AB", 24);
    ovrHMD->FirmwareMajor = 0;
    ovrHMD->FirmwareMinor = 18;
    ovrHMD->CameraFrustumHFovInRadians = 0;
    ovrHMD->CameraFrustumVFovInRadians = 0;
    ovrHMD->CameraFrustumNearZInMeters = 0;
    ovrHMD->CameraFrustumFarZInMeters = 0;
    ovrHMD->HmdCaps = 0x1009;
    ovrHMD->TrackingCaps = 0x30;
    ovrHMD->DistortionCaps = 0x1006B;
    ovrHMD->DefaultEyeFov[0].UpTan = 2.1380f;
    ovrHMD->DefaultEyeFov[0].DownTan = 2.1380f;
    ovrHMD->DefaultEyeFov[0].LeftTan = 2.117f;
    ovrHMD->DefaultEyeFov[0].RightTan = 0.9707f;
    ovrHMD->DefaultEyeFov[1].UpTan = 2.1380f;
    ovrHMD->DefaultEyeFov[1].DownTan = 2.1380f;
    ovrHMD->DefaultEyeFov[1].LeftTan = 0.9707f;
    ovrHMD->DefaultEyeFov[1].RightTan = 2.117f;
    ovrHMD->MaxEyeFov[0].UpTan = 3.7238f;
    ovrHMD->MaxEyeFov[0].DownTan = 3.7238f;
    ovrHMD->MaxEyeFov[0].LeftTan = 2.4865f;
    ovrHMD->MaxEyeFov[0].RightTan = 0.9707f;
    ovrHMD->MaxEyeFov[1].UpTan = 3.7238f;
    ovrHMD->MaxEyeFov[1].DownTan = 3.7238f;
    ovrHMD->MaxEyeFov[1].LeftTan = 0.9707f;
    ovrHMD->MaxEyeFov[1].RightTan = 2.4865f;
    ovrHMD->EyeRenderOrder[0] = ovrEyeType::ovrEye_Left;
    ovrHMD->EyeRenderOrder[1] = ovrEyeType::ovrEye_Right;
    ovrHMD->Resolution.w = hmdWidth;
    ovrHMD->Resolution.h = hmdHeight;
    ovrHMD->WindowsPos.x = 0;
    ovrHMD->WindowsPos.y = 0;
    ovrHMD->DisplayDeviceName = ""; // "\\\\.\\DISPLAY1\\Monitor0";
    ovrHMD->DisplayId = -1;
}

void ReleaseFakeDK1()
{ 
    delete ovrHMD->Handle;
    ovrHMD->Handle = nullptr;
    delete ovrHMD;
    ovrHMD = nullptr;
}

void InitDetours(HANDLE hModule)
{
    if (doLog) logError << "-- InitDetours Start " << std::endl;

    uintptr_t procId = GetCurrentProcessId();
    GAME_PROCESS_OFFSET = GetModuleBaseAddress(procId, GAME_PROCESS_NAME);

    //----
    // Get function memory locations
    //----
    sub_041990 = (void(__thiscall*)(int, int, int, int))(0x041990 + GAME_PROCESS_OFFSET); // Create Texture2D

    sub_099E60 = (ovrHmdDesc * (*)(int))(0x099E60 + GAME_PROCESS_OFFSET); // Find_HMD
    sub_01AFA0 = (void(__thiscall*)(void*, int, ovrHmdDesc*))(0x01AFA0 + GAME_PROCESS_OFFSET); // ovrSetupRender
    sub_099FA0 = (void(*)(ovrHmdDesc*))(0x099FA0 + GAME_PROCESS_OFFSET); // ReleaseOVR
    sub_09A620 = (void(*)())(0x09A620 + GAME_PROCESS_OFFSET); // ReleaseOVR
    sub_01CC00 = (int(__thiscall*)(void*, int))(0x01CC00 + GAME_PROCESS_OFFSET); // DiGetKey
    sub_09A4F0 = (void(*)(XMMATRIX*, float, float, float, float, float, float, bool))(0x09A4F0 + GAME_PROCESS_OFFSET); // CreateProjection

    sub_09A050 = (void(*)(ovrPosef*, ovrHmdDesc*, int))(0x09A050 + GAME_PROCESS_OFFSET); // GetTrackingData
    sub_09A190 = (void(*)(ovrEyeRenderDesc*, ovrHmdDesc*, ovrEyeType, float, float, float, float))(0x09A190 + GAME_PROCESS_OFFSET); // CalcFOV

    sub_09A3E0 = (void(*)(ovrHmdDesc*, unsigned int))(0x09A3E0 + GAME_PROCESS_OFFSET); // Pre Tracking
    sub_09F270 = (void(__fastcall*)(void*, void*, int))(0x09F270 + GAME_PROCESS_OFFSET);
    sub_09A020 = (void(*)(ovrHmdDesc*))(0x09A020 + GAME_PROCESS_OFFSET);

    sub_01C030 = (void(*)())(0x01C030 + GAME_PROCESS_OFFSET); // Present

    //----
    // Modify CreateDXGIFactory function address to point to CreateDXGIFactory1
    //----
    HMODULE dxgiOffset = (HMODULE)GetModuleBaseAddress(procId, L"dxgi.dll");
    DWORD addrCreateDXGIFactory = (DWORD)GetProcAddress(dxgiOffset, "CreateDXGIFactory");
    DWORD addrCreateDXGIFactory1 = (DWORD)GetProcAddress(dxgiOffset, "CreateDXGIFactory1");
    DWORD ptrCreateDXGIFactory = (0xD537C + GAME_PROCESS_OFFSET);

    clsMemManager mem = clsMemManager();
    if (mem.attachWindow(procId) && addrCreateDXGIFactory == *(DWORD*)(ptrCreateDXGIFactory))
    {
        // Change the protection level around the variable
        // then change CreateDXGIFactory pointer to CreateDXGIFactory1
        // then change the protection level back
        DWORD oldVirtProt = 0;
        VirtualProtectEx(mem.getProcessHandle(), (LPVOID)ptrCreateDXGIFactory, 4, PAGE_EXECUTE_READWRITE, &oldVirtProt);
        mem.writeMemoryL(ptrCreateDXGIFactory, 4, addrCreateDXGIFactory1);
        VirtualProtectEx(mem.getProcessHandle(), (LPVOID)ptrCreateDXGIFactory, 4, oldVirtProt, &oldVirtProt);
    }
    mem.closeProcess();

    //----
    // Setup fake dk1
    //----
    InitFakeDK1();

    //-----
    // Start detours
    //----
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    DetourAttach((PVOID*)&sub_099E60, (PVOID)msub_099E60); // Find_HMD
    DetourAttach((PVOID*)&sub_01AFA0, (PVOID)msub_01AFA0); // ovrSetupRender
    DetourAttach((PVOID*)&sub_099FA0, (PVOID)msub_099FA0); // ReleaseOVR
    DetourAttach((PVOID*)&sub_09A620, (PVOID)msub_09A620); // ReleaseOVR
    DetourAttach((PVOID*)&sub_01CC00, (PVOID)msub_01CC00); // DiGetKey
    DetourAttach((PVOID*)&sub_09A4F0, (PVOID)msub_09A4F0); // CreateProjection

    DetourAttach((PVOID*)&sub_09A050, (PVOID)msub_09A050); // GetTrackingData
    DetourAttach((PVOID*)&sub_09A190, (PVOID)msub_09A190); // CalcFOV

    DetourAttach((PVOID*)&sub_09A3E0, (PVOID)msub_09A3E0); // Pre Tracking
    DetourAttach((PVOID*)&sub_09F270, (PVOID)msub_09F270);
    DetourAttach((PVOID*)&sub_09A020, (PVOID)msub_09A020);
    DetourAttach((PVOID*)&sub_01C030, (PVOID)msub_01C030); // Present

    if (DetourTransactionCommit() == NO_ERROR)
        OutputDebugString(L"detoured successfully started");

    return;
}

void ExitDetours()
{
    if (doLog) logError << "-- ExitDetours Start" << std::endl;

    //----
    // Stop Detours
    //----
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    DetourDetach((PVOID*)&sub_099E60, (PVOID)msub_099E60); // Find_HMD
    DetourDetach((PVOID*)&sub_01AFA0, (PVOID)msub_01AFA0); // ovrSetupRender
    DetourDetach((PVOID*)&sub_099FA0, (PVOID)msub_099FA0); // ReleaseOVR
    DetourDetach((PVOID*)&sub_09A620, (PVOID)msub_09A620); // ReleaseOVR
    DetourDetach((PVOID*)&sub_01CC00, (PVOID)msub_01CC00); // DiGetKey
    DetourDetach((PVOID*)&sub_09A4F0, (PVOID)msub_09A4F0); // CreateProjection

    DetourDetach((PVOID*)&sub_09A050, (PVOID)msub_09A050); // GetTrackingData
    DetourDetach((PVOID*)&sub_09A190, (PVOID)msub_09A190); // CalcFOV

    DetourDetach((PVOID*)&sub_09A3E0, (PVOID)msub_09A3E0); // Pre Tracking
    DetourDetach((PVOID*)&sub_09F270, (PVOID)msub_09F270);
    DetourDetach((PVOID*)&sub_09A020, (PVOID)msub_09A020);
    DetourDetach((PVOID*)&sub_01C030, (PVOID)msub_01C030); // Present

    if (DetourTransactionCommit() == NO_ERROR)
        OutputDebugString(L"detoured successfully stopped");

    //----
    // Release fake dk1
    //----
    ReleaseFakeDK1();

    return;
}

int RunControllerGame()
{
    //----
    // Set the current action set
    //----
    vr::VRActiveActionSet_t actionSet = { 0 };
    actionSet.ulActionSet = input.game.setHandle;
    uint32_t setSize = sizeof(actionSet);
    uint32_t setCount = setSize / sizeof(vr::VRActiveActionSet_t);

    vr::ETrackingUniverseOrigin eOrigin = vr::TrackingUniverseSeated;

    vr::InputDigitalActionData_t digitalActionData = { 0 };
    vr::InputAnalogActionData_t analogActionData = { 0 };
    vr::InputPoseActionData_t poseActionData = { 0 };

    int currentKeys = 0;

    //----
    // If we successfully update the actions this frame, use them
    //----
    if (vr::VRInput()->UpdateActionState(&actionSet, setSize, setCount) == vr::VRInputError_None)
    {
        // Movement
        if (vr::VRInput()->GetAnalogActionData(input.game.movement, &analogActionData, sizeof(analogActionData), vr::k_ulInvalidInputValueHandle) == vr::VRInputError_None && analogActionData.bActive == true)
        {
            if (std::fabs(analogActionData.x) > 0.25f)
            {
                if (analogActionData.x < 0)
                    currentKeys |= USED_KEYS_LEFT;
                else if (analogActionData.x > 0)
                    currentKeys |= USED_KEYS_RIGHT;
            }

            if (std::fabs(analogActionData.y) > 0.25f)
            {
                if (analogActionData.y < 0)
                    currentKeys |= USED_KEYS_DOWN;
                else if (analogActionData.y > 0)
                    currentKeys |= USED_KEYS_UP;
            }
        }

        // shoot
        if (vr::VRInput()->GetDigitalActionData(input.game.shoot, &digitalActionData, sizeof(digitalActionData), vr::k_ulInvalidInputValueHandle) == vr::VRInputError_None && digitalActionData.bActive == true)
        {
            if (digitalActionData.bState == true)
                currentKeys |= USED_KEYS_Z;
        }

        // jump
        if (vr::VRInput()->GetDigitalActionData(input.game.jump, &digitalActionData, sizeof(digitalActionData), vr::k_ulInvalidInputValueHandle) == vr::VRInputError_None && digitalActionData.bActive == true)
        {
            if (digitalActionData.bState == true)
                currentKeys |= USED_KEYS_SHIFT;
        }

        // bomb
        if (vr::VRInput()->GetDigitalActionData(input.game.bomb, &digitalActionData, sizeof(digitalActionData), vr::k_ulInvalidInputValueHandle) == vr::VRInputError_None && digitalActionData.bActive == true)
        {
            if (digitalActionData.bState == true)
                currentKeys |= USED_KEYS_X;
        }

        // rotate left
        if (vr::VRInput()->GetDigitalActionData(input.game.rotateleft, &digitalActionData, sizeof(digitalActionData), vr::k_ulInvalidInputValueHandle) == vr::VRInputError_None && digitalActionData.bActive == true)
        {
            if (digitalActionData.bState == true)
                currentKeys |= USED_KEYS_A;
        }

        // rotate right
        if (vr::VRInput()->GetDigitalActionData(input.game.rotateright, &digitalActionData, sizeof(digitalActionData), vr::k_ulInvalidInputValueHandle) == vr::VRInputError_None && digitalActionData.bActive == true)
        {
            if (digitalActionData.bState == true)
                currentKeys |= USED_KEYS_D;
        }

        // rotate up
        if (vr::VRInput()->GetDigitalActionData(input.game.rotateup, &digitalActionData, sizeof(digitalActionData), vr::k_ulInvalidInputValueHandle) == vr::VRInputError_None && digitalActionData.bActive == true)
        {
            if (digitalActionData.bState == true)
                currentKeys |= USED_KEYS_W;
        }

        // rotate down
        if (vr::VRInput()->GetDigitalActionData(input.game.rotatedown, &digitalActionData, sizeof(digitalActionData), vr::k_ulInvalidInputValueHandle) == vr::VRInputError_None && digitalActionData.bActive == true)
        {
            if (digitalActionData.bState == true)
                currentKeys |= USED_KEYS_S;
        }

        // escape
        if (vr::VRInput()->GetDigitalActionData(input.game.escape, &digitalActionData, sizeof(digitalActionData), vr::k_ulInvalidInputValueHandle) == vr::VRInputError_None && digitalActionData.bActive == true)
        {
            if (digitalActionData.bState == true)
                currentKeys |= USED_KEYS_ESC;
        }
    }
    return currentKeys;
}

