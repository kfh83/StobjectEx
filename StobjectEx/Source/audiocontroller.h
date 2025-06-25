#ifndef _AUDIOCONTROLLER_H
#define _AUDIOCONTROLLER_H

#include <Endpointvolume.h>
#include <mmdeviceapi.h>

#define MYWM_AUDIODEV_CHANGE (WM_APP + 100+7)
#define MYWM_AUDIOVOL_CHANGE (WM_APP + 101+8)

// :)
static const CLSID CLSID_MMDeviceEnumerator = { 0xBCDE0395, 0xE52F, 0x467C, {0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E } };
static const IID IID_IAudioEndpointVolumeCallback = { 0x657804FA, 0xD6AD, 0x4496, {0x8A, 0x60, 0x35, 0x27, 0x52, 0xAF, 0x4F, 0x89} };
static const IID IID_IAudioEndpointVolume = { 0x5CDF2C82, 0x841E, 0x4546, {0x97, 0x22, 0x0C, 0xF7, 0x40, 0x78, 0x22, 0x9A} };
static const IID IID_IMMDeviceEnumerator = { 0xA95664D2, 0x9614, 0x4F35, {0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6} };
static const IID IID_IMMNotificationClient = { 0x7991EEC9, 0x7E89, 0x4D85, {0x83, 0x90, 0x6C, 0x70, 0x3C, 0xEC, 0x60, 0xC0} };

typedef struct
{
	IMMNotificationClient* pIMMNotificationClient;
	IAudioEndpointVolumeCallback* pIAudioEndpointVolumeCallback;

	IMMDevice* pIMMDevice;
	IMMDeviceEnumerator* pIMMDeviceEnumerator;
	IAudioEndpointVolume* pIAudioEndpointVolume;

	ULONG ref;
	HWND hwnd;
} AudioController;

HRESULT AudioController_Initialize(AudioController* this, HWND hwnd);
void AudioController_Dispose(AudioController* this);
void AudioController_Reattach(AudioController* this);

BOOL AudioController_GetMute(AudioController* this);
float AudioController_GetVolume(AudioController* this);
BOOL AudioController_AttachStatus(AudioController* this);

void AudioController_SetMute(AudioController* this, BOOL bMute);
void AudioController_SetVolume(AudioController* this, float fVol);

#endif
