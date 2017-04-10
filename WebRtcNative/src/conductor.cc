
#include "defaults.h"
#include "conductor.h"

#include "webrtc/base/checks.h"
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

//#define L_LITTLE_ENDIAN
//#include "leptonica/allheaders.h"
#include "turbojpeg/turbojpeg.h"

extern "C"
{
	__declspec(dllexport) void * WINAPI NewConductor()
	{
		return new Native::Conductor();
	}

	__declspec(dllexport) void WINAPI DeleteConductor(Native::Conductor * cd)
	{
		delete cd;
	}

	__declspec(dllexport) void WINAPI InitializeSSL()
	{
		Native::InitializeSSL();
	}

	__declspec(dllexport) void WINAPI CleanupSSL()
	{
		Native::CleanupSSL();
	}

	//------------------------------------------------

	__declspec(dllexport) void WINAPI onRenderLocal(Native::Conductor * cd, Native::OnRenderCallbackNative callback)
	{
		cd->onRenderLocal = callback;
	}

	__declspec(dllexport) void WINAPI onRenderRemote(Native::Conductor * cd, Native::OnRenderCallbackNative callback)
	{
		cd->onRenderRemote = callback;
	}

	__declspec(dllexport) void WINAPI onError(Native::Conductor * cd, Native::OnErrorCallbackNative callback)
	{
		cd->onError = callback;
	}

	__declspec(dllexport) void WINAPI onSuccess(Native::Conductor * cd, Native::OnSuccessCallbackNative callback)
	{
		cd->onSuccess = callback;
	}

	__declspec(dllexport) void WINAPI onFailure(Native::Conductor * cd, Native::OnFailureCallbackNative callback)
	{
		cd->onFailure = callback;
	}

	__declspec(dllexport) void WINAPI onDataMessage(Native::Conductor * cd, Native::OnDataMessageCallbackNative callback)
	{
		cd->onDataMessage = callback;
	}

	__declspec(dllexport) void WINAPI onDataBinaryMessage(Native::Conductor * cd, Native::OnDataBinaryMessageCallbackNative callback)
	{
		cd->onDataBinaryMessage = callback;
	}

	__declspec(dllexport) void WINAPI onIceCandidate(Native::Conductor * cd, Native::OnIceCandidateCallbackNative callback)
	{
		cd->onIceCandidate = callback;
	}

	//------------------------------------------------

	__declspec(dllexport) bool WINAPI InitializePeerConnection(Native::Conductor * cd)
	{
		return cd->InitializePeerConnection();
	}

	__declspec(dllexport) void WINAPI CreateOffer(Native::Conductor * cd)
	{
		cd->CreateOffer();
	}

	__declspec(dllexport) bool WINAPI ProcessMessages(Native::Conductor * cd, int delay)
	{
		return cd->ProcessMessages(delay);
	}

	__declspec(dllexport) bool WINAPI OpenVideoCaptureDevice(Native::Conductor * cd, char * name)
	{
		return cd->OpenVideoCaptureDevice(std::string(name));
	}

	//__declspec(dllexport) static System::Collections::Generic::List<String^> ^ GetVideoDevices()
	//{
	//	System::Collections::Generic::List<String^> ^ ret = nullptr;
	//	std::vector<std::string> devices = Native::Conductor::GetVideoDevices();
	//	for (const auto & d : devices)
	//	{
	//		if (ret == nullptr)
	//		{
	//			ret = gcnew System::Collections::Generic::List<String^>();
	//		}
	//		ret->Add(marshal_as<String^>(d));
	//	}
	//	return ret;
	//}

	__declspec(dllexport) void WINAPI OnOfferReply(Native::Conductor * cd, char * type, char * sdp)
	{
		cd->OnOfferReply(type, sdp);
	}

	__declspec(dllexport) void WINAPI OnOfferRequest(Native::Conductor * cd, char * sdp)
	{
		cd->OnOfferRequest(sdp);
	}

	__declspec(dllexport) bool WINAPI AddIceCandidate(Native::Conductor * cd, char * sdp_mid, int sdp_mlineindex, char * sdp)
	{
		return cd->AddIceCandidate(sdp_mid, sdp_mlineindex, sdp);
	}

	__declspec(dllexport) void WINAPI AddServerConfig(Native::Conductor * cd, char * uri, char * username, char * password)
	{
		cd->AddServerConfig(uri, username, password);
	}

	__declspec(dllexport) void WINAPI CreateDataChannel(Native::Conductor * cd, char * label)
	{
		cd->CreateDataChannel(label);
	}

	__declspec(dllexport) void WINAPI DataChannelSendText(Native::Conductor * cd, char * text)
	{
		cd->DataChannelSendText(text);
	}

	__declspec(dllexport) void WINAPI DataChannelSendData(Native::Conductor * cd, uint8_t * array_data, int length)
	{
		rtc::CopyOnWriteBuffer writeBuffer(array_data, length);
		cd->DataChannelSendData(webrtc::DataBuffer(writeBuffer, true));
	}

	__declspec(dllexport) void WINAPI SetAudio(Native::Conductor * cd, bool enable)
	{
		cd->audioEnabled = enable;
	}

	__declspec(dllexport) void WINAPI SetVideoCapturer(Native::Conductor * cd, int width, int height, int caputureFps, bool barcodeEnabled)
	{
		cd->width_ = width;
		cd->height_ = height;
		cd->caputureFps = caputureFps;
		cd->barcodeEnabled = barcodeEnabled;
	}

	__declspec(dllexport) uint8_t * WINAPI VideoCapturerI420Buffer(Native::Conductor * cd)
	{
		return cd->VideoCapturerI420Buffer();
	}

	__declspec(dllexport) void WINAPI PushFrame(Native::Conductor * cd, uint8_t * BGR)
	{
		cd->PushFrame(BGR);
	}

	__declspec(dllexport) uint8_t * WINAPI DesktopCapturerRGBAbuffer(Native::Conductor * cd)
	{
#if DESKTOP_CAPTURE
		return cd->DesktopCapturerRGBAbuffer();
#else
		return nullptr;
		//throw gcnew System::NotImplementedException("set internals.h #define DESKTOP_CAPTURE 1");
#endif
	}

	__declspec(dllexport) void WINAPI DesktopCapturerSize(Native::Conductor * cd, int & w, int & h)
	{
#if DESKTOP_CAPTURE
		int wn = 0, hn = 0;
		cd->DesktopCapturerSize(wn, hn);
		w = wn;
		h = hn;
#else
		//throw System::NotImplementedException("set internals.h #define DESKTOP_CAPTURE 1");
#endif
	}

	__declspec(dllexport) void WINAPI CaptureFrame(Native::Conductor * cd)
	{
#if DESKTOP_CAPTURE
		cd->CaptureFrame();
#else
		//throw gcnew System::NotImplementedException("set internals.h #define DESKTOP_CAPTURE 1");
#endif
	}

	__declspec(dllexport) bool WINAPI RunStunServer(Native::Conductor * cd, char * bindIp)
	{
		return cd->RunStunServer(bindIp);
	}

	// File is stored as lines of <username>=<HA1>.
	// Generate HA1 via "echo -n "<username>:<realm>:<password>" | md5sum"
	__declspec(dllexport) bool WINAPI RunTurnServer(Native::Conductor * cd, char * bindIp, char * ip, char * realm, char * authFile)
	{
		return cd->RunTurnServer(bindIp, ip, realm, authFile);
	}
}

