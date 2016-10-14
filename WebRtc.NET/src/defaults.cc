
#include "defaults.h"
#include "internals.h"
#include "conductor.h"

class YuvFramesCapturer2::YuvFramesThread : public rtc::Thread, public rtc::MessageHandler
{
public:
	explicit YuvFramesThread(YuvFramesCapturer2* capturer)
		: capturer_(capturer),
		finished_(false)
	{
	}

	virtual ~YuvFramesThread()
	{
		Stop();
	}

	// Override virtual method of parent Thread. Context: Worker Thread.
	virtual void Run()
	{
		// Read the first frame and start the message pump. The pump runs until
		// Stop() is called externally or Quit() is called by OnMessage().
		
		if (capturer_)
		{
			waiting_time_ms = (rtc::kNumMillisecsPerSec / capturer_->con->caputureFps);
			waiting_time_ms *= 0.9;

			capturer_->ReadFrame(true);
			PostDelayed(RTC_FROM_HERE, waiting_time_ms, this);
			Thread::Run();
		}

		rtc::CritScope cs(&crit_);
		finished_ = true;
	}

	// Override virtual method of parent MessageHandler. Context: Worker Thread.
	virtual void OnMessage(rtc::Message* /*pmsg*/)
	{
		if (capturer_)
		{
			capturer_->ReadFrame(false);
			PostDelayed(RTC_FROM_HERE, waiting_time_ms, this);
		}
		else
		{
			Quit();
		}
	}

	// Check if Run() is finished.
	bool Finished() const
	{
		rtc::CritScope cs(&crit_);
		return finished_;
	}

private:
	YuvFramesCapturer2* capturer_;
	mutable rtc::CriticalSection crit_;
	bool finished_;
	int waiting_time_ms;

	RTC_DISALLOW_COPY_AND_ASSIGN(YuvFramesThread);
};

namespace
{
	int I420DataSize(int height, int stride_y, int stride_u, int stride_v)
	{
		return stride_y * height + (stride_u + stride_v) * ((height + 1) / 2);
	}
}

YuvFramesCapturer2::YuvFramesCapturer2(Conductor & c)
	: frames_generator_thread(NULL),
	width_(640),
	height_(360),
	frame_index_(0),
	barcode_interval_(1),
	con(&c)
{
	frame_generator_ = new cricket::YuvFrameGenerator(width_, height_, true);
	
	video_buffer = webrtc::I420Buffer::Create(width_, height_);
	frame_data_size_ = I420DataSize(height_, video_buffer->StrideY(), video_buffer->StrideU(), video_buffer->StrideV());

	video_frame = new cricket::WebRtcVideoFrame(video_buffer, webrtc::VideoRotation::kVideoRotation_0, 0, 0);

	// Enumerate the supported formats. We have only one supported format.
	cricket::VideoFormat format(width_, height_, cricket::VideoFormat::FpsToInterval(con->caputureFps), cricket::FOURCC_IYUV);
	std::vector<cricket::VideoFormat> supported;
	supported.push_back(format);
	SetSupportedFormats(supported);

	set_enable_video_adapter(false);
	//set_square_pixel_aspect_ratio(false);
}

YuvFramesCapturer2::~YuvFramesCapturer2()
{
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

	// Create a thread to generate frames.
	frames_generator_thread = new YuvFramesThread(this);
	bool ret = frames_generator_thread->Start();
	if (ret)
	{
		LOG(LS_INFO) << "Yuv Frame Generator started";
		return cricket::CS_RUNNING;
	}
	else
	{
		LOG(LS_ERROR) << "Yuv Frame Generator failed to start";
		return cricket::CS_FAILED;
	}
}

bool YuvFramesCapturer2::IsRunning()
{
	return frames_generator_thread && !frames_generator_thread->Finished();
}

void YuvFramesCapturer2::Stop()
{
	if (frames_generator_thread)
	{
		frames_generator_thread->Stop();
		delete frames_generator_thread;
		frames_generator_thread = NULL;
		LOG(LS_INFO) << "Yuv Frame Generator stopped";
	}
	SetCaptureFormat(NULL);
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

// Executed in the context of YuvFramesThread.
void YuvFramesCapturer2::ReadFrame(bool first_frame)
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

	if (AdaptFrame(width_,
				   height_,
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
			frame_generator_->GenerateNextFrame((uint8_t*)video_buffer->DataY(), static_cast<int32_t>(rtc::TimeNanos() - barcode_reference_timestamp_millis_));
		}
		else
		{
			con->OnFillBuffer((uint8_t*)video_buffer->DataY(), frame_data_size_);
		}

		video_frame->set_timestamp_us(translated_camera_time_us);

		OnFrame(*video_frame, width_, height_);
	}
}