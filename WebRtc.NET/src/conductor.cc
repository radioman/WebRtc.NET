
#include "defaults.h"
#include "conductor.h"

#include "webrtc/api/test/fakeconstraints.h"
#include "webrtc/video_encoder.h"
#include "webrtc/modules/video_coding/codecs/vp8/simulcast_encoder_adapter.h"
#include "webrtc/modules/video_coding/codecs/vp8/include/vp8.h"
#include "webrtc/modules/video_capture/video_capture_factory.h"
#include "webrtc/media/engine/webrtcvideocapturerfactory.h"

// for servers
#include "webrtc/p2p/base/relayserver.h"
#include "webrtc/p2p/base/stunserver.h"
#include "webrtc/p2p/base/basicpacketsocketfactory.h"
#include "webrtc/p2p/base/turnserver.h"
#include "webrtc/base/asyncudpsocket.h"
#include "webrtc/base/optionsfile.h"
#include "webrtc/base/stringencode.h"
#include "webrtc/base/thread.h"

namespace Native
{
	const char kVideoLabel[] = "video_label";
	const char kStreamLabel[] = "stream_label";
	const char kSoftware[] = "libjingle TurnServer";

	class TurnFileAuth : public cricket::TurnAuthInterface
	{
	public:
		explicit TurnFileAuth(const std::string& path) : file_(path)
		{
		}

		bool Load()
		{
			return file_.Load();
		}

		virtual bool GetKey(const std::string& username, const std::string& realm, std::string* key)
		{
			// File is stored as lines of <username>=<HA1>.
			// Generate HA1 via "echo -n "<username>:<realm>:<password>" | md5sum"
			std::string hex;
			bool ret = file_.GetStringValue(username, &hex);
			if (ret)
			{
				char buf[32];
				size_t len = rtc::hex_decode(buf, sizeof(buf), hex);
				*key = std::string(buf, len);
			}
			return ret;
		}
	private:
		rtc::OptionsFile file_;
	};

	Conductor::Conductor()
	{
		onError = nullptr;
		onSuccess = nullptr;
		onFailure = nullptr;
		onIceCandidate = nullptr;
		capturer = nullptr;
		caputureFps = 5;
		barcodeEnabled = false;
		turnServer = nullptr;
	}

	Conductor::~Conductor()
	{
		DeletePeerConnection();
		ASSERT(peer_connection_ == nullptr);

		if (turnServer)
		{
			turnServer->disconnect_all();
			delete turnServer;
			turnServer = nullptr;
		}

		if (stunServer)
		{
			stunServer->disconnect_all();
			delete stunServer;
			stunServer = nullptr;
		}

		if (turnServer || stunServer)
		{
			rtc::Thread::Current()->Quit();
		}
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

			peer_connection_->Close();
			peer_connection_ = nullptr;
		}

		pc_factory_ = nullptr;
		capturer = nullptr;

		serverConfigs.clear();
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

		webrtc::PeerConnectionFactoryInterface::Options opt;
		{
			//opt.disable_encryption = true;
			//opt.disable_network_monitor = true;
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
		config.tcp_candidate_policy = webrtc::PeerConnectionInterface::kTcpCandidatePolicyDisabled;
		config.disable_ipv6 = true;
		config.enable_dtls_srtp = rtc::Optional<bool>(dtls);

		for each (auto server in serverConfigs)
		{
			config.servers.push_back(server);
		}

		webrtc::FakeConstraints constraints;
		constraints.SetMandatoryReceiveVideo(false);
		constraints.SetMandatoryReceiveAudio(false);
		constraints.SetMandatoryIceRestart(true);
		constraints.AddMandatory(webrtc::MediaConstraintsInterface::kVoiceActivityDetection, "false");
		constraints.AddMandatory(webrtc::MediaConstraintsInterface::kEnableIPv6, "false");

		peer_connection_ = pc_factory_->CreatePeerConnection(config, &constraints, NULL, NULL, this);
		return peer_connection_ != nullptr;
	}

	void Conductor::AddServerConfig(std::string uri, std::string username, std::string password)
	{
		webrtc::PeerConnectionInterface::IceServer server;
		server.uri = uri;
		server.username = username;
		server.password = password;

		serverConfigs.push_back(server);
	}

	void Conductor::CreateOffer()
	{
		if (!peer_connection_)
			return;

		peer_connection_->CreateOffer(this, nullptr);
	}

