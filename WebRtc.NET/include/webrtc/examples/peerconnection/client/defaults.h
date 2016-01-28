/*
 *  Copyright 2011 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_EXAMPLES_PEERCONNECTION_CLIENT_DEFAULTS_H_
#define WEBRTC_EXAMPLES_PEERCONNECTION_CLIENT_DEFAULTS_H_
#pragma once

#include <string>

#include "webrtc/base/basictypes.h"

////////
#include "talk/media/base/videocapturer.h"
#include "talk/media/devices/yuvframescapturer.h"
#include "webrtc/base/common.h"
////////

extern const char kAudioLabel[];
extern const char kVideoLabel[];
extern const char kStreamLabel[];
extern const uint16_t kDefaultServerPort;

std::string GetEnvVarOrDefault(const char* env_var_name,
                               const char* default_value);
std::string GetPeerConnectionString();
std::string GetDefaultServerName();
std::string GetPeerName();

// GYP_DEFINES=component=shared_library
// ---------------------------------------------------------------------

namespace videocapture
{
	class CustomVideoCapturer : public cricket::VideoCapturer
	{
	public:
		CustomVideoCapturer(int deviceId);
		virtual ~CustomVideoCapturer();

		// cricket::VideoCapturer implementation.
		virtual cricket::CaptureState Start(const cricket::VideoFormat& capture_format) override;
		virtual void Stop() override;
		virtual bool IsRunning() override;
		virtual bool GetPreferredFourccs(std::vector<uint32_t>* fourccs) override;
		virtual bool GetBestCaptureFormat(const cricket::VideoFormat& desired, cricket::VideoFormat* best_format) override;
		virtual bool IsScreencast() const override;

	private:
		RTC_DISALLOW_COPY_AND_ASSIGN(CustomVideoCapturer);

		static void* grabCapture(void* arg);

		//to call the SignalFrameCaptured call on the main thread
		void SignalFrameCapturedOnStartThread(const cricket::CapturedFrame* frame);

		//cv::VideoCapture m_VCapture; //opencv capture object
		rtc::Thread*  m_startThread; //video capture thread
	};

	class VideoCapturerFactoryCustom : public cricket::VideoDeviceCapturerFactory
	{
	public:
		VideoCapturerFactoryCustom() {}
		virtual ~VideoCapturerFactoryCustom() {}

		virtual cricket::VideoCapturer* Create(const cricket::Device& device)
		{
			// XXX: WebRTC uses device name to instantiate the capture, which is always 0.
			//return new CustomVideoCapturer(atoi(device.id.c_str()));
			return new cricket::YuvFramesCapturer();
		}
	};

} // namespace videocapture

#endif  // WEBRTC_EXAMPLES_PEERCONNECTION_CLIENT_DEFAULTS_H_
