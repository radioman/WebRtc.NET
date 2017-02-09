
#ifndef WEBRTC_NET_CONDUCTOR_H_
#define WEBRTC_NET_CONDUCTOR_H_
#pragma once

#include "webrtc/api/mediastreaminterface.h"
#include "webrtc/api/peerconnectioninterface.h"

#include "internals.h"

namespace cricket
{
	class TurnServer;
	class StunServer;
}

namespace Native
{
	typedef void(__stdcall *OnErrorCallbackNative)();
	typedef void(__stdcall *OnSuccessCallbackNative)(const char * type, const char * sdp);
	typedef void(__stdcall *OnFailureCallbackNative)(const char * error);
	typedef void(__stdcall *OnIceCandidateCallbackNative)(const char * sdp_mid, int sdp_mline_index, const char * sdp);
	typedef void(__stdcall *OnRenderCallbackNative)(uint8_t * frame_buffer, uint32_t w, uint32_t h);
	typedef void(__stdcall *OnDataMessageCallbackNative)(const char * msg);

	class Conductor : public webrtc::PeerConnectionObserver,
		public webrtc::CreateSessionDescriptionObserver,
		public webrtc::SetSessionDescriptionObserver,
		public webrtc::DataChannelObserver
	{
	public:

		Conductor();
		~Conductor();

		bool InitializePeerConnection();
		void CreateOffer();
		void OnOfferReply(std::string type, std::string sdp);
		void OnOfferRequest(std::string sdp);
		bool AddIceCandidate(std::string sdp_mid, int sdp_mlineindex, std::string sdp);

		bool ProcessMessages(int delay)
		{
			return rtc::Thread::Current()->ProcessMessages(delay);
		}

		static std::vector<std::string> GetVideoDevices();
		bool OpenVideoCaptureDevice(std::string & name);
		void AddServerConfig(std::string uri, std::string username, std::string password);

		uint8_t * VideoCapturerI420Buffer()
		{
			if (capturer)
			{
				return (uint8_t*)capturer->video_buffer->DataY();
			}
			return nullptr;
		}

		void PushFrame()
		{
			if (capturer)
			{
				capturer->PushFrame();
			}
		}

#if DESKTOP_CAPTURE
		void DesktopCapturerSize(int & w, int & h)
		{
			if (capturer && capturer->desktop_frame)
			{
				webrtc::DesktopSize s = capturer->desktop_frame->size();
				w = s.width();
				h = s.height();
			}
		}

		uint8_t * DesktopCapturerRGBAbuffer()
		{
			if (capturer && capturer->desktop_frame)
			{
				return capturer->desktop_frame->data();
			}
			return nullptr;
		}

		void CaptureFrame()
		{
			if (capturer)
			{
				capturer->CaptureFrame();
			}
		}
#endif
		void CreateDataChannel(const std::string & label);
		void DataChannelSendText(const std::string & text);

		OnErrorCallbackNative onError;
		OnSuccessCallbackNative onSuccess;
		OnFailureCallbackNative onFailure;
		OnIceCandidateCallbackNative onIceCandidate;
		OnRenderCallbackNative onRenderLocal;
		OnRenderCallbackNative onRenderRemote;
		OnDataMessageCallbackNative onDataMessage;

		bool RunStunServer(const std::string & bindIp);
		bool RunTurnServer(const std::string & bindIp, const std::string & ip,
						   const std::string & realm, const std::string & authFile);

	protected:

#pragma region -- SetSessionDescriptionObserver --

		virtual void webrtc::SetSessionDescriptionObserver::OnSuccess()
		{
			LOG(INFO) << __FUNCTION__;
		}

#pragma endregion
		
#pragma region -- CreateSessionDescriptionObserver --

		virtual void webrtc::CreateSessionDescriptionObserver::OnSuccess(webrtc::SessionDescriptionInterface* desc);
		virtual void OnFailure(const std::string& error);

#pragma endregion

#pragma region -- PeerConnectionObserver --

		virtual void OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream);
		virtual void OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream);
		virtual void OnError();
		virtual void OnIceCandidate(const webrtc::IceCandidateInterface* candidate);

		virtual void OnIceChange()
		{
			LOG(INFO) << __FUNCTION__ << " ";
		}
		
		virtual void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState state)
		{
			LOG(INFO) << __FUNCTION__ << " " << state;
		}

		virtual void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState state)
		{
			LOG(INFO) << __FUNCTION__ << " " << state;
		}

		virtual void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState state)
		{
			LOG(INFO) << __FUNCTION__ << " " << state;
		}

		virtual void OnStateChange(webrtc::PeerConnectionObserver::StateType state_changed)
		{
			LOG(INFO) << __FUNCTION__ << " " << state_changed;
		}

		virtual void OnRenegotiationNeeded()
		{
			LOG(INFO) << __FUNCTION__ << " ";
		}

#pragma endregion
		
#pragma region -- DataChannelObserver --
		
		virtual void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel);

		// The data channel state have changed.
		virtual void OnStateChange()
		{
			LOG(INFO) << __FUNCTION__;
		}

		//  A data buffer was successfully received.
		virtual void OnMessage(const webrtc::DataBuffer& buffer);

		// The data channel's buffered_amount has changed.
		virtual void OnBufferedAmountChange(uint64_t previous_amount)
		{
			LOG(INFO) << __FUNCTION__;
		}

#pragma endregion

		int AddRef() const
		{
			return 0;
		};
		int Release() const
		{
			return 0;
		};

	private:

		bool CreatePeerConnection(bool dtls);
		void DeletePeerConnection();
		void AddStreams();		

		rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
		rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> pc_factory_;
		std::map<std::string, rtc::scoped_refptr<webrtc::MediaStreamInterface>> active_streams_;
		rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel;
		std::vector<webrtc::PeerConnectionInterface::IceServer> serverConfigs;

		YuvFramesCapturer2 * capturer;
		cricket::VideoCapturer * capturer_internal;

		std::unique_ptr<VideoRenderer> local_video;
		std::unique_ptr<VideoRenderer> remote_video;
		std::unique_ptr<AudioRenderer> remote_audio;

		std::unique_ptr<cricket::TurnServer> turnServer;
		std::unique_ptr<cricket::StunServer> stunServer;

	public:
		int caputureFps;
		bool audioEnabled;
		bool barcodeEnabled;

		int width_;
		int height_;
	};
}
#endif  // WEBRTC_NET_CONDUCTOR_H_
