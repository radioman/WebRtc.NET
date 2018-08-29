/*
 *  Copyright (c) 2018 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef API_UNITS_DATA_RATE_H_
#define API_UNITS_DATA_RATE_H_
#include <stdint.h>
#include <cmath>
#include <limits>
#include <string>

#include "rtc_base/checks.h"
#include "rtc_base/numerics/safe_conversions.h"

#include "api/units/data_size.h"
#include "api/units/time_delta.h"

namespace webrtc {
namespace data_rate_impl {
constexpr int64_t kPlusInfinityVal = std::numeric_limits<int64_t>::max();

inline int64_t Microbits(const DataSize& size) {
  constexpr int64_t kMaxBeforeConversion =
      std::numeric_limits<int64_t>::max() / 8000000;
  RTC_DCHECK_LE(size.bytes(), kMaxBeforeConversion)
      << "size is too large to be expressed in microbytes";
  return size.bytes() * 8000000;
}
}  // namespace data_rate_impl

// DataRate is a class that represents a given data rate. This can be used to
// represent bandwidth, encoding bitrate, etc. The internal storage is bits per
// second (bps).
class DataRate {
 public:
  DataRate() = delete;
  static DataRate Zero() { return DataRate(0); }
  static DataRate Infinity() {
    return DataRate(data_rate_impl::kPlusInfinityVal);
  }

  template <
      typename T,
      typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
  static DataRate bps(T bits_per_second) {
    RTC_DCHECK_GE(bits_per_second, 0);
    RTC_DCHECK_LT(bits_per_second, data_rate_impl::kPlusInfinityVal);
    return DataRate(rtc::dchecked_cast<int64_t>(bits_per_second));
  }
  template <
      typename T,
      typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
  static DataRate kbps(T kilobits_per_sec) {
    RTC_DCHECK_GE(kilobits_per_sec, 0);
    RTC_DCHECK_LT(kilobits_per_sec, data_rate_impl::kPlusInfinityVal / 1000);
    return DataRate::bps(rtc::dchecked_cast<int64_t>(kilobits_per_sec) * 1000);
  }

  template <typename T,
            typename std::enable_if<std::is_floating_point<T>::value>::type* =
                nullptr>
  static DataRate bps(T bits_per_second) {
    if (bits_per_second == std::numeric_limits<T>::infinity()) {
      return Infinity();
    } else {
      RTC_DCHECK(!std::isnan(bits_per_second));
      RTC_DCHECK_GE(bits_per_second, 0);
      RTC_DCHECK_LT(bits_per_second, data_rate_impl::kPlusInfinityVal);
      return DataRate(rtc::dchecked_cast<int64_t>(bits_per_second));
    }
  }
  template <typename T,
            typename std::enable_if<std::is_floating_point<T>::value>::type* =
                nullptr>
  static DataRate kbps(T kilobits_per_sec) {
    return DataRate::bps(kilobits_per_sec * 1e3);
  }

  template <typename T = int64_t>
  typename std::enable_if<std::is_integral<T>::value, T>::type bps() const {
    RTC_DCHECK(IsFinite());
    return rtc::dchecked_cast<T>(bits_per_sec_);
  }
  template <typename T = int64_t>
  typename std::enable_if<std::is_integral<T>::value, T>::type kbps() const {
    return rtc::dchecked_cast<T>((bps() + 500) / 1000);
  }

  template <typename T>
  typename std::enable_if<std::is_floating_point<T>::value, T>::type bps()
      const {
    if (IsInfinite()) {
      return std::numeric_limits<T>::infinity();
    } else {
      return bits_per_sec_;
    }
  }
  template <typename T>
  typename std::enable_if<std::is_floating_point<T>::value, T>::type kbps()
      const {
    return bps<T>() * 1e-3;
  }

  bool IsZero() const { return bits_per_sec_ == 0; }
  bool IsInfinite() const {
    return bits_per_sec_ == data_rate_impl::kPlusInfinityVal;
  }
  bool IsFinite() const { return !IsInfinite(); }

  double operator/(const DataRate& other) const {
    return bps<double>() / other.bps<double>();
  }
  bool operator==(const DataRate& other) const {
    return bits_per_sec_ == other.bits_per_sec_;
  }
  bool operator!=(const DataRate& other) const {
    return bits_per_sec_ != other.bits_per_sec_;
  }
  bool operator<=(const DataRate& other) const {
    return bits_per_sec_ <= other.bits_per_sec_;
  }
  bool operator>=(const DataRate& other) const {
    return bits_per_sec_ >= other.bits_per_sec_;
  }
  bool operator>(const DataRate& other) const {
    return bits_per_sec_ > other.bits_per_sec_;
  }
  bool operator<(const DataRate& other) const {
    return bits_per_sec_ < other.bits_per_sec_;
  }

 private:
  // Bits per second used internally to simplify debugging by making the value
  // more recognizable.
  explicit DataRate(int64_t bits_per_second) : bits_per_sec_(bits_per_second) {}
  int64_t bits_per_sec_;
};

inline DataRate operator*(const DataRate& rate, const double& scalar) {
  return DataRate::bps(std::round(rate.bps() * scalar));
}
inline DataRate operator*(const double& scalar, const DataRate& rate) {
  return rate * scalar;
}
inline DataRate operator*(const DataRate& rate, const int64_t& scalar) {
  return DataRate::bps(rate.bps() * scalar);
}
inline DataRate operator*(const int64_t& scalar, const DataRate& rate) {
  return rate * scalar;
}
inline DataRate operator*(const DataRate& rate, const int32_t& scalar) {
  return DataRate::bps(rate.bps() * scalar);
}
inline DataRate operator*(const int32_t& scalar, const DataRate& rate) {
  return rate * scalar;
}

inline DataRate operator/(const DataSize& size, const TimeDelta& duration) {
  return DataRate::bps(data_rate_impl::Microbits(size) / duration.us());
}
inline TimeDelta operator/(const DataSize& size, const DataRate& rate) {
  return TimeDelta::us(data_rate_impl::Microbits(size) / rate.bps());
}
inline DataSize operator*(const DataRate& rate, const TimeDelta& duration) {
  int64_t microbits = rate.bps() * duration.us();
  return DataSize::bytes((microbits + 4000000) / 8000000);
}
inline DataSize operator*(const TimeDelta& duration, const DataRate& rate) {
  return rate * duration;
}

std::string ToString(const DataRate& value);

}  // namespace webrtc

#endif  // API_UNITS_DATA_RATE_H_
