
#pragma once

#pragma unmanaged
#include <turbojpeg/turbojpeg.h>
#pragma managed

namespace WebRtc
{
	namespace NET
	{
		using namespace System;

		public ref class TurboJpegEncoder
		{
		private:
			tjhandle jpeg;
			tjhandle jpegc;
			tjscalingfactor * pBestFactor;
			int lwidth;
			int lheight;

			unsigned char * _rgbBuf;

			TurboJpegEncoder(tjhandle jpeg, tjhandle jpegc);

		public:

			~TurboJpegEncoder();

			static TurboJpegEncoder^ CreateEncoder();

			int EncodeJpegToI420(array<Byte> ^ buffer, array<Byte> ^% yuv);

			int EncodeI420(array<System::Byte> ^ buffer, Int32 w, Int32 h, Int32 pxFormat, System::Boolean fast, array<System::Byte> ^% yuv);
			int EncodeBGR24toI420(Byte * rgbBuf, Int32 w, Int32 h, Byte * yuv, Int64 yuvSize, Boolean fast);
			int EncodeI420toBGR24(Byte * yuv, UInt32 w, UInt32 h, array<System::Byte> ^% bgrBuffer, Boolean fast);

			int EncodeJpeg(array<Byte> ^ buffer, Int32 bufferSize, array<Byte> ^% rgb, Int32 maxwidth, Double scale, Int32 % jwidth, Int32 % jheight, Int32 % pitch, Int32 pxFormat);
			int DecodeJpeg(array<Byte> ^ buffer, Int32 bufferSize, Int32 maxwidth, Double scale, Int32 % jwidth, Int32 % jheight);

			int EncodeToJpeg(array<System::Byte> ^ buffer, Int32 w, Int32 h, Int32 pxFormat, array<System::Byte> ^% jpegBuffer, Int32 % jpegOutSize, Int32 jpegQual);
			int EncodeGrayToJpeg(array<System::Byte> ^ buffer, Int32 w, Int32 h, array<System::Byte> ^% jpegBuffer, Int32 % jpegOutSize, Int32 jpegQual);

			void ResetScale()
			{
				pBestFactor = nullptr;
				lwidth = 0;
				lheight = 0;
			}

			static property String^ TurboJpegEncoder::LastJpegError
			{
				String^ get()
				{
					return gcnew String(tjGetErrorStr());
				}
			}
		};

		/**
		* Pixel formats
		*/
		public enum class TJPF
		{
			/**
			* RGB pixel format.  The red, green, and blue components in the image are
			* stored in 3-byte pixels in the order R, G, B from lowest to highest byte
			* address within each pixel.
			*/
			TJPF_RGB = 0,
			/**
			* BGR pixel format.  The red, green, and blue components in the image are
			* stored in 3-byte pixels in the order B, G, R from lowest to highest byte
			* address within each pixel.
			*/
			TJPF_BGR,
			/**
			* RGBX pixel format.  The red, green, and blue components in the image are
			* stored in 4-byte pixels in the order R, G, B from lowest to highest byte
			* address within each pixel.  The X component is ignored when compressing
			* and undefined when decompressing.
			*/
			TJPF_RGBX,
			/**
			* BGRX pixel format.  The red, green, and blue components in the image are
			* stored in 4-byte pixels in the order B, G, R from lowest to highest byte
			* address within each pixel.  The X component is ignored when compressing
			* and undefined when decompressing.
			*/
			TJPF_BGRX,
			/**
			* XBGR pixel format.  The red, green, and blue components in the image are
			* stored in 4-byte pixels in the order R, G, B from highest to lowest byte
			* address within each pixel.  The X component is ignored when compressing
			* and undefined when decompressing.
			*/
			TJPF_XBGR,
			/**
			* XRGB pixel format.  The red, green, and blue components in the image are
			* stored in 4-byte pixels in the order B, G, R from highest to lowest byte
			* address within each pixel.  The X component is ignored when compressing
			* and undefined when decompressing.
			*/
			TJPF_XRGB,
			/**
			* Grayscale pixel format.  Each 1-byte pixel represents a luminance
			* (brightness) level from 0 to 255.
			*/
			TJPF_GRAY,
			/**
			* RGBA pixel format.  This is the same as @ref TJPF_RGBX, except that when
			* decompressing, the X component is guaranteed to be 0xFF, which can be
			* interpreted as an opaque alpha channel.
			*/
			TJPF_RGBA,
			/**
			* BGRA pixel format.  This is the same as @ref TJPF_BGRX, except that when
			* decompressing, the X component is guaranteed to be 0xFF, which can be
			* interpreted as an opaque alpha channel.
			*/
			TJPF_BGRA,
			/**
			* ABGR pixel format.  This is the same as @ref TJPF_XBGR, except that when
			* decompressing, the X component is guaranteed to be 0xFF, which can be
			* interpreted as an opaque alpha channel.
			*/
			TJPF_ABGR,
			/**
			* ARGB pixel format.  This is the same as @ref TJPF_XRGB, except that when
			* decompressing, the X component is guaranteed to be 0xFF, which can be
			* interpreted as an opaque alpha channel.
			*/
			TJPF_ARGB,
			/**
			* CMYK pixel format.  Unlike RGB, which is an additive color model used
			* primarily for display, CMYK (Cyan/Magenta/Yellow/Key) is a subtractive
			* color model used primarily for printing.  In the CMYK color model, the
			* value of each color component typically corresponds to an amount of cyan,
			* magenta, yellow, or black ink that is applied to a white background.  In
			* order to convert between CMYK and RGB, it is necessary to use a color
			* management system (CMS.)  A CMS will attempt to map colors within the
			* printer's gamut to perceptually similar colors in the display's gamut and
			* vice versa, but the mapping is typically not 1:1 or reversible, nor can it
			* be defined with a simple formula.  Thus, such a conversion is out of scope
			* for a codec library.  However, the TurboJPEG API allows for compressing
			* CMYK pixels into a YCCK JPEG image (see #TJCS_YCCK) and decompressing YCCK
			* JPEG images into CMYK pixels.
			*/
			TJPF_CMYK
		};
	}
}
