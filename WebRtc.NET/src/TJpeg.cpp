
#include "TJpeg.h"

#pragma unmanaged
#include <memory.h>
#include <math.h>
#include "internals.h"

#define L_LITTLE_ENDIAN
#include "leptonica/allheaders.h"

namespace Native
{
	__declspec(thread) PIX * px = nullptr;
	__declspec(thread) PIX * pxBg = nullptr;

	__declspec(thread) l_float32 f[4] = { 10, 5, 5, 5 };
	__declspec(thread) NUMA n;

	void FilterInitMemoryManager()
	{
		n.n = n.nalloc = 4;
		n.delx = n.refcount = 1;
		n.startx = 0;
		n.array = &f[0];

#if _DEBUG
		const char * log = "PixMemoryManagerDebug.log";
#else 
		const char * log = nullptr;
#endif
		pmsCreate(1024 * 10, 1024 * 100, &n, log);
		setPixMemoryManager(pmsCustomAlloc, pmsCustomDealloc);
	}

	void FilterCloseMemoryManager()
	{
		pmsDestroy();
	}

	void FilterMarkConnComp(PIX * bin)
	{
		BOXA * boxa = pixConnComp(bin, NULL, 8);
		int n = boxaGetCount(boxa);

		for (int i = 0; i < n; i++)
		{
			BOX * box = boxaGetBox(boxa, i, L_CLONE);
			if (box->w >= 8 && box->h >= 8)
			{
				pixRenderBox(bin, box, 2, L_FLIP_PIXELS);
			}
			boxDestroy(&box);
		}
		boxaDestroy(&boxa);
	}
}

#pragma managed

namespace WebRtc
{
	namespace NET
	{
		using namespace System::Diagnostics;
		using namespace System;
		using namespace System::Runtime::InteropServices;

		TurboJpegEncoder::TurboJpegEncoder(tjhandle jpeg, tjhandle jpegc)
		{
			this->jpeg = jpeg;
			this->jpegc = jpegc;
			pBestFactor = nullptr;
			lwidth = 0;
			lheight = 0;
		}

		TurboJpegEncoder::~TurboJpegEncoder()
		{
			tjDestroy(jpeg);
			tjDestroy(jpegc);
			tjFree(_rgbBuf);

			jpeg = nullptr;
			jpegc = nullptr;
			_rgbBuf = nullptr;
			pBestFactor = nullptr;
		}

		TurboJpegEncoder^ TurboJpegEncoder::CreateEncoder()
		{
			tjhandle jpeg = tjInitDecompress();
			if (jpeg == nullptr)
			{
				throw gcnew Exception("tjInitDecompress, LastJpegError: " + LastJpegError);
			}

			tjhandle jpegc = tjInitCompress();
			if (jpegc == nullptr)
			{
				throw gcnew Exception("tjInitCompress, LastJpegError: " + LastJpegError);
			}
			return gcnew TurboJpegEncoder(jpeg, jpegc);
		}

