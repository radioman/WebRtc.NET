
#ifndef WEBRTC_NET_CONDUCTOR_H_
#define WEBRTC_NET_CONDUCTOR_H_
#pragma once

#include "webrtc/api/mediastreaminterface.h"
#include "webrtc/api/peerconnectioninterface.h"

typedef void (__stdcall *OnErrorCallbackNative)();
typedef void(__stdcall *OnSuccessCallbackNative)(const char * type, const char * sdp);
typedef void(__stdcall *OnFailureCallbackNative)(const char * error);
typedef void(__stdcall *OnIceCandidateCallbackNative)(const char * sdp_mid, int sdp_mline_index, const char * sdp);
typedef void(__stdcall *OnFillBufferCallbackNative)(uint8_t * frame_buffer, uint32_t yuvSize);

class Conductor	: public webrtc::PeerConnectionObserver,
	              public webrtc::CreateSessionDescriptionObserver,
	              public webrtc::SetSessionDescriptionObserver
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

	bool OpenVideoCaptureDevice();
	void OnFillBuffer(uint8_t * frame_buffer, uint32_t yuvSize);

	void AddServerConfig(std::string uri, std::string username, std::string password);

	OnErrorCallbackNative onError;
	OnSuccessCallbackNative onSuccess;
	OnFailureCallbackNative onFailure;
	OnIceCandidateCallbackNative onIceCandidate;
	OnFillBufferCallbackNative onFillBuffer;	

protected:

	// SetSessionDescriptionObserver
	virtual void webrtc::SetSessionDescriptionObserver::OnSuccess()
	{
		LOG(INFO) << __FUNCTION__;
	}

	// CreateSessionDescriptionObserver implementation.
	virtual void webrtc::CreateSessionDescriptionObserver::OnSuccess(webrtc::SessionDescriptionInterface* desc);
	virtual void OnFailure(const std::string& error);

	//
	// PeerConnectionObserver implementation.
	//
	virtual void OnError();

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
	virtual void OnAddStream(webrtc::MediaStreamInterface* stream);
	virtual void OnRemoveStream(webrtc::MediaStreamInterface* stream);
	virtual void OnDataChannel(webrtc::DataChannelInterface* channel)
	{
	}
	virtual void OnRenegotiationNeeded()
	{
		LOG(INFO) << __FUNCTION__ << " ";
	}
	virtual void OnIceChange()
	{
		LOG(INFO) << __FUNCTION__ << " ";
	}
	virtual void OnIceCandidate(const webrtc::IceCandidateInterface* candidate);

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

	cricket::VideoCapturer * capturer;

	rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
	rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> pc_factory_;
	std::map<std::string, rtc::scoped_refptr<webrtc::MediaStreamInterface> > active_streams_;

	std::vector<webrtc::PeerConnectionInterface::IceServer> serverConfigs;

public:
	int caputureFps;
	bool barcodeEnabled;
};

#endif  // WEBRTC_NET_CONDUCTOR_H_
