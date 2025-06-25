#include "audiocontroller.h"

//
// IUnknown
//

HRESULT AudioController_QueryInterface(AudioController* this, REFIID iid, void** ppUnk)
{
	if (IsEqualIID(&IID_IUnknown, iid) || IsEqualIID(&IID_IMMNotificationClient, iid))
	{
		*ppUnk = &this->pIMMNotificationClient->lpVtbl;
	}
	else if (IsEqualIID(&IID_IAudioEndpointVolumeCallback, iid))
	{
		*ppUnk = &this->pIAudioEndpointVolumeCallback->lpVtbl;
	}
	else
	{
		*ppUnk = NULL;
		return E_NOINTERFACE;
	}

	return S_OK;
}

ULONG AudioController_AddRef(AudioController* this)
{
	return InterlockedIncrement(&this->ref);
}

ULONG AudioController_Release(AudioController* this)
{
	return InterlockedDecrement(&this->ref);
}

HRESULT AudioController_IAudioEndpointVolumeCallback_QueryInterface(void* this, REFIID iid, void** ppUnk)
{
	return AudioController_QueryInterface((IAudioEndpointVolumeCallback*)((char*)this - sizeof(IAudioEndpointVolumeCallback)), iid, ppUnk);
}

ULONG AudioController_IAudioEndpointVolumeCallback_AddRef(void* this)
{
	return AudioController_AddRef((IAudioEndpointVolumeCallback*)((char*)this - sizeof(IAudioEndpointVolumeCallback)));
}

ULONG AudioController_IAudioEndpointVolumeCallback_Release(void* this)
{
	return AudioController_Release((IAudioEndpointVolumeCallback*)((char*)this - sizeof(IAudioEndpointVolumeCallback)));
}

//
// IMMNotificationClient
//

HRESULT AudioController_OnDeviceStateChanged(AudioController* this, LPCWSTR pwstrDeviceId, DWORD dwNewState)
{
	return S_OK;
}

HRESULT AudioController_OnDeviceAdded(AudioController* this, LPCWSTR pwstrDeviceId)
{
	return S_OK;
}

HRESULT AudioController_OnDeviceRemoved(AudioController* this, LPCWSTR pwstrDeviceId)
{
	return S_OK;
}

HRESULT AudioController_OnDefaultDeviceChanged(AudioController* this, EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId)
{
	if (flow == eRender) {
		if (this->hwnd != NULL)
		{
			PostMessage(this->hwnd, MYWM_AUDIODEV_CHANGE, 0, 0);
		}
	}
	return S_OK;
}

HRESULT AudioController_OnPropertyValueChanged(AudioController* this, LPCWSTR pwstrDeviceId, const PROPERTYKEY key) 
{ 
	return S_OK; 
}

//
// IAudioEndpointVolumeCallback
//

HRESULT AudioController_IAudioEndpointVolumeCallback_OnNotify(AudioController* this, PAUDIO_VOLUME_NOTIFICATION_DATA pNotify)
{
	this = (IAudioEndpointVolumeCallback*)((char*)this - sizeof(IAudioEndpointVolumeCallback));
	if (this->hwnd != NULL)
	{
		BOOL ret = PostMessage(this->hwnd, MYWM_AUDIOVOL_CHANGE, 0, 0);
	}
	return S_OK;
}

static CONST_VTBL IMMNotificationClientVtbl AudioController_IMMNotificationClient_Vtbl = 
{
	AudioController_QueryInterface,
	AudioController_AddRef,
	AudioController_Release,
	AudioController_OnDeviceStateChanged,
	AudioController_OnDeviceAdded,
	AudioController_OnDeviceRemoved,
	AudioController_OnDefaultDeviceChanged,
	AudioController_OnPropertyValueChanged
};

static CONST_VTBL IAudioEndpointVolumeCallbackVtbl AudioController_IAudioEndpointVolumeCallback_Vtbl = 
{
	AudioController_IAudioEndpointVolumeCallback_QueryInterface,
	AudioController_IAudioEndpointVolumeCallback_AddRef,
	AudioController_IAudioEndpointVolumeCallback_Release,
	AudioController_IAudioEndpointVolumeCallback_OnNotify
};

///
/// AudioController
///