		int TurboJpegEncoder::EncodeJpegToI420(array<System::Byte> ^ buffer, array<System::Byte> ^% yuv)
		{
			//    DLLEXPORT int DLLCALL tjDecompressHeader3(tjhandle handle,
			//unsigned char *jpegBuf, unsigned long jpegSize, int *width, int *height,
			//int *jpegSubsamp, int *jpegColorspace);

			pin_ptr<unsigned char> bufPin = &buffer[0];
			unsigned char * jpegBuf = bufPin;
			unsigned long jpegSize = buffer->Length;

			int width, height, jpegSubsamp, jpegColorspace;
			int r = tjDecompressHeader3(jpeg, jpegBuf, jpegSize, &width, &height, &jpegSubsamp, &jpegColorspace);
			if (r != 0)
			{
				Debug::WriteLine(String::Format("tjDecompressHeader3, LastJpegError: {0}", LastJpegError));
				return r;
			}

			if (yuv == nullptr)
			{
				Debug::WriteLine(String::Format("tjDecompressHeader3 ok, width: {0}, height: {1}, jpegSubsamp: {2}, jpegColorspace: {3}", width, height, jpegSubsamp == TJSAMP_420 ? "TJSAMP_420" : jpegSubsamp.ToString(), jpegColorspace));
			}

			if (jpegSubsamp != TJSAMP_420)
			{
				//   DLLEXPORT int DLLCALL tjDecompress2(tjhandle handle,
				//unsigned char *jpegBuf, unsigned long jpegSize, unsigned char *dstBuf,
				//int width, int pitch, int height, int pixelFormat, int flags);

				int pad = 4;
				int pitch = 0;//TJPAD(tjPixelSize[TJPF_RGB] * width);
				if (_rgbBuf == nullptr)
				{
					int rgbSize = TJBUFSIZE(width, height);
					_rgbBuf = tjAlloc(rgbSize);
				}
				r = tjDecompress2(jpeg, jpegBuf, jpegSize, _rgbBuf, width, pitch, height, TJPF_RGB, TJFLAG_FASTDCT);
				if (r != 0)
				{
					Debug::WriteLine(String::Format("tjDecompress2, LastJpegError: {0}", LastJpegError));
					return r;
				}

				//Debug::WriteLine(String::Format("tjDecompress2 ok, rgbSize: {0}", rgbSize));

				//   DLLEXPORT int DLLCALL tjEncodeYUV3(tjhandle handle,
				//unsigned char *srcBuf, int width, int pitch, int height, int pixelFormat,
				//unsigned char *dstBuf, int pad, int subsamp, int flags);


				if (yuv == nullptr)
				{
					int yuvSize = tjBufSizeYUV2(width, pad, height, TJSAMP_420);
					yuv = gcnew array<System::Byte>(yuvSize);
				}
				pin_ptr<unsigned char> bufPinDest = &yuv[0];
				unsigned char * bufDest = bufPinDest;
				r = tjEncodeYUV3(jpegc, _rgbBuf, width, pitch, height, TJPF_RGB, bufDest, pad, TJSAMP_420, TJFLAG_FASTDCT);
				if (r != 0)
				{
					Debug::WriteLine(String::Format("tjEncodeYUV3, LastJpegError: {0}", LastJpegError));
					return r;
				}

				//Debug::WriteLine(String::Format("tjEncodeYUV3 ok, yuvSize: {0}", yuvSize));   
			}
			else // already TJSAMP_420
			{
				//    DLLEXPORT int DLLCALL tjDecompressToYUV2(tjhandle handle,
				//unsigned char *jpegBuf, unsigned long jpegSize, unsigned char *dstBuf,
				//int width, int pad, int height, int flags);

				int pad = 4;
				if (yuv == nullptr)
				{
					int yuvSize = tjBufSizeYUV2(width, pad, height, TJSAMP_420);
					yuv = gcnew array<System::Byte>(yuvSize);
				}
				pin_ptr<unsigned char> bufPinDest = &yuv[0];
				unsigned char * bufDest = bufPinDest;
				r = tjDecompressToYUV2(jpeg, jpegBuf, jpegSize, bufDest, width, pad, height, TJFLAG_FASTDCT);
				if (r != 0)
				{
					Debug::WriteLine(String::Format("tjDecompressToYUV2, LastJpegError: {0}", LastJpegError));
					return r;
				}

				//Debug::WriteLine(String::Format("tjDecompressToYUV2 ok, yuvSize: {0}", yuvSize));
			}

			return 0;
		}

		int TurboJpegEncoder::EncodeI420(array<System::Byte> ^ buffer, Int32 w, Int32 h, Int32 pxFormat, System::Boolean fast, array<System::Byte> ^% yuv)
		{
			pin_ptr<unsigned char> bufPin = &buffer[0];
			unsigned char * rgbBuf = bufPin;
			unsigned long rgbBufSize = buffer->Length;
			{
				int pad = 4;
				int pitch = 0;
				int width = w;
				int height = h;

				if (yuv == nullptr)
				{
					int yuvSize = tjBufSizeYUV2(width, pad, height, TJSAMP_420);
					yuv = gcnew array<System::Byte>(yuvSize);
				}
				pin_ptr<unsigned char> bufPinDest = &yuv[0];
				unsigned char * bufDest = bufPinDest;
				int r = tjEncodeYUV3(jpegc, rgbBuf, width, pitch, height, pxFormat, bufDest, pad, TJSAMP_420, fast ? TJFLAG_FASTDCT : TJFLAG_ACCURATEDCT);
				if (r != 0)
				{
					Debug::WriteLine(String::Format("tjEncodeYUV3, LastJpegError: {0}", LastJpegError));
					return r;
				}
			}
			return 0;
		}

		int TurboJpegEncoder::EncodeBGR24toI420(Byte * rgbBuf, Int32 w, Int32 h, Byte * yuv, Int64 yuvSize, Boolean fast)
		{
			int pad = 4;

			int width = w;
			int height = h;
			int pitch = TJPAD(tjPixelSize[TJPF_BGR] * width);

			if (yuvSize > 0)
			{
				int yuvSizeCheck = tjBufSizeYUV2(width, pad, height, TJSAMP_420);
				if (yuvSizeCheck != yuvSize)
				{
					Debug::WriteLine(String::Format("tjBufSizeYUV2, yuvSizeCheck[{0}] != {1}", yuvSizeCheck, yuvSize));
					return -1;
				}
			}

			int r = tjEncodeYUV3(jpegc, rgbBuf, width, pitch, height, TJPF_BGR, yuv, pad, TJSAMP_420, fast ? TJFLAG_FASTDCT : TJFLAG_ACCURATEDCT);
			if (r != 0)
			{
				Debug::WriteLine(String::Format("tjEncodeYUV3, LastJpegError: {0}", LastJpegError));
				return r;
			}
			return 0;
		}

