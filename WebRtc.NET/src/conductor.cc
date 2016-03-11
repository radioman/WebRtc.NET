
#include "defaults.h"
#include "conductor.h"

#include "webrtc/api/videosourceinterface.h"
#include "webrtc/api/test/fakeconstraints.h"

#include "webrtc/video_encoder.h"
#include "webrtc/modules/video_coding/codecs/vp8/simulcast_encoder_adapter.h"
#include "webrtc/media/engine/webrtcvideoencoderfactory.h"
#include "webrtc/modules/video_coding/codecs/vp8/include/vp8.h"

const char kVideoLabel[] = "video_label";
const char kStreamLabel[] = "stream_label";

namespace
{
	// Wrap cricket::WebRtcVideoEncoderFactory as a webrtc::VideoEncoderFactory.
	class EncoderFactoryAdapter : public webrtc::VideoEncoderFactory
	{
	public:
		EncoderFactoryAdapter()
		{
		}

		virtual ~EncoderFactoryAdapter()
		{
		}

		// Implement webrtc::VideoEncoderFactory.
		webrtc::VideoEncoder* Create() override
		{
			return webrtc::VP8Encoder::Create();
		}

		void Destroy(webrtc::VideoEncoder* encoder) override
		{
			delete encoder;
		}

	private:

	};

	// An encoder factory that wraps Create requests for simulcastable codec types
	// with a webrtc::SimulcastEncoderAdapter. Non simulcastable codec type
	// requests are just passed through to the contained encoder factory.
	class WebRtcSimulcastEncoderFactory : public cricket::WebRtcVideoEncoderFactory
	{
	public:
		WebRtcSimulcastEncoderFactory()
		{
			VideoCodec c(webrtc::kVideoCodecVP8, "VP8", 752, 640, 10);
			codecs_.push_back(c);
		}

		webrtc::VideoEncoder* CreateVideoEncoder(webrtc::VideoCodecType type) override
		{
			// If it's a codec type we can simulcast, create a wrapped encoder.
			if (type == webrtc::kVideoCodecVP8)
			{
				return new webrtc::SimulcastEncoderAdapter(new EncoderFactoryAdapter());
			}
			return NULL;
		}

		const std::vector<VideoCodec>& codecs() const override
		{
			return codecs_;
		}

		bool EncoderTypeHasInternalSource(webrtc::VideoCodecType type) const override
		{
			return false;
		}

		void DestroyVideoEncoder(webrtc::VideoEncoder* encoder) override
		{
			// Otherwise, SimulcastEncoderAdapter can be deleted directly, and will call
			// DestroyVideoEncoder on the factory for individual encoder instances.
			delete encoder;
		}

	private:

		std::vector<VideoCodec> codecs_;
	};
}
//----------------------------------------------------------

Conductor::Conductor()
{
	onError = nullptr;
	onSuccess = nullptr;
	onFailure = nullptr;
	onIceCandidate = nullptr;
	capturer = nullptr;
	worker_thread_ = nullptr;
}

Conductor::~Conductor()
{
	DeletePeerConnection();

	ASSERT(peer_connection_ == nullptr);
}

void Conductor::DeletePeerConnection()
{
	if (peer_connection_)
	{
		for (auto it = active_streams_.begin(); it != active_streams_.end(); ++it)
		{
			peer_connection_->RemoveStream(it->second);
		}
		active_streams_.clear();

		peer_connection_ = nullptr;
	}

	worker_thread_ = nullptr;
	pc_factory_ = nullptr;
	capturer = nullptr;
}

bool Conductor::InitializePeerConnection()
{
	ASSERT(pc_factory_ == nullptr);
	ASSERT(peer_connection_ == nullptr);

	//worker_thread_ = new rtc::Thread();
	//worker_thread_->Start();

	//pc_factory_ = webrtc::CreatePeerConnectionFactory(
	//	worker_thread_,
	//	rtc::ThreadManager::Instance()->CurrentThread(),
	//	NULL,
	//	new WebRtcSimulcastEncoderFactory(),
	//	NULL);

	pc_factory_ = webrtc::CreatePeerConnectionFactory();

	if (!pc_factory_)
	{
		DeletePeerConnection();
		return false;
	}

	webrtc::PeerConnectionFactoryInterface::Options opt;
	{
		//opt.disable_encryption = true;
		opt.disable_sctp_data_channels = true;
		pc_factory_->SetOptions(opt);
	}

	if (!CreatePeerConnection(true))
	{
		DeletePeerConnection();
		return false;
	}
	AddStreams();
	return peer_connection_ != nullptr;
}

