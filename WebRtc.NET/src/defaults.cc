
#include "defaults.h"
#include "internals.h"

#include "webrtc/base/bind.h"

namespace cricket
{
	///////////////////////////////////////////////////////////////////////
	// Definition of private class YuvFramesThread that periodically generates
	// frames.
	///////////////////////////////////////////////////////////////////////
	class YuvFramesCapturer2::YuvFramesThread
		: public rtc::Thread, public rtc::MessageHandler
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
			int waiting_time_ms = 0;
			if (capturer_)
			{
				_OnMainForm(true);

				capturer_->ReadFrame(true);
				PostDelayed(waiting_time_ms, this);
				Thread::Run();
			}

			rtc::CritScope cs(&crit_);
			finished_ = true;

			_OnMainForm(false);
		}

		// Override virtual method of parent MessageHandler. Context: Worker Thread.
		virtual void OnMessage(rtc::Message* /*pmsg*/)
		{
			int waiting_time_ms = 200;
			if (capturer_)
			{
				capturer_->ReadFrame(false);
				PostDelayed(waiting_time_ms, this);
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

		RTC_DISALLOW_COPY_AND_ASSIGN(YuvFramesThread);
	};

	/////////////////////////////////////////////////////////////////////
	// Implementation of class YuvFramesCapturer.
	/////////////////////////////////////////////////////////////////////

	const char* YuvFramesCapturer2::kYuvFrameDeviceName = "YuvFramesGenerator";

	// TODO(shaowei): allow width_ and height_ to be configurable.
	YuvFramesCapturer2::YuvFramesCapturer2()
		: frames_generator_thread(NULL),
		width_(752),
		height_(480),
		frame_index_(0),
		barcode_interval_(1),
		startThread_(NULL)
	{
		Init();
	}

	YuvFramesCapturer2::~YuvFramesCapturer2()
	{
		Stop();
		delete[] static_cast<char*>(captured_frame_.data);
	}

	void YuvFramesCapturer2::Init()
	{
		int size = width_ * height_;
		int qsize = size / 4;
		frame_generator_ = new YuvFrameGenerator(width_, height_, true);
		frame_data_size_ = size + 2 * qsize;
		captured_frame_.data = new char[frame_data_size_];
		captured_frame_.fourcc = FOURCC_IYUV;
		captured_frame_.pixel_height = 1;
		captured_frame_.pixel_width = 1;
		captured_frame_.width = width_;
		captured_frame_.height = height_;
		captured_frame_.data_size = frame_data_size_;

		// Enumerate the supported formats. We have only one supported format.
		VideoFormat format(width_, height_, VideoFormat::FpsToInterval(5), FOURCC_IYUV);
		std::vector<VideoFormat> supported;
		supported.push_back(format);
		SetSupportedFormats(supported);
	}

	CaptureState YuvFramesCapturer2::Start(const VideoFormat& capture_format)
	{
		if (IsRunning())
		{
			LOG(LS_ERROR) << "Yuv Frame Generator is already running";
			return CS_FAILED;
		}
		SetCaptureFormat(&capture_format);

		barcode_reference_timestamp_millis_ = rtc::TimeNanos();
		
		startThread_ = rtc::Thread::Current();

		// Create a thread to generate frames.
		frames_generator_thread = new YuvFramesThread(this);
		bool ret = frames_generator_thread->Start();
		if (ret)
		{
			LOG(LS_INFO) << "Yuv Frame Generator started";
			return CS_RUNNING;
		}
		else
		{
			LOG(LS_ERROR) << "Yuv Frame Generator failed to start";
			return CS_FAILED;
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
			frames_generator_thread = NULL;
			LOG(LS_INFO) << "Yuv Frame Generator stopped";
		}
		SetCaptureFormat(NULL);

		startThread_ = NULL;
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
		// 1. Signal the previously read frame to downstream.
		if (!first_frame)
		{
			if (startThread_->IsCurrent())
			{
				SignalFrameCaptured(this, &captured_frame_);
			}
			else
			{
				startThread_->Invoke<void>(rtc::Bind(&YuvFramesCapturer2::SignalFrameCapturedOnStartThread, this, &captured_frame_));
			}

		}

		captured_frame_.time_stamp = rtc::TimeNanos();

		frame_generator_->GenerateNextFrame((uint8_t*)captured_frame_.data, GetBarcodeValue());
	}

	int32_t YuvFramesCapturer2::GetBarcodeValue()
	{
		if (barcode_reference_timestamp_millis_ == -1 ||
			frame_index_ % barcode_interval_ != 0)
		{
			return -1;
		}
		return static_cast<int32_t>(captured_frame_.time_stamp - barcode_reference_timestamp_millis_);
	}

}  // namespace cricket