		int TurboJpegEncoder::EncodeI420toBGR24(Byte * yuv, UInt32 w, UInt32 h, array<System::Byte> ^% bgrBuffer, Boolean fast)
		{
			int pad = 4;
			int width = w;
			int height = h;
			int pitch = TJPAD(tjPixelSize[TJPF_BGR] * width);

			if (bgrBuffer == nullptr)
			{
				int rgbSize = pitch * height;
				bgrBuffer = gcnew array<System::Byte>(rgbSize);
			}
			pin_ptr<unsigned char> bufPinDest = &bgrBuffer[0];
			unsigned char * bufDest = bufPinDest;

			int r = tjDecodeYUV(jpeg, yuv, pad, TJSAMP_420, bufDest, width, pitch, height, TJPF_BGR, fast ? TJFLAG_FASTDCT : TJFLAG_ACCURATEDCT);
			if (r != 0)
			{
				Debug::WriteLine(String::Format("tjDecodeYUV, LastJpegError: {0}", LastJpegError));
				return r;
			}
			return 0;
		}

		int TurboJpegEncoder::EncodeToJpeg(array<System::Byte> ^ buffer, Int32 w, Int32 h, Int32 pxFormat, array<System::Byte> ^% jpegBuffer, Int32 % jpegOutSize, Int32 jpegQual)
		{
			//DLLEXPORT int DLLCALL tjCompress2(tjhandle handle, unsigned char *srcBuf,
			//								  int width, int pitch, int height, int pixelFormat, unsigned char **jpegBuf,
			//								  unsigned long *jpegSize, int jpegSubsamp, int jpegQual, int flags);

			pin_ptr<unsigned char> bufPin = &buffer[0];
			unsigned char * rgbBuf = bufPin;

			int width = w;
			int height = h;
			int pitch = TJPAD(tjPixelSize[pxFormat] * width);

			int maxJpegSize = tjBufSize(width, height, TJSAMP_420);
			if (jpegBuffer == nullptr || jpegBuffer->Length < maxJpegSize)
			{
				jpegBuffer = gcnew array<System::Byte>(maxJpegSize);
			}
			pin_ptr<unsigned char> bufPinDest = &jpegBuffer[0];
			unsigned char * bufDest = bufPinDest;

			unsigned long jpegSize = 0;
			int r = tjCompress2(jpegc, rgbBuf, width, pitch, height, pxFormat, &bufDest, &jpegSize, TJSAMP_420, jpegQual, TJFLAG_NOREALLOC);
			if (r != 0)
			{
				Debug::WriteLine(String::Format("tjCompress2, LastJpegError: {0}", LastJpegError));
				return r;
			}
			jpegOutSize = jpegSize;
			return 0;
		}

		int TurboJpegEncoder::EncodeJpeg(array<Byte> ^ buffer, Int32 bufferSize, array<Byte> ^% rgb, Int32 maxwidth, Double scale, Int32 % jwidth, Int32 % jheight, Int32 % pitch, Int32 pxFormat)
		{
			try
			{
				pin_ptr<unsigned char> bufPin = &buffer[0];
				unsigned char * jpegBuf = bufPin;
				unsigned long jpegSize = bufferSize;

				int width, height, jpegSubsamp, jpegColorspace;
				int r = tjDecompressHeader3(jpeg, jpegBuf, jpegSize, &width, &height, &jpegSubsamp, &jpegColorspace);
				if (r != 0)
				{
					Debug::WriteLine(String::Format("tjDecompressHeader3, LastJpegError: {0}", LastJpegError));
					return r;
				}

				if (rgb == nullptr)
				{
					Debug::WriteLine(String::Format("tjDecompressHeader3 ok, width: {0}, height: {1}, jpegSubsamp: {2}, jpegColorspace: {3}", width, height, jpegSubsamp == TJSAMP_420 ? "TJSAMP_420" : jpegSubsamp.ToString(), jpegColorspace));
				}

				// Select best scaling factor depending of the desired size
				if (pBestFactor == nullptr || width != lwidth || height != lheight)
				{
					lwidth = width;
					lheight = height;

					double fDesiredFactor = scale > 0 ? scale : ((double)(maxwidth < width ? maxwidth : width) / (double)width);

					int iNumScalingFactors;
					tjscalingfactor* pListFactors = tjGetScalingFactors(&iNumScalingFactors);
					double fBestFactor = 1.0;
					double fFactor;

					for (int i = 0; i < iNumScalingFactors; i++)
					{
						fFactor = ((double)pListFactors[i].num / (double)pListFactors[i].denom);
						if (fFactor == fDesiredFactor)
						{
							// We found the best
							pBestFactor = &pListFactors[i];
							fBestFactor = fFactor;
							break;
						}

						if (!pBestFactor)
						{
							pBestFactor = &pListFactors[i];
							fBestFactor = fFactor;
						}
						else
						{
							if (fabs(fFactor - fDesiredFactor) < fabs(fBestFactor - fDesiredFactor))
							{
								pBestFactor = &pListFactors[i];
								fBestFactor = fFactor;
							}
						}
					}
				}

				jwidth = TJSCALED(width, (*pBestFactor));
				jheight = TJSCALED(height, (*pBestFactor));
				pitch = TJPAD(tjPixelSize[pxFormat] * jwidth);

				if (rgb == nullptr || rgb->Length < pitch * jheight)
				{
					rgb = gcnew array<System::Byte>(pitch * jheight);
				}
				pin_ptr<unsigned char> rgbPin = &rgb[0];
				unsigned char * rgbBuf = rgbPin;

				r = tjDecompress2(jpeg, jpegBuf, jpegSize, rgbBuf, jwidth, pitch, jheight, pxFormat, TJFLAG_FASTDCT);
				if (r != 0)
				{
					Debug::WriteLine(String::Format("tjDecompress2, LastJpegError: {0}", LastJpegError));
				}

				//Debug::WriteLine(String::Format("tjDecompress2 ok, rgbSize: {0}", rgbSize));

				return r;
			}
			catch (...)
			{
				Debug::WriteLine("TurboJpegEncoder::EncodeJpegToRGB24: unknown error");
			}
			return -1;
		}