bool Conductor::CreatePeerConnection(bool dtls)
{
	ASSERT(pc_factory_ != nullptr);
	ASSERT(peer_connection_ == nullptr);

	webrtc::PeerConnectionInterface::RTCConfiguration config;

	//webrtc::PeerConnectionInterface::IceServer server;
	//server.uri = "stun:stun.l.google.com:19302";
	//config.servers.push_back(server);

	webrtc::FakeConstraints constraints;
	if (dtls)
	{
		constraints.AddOptional(webrtc::MediaConstraintsInterface::kEnableDtlsSrtp, "true");
	}
	else
	{
		constraints.AddOptional(webrtc::MediaConstraintsInterface::kEnableDtlsSrtp, "false");
	}
	constraints.SetMandatoryReceiveVideo(false);
	constraints.SetMandatoryReceiveAudio(false);
	constraints.AddMandatory(webrtc::MediaConstraintsInterface::kVoiceActivityDetection, "false");

	peer_connection_ = pc_factory_->CreatePeerConnection(config, &constraints, NULL, NULL, this);
	return peer_connection_ != nullptr;
}

void Conductor::CreateOffer()
{
	peer_connection_->CreateOffer(this, nullptr);
}

void Conductor::OnOfferReply(std::string type, std::string sdp)
{
	webrtc::SdpParseError error;
	webrtc::SessionDescriptionInterface* session_description(webrtc::CreateSessionDescription(type, sdp, &error));
	if (!session_description)
	{
		LOG(WARNING) << "Can't parse received session description message. " << "SdpParseError was: " << error.description;
		return;
	}
	peer_connection_->SetRemoteDescription(this, session_description);
}

void Conductor::OnOfferRequest(std::string sdp)
{
	webrtc::SdpParseError error;
	webrtc::SessionDescriptionInterface* session_description(webrtc::CreateSessionDescription("offer", sdp, &error));
	if (!session_description)
	{
		LOG(WARNING) << "Can't parse received session description message. " << "SdpParseError was: " << error.description;
		return;
	}
	peer_connection_->SetRemoteDescription(this, session_description);
	peer_connection_->CreateAnswer(this, nullptr);
}

bool Conductor::AddIceCandidate(std::string sdp_mid, int sdp_mlineindex, std::string sdp)
{
	webrtc::SdpParseError error;
	rtc::scoped_ptr<webrtc::IceCandidateInterface> candidate(
		webrtc::CreateIceCandidate(sdp_mid, sdp_mlineindex, sdp, &error));

	if (!candidate.get())
	{
		LOG(WARNING) << "Can't parse received candidate message. "
			<< "SdpParseError was: " << error.description;
		return false;
	}
	if (!peer_connection_->AddIceCandidate(candidate.get()))
	{
		LOG(WARNING) << "Failed to apply the received candidate";
		return false;
	}
	return true;
}

// ...

void Conductor::OnFillBuffer(uint8_t * frame_buffer, uint32_t yuvSize)
{
	if (onFillBuffer)
	{
		onFillBuffer(frame_buffer, yuvSize);
	}
}

bool Conductor::OpenVideoCaptureDevice()
{
	if (!capturer)
	{
		rtc::scoped_ptr<cricket::DeviceManagerInterface> dev_manager(cricket::DeviceManagerFactory::Create());
		if (!dev_manager->Init())
		{
			LOG(LS_ERROR) << "Can't create device manager";
			return false;
		}

		std::vector<cricket::Device> devs;
		if (!dev_manager->GetVideoCaptureDevices(&devs))
		{
			LOG(LS_ERROR) << "Can't enumerate video devices";
			return false;
		}

		for (auto dev : devs)
		{
			capturer = dev_manager->CreateVideoCapturer(dev);
			if (capturer != nullptr)
				return true;
		}

		if (capturer == nullptr)
		{
			LOG(LS_ERROR) << "Capturer is NULL!";
			return false;
		}
	}

	LOG(LS_ERROR) << "Capturer != NULL!";
	return false;
}