namespace Native
{
	const char kAudioLabel[] = "audio_label";
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

		width_ = 640;
	    height_ = 360;
		caputureFps = 5;
		audioEnabled = false;

		barcodeEnabled = false;

		turnServer = nullptr;
		data_channel = nullptr;
		onDataMessage = nullptr;
		capturer_internal = nullptr;
		capturer = nullptr;

		jpegc = nullptr;
	}

	Conductor::~Conductor()
	{
		if (jpegc != nullptr)
			tjDestroy(jpegc);

		DeletePeerConnection();
		RTC_DCHECK(peer_connection_ == nullptr);

		if (turnServer)
		{
			turnServer->disconnect_all();
		}

		if (stunServer)
		{
			stunServer->disconnect_all();
		}

		if (turnServer || stunServer)
		{
			auto c = rtc::Thread::Current();
			if (c)
			{
				c->Quit();
			}
		}
	}

	void Conductor::DeletePeerConnection()
	{
		if (peer_connection_.get())
		{
			local_video.reset();

			for (auto it = active_streams_.begin(); it != active_streams_.end(); ++it)
			{
				peer_connection_->RemoveStream(it->second);
			}
			active_streams_.clear();

			peer_connection_->Close();
			peer_connection_ = nullptr;
		}

		pc_factory_ = nullptr;

		if (data_channel)
		{
			data_channel->UnregisterObserver();
			data_channel = nullptr;
		}
		serverConfigs.clear();

		capturer_internal = nullptr;
		capturer = nullptr;
	}

	bool Conductor::InitializePeerConnection()
	{
		RTC_DCHECK(pc_factory_ == nullptr);
		RTC_DCHECK(peer_connection_ == nullptr);

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
			//opt.disable_sctp_data_channels = true;
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
		RTC_DCHECK(pc_factory_ != nullptr);
		RTC_DCHECK(peer_connection_ == nullptr);

		webrtc::PeerConnectionInterface::RTCConfiguration config;
		config.tcp_candidate_policy = webrtc::PeerConnectionInterface::kTcpCandidatePolicyDisabled;
		config.disable_ipv6 = true;
		config.enable_dtls_srtp = rtc::Optional<bool>(dtls);
		config.rtcp_mux_policy = webrtc::PeerConnectionInterface::kRtcpMuxPolicyRequire;

		for each (auto server in serverConfigs)
		{
			config.servers.push_back(server);
		}

		webrtc::FakeConstraints constraints;
		constraints.SetAllowDtlsSctpDataChannels();
		constraints.SetMandatoryReceiveVideo(false);
		constraints.SetMandatoryReceiveAudio(false);
		constraints.SetMandatoryIceRestart(true);
		constraints.SetMandatoryUseRtpMux(true);
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
			o.offer_to_receive_audio = webrtc::PeerConnectionInterface::RTCOfferAnswerOptions::kOfferToReceiveMediaTrue;
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

	std::vector<std::string> Conductor::GetVideoDevices()
	{
		std::vector<std::string> device_names;
		{
			std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> info(webrtc::VideoCaptureFactory::CreateDeviceInfo());
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
		return device_names;
	}

	bool Conductor::OpenVideoCaptureDevice(std::string & name)
	{
		if (!capturer_internal)
		{
			cricket::WebRtcVideoDeviceCapturerFactory factory;
			capturer_internal = factory.Create(cricket::Device(name, 0)).get();
			if (capturer_internal)
			{
				LOG(LS_ERROR) << "Capturer != NULL!";
				return true;
			}
		}
		return false;
	}

	void Conductor::PushFrame(uint8_t * BGR)
	{
		if (capturer)
		{
			uint8_t * yuv = VideoCapturerI420Buffer();
			if (yuv != nullptr)
			{
				int r = 0;

				if (BGR != nullptr)
				{
					const int pad = 4;
					const int pxFormat = TJPF_BGR;
					int pitch = TJPAD(tjPixelSize[pxFormat] * width_);

					if (jpegc == nullptr)
						jpegc = tjInitCompress();

					if (jpegc)
						r = tjEncodeYUV3(jpegc, BGR, width_, pitch, height_, pxFormat, yuv, pad, TJSAMP_420, true ? TJFLAG_FASTDCT : TJFLAG_ACCURATEDCT);
				}

				if (r != 0)
				{
					//Debug::WriteLine(String::Format("tjEncodeYUV3, LastJpegError: {0}", LastJpegError));
				}
				else
				{
					capturer->PushFrame();
				}
			}
		}
	}

	void Conductor::AddStreams()
	{
		if (active_streams_.find(kStreamLabel) != active_streams_.end())
			return;  // Already added.

		cricket::VideoCapturer * vc = nullptr;
		if (capturer_internal)
		{
			vc = capturer_internal;
		}
		else
		{
			vc = capturer = new Native::YuvFramesCapturer2(*this);
		}

		auto v = pc_factory_->CreateVideoSource(vc);
		auto video_track = pc_factory_->CreateVideoTrack(kVideoLabel, v);
		if (onRenderLocal)
		{
			local_video.reset(new VideoRenderer(*this, false, video_track));
	    }

		auto stream = pc_factory_->CreateLocalMediaStream(kStreamLabel);
		{
			if (audioEnabled)
			{
				auto a = pc_factory_->CreateAudioSource(NULL);
				auto audio_track = pc_factory_->CreateAudioTrack(kAudioLabel, a);
				stream->AddTrack(audio_track);
			}
			stream->AddTrack(video_track);

			if (!peer_connection_->AddStream(stream))
			{
				LOG(LS_ERROR) << "Adding stream to PeerConnection failed";
			}
			typedef std::pair<std::string, rtc::scoped_refptr<webrtc::MediaStreamInterface> >	MediaStreamPair;
			active_streams_.insert(MediaStreamPair(stream->label(), stream));
		}
	}

	// Called when a remote stream is added
	void Conductor::OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream)
	{
		LOG(INFO) << __FUNCTION__ << " " << stream->label();

		if (onRenderRemote)
		{
			webrtc::VideoTrackVector tracks = stream->GetVideoTracks();
			if (!tracks.empty())
			{
				webrtc::VideoTrackInterface* track = tracks[0];
				remote_video.reset(new Native::VideoRenderer(*this, true, track));
			}
		}

		if (audioEnabled)
		{
			webrtc::AudioTrackVector atracks = stream->GetAudioTracks();
			if (!atracks.empty())
			{
				webrtc::AudioTrackInterface* track = atracks[0];
				remote_audio.reset(new Native::AudioRenderer(*this, true, track));
			}
		}
	}

	void Conductor::OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream)
	{
		LOG(INFO) << __FUNCTION__ << " " << stream->label();
		remote_video.reset(nullptr);
		remote_audio.reset(nullptr);

		// lost ownership, do not delete
		capturer = nullptr;
		capturer_internal = nullptr;
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

	void Conductor::CreateDataChannel(const std::string & label)
	{
		if (!peer_connection_)
			return;

		webrtc::DataChannelInit dc_options;
		//dc_options.id = 1;
		dc_options.maxRetransmits = 1;
		dc_options.negotiated = false;
		dc_options.ordered = false;

		data_channel = peer_connection_->CreateDataChannel(label, &dc_options);
		data_channel->RegisterObserver(this);
	}

	void Conductor::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel)
	{
		LOG(INFO) << __FUNCTION__ << " " << channel->label();

		data_channel = channel.get();
		data_channel->RegisterObserver(this);
	}

	void Conductor::DataChannelSendText(const std::string & text)
	{
		data_channel->Send(webrtc::DataBuffer(text));
	}

	void Conductor::DataChannelSendData(const webrtc::DataBuffer & data)
	{
		data_channel->Send(data);
	}

	//  A data buffer was successfully received.
	void Conductor::OnMessage(const webrtc::DataBuffer& buffer)
	{
		LOG(INFO) << __FUNCTION__;

		if (buffer.binary)
		{
			if (onDataBinaryMessage != nullptr)
			{
				auto * data = buffer.data.data();
				onDataBinaryMessage(data, buffer.size());
			}
		}
		else
		{
			if (onDataMessage != nullptr)
			{
				std::string msg(buffer.data.data<char>(), buffer.size());
				onDataMessage(msg.c_str());
			}
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

		stunServer.reset(new cricket::StunServer(server_socket));

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


		auto t = new cricket::TurnServer(main);
		turnServer.reset(t);

		t->set_realm(realm);
		t->set_software(kSoftware);
		t->set_auth_hook(auth);
		t->AddInternalSocket(int_socket, cricket::PROTO_UDP);
		t->SetExternalSocketFactory(new rtc::BasicPacketSocketFactory(),
											 rtc::SocketAddress(ext_addr, 0));

		LOG(INFO) << "Listening internally at " << int_addr.ToString() << std::endl;

		return true;
	}
}