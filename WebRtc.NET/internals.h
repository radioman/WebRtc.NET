
#pragma once

#ifndef WEBRTC_NET_INTERNALS_H_
#define WEBRTC_NET_INTERNALS_H_

// enable webrtc::DesktopCapturer
#define DESKTOP_CAPTURE 0

namespace Internal
{
	void Encode(unsigned char * data, unsigned int size, int part_idx, bool keyFrame);
}

namespace Native
{
	extern bool CFG_quality_scaler_enabled_;

	void InitializeSSL();
	void CleanupSSL();
}

#endif // WEBRTC_NET_INTERNALS_H_