HRESULT AudioController_Initialize(AudioController* this, HWND hwnd)
{
	if (!this || !hwnd)
	{
		return E_INVALIDARG;
	}

	HRESULT hr;

	this->pIMMNotificationClient = &AudioController_IMMNotificationClient_Vtbl;
	this->hwnd = hwnd;

	this->pIMMDeviceEnumerator = NULL;

	hr = CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_INPROC_SERVER, &IID_IMMDeviceEnumerator, (void**)&this->pIMMDeviceEnumerator );
	if (SUCCEEDED(hr))
	{
		hr = this->pIMMDeviceEnumerator->lpVtbl->RegisterEndpointNotificationCallback(
			this->pIMMDeviceEnumerator, &this->pIMMNotificationClient);
		if (SUCCEEDED(hr))
		{
			hr = AudioController_Attach(this);
		}
	}

	return hr;
}

HRESULT AudioController_Attach(AudioController* this)
{
	HRESULT hr;

	this->pIAudioEndpointVolumeCallback = &AudioController_IAudioEndpointVolumeCallback_Vtbl;

	hr = this->pIMMDeviceEnumerator->lpVtbl->GetDefaultAudioEndpoint(this->pIMMDeviceEnumerator, eRender, eMultimedia, &this->pIMMDevice);
	if (SUCCEEDED(hr))
	{
		hr = this->pIMMDevice->lpVtbl->Activate(
			this->pIMMDevice, &IID_IAudioEndpointVolume, CLSCTX_INPROC_SERVER, NULL, (void**)&this->pIAudioEndpointVolume);
		if (SUCCEEDED(hr))
		{
			hr = this->pIAudioEndpointVolume->lpVtbl->RegisterControlChangeNotify(
				this->pIAudioEndpointVolume, &this->pIAudioEndpointVolumeCallback);
		}
	}
	else
	{
		this->pIAudioEndpointVolume = NULL;
		this->pIMMDevice = NULL;
	}

	return hr;
}

void AudioController_Detach(AudioController* this)
{
	if (this->pIAudioEndpointVolume != NULL)
	{
		this->pIAudioEndpointVolume->lpVtbl->UnregisterControlChangeNotify(
			this->pIAudioEndpointVolume, &this->pIAudioEndpointVolumeCallback);
		this->pIAudioEndpointVolume->lpVtbl->Release(this->pIAudioEndpointVolume);
		this->pIAudioEndpointVolume = NULL;
	}

	if (this->pIMMDevice != NULL)
	{
		this->pIMMDevice->lpVtbl->Release(this->pIMMDevice);
		this->pIMMDevice = NULL;

	}
}

void AudioController_Dispose(AudioController* this)
{
	AudioController_Detach(this);
	if (this->pIMMDeviceEnumerator != NULL)
	{
		this->pIMMDeviceEnumerator->lpVtbl->UnregisterEndpointNotificationCallback(
			this->pIMMDeviceEnumerator, &this->pIMMNotificationClient);
	}
}

void AudioController_Reattach(AudioController* this)
{
	AudioController_Detach(this);
	AudioController_Attach(this);
}

BOOL AudioController_GetMute(AudioController* this)
{
	BOOL bMute = TRUE;

	if (this->pIAudioEndpointVolume != NULL)
	{
		this->pIAudioEndpointVolume->lpVtbl->GetMute(this->pIAudioEndpointVolume, &bMute);
	}

	return bMute;
}

float AudioController_GetVolume(AudioController* this)
{
	float fVol = 0.f;

	if (this->pIAudioEndpointVolume != NULL)
	{
		this->pIAudioEndpointVolume->lpVtbl->GetMasterVolumeLevelScalar(this->pIAudioEndpointVolume, &fVol);
	}

	return fVol;
}

BOOL AudioController_AttachStatus(AudioController* this)
{
	return this->pIAudioEndpointVolume != NULL && this->pIMMDevice != NULL;
}

void AudioController_SetMute(AudioController* this, BOOL bMute)
{
	if (this->pIAudioEndpointVolume != NULL)
	{
		this->pIAudioEndpointVolume->lpVtbl->SetMute(this->pIAudioEndpointVolume, bMute, NULL);
	}
}

void AudioController_SetVolume(AudioController* this, float fVol)
{
	if (this->pIAudioEndpointVolume != NULL)
	{
		this->pIAudioEndpointVolume->lpVtbl->SetMasterVolumeLevelScalar(this->pIAudioEndpointVolume, fVol, NULL);
	}
}