		int TurboJpegEncoder::DecodeJpeg(array<Byte> ^ buffer, Int32 bufferSize, Int32 maxwidth, Double scale, Int32 % jwidth, Int32 % jheight)
		{
			try
			{
				pin_ptr<unsigned char> bufPin = &buffer[0];
				unsigned char * jpegBuf = bufPin;
				unsigned long jpegSize = bufferSize;

				int width, height, jpegSubsamp, jpegColorspace;
				int r = tjDecompressHeader3(jpeg, jpegBuf, jpegSize, &width, &height, &jpegSubsamp, &jpegColorspace);
				if (r != 0)
				{
					Debug::WriteLine(String::Format("tjDecompressHeader3, LastJpegError: {0}", LastJpegError));
					return r;
				}

				tjscalingfactor * scaleFactor = nullptr;

				// Select best scaling factor depending of the desired size
				if (scaleFactor == nullptr)
				{
					double fDesiredFactor = scale > 0 ? scale : ((double)(maxwidth < width ? maxwidth : width) / (double)width);

					int iNumScalingFactors;
					tjscalingfactor* pListFactors = tjGetScalingFactors(&iNumScalingFactors);
					double fBestFactor = 1.0;
					double fFactor;

					for (int i = 0; i < iNumScalingFactors; i++)
					{
						fFactor = ((double)pListFactors[i].num / (double)pListFactors[i].denom);
						if (fFactor == fDesiredFactor)
						{
							// We found the best
							scaleFactor = &pListFactors[i];
							fBestFactor = fFactor;
							break;
						}

						if (!scaleFactor)
						{
							scaleFactor = &pListFactors[i];
							fBestFactor = fFactor;
						}
						else
						{
							if (fabs(fFactor - fDesiredFactor) < fabs(fBestFactor - fDesiredFactor))
							{
								scaleFactor = &pListFactors[i];
								fBestFactor = fFactor;
							}
						}
					}
				}

				jwidth = TJSCALED(width, (*scaleFactor));
				jheight = TJSCALED(height, (*scaleFactor));

				return r;
			}
			catch (...)
			{
				Debug::WriteLine("TurboJpegEncoder::DecodeJpeg: unknown error");
			}
			return -1;
		}

		int TurboJpegEncoder::EncodeGrayToJpeg(array<System::Byte> ^ buffer, Int32 w, Int32 h, array<System::Byte> ^% jpegBuffer, Int32 % jpegOutSize, Int32 jpegQual)
		{
			//DLLEXPORT int DLLCALL tjCompress2(tjhandle handle, unsigned char *srcBuf,
			//								  int width, int pitch, int height, int pixelFormat, unsigned char **jpegBuf,
			//								  unsigned long *jpegSize, int jpegSubsamp, int jpegQual, int flags);

			pin_ptr<unsigned char> bufPin = &buffer[0];
			unsigned char * rgbBuf = bufPin;

			int width = w;
			int height = h;
			int pitch = TJPAD(tjPixelSize[TJPF_GRAY] * width);

			int maxJpegSize = tjBufSize(width, height, TJSAMP_GRAY);
			if (jpegBuffer == nullptr || jpegBuffer->Length < maxJpegSize)
			{
				jpegBuffer = gcnew array<System::Byte>(maxJpegSize);
			}
			pin_ptr<unsigned char> bufPinDest = &jpegBuffer[0];
			unsigned char * bufDest = bufPinDest;

			unsigned long jpegSize = 0;
			int r = tjCompress2(jpegc, rgbBuf, width, pitch, height, TJPF_GRAY, &bufDest, &jpegSize, TJSAMP_GRAY, jpegQual, TJFLAG_NOREALLOC);
			if (r != 0)
			{
				Debug::WriteLine(String::Format("tjCompress2, LastJpegError: {0}", LastJpegError));
				return r;
			}
			jpegOutSize = jpegSize;
			return 0;
		}
	
