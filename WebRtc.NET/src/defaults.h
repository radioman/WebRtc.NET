
#ifndef WEBRTC_NET_DEFAULTS_H_
#define WEBRTC_NET_DEFAULTS_H_
#pragma once

#include "webrtc/media/base/videocapturer.h"
#include "webrtc/media/base/yuvframegenerator.h"
#include "webrtc/api/mediastreaminterface.h"
#include "webrtc/modules/desktop_capture/desktop_capturer.h"

#include "internals.h"

namespace Native
{
	class Conductor;

	class YuvFramesCapturer2 : public cricket::VideoCapturer
#if DESKTOP_CAPTURE
		, webrtc::DesktopCapturer::Callback
#endif
	{
	public:
		YuvFramesCapturer2(Conductor & c);
		virtual ~YuvFramesCapturer2();

		// Override virtual methods of parent class VideoCapturer.
		virtual cricket::CaptureState Start(const cricket::VideoFormat& capture_format);
		virtual void Stop();
		virtual bool IsRunning();
		virtual bool IsScreencast() const
		{
			return false;
		}

		void PushFrame();

#if DESKTOP_CAPTURE
		void CaptureFrame();
		virtual void OnCaptureResult(webrtc::DesktopCapturer::Result result, std::unique_ptr<webrtc::DesktopFrame> frame);
		std::unique_ptr<webrtc::DesktopFrame> desktop_frame;
		webrtc::DesktopCapturer::SourceList desktop_screens;
#endif
		rtc::scoped_refptr<webrtc::I420Buffer> video_buffer;
		uint32_t frame_data_size_;

	protected:

		virtual bool GetPreferredFourccs(std::vector<uint32_t>* fourccs);

	private:

		Conductor * con;
		cricket::YuvFrameGenerator* frame_generator_;		
		webrtc::VideoFrame * video_frame;

		int64_t barcode_reference_timestamp_millis_;
		int32_t barcode_interval_;
		bool run;

#if DESKTOP_CAPTURE
		std::unique_ptr<webrtc::DesktopCapturer> desktop_capturer;
#endif
	};

	class VideoRenderer : public rtc::VideoSinkInterface<webrtc::VideoFrame>
	{
	public:
		VideoRenderer(Conductor & c, bool remote, webrtc::VideoTrackInterface * track_to_render) :
			rendered_track_(track_to_render), con(&c), remote(remote)
		{
			rendered_track_->AddOrUpdateSink(this, rtc::VideoSinkWants());
		}
		virtual ~VideoRenderer()
		{
			if(rendered_track_.get())
				rendered_track_->RemoveSink(this);
		}

		// VideoSinkInterface implementation
		void OnFrame(const webrtc::VideoFrame& frame) override;

	protected:

		bool remote;
		Conductor * con;
		rtc::scoped_refptr<webrtc::VideoTrackInterface> rendered_track_;		
	};

	class AudioRenderer : public webrtc::AudioTrackSinkInterface
	{
	public:
		AudioRenderer(Conductor & c, bool remote, webrtc::AudioTrackInterface * track_to_render) :
			audio_track_(track_to_render), con(&c), remote(remote)
		{
			audio_track_->AddSink(this);
		}
		virtual ~AudioRenderer()
		{
			audio_track_->RemoveSink(this);
		}

		virtual void OnData(const void* audio_data,
							int bits_per_sample,
							int sample_rate,
							size_t number_of_channels,
							size_t number_of_frames) override;

	protected:

		bool remote;
		Conductor * con;
		rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track_;
	};	
}
#endif  // WEBRTC_NET_DEFAULTS_H_
