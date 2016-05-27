
#ifndef WEBRTC_NET_INTERNALS_H_
#define WEBRTC_NET_INTERNALS_H_
#pragma once

#include <stdint.h>

extern bool CFG_quality_scaler_enabled_;

void _InitializeSSL();
void _CleanupSSL();

//void _EncodeInternal(unsigned char * data, unsigned int size);

#endif // WEBRTC_NET_INTERNALS_H_