		namespace Filters
		{
			public ref class Filter
			{
			private:
				bool m_isDisposed;
				GCHandle ^ data_handle;
				IntPtr dataPointer;

				static void DestroyPIX(interior_ptr<PIX> pi)
				{
					pin_ptr<PIX> pinPx = pi;
					PIX * px = pinPx;
					pixDestroy(&px);
				}

			public:

				Filter()
				{
					m_isDisposed = false;
					data_handle = nullptr;
					data = nullptr;
					dataPointer = IntPtr::Zero;

					pxIn = nullptr;
					pxOut = nullptr;

					ownedIn = false;
					ownedOut = true;

					Native::FilterInitMemoryManager();
				}

				~Filter()
				{
					if (m_isDisposed)
						return;

					// dispose managed data
					if (data_handle != nullptr)
					{
						data_handle->Free();
						data_handle = nullptr;
					}
					dataPointer = IntPtr::Zero;

					this->!Filter(); // call finalizer

					m_isDisposed = true;
				}

				void PinnBuffer(array<Byte> ^ buffer)
				{
					try
					{
						if (data_handle != nullptr)
						{
							data_handle->Free();
							data_handle = nullptr;
						}
						data_handle = GCHandle::Alloc(buffer, GCHandleType::Pinned);
						dataPointer = data_handle->AddrOfPinnedObject();
						data = (unsigned char*)dataPointer.ToPointer();
					}
					catch (...)
					{
						throw gcnew Exception("PinnBuffer failed...");
					}
				}

				void SetPinnedBuffer(IntPtr h)
				{
					try
					{
						if (data_handle != nullptr)
						{
							data_handle->Free();
							data_handle = nullptr;
						}
						dataPointer = h;
						data = (unsigned char*)dataPointer.ToPointer();
					}
					catch (...)
					{
						throw gcnew Exception("SetPinnedBuffer failed...");
					}
				}

				property IntPtr BufferHandle
				{
					IntPtr get()
					{
						return dataPointer;
					}
				}

				property Boolean BufferHandleValid
				{
					Boolean get()
					{
						return (dataPointer != IntPtr::Zero);
					}
				}

				void SetInput(Int32 w, Int32 h)
				{
					try
					{
						if (pxIn != nullptr && ownedIn)
						{
							pxIn->data = nullptr;
							DestroyPIX(pxIn);
							pxIn = nullptr;
						}
						pxIn = pixCreateHeader(w, h, 8);
						pxIn->data = (l_uint32*)data;
						ownedIn = true;
					}
					catch (...)
					{
						throw gcnew Exception("Filter::SetInput failed...");
					}
				}

				void SetInput(Int32 w, Int32 h, Int32 d)
				{
					try
					{
						if (pxIn != nullptr && ownedIn)
						{
							pxIn->data = nullptr;
							DestroyPIX(pxIn);
							pxIn = nullptr;
						}
						pxIn = pixCreateHeader(w, h, d);
						pxIn->data = (l_uint32*)data;
						ownedIn = true;
					}
					catch (...)
					{
						throw gcnew Exception("Filter::SetInput2 failed...");
					}
				}

				void SwapIn()
				{
					try
					{
						pin_ptr<PIX> pinPx = pxIn;
						PIX * px = pinPx;
						pixEndianByteSwap(px);
					}
					catch (...)
					{
						throw gcnew Exception("Filter::SwapIn failed...");
					}
				}

				void SwapOut()
				{
					try
					{
						pin_ptr<PIX> pinPx = pxOut;
						PIX * px = pinPx;
						pixEndianByteSwap(px);
					}
					catch (...)
					{
						throw gcnew Exception("Filter::SwapOut failed...");
					}
				}

				virtual void Apply()
				{
					if (pxOut != nullptr && ownedOut)
					{
						DestroyPIX(pxOut);
						pxOut = nullptr;
					}
				}

				void SetInputFromInput(Filter ^ f)
				{
					pxIn = f->pxIn;
					ownedIn = false;
				}

				void SetInputFromOutput(Filter ^ f)
				{
					pxIn = f->pxOut;
					ownedIn = false;
				}

				void SetOutputFromInput(Filter ^ f)
				{
					pxOut = f->pxIn;
					ownedOut = false;
				}

				void SetOutputFromOutput(Filter ^ f)
				{
					pxOut = f->pxOut;
					ownedOut = false;
				}

				array<Byte> ^ GetOutClone()
				{
					try
					{
						array<Byte> ^ r = gcnew array<System::Byte>(4 * pxOut->wpl * pxOut->h);

						pin_ptr<unsigned char> bufPinDest = &r[0];
						unsigned char * d = bufPinDest;

						memcpy(d, pxOut->data, r->Length);

						return r;
					}
					catch (...)
					{
						throw gcnew Exception("Filter::GetOut failed...");
					}
				}

				void GetOutSize(System::Int32 % w, System::Int32 % h)
				{
					try
					{
						w = (System::Int32)pxOut->w;
						h = (System::Int32)pxOut->h;
					}
					catch (...)
					{
						throw gcnew Exception("Filter::GetOutSize failed...");
					}
				}

				System::Single GetInAvgIntensity()
				{
					try
					{
						pin_ptr<PIX> pinPx = pxIn;
						PIX * px = pinPx;

						l_float32 border_avg = pixAverageOnLine(px, 0, 0, px->w - 1, 0, 1);        // Top 
						border_avg += pixAverageOnLine(px, 0, px->h - 1, px->w - 1, px->h - 1, 1); // Bottom
						border_avg += pixAverageOnLine(px, 0, 0, 0, px->h - 1, 1);                 // Left 
						border_avg += pixAverageOnLine(px, px->w - 1, 0, px->w - 1, px->h - 1, 1); // Right  
						border_avg /= 4.0f;

						return System::Math::Round(border_avg, 2, System::MidpointRounding::AwayFromZero);
					}
					catch (...)
					{
						throw gcnew Exception("Filter::GetInAvgIntensity failed...");
					}
				}


			protected:

				unsigned char * data;
				bool ownedIn;
				bool ownedOut;
				PIX * pxIn;
				PIX * pxOut;

				!Filter()
				{
					data = nullptr;

					if (pxIn != nullptr && ownedIn)
					{
						pxIn->data = nullptr;
						DestroyPIX(pxIn);
					}
					pxIn = nullptr;

					if (pxOut != nullptr && ownedOut)
					{
						DestroyPIX(pxOut);
						pxOut = nullptr;
					}
				}

				static Single DegreeToRadian(Single angle)
				{
					return Math::PI * angle / 180.0;
				}
			};

