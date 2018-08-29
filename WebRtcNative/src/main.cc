
#pragma comment(lib,"crypt32.lib")
#pragma comment(lib,"iphlpapi.lib")
#pragma comment(lib,"secur32.lib")
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"dmoguids.lib")
#pragma comment(lib,"wmcodecdspuuid.lib")
#pragma comment(lib,"amstrmid.lib")
#pragma comment(lib,"msdmo.lib")
#pragma comment(lib,"Strmiids.lib")

// common
#pragma comment(lib,"wininet.lib")
#pragma comment(lib,"dnsapi.lib")
#pragma comment(lib,"version.lib")
#pragma comment(lib,"msimg32.lib")
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"usp10.lib")
#pragma comment(lib,"psapi.lib")
#pragma comment(lib,"dbghelp.lib")
#pragma comment(lib,"shlwapi.lib")
#pragma comment(lib,"kernel32.lib")
#pragma comment(lib,"gdi32.lib")
#pragma comment(lib,"winspool.lib")
#pragma comment(lib,"comdlg32.lib")
#pragma comment(lib,"advapi32.lib")
#pragma comment(lib,"shell32.lib")
#pragma comment(lib,"ole32.lib")
#pragma comment(lib,"oleaut32.lib")
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"uuid.lib")
#pragma comment(lib,"odbc32.lib")
#pragma comment(lib,"odbccp32.lib")
#pragma comment(lib,"delayimp.lib")
#pragma comment(lib,"credui.lib")
#pragma comment(lib,"netapi32.lib")

// internal
#pragma comment(lib,"liblept168.lib")
#pragma comment(lib,"webrtc.lib")
#pragma comment(lib,"builtin_audio_decoder_factory.lib")
#pragma comment(lib,"audio_decoder_L16.lib")
#pragma comment(lib,"audio_decoder_g711.lib")
#pragma comment(lib,"audio_decoder_g722.lib")
#pragma comment(lib,"audio_decoder_ilbc.lib")
#pragma comment(lib,"audio_decoder_isac_float.lib")
#pragma comment(lib,"audio_decoder_opus.lib")

#include "internals.h"

#if DESKTOP_CAPTURE
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3d11.lib")
#endif

#include "webrtc/rtc_base/ssladapter.h"
#include "webrtc/rtc_base/win32socketinit.h"
#include "webrtc/rtc_base/win32socketserver.h"

namespace Native
{
	bool CFG_quality_scaler_enabled_ = false;

	void InitializeSSL()
	{
		// rtc::LogMessage::LogToDebug(rtc::LS_VERBOSE);
		rtc::EnsureWinsockInit();
		rtc::InitializeSSL();
	}

	void CleanupSSL()
	{
		rtc::CleanupSSL();
	}
}

FILE _iob[] = { *stdin, *stdout, *stderr };

extern "C" FILE * __cdecl __iob_func(void)
{
	return _iob;
}