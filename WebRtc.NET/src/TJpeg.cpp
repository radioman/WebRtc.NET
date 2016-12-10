
#include "TJpeg.h"

#pragma unmanaged
#include <math.h>
#include "internals.h"
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

				int yuvSize = tjBufSizeYUV2(width, pad, height, TJSAMP_420);
				if (yuv == nullptr)
				{
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
				int yuvSize = tjBufSizeYUV2(width, pad, height, TJSAMP_420);
				if (yuv == nullptr)
				{
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

		int TurboJpegEncoder::EncodeRGB24toI420(array<System::Byte> ^ buffer, Int32 w, Int32 h, array<System::Byte> ^% yuv, System::Boolean fast)
		{
			pin_ptr<unsigned char> bufPin = &buffer[0];
			unsigned char * rgbBuf = bufPin;
			unsigned long rgbBufSize = buffer->Length;
			{
				int pad = 4;
				int width = w;
				int height = h;
				int pitch = TJPAD(tjPixelSize[TJPF_RGB] * width);

				int yuvSize = tjBufSizeYUV2(width, pad, height, TJSAMP_420);
				if (yuv == nullptr)
				{
					yuv = gcnew array<System::Byte>(yuvSize);
				}
				pin_ptr<unsigned char> bufPinDest = &yuv[0];
				unsigned char * bufDest = bufPinDest;
				int r = tjEncodeYUV3(jpegc, rgbBuf, width, pitch, height, TJPF_RGB, bufDest, pad, TJSAMP_420, fast ? TJFLAG_FASTDCT : TJFLAG_ACCURATEDCT);
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

		int TurboJpegEncoder::EncodeBGR24toJpeg(array<System::Byte> ^ buffer, Int32 w, Int32 h, array<System::Byte> ^% jpegBuffer, Int32 % jpegOutSize, Int32 jpegQual)
		{
			//DLLEXPORT int DLLCALL tjCompress2(tjhandle handle, unsigned char *srcBuf,
			//								  int width, int pitch, int height, int pixelFormat, unsigned char **jpegBuf,
			//								  unsigned long *jpegSize, int jpegSubsamp, int jpegQual, int flags);

			pin_ptr<unsigned char> bufPin = &buffer[0];
			unsigned char * rgbBuf = bufPin;

			int width = w;
			int height = h;
			int pitch = TJPAD(tjPixelSize[TJPF_BGR] * width);

			int maxJpegSize = tjBufSize(width, height, TJSAMP_420);
			if (jpegBuffer == nullptr)
			{
				jpegBuffer = gcnew array<System::Byte>(maxJpegSize);
			}
			pin_ptr<unsigned char> bufPinDest = &jpegBuffer[0];
			unsigned char * bufDest = bufPinDest;

			unsigned long jpegSize = 0;
			int r = tjCompress2(jpegc, rgbBuf, width, pitch, height, TJPF_BGR, &bufDest, &jpegSize, TJSAMP_420, jpegQual, TJFLAG_NOREALLOC);
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
				if (pBestFactor == nullptr)
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
				if (rgb == nullptr)
				{
					int rgbSize = pitch * jheight;
					rgb = gcnew array<System::Byte>(rgbSize);
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

		void TurboJpegEncoder::FilterGray(array<Byte> ^ bufferBg, array<Byte> ^ buffer, Int32 w, Int32 h, FilterGrayParams ^ p)
		{
			try
			{
				pin_ptr<unsigned char> bufPin = &buffer[0];
				unsigned char * g = bufPin;
				Native::FilterStart(g, w, h);

				if (bufferBg != nullptr)
				{
					pin_ptr<unsigned char> bufPinBg = &bufferBg[0];
					unsigned char * gBg = bufPinBg;
					Native::FilterStartBg(gBg, w, h);

					Native::FilterSubtractBg();
				}

				if (p->FilterUnsharpMaskingGray)
				{
					Native::FilterUnsharpMaskingGray(p->FilterUnsharpMaskingGray_usm_halfwidth, p->FilterUnsharpMaskingGray_usm_fract);
				}

				if (p->FilterInvertGray)
				{
					Native::FilterInvertGray();
				}

				if (p->FilterBilateralGray)
				{
					Native::FilterBilateralGray();
				}

				if (p->FilterEqualizeTRC)
				{
					Native::FilterEqualizeTRC(p->FilterEqualizeTRC_fract, p->FilterEqualizeTRC_factor);
				}

				if (p->FilterContrastTRC)
				{
					Native::FilterContrastTRC(p->FilterContrastTRC_factor);
				}

				if (p->FilterContrastNorm)
				{
					Native::FilterContrastNorm();
				}

				if (p->FilterBlockconvGray)
				{
					Native::FilterBlockconvGray(p->FilterBlockconvGray_wc, p->FilterBlockconvGray_hc);
				}

				if (p->FilterSobelEdgeFilter)
				{
					Native::FilterSobelEdgeFilter();
				}

				if (p->FilterSauvolaBinarize)
				{
					Native::FilterSauvolaBinarize(p->FilterSauvolaBinarize_mark);
				}

				if (p->FilterOtsuAdaptiveThreshold)
				{
					Native::FilterOtsuAdaptiveThreshold(p->FilterOtsuAdaptiveThreshold_mark);
				}

				Native::FilterComplete();
			}
			catch (...)
			{
				Debug::WriteLine("TurboJpegEncoder::FilterInvertGray: unknown error");
			}
		}
	}
}