			public ref class FilterSelectBox : Filter
			{
			public:

				Int32 boxX;
				Int32 boxY;
				Int32 boxW;
				Int32 boxH;

				void Apply() override
				{
					try
					{
						Filter::Apply();

						pin_ptr<PIX> pinPx = pxIn;
						PIX * px = pinPx;

						BOX b;
						b.x = boxX;
						b.y = boxY;
						b.w = boxW;
						b.h = boxH;

						pxOut = pixClipRectangle(px, &b, nullptr);
					}
					catch (...)
					{
						Trace::WriteLine("FilterSelectBox::Apply: unknown error");
					}
				}
			};

			public ref class FilterUnsharpMaskingGray : Filter
			{
			public:

				Int32 usm_halfwidth = 5;
				Single usm_fract = 1.0f;

				void Apply() override
				{
					try
					{
						Filter::Apply();

						pin_ptr<PIX> pinPx = pxIn;
						PIX * px = pinPx;

						//*\param[in]    halfwidth  "half-width" of smoothing filter
						//	* \param[in]    fract  fraction of edge added back into image

						//*(1) We use symmetric smoothing filters of odd dimension,
						//	*typically use sizes of 3, 5, 7, etc.The %halfwidth parameter
						//	*          for these is(size - 1) / 2; i.e., 1, 2, 3, etc.
						//	*      (2) The fract parameter is typically taken in the range :
						//*0.2 \< fract \< 0.7

						pxOut = pixUnsharpMaskingGray(px, usm_halfwidth, usm_fract);
					}
					catch (...)
					{
						Trace::WriteLine("FilterUnsharpMaskingGray::Apply: unknown error");
					}
				}
			};

			public ref class FilterEqualizeTRC : Filter
			{
			public:
				Single fract = 0.9f;
				Int32 factor = 1;

				void Apply() override
				{
					try
					{
						Filter::Apply();

						pin_ptr<PIX> pinPx = pxIn;
						PIX * px = pinPx;

						//*\param[in]    fract fraction of equalization movement of pixel values
						//	* \param[in]    factor subsampling factor; integer >= 1

						//*(3) If fract == 0.0, no equalization is performed; return a copy
						//	*          unless in - place, in which case this is a no - op.
						//	*          If fract == 1.0, equalization is complete.

						pxOut = pixEqualizeTRC(nullptr, px, fract, factor);
					}
					catch (...)
					{
						Trace::WriteLine("FilterEqualizeTRC::Apply: unknown error");
					}
				}
			};

			public ref class FilterContrastTRC : Filter
			{
			public:
				Single factor = 1.0f;

				void Apply() override
				{
					try
					{
						Filter::Apply();

						pin_ptr<PIX> pinPx = pxIn;
						PIX * px = pinPx;

						//(4) The useful range for the contrast factor is scaled to
						//	*          be in(0.0 to 1.0), but larger values can also be used.
						pxOut = pixContrastTRC(nullptr, px, factor);
					}
					catch (...)
					{
						Trace::WriteLine("FilterContrastTRC::Apply: unknown error");
					}
				}
			};

			public ref class FilterInvertGray : Filter
			{
			public:

				void Apply() override
				{
					try
					{
						Filter::Apply();

						pin_ptr<PIX> pinPx = pxIn;
						PIX * px = pinPx;

						pxOut = pixInvert(nullptr, px);
					}
					catch (...)
					{
						Trace::WriteLine("FilterInvertGray::Apply: unknown error");
					}
				}
			};

			public ref class FilterSauvolaBinarize : Filter
			{
			public:
				Boolean mark = false;

				void Apply() override
				{
					try
					{
						Filter::Apply();

						pin_ptr<PIX> pinPx = pxIn;
						PIX * px = pinPx;

						//*\param[in]    whsize window half - width for measuring local statistics
						//	* \param[in]    factor factor for reducing threshold due to variance; >= 0
						//	* \param[in]    nx, ny subdivision into tiles; >= 1

						PIX * bin;
						pixSauvolaBinarizeTiled(px, 8, 0.34, 1, 1, NULL, &bin);

						if (mark)
						{
							Native::FilterMarkConnComp(bin);
						}

						pxOut = pixConvert1To8(nullptr, bin, 255, 0);
						pixDestroy(&bin);
					}
					catch (...)
					{
						Trace::WriteLine("FilterSauvolaBinarize::Apply: unknown error");
					}
				}
			};

