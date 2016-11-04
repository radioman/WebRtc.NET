
#pragma once

#ifndef WEBRTC_NET_INTERNALS_H_
#define WEBRTC_NET_INTERNALS_H_

//#include <stdint.h>

namespace Internal
{
	void Encode(unsigned char * data, unsigned int size, int part_idx, bool keyFrame);
}

namespace Native
{
	extern bool CFG_quality_scaler_enabled_;

	void InitializeSSL();
	void CleanupSSL();

	void FilterInitMemoryManager();
	void FilterCloseMemoryManager();

	void FilterStart(unsigned char * gray, int w, int h);
	void FilterComplete();

	void FilterStartBg(unsigned char * gray, int w, int h);

	void FilterSubtractBg();

	void FilterInvertGray();
	void FilterEqualizeTRC(float fract, int factor);
	void FilterUnsharpMaskingGray(int usm_halfwidth, float usm_fract);
	void FilterContrastNorm();
	void FilterContrastTRC(float factor);
	void FilterBlockconvGray(int wc, int hc);
	void FilterSobelEdgeFilter();
	void FilterBilateralGray();

	void FilterOtsuAdaptiveThreshold(bool mark);
	void FilterSauvolaBinarize(bool mark);
}

#endif // WEBRTC_NET_INTERNALS_H_


