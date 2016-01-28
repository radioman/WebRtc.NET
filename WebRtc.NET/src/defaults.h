
#ifndef WEBRTC_EXAMPLES_PEERCONNECTION_CLIENT_DEFAULTS_H_
#define WEBRTC_EXAMPLES_PEERCONNECTION_CLIENT_DEFAULTS_H_
#pragma once

#include "talk/media/base/videocapturer.h"
#include "talk/media/base/yuvframegenerator.h"

// ---------------------------------------------------------------------

namespace rtc
{
	class FileStream;
}

namespace cricket
{
	// Simulated video capturer that periodically reads frames from a file.
	class YuvFramesCapturer2 : public cricket::VideoCapturer
	{
	public:
		YuvFramesCapturer2();
		//YuvFramesCapturer2(int width, int height);
		virtual ~YuvFramesCapturer2();

		static const char* kYuvFrameDeviceName;
		static cricket::Device CreateYuvFramesCapturerDevice()
		{
			std::stringstream id;
			id << kYuvFrameDeviceName;
			return cricket::Device(id.str(), id.str());
		}
		static bool IsYuvFramesCapturerDevice(const cricket::Device& device)
		{
			return rtc::starts_with(device.id.c_str(), kYuvFrameDeviceName);
		}

		void Init();
		// Override virtual methods of parent class VideoCapturer.
		virtual cricket::CaptureState Start(const cricket::VideoFormat& capture_format);
		virtual void Stop();
		virtual bool IsRunning();
		virtual bool IsScreencast() const
		{
			return false;
		}

	protected:
		// Override virtual methods of parent class VideoCapturer.
		virtual bool GetPreferredFourccs(std::vector<uint32_t>* fourccs);

		// Read a frame and determine how long to wait for the next frame.
		void ReadFrame(bool first_frame);

	private:
		class YuvFramesThread;  // Forward declaration, defined in .cc.

		cricket::YuvFrameGenerator* frame_generator_;
		cricket::CapturedFrame captured_frame_;
		YuvFramesThread* frames_generator_thread;
		int width_;
		int height_;
		uint32_t frame_data_size_;
		uint32_t frame_index_;

		int64_t barcode_reference_timestamp_millis_;
		int32_t barcode_interval_;
		int32_t GetBarcodeValue();

		RTC_DISALLOW_COPY_AND_ASSIGN(YuvFramesCapturer2);
	};

	class VideoCapturerFactoryCustom : public cricket::VideoDeviceCapturerFactory
	{
	public:
		VideoCapturerFactoryCustom()
		{
		}
		virtual ~VideoCapturerFactoryCustom()
		{
		}

		virtual cricket::VideoCapturer* Create(const cricket::Device& device)
		{
			// XXX: WebRTC uses device name to instantiate the capture, which is always 0.
			return new YuvFramesCapturer2();
		}
	};

} // namespace videocapture

#endif  // WEBRTC_EXAMPLES_PEERCONNECTION_CLIENT_DEFAULTS_H_