void Conductor::AddStreams()
{
	if (active_streams_.find(kStreamLabel) != active_streams_.end())
		return;  // Already added.

	//webrtc::AudioTrackInterface * audio_track = peer_connection_factory_->CreateAudioTrack(
	//		kAudioLabel, peer_connection_factory_->CreateAudioSource(NULL));

	if (!capturer)
	{
		capturer = new YuvFramesCapturer2(*this);
	}

	auto v = pc_factory_->CreateVideoSource(capturer, NULL);

	auto video_track = pc_factory_->CreateVideoTrack(kVideoLabel, v);
	//main_wnd_->StartLocalRenderer(video_track);

	auto stream = pc_factory_->CreateLocalMediaStream(kStreamLabel);
	{
		//stream->AddTrack(audio_track);
		stream->AddTrack(video_track);

		if (!peer_connection_->AddStream(stream))
		{
			LOG(LS_ERROR) << "Adding stream to PeerConnection failed";
		}
		typedef std::pair<std::string, rtc::scoped_refptr<webrtc::MediaStreamInterface> >	MediaStreamPair;
		active_streams_.insert(MediaStreamPair(stream->label(), stream));
	}
}

//
// PeerConnectionObserver implementation.
//

// Called when a remote stream is added
void Conductor::OnAddStream(webrtc::MediaStreamInterface* stream)
{
	LOG(INFO) << __FUNCTION__ << " " << stream->label();

	//stream->AddRef();
	//main_wnd_->QueueUIThreadCallback(NEW_STREAM_ADDED, stream);

	//webrtc::MediaStreamInterface* stream = reinterpret_cast<webrtc::MediaStreamInterface*>(data);
	//webrtc::VideoTrackVector tracks = stream->GetVideoTracks();
	//// Only render the first track.
	//if (!tracks.empty())
	//{
	//	webrtc::VideoTrackInterface* track = tracks[0];
	//	//main_wnd_->StartRemoteRenderer(track);
	//}
	//stream->Release();
}

void Conductor::OnRemoveStream(webrtc::MediaStreamInterface* stream)
{
	LOG(INFO) << __FUNCTION__ << " " << stream->label();
	//stream->AddRef();
	//main_wnd_->QueueUIThreadCallback(STREAM_REMOVED, stream);

	// Remote peer stopped sending a stream.
	//webrtc::MediaStreamInterface* stream = reinterpret_cast<webrtc::MediaStreamInterface*>(stream);
	//stream->Release();
}

void Conductor::OnIceCandidate(const webrtc::IceCandidateInterface* candidate)
{
	LOG(INFO) << __FUNCTION__ << " " << candidate->sdp_mline_index();

	std::string sdp;
	if (!candidate->ToString(&sdp))
	{
		LOG(LS_ERROR) << "Failed to serialize candidate";
		return;
	}

	if (onIceCandidate != nullptr)
	{
		onIceCandidate(candidate->sdp_mid().c_str(), candidate->sdp_mline_index(), sdp.c_str());
	}
}

void Conductor::OnSuccess(webrtc::SessionDescriptionInterface* desc)
{
	peer_connection_->SetLocalDescription(this, desc);

	std::string sdp;
	desc->ToString(&sdp);

	if (onSuccess != nullptr)
	{
		onSuccess(desc->type().c_str(), sdp.c_str());
	}
}

void Conductor::OnFailure(const std::string& error)
{
	LOG(LERROR) << error;

	if (onFailure != nullptr)
	{
		onFailure(error.c_str());
	}
}

void Conductor::OnError()
{
	if (onError != nullptr)
	{
		onError();
	}
}