
#ifndef WEBRTC_EXAMPLES_PEERCONNECTION_CLIENT_CONDUCTOR_H_
#define WEBRTC_EXAMPLES_PEERCONNECTION_CLIENT_CONDUCTOR_H_
#pragma once

#include "talk/app/webrtc/mediastreaminterface.h"
#include "talk/app/webrtc/peerconnectioninterface.h"

typedef void (__stdcall *OnErrorCallbackNative)();
typedef void(__stdcall *OnSuccessCallbackNative)(const char * type, const char * sdp);
typedef void(__stdcall *OnFailureCallbackNative)(const char * error);
typedef void(__stdcall *OnIceCandidateCallbackNative)(const char * sdp_mid, int sdp_mline_index, const char * sdp);

class Conductor	: public webrtc::PeerConnectionObserver, public webrtc::CreateSessionDescriptionObserver
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

	void Quit()
	{
		rtc::Thread::Current()->Quit();
	}

	OnErrorCallbackNative onError;
	OnSuccessCallbackNative onSuccess;
	OnFailureCallbackNative onFailure;
	OnIceCandidateCallbackNative onIceCandidate;

protected:

	// CreateSessionDescriptionObserver implementation.
	virtual void OnSuccess(webrtc::SessionDescriptionInterface* desc);
	virtual void OnFailure(const std::string& error);

	//
	// PeerConnectionObserver implementation.
	//
	virtual void OnError();

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
		return 1;
	};
	int Release() const
	{
		return 0;
	};

private:

	bool CreatePeerConnection(bool dtls);
	void DeletePeerConnection();
	void AddStreams();
	cricket::VideoCapturer* OpenVideoCaptureDevice();

	rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
	rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory_;
	std::map<std::string, rtc::scoped_refptr<webrtc::MediaStreamInterface> > active_streams_;
};

#endif  // WEBRTC_EXAMPLES_PEERCONNECTION_CLIENT_CONDUCTOR_H_
