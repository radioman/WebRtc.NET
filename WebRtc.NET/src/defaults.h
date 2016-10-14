
#ifndef WEBRTC_NET_DEFAULTS_H_
#define WEBRTC_NET_DEFAULTS_H_
#pragma once


#include "webrtc/media/base/videocapturer.h"
#include "webrtc/media/base/yuvframegenerator.h"

class Conductor;

class YuvFramesCapturer2 : public cricket::VideoCapturer
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

protected:
	// Override virtual methods of parent class VideoCapturer.
	virtual bool GetPreferredFourccs(std::vector<uint32_t>* fourccs);

	// Read a frame and determine how long to wait for the next frame.
	void ReadFrame(bool first_frame);

private:
	class YuvFramesThread;  // Forward declaration, defined in .cc.

	Conductor * con;
	YuvFramesThread* frames_generator_thread;
	cricket::YuvFrameGenerator* frame_generator_;	

	rtc::scoped_refptr<webrtc::I420Buffer> video_buffer;
	cricket::VideoFrame * video_frame;
	
	int width_;
	int height_;
	uint32_t frame_data_size_;
	uint32_t frame_index_;

	int64_t barcode_reference_timestamp_millis_;
	int32_t barcode_interval_;

	RTC_DISALLOW_COPY_AND_ASSIGN(YuvFramesCapturer2);
};

#endif  // WEBRTC_NET_DEFAULTS_H_