			public ref class FilterOtsuAdaptiveThreshold : Filter
			{
			public:
				Boolean mark = false;

				void Apply() override
				{
					try
					{
						Filter::Apply();

						pin_ptr<PIX> pinPx = pxIn;
						PIX * px = pinPx;

						l_int32 otsu_sx = 2000;
						l_int32 otsu_sy = 2000;
						l_int32 otsu_smoothx = 0;
						l_int32 otsu_smoothy = 0;
						l_float32 otsu_scorefract = 0.1f;

						//*\param[in]    sx, sy desired tile dimensions; actual size may vary
						//	* \param[in]    smoothx, smoothy half - width of convolution kernel applied to
						//	*                                threshold array: use 0 for no smoothing
						//	* \param[in]    scorefract fraction of the max Otsu score; typ. 0.1;
						//*use 0.0 for standard Otsu

						PIX * bin;
						pixOtsuAdaptiveThreshold(px, otsu_sx, otsu_sy, otsu_smoothx, otsu_smoothy, otsu_scorefract, NULL, &bin);

						if (mark)
						{
							Native::FilterMarkConnComp(bin);
						}

						pxOut = pixConvert1To8(nullptr, bin, 255, 0);
						pixDestroy(&bin);
					}
					catch (...)
					{
						Trace::WriteLine("FilterOtsuAdaptiveThreshold::Apply: unknown error");
					}
				}
			};

			public ref class FilterSobelEdge : Filter
			{
			public:
				void Apply() override
				{
					try
					{
						Filter::Apply();

						pin_ptr<PIX> pinPx = pxIn;
						PIX * px = pinPx;

						pxOut = pixSobelEdgeFilter(px, L_ALL_EDGES);
					}
					catch (...)
					{
						Trace::WriteLine("FilterSobelEdgeFilter::Apply: unknown error");
					}
				}
			};

			public ref class FilterBlockconvGray : Filter
			{
			public:
				Int32 wc = 2;
				Int32 hc = 2;

				void Apply() override
				{
					try
					{
						Filter::Apply();

						pin_ptr<PIX> pinPx = pxIn;
						PIX * px = pinPx;

						// * \param[in]    wc, hc   half width/height of convolution kernel
						//*(4) Require that w \ >= 2 * wc + 1 and h \ >= 2 * hc + 1,
						//	*where(w, h) are the dimensions of pixs.

						pxOut = pixBlockconvGray(px, nullptr, wc, hc);
					}
					catch (...)
					{
						Trace::WriteLine("FilterBlockconvGray::Apply: unknown error");
					}
				}
			};

			public ref class FilterBilateralGray : Filter
			{
			public:

				void Apply() override
				{
					try
					{
						Filter::Apply();

						pin_ptr<PIX> pinPx = pxIn;
						PIX * px = pinPx;

						//spatial_stdev(of gaussian kernel; in pixels, > 0.5)
						//	range_stdev(of gaussian range kernel; > 5.0; typ. 50.0)
						//	ncomps(number of intermediate sums J(k, x); in[4 ... 30])
						//	reduction(1, 2 or 4)

						pxOut = pixBilateralGray(px, 4, 45, 30, 4);
					}
					catch (...)
					{
						Trace::WriteLine("FilterBilateralGray::Apply: unknown error");
					}
				}
			};

			public ref class FilterMedian : Filter
			{
			public:
				Int32 wf = 5;
				Int32 hf = 5;

				void Apply() override
				{
					try
					{
						Filter::Apply();

						pin_ptr<PIX> pinPx = pxIn;
						PIX * px = pinPx;

						pxOut = pixMedianFilter(px, wf, hf);
					}
					catch (...)
					{
						Trace::WriteLine("FilterMedian::Apply: unknown error");
					}
				}
			};

			public ref class FilterContrastNorm : Filter
			{
			public:

				void Apply() override
				{
					try
					{
						Filter::Apply();

						pin_ptr<PIX> pinPx = pxIn;
						PIX * px = pinPx;

						//*\param[in]    sx, sy tile dimensions
						//	* \param[in]    mindiff minimum difference to accept as valid
						//	* \param[in]    smoothx, smoothy half - width of convolution kernel applied to
						//	*                                min and max arrays : use 0 for no smoothing
						pxOut = pixContrastNorm(nullptr, px, 100, 100, 55, 1, 1);
					}
					catch (...)
					{
						Trace::WriteLine("FilterContrastNorm::Apply: unknown error");
					}
				}
			};

