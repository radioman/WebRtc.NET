/*
 *  Copyright (c) 2018 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef MODULES_RTP_RTCP_SOURCE_RTP_VIDEO_HEADER_H_
#define MODULES_RTP_RTCP_SOURCE_RTP_VIDEO_HEADER_H_

#include "absl/types/variant.h"
#include "api/video/video_content_type.h"
#include "api/video/video_rotation.h"
#include "api/video/video_timing.h"
#include "common_types.h"  // NOLINT(build/include)
#include "modules/video_coding/codecs/h264/include/h264_globals.h"
#include "modules/video_coding/codecs/vp8/include/vp8_globals.h"
#include "modules/video_coding/codecs/vp9/include/vp9_globals.h"

namespace webrtc {
using RTPVideoTypeHeader =
    absl::variant<RTPVideoHeaderVP8, RTPVideoHeaderVP9, RTPVideoHeaderH264>;

struct RTPVideoHeader {
  RTPVideoHeader();
  RTPVideoHeader(const RTPVideoHeader& other);

  // TODO(philipel): Remove when downstream projects have been updated.
  RTPVideoHeaderVP8& vp8() {
    if (!absl::holds_alternative<RTPVideoHeaderVP8>(video_type_header))
      video_type_header.emplace<RTPVideoHeaderVP8>();

    return absl::get<RTPVideoHeaderVP8>(video_type_header);
  }
  // TODO(philipel): Remove when downstream projects have been updated.
  const RTPVideoHeaderVP8& vp8() const {
    if (!absl::holds_alternative<RTPVideoHeaderVP8>(video_type_header))
      video_type_header.emplace<RTPVideoHeaderVP8>();

    return absl::get<RTPVideoHeaderVP8>(video_type_header);
  }
  // TODO(philipel): Remove when downstream projects have been updated.
  RTPVideoHeaderVP9& vp9() {
    if (!absl::holds_alternative<RTPVideoHeaderVP9>(video_type_header))
      video_type_header.emplace<RTPVideoHeaderVP9>();

    return absl::get<RTPVideoHeaderVP9>(video_type_header);
  }
  // TODO(philipel): Remove when downstream projects have been updated.
  const RTPVideoHeaderVP9& vp9() const {
    if (!absl::holds_alternative<RTPVideoHeaderVP9>(video_type_header))
      video_type_header.emplace<RTPVideoHeaderVP9>();

    return absl::get<RTPVideoHeaderVP9>(video_type_header);
  }
  // TODO(philipel): Remove when downstream projects have been updated.
  RTPVideoHeaderH264& h264() {
    if (!absl::holds_alternative<RTPVideoHeaderH264>(video_type_header))
      video_type_header.emplace<RTPVideoHeaderH264>();

    return absl::get<RTPVideoHeaderH264>(video_type_header);
  }
  // TODO(philipel): Remove when downstream projects have been updated.
  const RTPVideoHeaderH264& h264() const {
    if (!absl::holds_alternative<RTPVideoHeaderH264>(video_type_header))
      video_type_header.emplace<RTPVideoHeaderH264>();

    return absl::get<RTPVideoHeaderH264>(video_type_header);
  }

  uint16_t width;
  uint16_t height;
  VideoRotation rotation;
  PlayoutDelay playout_delay;
  VideoContentType content_type;
  VideoSendTiming video_timing;
  bool is_first_packet_in_frame;
  uint8_t simulcastIdx;
  VideoCodecType codec;
  // TODO(philipel): remove mutable when downstream projects have been updated.
  mutable RTPVideoTypeHeader video_type_header;
};

}  // namespace webrtc

#endif  // MODULES_RTP_RTCP_SOURCE_RTP_VIDEO_HEADER_H_
