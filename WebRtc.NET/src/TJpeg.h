
#pragma once

#pragma unmanaged
#include <turbojpeg/turbojpeg.h>
#pragma managed

namespace TurboJpeg
{
	using namespace System;

	public ref class TurboJpegEncoder
	{
	private:
		tjhandle jpeg;
		tjhandle jpegc;

		static unsigned char * _rgbBuf;

		static TurboJpegEncoder();

		TurboJpegEncoder(tjhandle jpeg, tjhandle jpegc);

	public:
		~TurboJpegEncoder();

		static TurboJpegEncoder^ CreateEncoder();

		int EncodeJpegToI420(array<Byte> ^ buffer, array<Byte> ^% yuv);
		int EncodeRGBtoI420(array<Byte> ^ buffer, Int32 w, Int32 h, array<Byte> ^% yuv, Boolean fast);
		
		int EncodeRGB24toI420(Byte * rgbBuf, Int32 w, Int32 h, Byte * yuv, Int64 yuvSize, Boolean fast);
		int EncodeJpegToRGB24(array<Byte> ^ buffer, Int32 bufferSize, array<Byte> ^% rgb, Int32 % jwidth, Int32 % jheight);

	    static property String^ TurboJpegEncoder::LastJpegError
		{
			String^ get()
			{
				return gcnew String(tjGetErrorStr());
			}
		}
	};
}