
#include "defaults.h"
#include "conductor.h"

#include "talk/app/webrtc/videosourceinterface.h"
#include "talk/app/webrtc/test/fakeconstraints.h"

 // Names used for a IceCandidate JSON object.
const char kCandidateSdpMidName[] = "sdpMid";
const char kCandidateSdpMlineIndexName[] = "sdpMLineIndex";
const char kCandidateSdpName[] = "candidate";

// Names used for a SessionDescription JSON object.
const char kSessionDescriptionTypeName[] = "type";
const char kSessionDescriptionSdpName[] = "sdp";

const char kVideoLabel[] = "video_label";
const char kStreamLabel[] = "stream_label";

#define DTLS_ON  true
#define DTLS_OFF false

Conductor::Conductor()
{
	onError = nullptr;
	onSuccess = nullptr;
	onFailure = nullptr;
	onIceCandidate = nullptr;
	capturer = nullptr;
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
		pc_factory_ = nullptr;

		dev_manager->Terminate();	
		*(dev_manager.use()) = nullptr; // disable autodestruct of this

		dev_manager = nullptr;
		capturer = nullptr;
	}
}

bool Conductor::InitializePeerConnection()
{
	ASSERT(pc_factory_ == nullptr);
	ASSERT(peer_connection_ == nullptr);

	pc_factory_ = webrtc::CreatePeerConnectionFactory();
	if (!pc_factory_)
	{
		DeletePeerConnection();
		return false;
	}

	if (!CreatePeerConnection(DTLS_ON))
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

cricket::VideoCapturer * Conductor::Create(const cricket::Device& device)
{
	return new YuvFramesCapturer2(*this);
}

void Conductor::OnFillBuffer(uint8_t * frame_buffer, uint32_t yuvSize)
{
	if (onFillBuffer)
	{
		onFillBuffer(frame_buffer, yuvSize);
	}
}

cricket::VideoCapturer * Conductor::OpenVideoCaptureDevice()
{
	if (!dev_manager)
	{
		rtc::scoped_ptr<cricket::DeviceManagerInterface> d(cricket::DeviceManagerFactory::Create());
		
		dev_manager.reset(d.get());
		*(d.use()) = nullptr; // disable local autodestruct

		if (!dev_manager->Init())
		{
			LOG(LS_ERROR) << "Can't create device manager";
			return nullptr;
		}		
		dev_manager->SetVideoDeviceCapturerFactory(this);
	}

	if (!capturer)
	{
		cricket::Device dummyDevice;
		dummyDevice.name = "custom dummy device";
		capturer = dev_manager->CreateVideoCapturer(dummyDevice);
		if (capturer == nullptr)
		{
			LOG(LS_ERROR) << "Capturer is NULL!";
		}
	}
	return capturer;
}

void Conductor::AddStreams()
{
	if (active_streams_.find(kStreamLabel) != active_streams_.end())
		return;  // Already added.

	//webrtc::AudioTrackInterface * audio_track = peer_connection_factory_->CreateAudioTrack(
	//		kAudioLabel, peer_connection_factory_->CreateAudioSource(NULL));

	cricket::VideoCapturer * c = OpenVideoCaptureDevice();
	auto v = pc_factory_->CreateVideoSource(c, NULL);
	auto video_track =	pc_factory_->CreateVideoTrack(kVideoLabel, v);
	//main_wnd_->StartLocalRenderer(video_track);

	auto stream = pc_factory_->CreateLocalMediaStream(kStreamLabel);
	{
		//stream->AddTrack(audio_track);
		stream->AddTrack(video_track);

		if (!peer_connection_->AddStream(stream))
		{
			LOG(LS_ERROR) << "Adding stream to PeerConnection failed";
		}
		typedef std::pair<std::string,	rtc::scoped_refptr<webrtc::MediaStreamInterface> >	MediaStreamPair;
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