	void Conductor::OnOfferReply(std::string type, std::string sdp)
	{
		if (!peer_connection_)
			return;

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
		if (!peer_connection_)
			return;

		webrtc::SdpParseError error;
		webrtc::SessionDescriptionInterface* session_description(webrtc::CreateSessionDescription("offer", sdp, &error));
		if (!session_description)
		{
			LOG(WARNING) << "Can't parse received session description message. " << "SdpParseError was: " << error.description;
			return;
		}
		peer_connection_->SetRemoteDescription(this, session_description);

		webrtc::PeerConnectionInterface::RTCOfferAnswerOptions o;
		{
			o.voice_activity_detection = false;
			o.offer_to_receive_audio = false;
			o.offer_to_receive_video = webrtc::PeerConnectionInterface::RTCOfferAnswerOptions::kOfferToReceiveMediaTrue;
		}
		peer_connection_->CreateAnswer(this, o);
	}

	bool Conductor::AddIceCandidate(std::string sdp_mid, int sdp_mlineindex, std::string sdp)
	{
		webrtc::SdpParseError error;
		webrtc::IceCandidateInterface * candidate = webrtc::CreateIceCandidate(sdp_mid, sdp_mlineindex, sdp, &error);
		if (!candidate)
		{
			LOG(WARNING) << "Can't parse received candidate message. "
				<< "SdpParseError was: " << error.description;
			return false;
		}

		if (!peer_connection_)
			return false;

		if (!peer_connection_->AddIceCandidate(candidate))
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
		std::vector<std::string> device_names;
		{
			std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> info(webrtc::VideoCaptureFactory::CreateDeviceInfo(0));
			if (info)
			{
				int num_devices = info->NumberOfDevices();
				for (int i = 0; i < num_devices; ++i)
				{
					const uint32_t kSize = 256;
					char name[kSize] = { 0 };
					char id[kSize] = { 0 };
					if (info->GetDeviceName(i, name, kSize, id, kSize) != -1)
					{
						device_names.push_back(name);
					}
				}
			}
		}

		cricket::WebRtcVideoDeviceCapturerFactory factory;
		cricket::VideoCapturer * capturer = nullptr;
		for (const auto& name : device_names)
		{
			capturer = factory.Create(cricket::Device(name, 0));
			if (capturer)
			{
				LOG(LS_ERROR) << "Capturer != NULL!";
				return true;
			}
		}

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
			capturer = new Native::YuvFramesCapturer2(*this);
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
	void Conductor::OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream)
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

	void Conductor::OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream)
	{
		LOG(INFO) << __FUNCTION__ << " " << stream->label();
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
		if (!peer_connection_)
			return;

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

	bool Conductor::RunStunServer(const std::string & bindIp)
	{
		rtc::SocketAddress server_addr;
		if (!server_addr.FromString(bindIp))
		{
			LOG(LERROR) << "Unable to parse IP address: " << bindIp;
			return false;
		}

		rtc::Thread * main = rtc::Thread::Current();

		rtc::AsyncUDPSocket* server_socket = rtc::AsyncUDPSocket::Create(main->socketserver(), server_addr);
		if (!server_socket)
		{
			LOG(LERROR) << "Failed to create a UDP socket" << std::endl;
			return false;
		}

		stunServer = new cricket::StunServer(server_socket);

		LOG(INFO) << "Listening at " << server_addr.ToString() << std::endl;

		return true;
	}

	bool Conductor::RunTurnServer(const std::string & bindIp, const std::string & ip,
								  const std::string & realm, const std::string & authFile)
	{
		rtc::SocketAddress int_addr;
		if (!int_addr.FromString(bindIp))
		{
			LOG(LERROR) << "Unable to parse IP address: " << bindIp << std::endl;
			return false;
		}

		rtc::IPAddress ext_addr;
		if (!IPFromString(ip, &ext_addr))
		{
			LOG(LERROR) << "Unable to parse IP address: " << ip << std::endl;
			return false;
		}

		rtc::Thread* main = rtc::Thread::Current();
		rtc::AsyncUDPSocket * int_socket = rtc::AsyncUDPSocket::Create(main->socketserver(), int_addr);
		if (!int_socket)
		{
			LOG(LERROR) << "Failed to create a UDP socket bound at" << int_addr.ToString() << std::endl;
			return false;
		}

		TurnFileAuth * auth = new TurnFileAuth(authFile);
		if (!auth->Load())
		{
			LOG(LERROR) << "Failed to load auth file " << authFile << std::endl;
			return false;
		}

		turnServer = new cricket::TurnServer(main);
		turnServer->set_realm(realm);
		turnServer->set_software(kSoftware);
		turnServer->set_auth_hook(auth);
		turnServer->AddInternalSocket(int_socket, cricket::PROTO_UDP);
		turnServer->SetExternalSocketFactory(new rtc::BasicPacketSocketFactory(),
											 rtc::SocketAddress(ext_addr, 0));

		LOG(INFO) << "Listening internally at " << int_addr.ToString() << std::endl;

		return true;
	}
}