
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

class DummySetSessionDescriptionObserver
	: public webrtc::SetSessionDescriptionObserver
{
public:
	static DummySetSessionDescriptionObserver* Create()
	{
		return
			new rtc::RefCountedObject<DummySetSessionDescriptionObserver>();
	}
	virtual void OnSuccess()
	{
		LOG(INFO) << __FUNCTION__;
	}
	virtual void OnFailure(const std::string& error)
	{
		LOG(INFO) << __FUNCTION__ << " " << error;
	}

protected:
	DummySetSessionDescriptionObserver()
	{
	}
	~DummySetSessionDescriptionObserver()
	{
	}
};

Conductor::Conductor()
{
	onError = NULL;
	onSuccess = NULL;
	onFailure = NULL;
}

Conductor::~Conductor()
{
	DeletePeerConnection();
	ASSERT(peer_connection_.get() == NULL);
}

bool Conductor::InitializePeerConnection()
{
	ASSERT(peer_connection_factory_.get() == NULL);
	ASSERT(peer_connection_.get() == NULL);

	peer_connection_factory_ = webrtc::CreatePeerConnectionFactory();

	if (!peer_connection_factory_.get())
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
	return peer_connection_.get() != NULL;
}

bool Conductor::CreatePeerConnection(bool dtls)
{
	ASSERT(peer_connection_factory_.get() != NULL);
	ASSERT(peer_connection_.get() == NULL);

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

	peer_connection_ = peer_connection_factory_->CreatePeerConnection(config, &constraints, NULL, NULL, this);
	return peer_connection_.get() != NULL;
}

void Conductor::DeletePeerConnection()
{
	peer_connection_ = NULL;
	active_streams_.clear();
	peer_connection_factory_ = NULL;
}

void Conductor::CreateOffer()
{
	peer_connection_->CreateOffer(this, NULL);
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
	peer_connection_->SetRemoteDescription(DummySetSessionDescriptionObserver::Create(), session_description);
}

void Conductor::OnOfferRequest(std::string type, std::string sdp)
{
	webrtc::SdpParseError error;
	webrtc::SessionDescriptionInterface* session_description(webrtc::CreateSessionDescription(type, sdp, &error));
	if (!session_description)
	{
		LOG(WARNING) << "Can't parse received session description message. " << "SdpParseError was: " << error.description;
		return;
	}
	peer_connection_->SetRemoteDescription(DummySetSessionDescriptionObserver::Create(), session_description);
	peer_connection_->CreateAnswer(this, NULL);
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

void Conductor::OnError()
{
	if (onError != NULL)
	{
		onError();
	}
}

cricket::VideoCapturer* Conductor::OpenVideoCaptureDevice()
{
	rtc::scoped_ptr<cricket::DeviceManagerInterface> dev_manager(cricket::DeviceManagerFactory::Create());
	if (!dev_manager->Init())
	{
		LOG(LS_ERROR) << "Can't create device manager";
		return NULL;
	}

	//Inject our video capturer
	cricket::DeviceManager* device_manager = static_cast<cricket::DeviceManager*>(dev_manager.get());
	device_manager->SetVideoDeviceCapturerFactory(new cricket::VideoCapturerFactoryCustom());

	cricket::VideoCapturer* capturer = NULL;

	cricket::Device dummyDevice;
	dummyDevice.name = "custom dummy device";
	capturer = dev_manager->CreateVideoCapturer(dummyDevice);
	if (capturer == NULL)
	{
		LOG(LS_ERROR) << "Capturer is NULL!";
	}

	return capturer;
}

void Conductor::AddStreams()
{
	if (active_streams_.find(kStreamLabel) != active_streams_.end())
		return;  // Already added.

	//rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track(
	//	peer_connection_factory_->CreateAudioTrack(
	//		kAudioLabel, peer_connection_factory_->CreateAudioSource(NULL)));

	rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track(
		peer_connection_factory_->CreateVideoTrack(
			kVideoLabel,
			peer_connection_factory_->CreateVideoSource(OpenVideoCaptureDevice(), NULL)));
	//main_wnd_->StartLocalRenderer(video_track);

	rtc::scoped_refptr<webrtc::MediaStreamInterface> stream =
		peer_connection_factory_->CreateLocalMediaStream(kStreamLabel);

	//stream->AddTrack(audio_track);
	stream->AddTrack(video_track);
	if (!peer_connection_->AddStream(stream))
	{
		LOG(LS_ERROR) << "Adding stream to PeerConnection failed";
	}
	typedef std::pair<std::string,
		rtc::scoped_refptr<webrtc::MediaStreamInterface> >
		MediaStreamPair;
	active_streams_.insert(MediaStreamPair(stream->label(), stream));
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

	//jmessage[kCandidateSdpMidName] = candidate->sdp_mid();
	//jmessage[kCandidateSdpMlineIndexName] = candidate->sdp_mline_index();

	std::string sdp;
	if (!candidate->ToString(&sdp))
	{
		LOG(LS_ERROR) << "Failed to serialize candidate";
		return;
	}
	//jmessage[kCandidateSdpName] = sdp;
	//SendMessage(writer.write(jmessage));
}

void Conductor::OnSuccess(webrtc::SessionDescriptionInterface* desc)
{
	peer_connection_->SetLocalDescription(DummySetSessionDescriptionObserver::Create(), desc);

	std::string sdp;
	desc->ToString(&sdp);

	//Json::StyledWriter writer;
	//Json::Value jmessage;
	//jmessage[kSessionDescriptionTypeName] = desc->type();
	//jmessage[kSessionDescriptionSdpName] = sdp;
	//SendMessage(writer.write(jmessage));

	if (onSuccess != NULL)
	{
		onSuccess(desc->type().c_str(), sdp.c_str());
	}
}

void Conductor::OnFailure(const std::string& error)
{
	LOG(LERROR) << error;

	if (onFailure != NULL)
	{
		onFailure(error.c_str());
	}
}