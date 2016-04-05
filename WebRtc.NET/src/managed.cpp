
#pragma unmanaged
#include "internals.h"
#include "defaults.h"
#include "conductor.h"
#pragma managed

#include "msclr\marshal_cppstd.h"

using namespace System::Runtime::InteropServices;
using namespace System::Reflection;
using namespace System;
using namespace System::IO;
using namespace WebRtc::NET::Utils;
using namespace System::Diagnostics;

using namespace msclr::interop;

[assembly:System::Runtime::Versioning::TargetFrameworkAttribute(L".NETFramework,Version=v4.0", FrameworkDisplayName = L".NET Framework 4")];

namespace WebRtc
{
	namespace NET
	{
		public ref class ManagedConductor
		{
		private:

			bool m_isDisposed;
			Conductor * cd;

			delegate void _OnFillBufferCallback(uint8_t * frame_buffer, uint32_t yuvSize);
			_OnFillBufferCallback ^ onFillBuffer;
			GCHandle ^ onFillBufferHandle;

			delegate void _OnErrorCallback();
			_OnErrorCallback ^ onError;
			GCHandle ^ onErrorHandle;

			delegate void _OnSuccessCallback(String ^ type, String ^ sdp);
			_OnSuccessCallback ^ onSuccess;
			GCHandle ^ onSuccessHandle;

			delegate void _OnFailureCallback(String ^ error);
			_OnFailureCallback ^ onFailure;
			GCHandle ^ onFailureHandle;

			delegate void _OnIceCandidateCallback(String ^ sdp_mid, Int32 sdp_mline_index, String ^ sdp);
			_OnIceCandidateCallback ^ onIceCandidate;
			GCHandle ^ onIceCandidateHandle;

			void _OnError()
			{
				Debug::WriteLine("OnError");

				OnError();
			}

			void _OnSuccess(String ^ type, String ^ sdp)
			{
				Debug::WriteLine(String::Format("OnSuccess: {0} -> {1}", type, sdp));

				if (type == "offer")
				{
					OnSuccessOffer(sdp);
				}
				else if (type == "answer")
				{
					OnSuccessAnswer(sdp);
				}
			}

			void _OnIceCandidate(String ^ sdp_mid, Int32 sdp_mline_index, String ^ sdp)
			{
				Debug::WriteLine(String::Format("OnIceCandidate: {0}", sdp));

				OnIceCandidate(sdp_mid, sdp_mline_index, sdp);
			}

			void _OnFailure(String ^ error)
			{
				Debug::WriteLine(String::Format("OnFailure: {0}", error));

				OnFailure(error);
			}

			void FreeGCHandle(GCHandle ^% g)
			{
				if (g != nullptr)
				{
					g->Free();
					g = nullptr;
				}
			}

			void _OnFillBuffer(uint8_t * frame_buffer, uint32_t yuvSize)
			{
				OnFillBuffer(frame_buffer, yuvSize);
			}

		public:

			delegate void OnCallbackSdp(String ^ sdp);
			event OnCallbackSdp ^ OnSuccessOffer;
			event OnCallbackSdp ^ OnSuccessAnswer;

			delegate void OnCallbackIceCandidate(String ^ sdp_mid, Int32 sdp_mline_index, String ^ sdp);
			event OnCallbackIceCandidate ^ OnIceCandidate;

			event Action ^ OnError;

			delegate void OnCallbackError(String ^ error);
			event OnCallbackError ^ OnFailure;

			delegate void OnCallbackFillBuffer(System::Byte * frame_buffer, System::Int64 yuvSize);
			event OnCallbackFillBuffer ^ OnFillBuffer;

			ManagedConductor()
			{
				m_isDisposed = false;
				cd = new Conductor();

				onFillBuffer = gcnew _OnFillBufferCallback(this, &ManagedConductor::_OnFillBuffer);
				onFillBufferHandle = GCHandle::Alloc(onFillBuffer);
				cd->onFillBuffer = static_cast<OnFillBufferCallbackNative>(Marshal::GetFunctionPointerForDelegate(onFillBuffer).ToPointer());

				onError = gcnew _OnErrorCallback(this, &ManagedConductor::_OnError);
				onErrorHandle = GCHandle::Alloc(onError);
				cd->onError = static_cast<OnErrorCallbackNative>(Marshal::GetFunctionPointerForDelegate(onError).ToPointer());

				onSuccess = gcnew _OnSuccessCallback(this, &ManagedConductor::_OnSuccess);
				onSuccessHandle = GCHandle::Alloc(onSuccess);
				cd->onSuccess = static_cast<OnSuccessCallbackNative>(Marshal::GetFunctionPointerForDelegate(onSuccess).ToPointer());

				onFailure = gcnew _OnFailureCallback(this, &ManagedConductor::_OnFailure);
				onFailureHandle = GCHandle::Alloc(onFailure);
				cd->onFailure = static_cast<OnFailureCallbackNative>(Marshal::GetFunctionPointerForDelegate(onFailure).ToPointer());

				onIceCandidate = gcnew _OnIceCandidateCallback(this, &ManagedConductor::_OnIceCandidate);
				onIceCandidateHandle = GCHandle::Alloc(onIceCandidate);
				cd->onIceCandidate = static_cast<OnIceCandidateCallbackNative>(Marshal::GetFunctionPointerForDelegate(onIceCandidate).ToPointer());
			}

			~ManagedConductor()
			{
				if (m_isDisposed)
					return;

				// dispose managed data
				FreeGCHandle(onErrorHandle);
				FreeGCHandle(onSuccessHandle);
				FreeGCHandle(onFailureHandle);
				FreeGCHandle(onIceCandidateHandle);
				FreeGCHandle(onFillBufferHandle);
	
				this->!ManagedConductor(); // call finalizer

				m_isDisposed = true;
			}

			static void InitializeSSL()
			{
				_InitializeSSL();
			}

			static void CleanupSSL()
			{
				_CleanupSSL();
			}

			bool InitializePeerConnection()
			{
				return cd->InitializePeerConnection();
			}

			void CreateOffer()
			{
				cd->CreateOffer();
			}

			bool ProcessMessages(Int32 delay)
			{
				return cd->ProcessMessages(delay);
			}

			bool OpenVideoCaptureDevice()
			{
				return cd->OpenVideoCaptureDevice();
			}

			void OnOfferReply(String ^ type, String ^ sdp)
			{
				cd->OnOfferReply(marshal_as<std::string>(type), marshal_as<std::string>(sdp));
			}

			void OnOfferRequest(String ^ sdp)
			{
				cd->OnOfferRequest(marshal_as<std::string>(sdp));
			}

			bool AddIceCandidate(String ^ sdp_mid, Int32 sdp_mlineindex, String ^ sdp)
			{
				return cd->AddIceCandidate(marshal_as<std::string>(sdp_mid), sdp_mlineindex, marshal_as<std::string>(sdp));
			}

		protected:

			!ManagedConductor()
			{
				// free unmanaged data
				if (cd != NULL)
				{
					delete cd;
					cd = NULL;
				}
			}
		};
	}
}
