
#include <memory>

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

	void FilterStart(unsigned char * gray, int w, int h)
	{
		if (px == nullptr)
		{
			px = pixCreateHeader(w, h, 8);
		}
		px->data = (l_uint32*)gray;

		pixEndianByteSwap(px);
	}

	void FilterComplete()
	{
		pixEndianByteSwap(px);
	}

	void FilterStartBg(unsigned char * gray, int w, int h)
	{
		if (pxBg == nullptr)
		{
			pxBg = pixCreateHeader(w, h, 8);
		}
		pxBg->data = (l_uint32*)gray;

		pixEndianByteSwap(pxBg);
	}

	void FilterSubtractBg()
	{
		pixSubtractGray(px, px, pxBg);
	}

	void FilterInvertGray()
	{
		pixInvert(px, px);
	}

	void FilterEqualizeTRC(float fract, int factor)
	{
		//*\param[in]    fract fraction of equalization movement of pixel values
		//	* \param[in]    factor subsampling factor; integer >= 1

		//*(3) If fract == 0.0, no equalization is performed; return a copy
		//	*          unless in - place, in which case this is a no - op.
		//	*          If fract == 1.0, equalization is complete.
		pixEqualizeTRC(px, px, fract, factor);
	}

	void FilterUnsharpMaskingGray(int usm_halfwidth, float usm_fract)
	{
		//*\param[in]    halfwidth  "half-width" of smoothing filter
		//	* \param[in]    fract  fraction of edge added back into image

		//*(1) We use symmetric smoothing filters of odd dimension,
		//	*typically use sizes of 3, 5, 7, etc.The %halfwidth parameter
		//	*          for these is(size - 1) / 2; i.e., 1, 2, 3, etc.
		//	*      (2) The fract parameter is typically taken in the range :
		//*0.2 \< fract \< 0.7
		PIX * tmp = pixUnsharpMaskingGray(px, usm_halfwidth, usm_fract);

		memcpy(px->data, tmp->data, px->w * px->h);
		pixDestroy(&tmp);
	}

	void FilterContrastNorm()
	{
		//*\param[in]    sx, sy tile dimensions
		//	* \param[in]    mindiff minimum difference to accept as valid
		//	* \param[in]    smoothx, smoothy half - width of convolution kernel applied to
		//	*                                min and max arrays : use 0 for no smoothing
		pixContrastNorm(px, px, 100, 100, 55, 1, 1);
	}

	void FilterContrastTRC(float factor)
	{
		//(4) The useful range for the contrast factor is scaled to
		//	*          be in(0.0 to 1.0), but larger values can also be used.
		pixContrastTRC(px, px, factor);
	}

	void FilterBlockconvGray(int wc, int hc)
	{
		// * \param[in]    wc, hc   half width/height of convolution kernel
		//*(4) Require that w \ >= 2 * wc + 1 and h \ >= 2 * hc + 1,
		//	*where(w, h) are the dimensions of pixs.

		PIX * tmp = pixBlockconvGray(px, NULL, wc, hc);
		memcpy(px->data, tmp->data, px->w * px->h);
		pixDestroy(&tmp);
	}

	void FilterSobelEdgeFilter()
	{
		PIX * tmp = pixSobelEdgeFilter(px, L_ALL_EDGES);
		memcpy(px->data, tmp->data, px->w * px->h);

		pixDestroy(&tmp);
	}

	void FilterBilateralGray()
	{
		//spatial_stdev(of gaussian kernel; in pixels, > 0.5)
		//	range_stdev(of gaussian range kernel; > 5.0; typ. 50.0)
		//	ncomps(number of intermediate sums J(k, x); in[4 ... 30])
		//	reduction(1, 2 or 4)

		PIX * tmp = pixBilateralGray(px, 4, 45, 30, 4);

		memcpy(px->data, tmp->data, px->w * px->h);

		pixDestroy(&tmp);
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

	void FilterOtsuAdaptiveThreshold(bool mark)
	{
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
			FilterMarkConnComp(bin);
		}

		pixConvert1To8(px, bin, 255, 0);
		pixDestroy(&bin);
	}

	void FilterSauvolaBinarize(bool mark)
	{
		//*\param[in]    whsize window half - width for measuring local statistics
		//	* \param[in]    factor factor for reducing threshold due to variance; >= 0
		//	* \param[in]    nx, ny subdivision into tiles; >= 1

		PIX * bin;
		pixSauvolaBinarizeTiled(px, 8, 0.34, 1, 1, NULL, &bin);

		if (mark)
		{
			FilterMarkConnComp(bin);
		}

		pixConvert1To8(px, bin, 255, 0);
		pixDestroy(&bin);
	}
}