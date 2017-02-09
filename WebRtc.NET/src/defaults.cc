
#include "defaults.h"
#include "internals.h"
#include "conductor.h"


#include "webrtc/modules/desktop_capture/desktop_capture_options.h"

namespace Native
{
	int I420DataSize(int height, int stride_y, int stride_u, int stride_v)
	{
		return stride_y * height + (stride_u + stride_v) * ((height + 1) / 2);
	}

	YuvFramesCapturer2::YuvFramesCapturer2(Conductor & c) :
		barcode_interval_(1),
		frame_generator_(nullptr),
#if DESKTOP_CAPTURE
		desktop_capturer(nullptr),
#endif
		run(false),
		con(&c)
	{
		video_buffer = webrtc::I420Buffer::Create(con->width_, con->height_);
		frame_data_size_ = I420DataSize(con->height_, video_buffer->StrideY(), video_buffer->StrideU(), video_buffer->StrideV());

		video_frame = new webrtc::VideoFrame(video_buffer, 0, 0, webrtc::VideoRotation::kVideoRotation_0);

		// Enumerate the supported formats. We have only one supported format.
		cricket::VideoFormat format(con->width_, con->height_, cricket::VideoFormat::FpsToInterval(con->caputureFps), cricket::FOURCC_IYUV);
		std::vector<cricket::VideoFormat> supported;
		supported.push_back(format);
		SetSupportedFormats(supported);

		set_enable_video_adapter(false);
		//set_square_pixel_aspect_ratio(false);
	}

	YuvFramesCapturer2::~YuvFramesCapturer2()
	{
		if (video_frame)
		{
			delete video_frame;
			video_frame = nullptr;
		}

		if (frame_generator_)
		{
			delete frame_generator_;
		}
		Stop();
	}

	cricket::CaptureState YuvFramesCapturer2::Start(const cricket::VideoFormat& capture_format)
	{
		if (IsRunning())
		{
			LOG(LS_ERROR) << "Yuv Frame Generator is already running";
			return cricket::CS_FAILED;
		}
		SetCaptureFormat(&capture_format);

		barcode_reference_timestamp_millis_ = rtc::TimeNanos();
		run = true;

#if DESKTOP_CAPTURE
		{
			webrtc::DesktopCaptureOptions co;
			co.set_allow_directx_capturer(true);
			desktop_capturer = webrtc::DesktopCapturer::CreateScreenCapturer(co);

			desktop_capturer->GetSourceList(&desktop_screens);
			for each(auto & s in desktop_screens)
			{
				LOG(INFO) << "screen: " << s.id << " -> " << s.title;
			}
			desktop_capturer->SelectSource(desktop_screens[0].id);
			desktop_capturer->Start(this);
		}
#endif

		LOG(LS_INFO) << "Yuv Frame Generator started";
		return cricket::CS_RUNNING;
	}

	bool YuvFramesCapturer2::IsRunning()
	{
		return run;
	}

	void YuvFramesCapturer2::Stop()
	{
		run = false;
		SetCaptureFormat(nullptr);
	}

	bool YuvFramesCapturer2::GetPreferredFourccs(std::vector<uint32_t>* fourccs)
	{
		if (!fourccs)
		{
			return false;
		}
		fourccs->push_back(GetSupportedFormats()->at(0).fourcc);
		return true;
	}

	void YuvFramesCapturer2::PushFrame()
	{
		int64_t camera_time_us = rtc::TimeMicros();
		int64_t system_time_us = camera_time_us;
		int out_width;
		int out_height;
		int crop_width;
		int crop_height;
		int crop_x;
		int crop_y;
		int64_t translated_camera_time_us;

		if (AdaptFrame(con->width_,
					   con->height_,
					   camera_time_us,
					   system_time_us,
					   &out_width,
					   &out_height,
					   &crop_width,
					   &crop_height,
					   &crop_x,
					   &crop_y,
					   &translated_camera_time_us))
		{
			if (con->barcodeEnabled)
			{
				if (frame_generator_ == nullptr)
				{
					frame_generator_ = new cricket::YuvFrameGenerator(con->width_, con->height_, true);
				}
				frame_generator_->GenerateNextFrame((uint8_t*)video_buffer->DataY(), static_cast<int32_t>(rtc::TimeNanos() - barcode_reference_timestamp_millis_));
			}

			video_frame->set_timestamp_us(translated_camera_time_us);

			OnFrame(*video_frame, con->width_, con->height_);
		}
	}

#if DESKTOP_CAPTURE
	void YuvFramesCapturer2::CaptureFrame()
	{
		if (desktop_capturer)
		{
			desktop_capturer->CaptureFrame();
		}
	}

	// webrtc::DesktopCapturer::Callback implementation
	void YuvFramesCapturer2::OnCaptureResult(webrtc::DesktopCapturer::Result result, std::unique_ptr<webrtc::DesktopFrame> frame)
	{
		if (desktop_capturer)
		{
			desktop_frame.reset(frame.release());	
		}
	}
#endif

	// VideoSinkInterface implementation
	void VideoRenderer::OnFrame(const webrtc::VideoFrame& frame)
	{
		if (remote && con->onRenderRemote)
		{
			auto b = frame.video_frame_buffer();
			con->onRenderRemote((uint8_t*)b->DataY(), b->width(), b->height());
		}
		else if (con->onRenderLocal)
		{
			auto b = frame.video_frame_buffer();
			con->onRenderLocal((uint8_t*)b->DataY(), b->width(), b->height());
		}
	}

	// AudioTrackSinkInterface implementation
	void AudioRenderer::OnData(const void* audio_data,
							   int bits_per_sample,
							   int sample_rate,
							   size_t number_of_channels,
							   size_t number_of_frames)
	{
		std::stringstream s;
		s << "AudioRenderer::OnData, bps: " << bits_per_sample 
		<< ", rate: " << sample_rate 
		<< ", channels: " << number_of_channels
		<< ", frames: " << number_of_frames;
		
		::OutputDebugStringA(s.str().c_str());
	}
}