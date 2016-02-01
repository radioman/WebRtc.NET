
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

void _OnMainForm(bool param)
{
	//Util::OnMainForm(param);
}

namespace WebRtc
{
	namespace NET
	{
		public ref class ManagedConductor
		{
		private:

			bool m_isDisposed;
			Conductor * cd;

			delegate void _OnErrorCallback();
			_OnErrorCallback ^ onError;

			delegate void _OnSuccessCallback(String ^ type, String ^ sdp);
			_OnSuccessCallback ^ onSuccess;

			delegate void _OnFailureCallback(String ^ error);
			_OnFailureCallback ^ onFailure;

			delegate void _OnIceCandidateCallback(String ^ sdp);
			_OnIceCandidateCallback ^ onIceCandidate;

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

			void _OnIceCandidate(String ^ sdp)
			{
				Debug::WriteLine(String::Format("OnIceCandidate: {0}", sdp));

				OnIceCandidate(sdp);
			}

			void _OnFailure(String ^ error)
			{
				Debug::WriteLine(String::Format("OnFailure: {0}", error));

				OnFailure(error);
			}

		public:

			delegate void OnCallbackSdp(String ^ sdp);
			event OnCallbackSdp ^ OnSuccessOffer;
			event OnCallbackSdp ^ OnSuccessAnswer;
			event OnCallbackSdp ^ OnIceCandidate;
			event Action ^ OnError;

			delegate void OnCallbackError(String ^ error);
			event OnCallbackError ^ OnFailure;

			ManagedConductor()
			{
				m_isDisposed = false;
				cd = new Conductor();

				onError = gcnew _OnErrorCallback(this, &ManagedConductor::_OnError);
				GCHandle::Alloc(onError);
				IntPtr p1 = Marshal::GetFunctionPointerForDelegate(onError);
				cd->onError = static_cast<OnErrorCallbackNative>(p1.ToPointer());

				onSuccess = gcnew _OnSuccessCallback(this, &ManagedConductor::_OnSuccess);
				GCHandle::Alloc(onSuccess);
				IntPtr p2 = Marshal::GetFunctionPointerForDelegate(onSuccess);
				cd->onSuccess = static_cast<OnSuccessCallbackNative>(p2.ToPointer());

				onFailure = gcnew _OnFailureCallback(this, &ManagedConductor::_OnFailure);
				GCHandle::Alloc(onFailure);
				IntPtr p3 = Marshal::GetFunctionPointerForDelegate(onFailure);
				cd->onFailure = static_cast<OnFailureCallbackNative>(p3.ToPointer());

				onIceCandidate = gcnew _OnIceCandidateCallback(this, &ManagedConductor::_OnIceCandidate);
				GCHandle::Alloc(onIceCandidate);
				IntPtr p4 = Marshal::GetFunctionPointerForDelegate(onIceCandidate);
				cd->onIceCandidate = static_cast<OnIceCandidateCallbackNative>(p4.ToPointer());
			}

			~ManagedConductor()
			{
				if (m_isDisposed)
					return;

				// dispose managed data
				//...

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

			void Quit()
			{
				cd->Quit();
			}

			void OnOfferReply(String ^ type, String ^ sdp)
			{
				cd->OnOfferReply(marshal_as<std::string>(type), marshal_as<std::string>(sdp));
			}

			void OnOfferRequest(String ^ type, String ^ sdp)
			{
				cd->OnOfferRequest(marshal_as<std::string>(type), marshal_as<std::string>(sdp));
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
