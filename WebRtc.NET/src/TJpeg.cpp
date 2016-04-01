
#include "TJpeg.h"

using namespace WebRtc::NET;
using namespace System::Diagnostics;

TurboJpegEncoder::TurboJpegEncoder(tjhandle jpeg, tjhandle jpegc)
{
	this->jpeg = jpeg;
	this->jpegc = jpegc;
}

TurboJpegEncoder::~TurboJpegEncoder()
{
	tjDestroy(jpeg);
	tjDestroy(jpegc);
	tjFree(_rgbBuf);

	jpeg = nullptr;
	jpegc = nullptr;
	_rgbBuf = nullptr;
}

static TurboJpegEncoder::TurboJpegEncoder()
{
	_rgbBuf = nullptr;
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

int TurboJpegEncoder::EncodeRGBtoI420(array<System::Byte> ^ buffer, Int32 w, Int32 h, array<System::Byte> ^% yuv, System::Boolean fast)
{
	pin_ptr<unsigned char> bufPin = &buffer[0];
	unsigned char * rgbBuf = bufPin;
	unsigned long rgbBufSize = buffer->Length;
	{
		int pad = 4;
		int pitch = 0;
		int width = w;
		int height = h;

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

int TurboJpegEncoder::EncodeRGB24toI420(Byte * rgbBuf, Int32 w, Int32 h, Byte * yuv, Int64 yuvSize, Boolean fast)
{
	int pad = 4;
	int pitch = 0;
	int width = w;
	int height = h;

	int yuvSizeCheck = tjBufSizeYUV2(width, pad, height, TJSAMP_420);
	if (yuvSizeCheck != yuvSize)
	{
		Debug::WriteLine(String::Format("tjBufSizeYUV2, yuvSizeCheck[{0}] != {1}", yuvSizeCheck, yuvSize));
		return -1;
	}

	int r = tjEncodeYUV3(jpegc, rgbBuf, width, pitch, height, TJPF_BGR, yuv, pad, TJSAMP_420, fast ? TJFLAG_FASTDCT : TJFLAG_ACCURATEDCT);
	if (r != 0)
	{
		Debug::WriteLine(String::Format("tjEncodeYUV3, LastJpegError: {0}", LastJpegError));
		return r;
	}
	return 0;
}

int TurboJpegEncoder::EncodeJpegToRGB24(array<Byte> ^ buffer, Int32 bufferSize, array<Byte> ^% rgb, Int32 maxwidth, Int32 % jwidth, Int32 % jheight)
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

		jwidth = maxwidth < width ? maxwidth : width;
		jheight = (int)(((double)maxwidth / width) * height);

		int pad = 4;
		int pitch = 0;
		if (rgb == nullptr)
		{
			int rgbSize = TJBUFSIZE(jwidth, jheight);
			rgb = gcnew array<System::Byte>(rgbSize);
		}
		pin_ptr<unsigned char> rgbPin = &rgb[0];
		unsigned char * rgbBuf = rgbPin;

		r = tjDecompress2(jpeg, jpegBuf, jpegSize, rgbBuf, jwidth, pitch, 0, TJPF_BGR, TJFLAG_FASTDCT);
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