			public ref class FilterSubtractBackground : Filter
			{
			public:

				Filter ^ Background;

				void Apply() override
				{
					try
					{
						Filter::Apply();

						pin_ptr<PIX> pinPx = pxIn;
						PIX * px = pinPx;

						//pxOut = pixSubtractGray(nullptr, px, Background->pxIn);
					}
					catch (...)
					{
						Trace::WriteLine("FilterSubtractBackground::Apply: unknown error");
					}
				}
			};

			public ref class FilterPaint : Filter
			{
			public:

				void Apply() override
				{
					try
					{
						Filter::Apply();

						pin_ptr<PIX> pinPx = pxIn;
						PIX * px = pinPx;

						pin_ptr<PIX> pinPxOut = pxOut;
						PIX * pxOut = pinPxOut;

						pixRasterop(pxOut, 0, 0, px->w, px->h, PIX_SRC, px, 0, 0);
					}
					catch (...)
					{
						Trace::WriteLine("FilterPaint::Apply: unknown error");
					}
				}
			};

			public ref class FilterScaleUp2x : Filter
			{
			public:

				void Apply() override
				{
					try
					{
						Filter::Apply();

						pin_ptr<PIX> pinPx = pxIn;
						PIX * px = pinPx;

						pxOut = pixScaleGray2xLI(px);
					}
					catch (...)
					{
						Trace::WriteLine("FilterScaleUp2x::Apply: unknown error");
					}
				}
			};

			public ref class FilterScaleUp4x : Filter
			{
			public:

				void Apply() override
				{
					try
					{
						Filter::Apply();

						pin_ptr<PIX> pinPx = pxIn;
						PIX * px = pinPx;

						pxOut = pixScaleGray4xLI(px);
					}
					catch (...)
					{
						Trace::WriteLine("FilterScaleUp4x::Apply: unknown error");
					}
				}
			};

			public ref class FilterScaleDown2x : Filter
			{
			public:

				void Apply() override
				{
					try
					{
						Filter::Apply();

						pin_ptr<PIX> pinPx = pxIn;
						PIX * px = pinPx;

						pxOut = pixScaleAreaMap(px, 0.5f, 0.5f);
					}
					catch (...)
					{
						Trace::WriteLine("FilterScaleDown2x::Apply: unknown error");
					}
				}
			};

			public ref class FilterScaleDown4x : Filter
			{
			public:

				void Apply() override
				{
					try
					{
						Filter::Apply();

						pin_ptr<PIX> pinPx = pxIn;
						PIX * px = pinPx;

						pxOut = pixScaleAreaMap(px, 0.25f, 0.25f);
					}
					catch (...)
					{
						Trace::WriteLine("FilterScaleDown4x::Apply: unknown error");
					}
				}
			};

			public ref class FilterScaleDown8x : Filter
			{
			public:

				void Apply() override
				{
					try
					{
						Filter::Apply();

						pin_ptr<PIX> pinPx = pxIn;
						PIX * px = pinPx;

						pxOut = pixScaleAreaMap(px, 0.125f, 0.125f);
					}
					catch (...)
					{
						Trace::WriteLine("FilterScaleDown8x::Apply: unknown error");
					}
				}
			};

			public ref class FilterScaleToSize : Filter
			{
			public:

				Int32 w;
				Int32 h;

				void Apply() override
				{
					try
					{
						Filter::Apply();

						pin_ptr<PIX> pinPx = pxIn;
						PIX * px = pinPx;

						pxOut = pixScaleBySamplingToSize(px, w, h);
					}
					catch (...)
					{
						Trace::WriteLine("FilterScaleToSize::Apply: unknown error");
					}
				}
			};

			public ref class FilterRotate : Filter
			{
			public:

				Single angle = 0.0f;

				void Apply() override
				{
					try
					{
						Filter::Apply();

						pin_ptr<PIX> pinPx = pxIn;
						PIX * px = pinPx;

						pxOut = pixRotate(px, DegreeToRadian(angle), L_ROTATE_AREA_MAP, L_BRING_IN_BLACK, 0, 0);
					}
					catch (...)
					{
						Trace::WriteLine("FilterRotate::Apply: unknown error");
					}
				}
			};

			public ref class FilterShear : Filter
			{
			public:

				Single angle = 0.0f;

				void Apply() override
				{
					try
					{
						Filter::Apply();

						pin_ptr<PIX> pinPx = pxIn;
						PIX * px = pinPx;

						pxOut = pixHShearCenter(nullptr, px, DegreeToRadian(angle), L_BRING_IN_BLACK);
					}
					catch (...)
					{
						Trace::WriteLine("FilterShear::Apply: unknown error");
					}
				}
			};

			public ref class FilterProjective : Filter
			{
			public:

				Single angle = 0.0f;

				void Apply() override
				{
					try
					{
						Filter::Apply();

						pin_ptr<PIX> pinPx = pxIn;
						PIX * px = pinPx;

						l_float32 p[8] =
						{
							0.5, 0.0f, 0.0f,
							0.0f,     1.0f, 0.0f,
							-0.00016f, 0.0f
						};
						pxOut = pixProjective(px, p, 0);
					}
					catch (...)
					{
						Trace::WriteLine("FilterProjective::Apply: unknown error");
					}
				}
			};
		}
   }
}
