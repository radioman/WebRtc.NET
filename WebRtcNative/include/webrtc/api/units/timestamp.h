/*
 *  Copyright (c) 2018 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef API_UNITS_TIMESTAMP_H_
#define API_UNITS_TIMESTAMP_H_

#include <stdint.h>
#include <limits>
#include <string>

#include "api/units/time_delta.h"
#include "rtc_base/checks.h"
#include "rtc_base/numerics/safe_conversions.h"

namespace webrtc {
namespace timestamp_impl {
constexpr int64_t kPlusInfinityVal = std::numeric_limits<int64_t>::max();
constexpr int64_t kMinusInfinityVal = std::numeric_limits<int64_t>::min();
}  // namespace timestamp_impl

// Timestamp represents the time that has passed since some unspecified epoch.
// The epoch is assumed to be before any represented timestamps, this means that
// negative values are not valid. The most notable feature is that the
// difference of two Timestamps results in a TimeDelta.
class Timestamp {
 public:
  Timestamp() = delete;
  static Timestamp Infinity() {
    return Timestamp(timestamp_impl::kPlusInfinityVal);
  }

  template <
      typename T,
      typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
  static Timestamp seconds(T seconds) {
    RTC_DCHECK_GE(seconds, 0);
    RTC_DCHECK_LT(seconds, timestamp_impl::kPlusInfinityVal / 1000000);
    return Timestamp(rtc::dchecked_cast<int64_t>(seconds) * 1000000);
  }

  template <
      typename T,
      typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
  static Timestamp ms(T milliseconds) {
    RTC_DCHECK_GE(milliseconds, 0);
    RTC_DCHECK_LT(milliseconds, timestamp_impl::kPlusInfinityVal / 1000);
    return Timestamp(rtc::dchecked_cast<int64_t>(milliseconds) * 1000);
  }

  template <
      typename T,
      typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
  static Timestamp us(T microseconds) {
    RTC_DCHECK_GE(microseconds, 0);
    RTC_DCHECK_LT(microseconds, timestamp_impl::kPlusInfinityVal);
    return Timestamp(rtc::dchecked_cast<int64_t>(microseconds));
  }

  template <typename T,
            typename std::enable_if<std::is_floating_point<T>::value>::type* =
                nullptr>
  static Timestamp seconds(T seconds) {
    return Timestamp::us(seconds * 1e6);
  }

  template <typename T,
            typename std::enable_if<std::is_floating_point<T>::value>::type* =
                nullptr>
  static Timestamp ms(T milliseconds) {
    return Timestamp::us(milliseconds * 1e3);
  }
  template <typename T,
            typename std::enable_if<std::is_floating_point<T>::value>::type* =
                nullptr>
  static Timestamp us(T microseconds) {
    if (microseconds == std::numeric_limits<double>::infinity()) {
      return Infinity();
    } else {
      RTC_DCHECK(!std::isnan(microseconds));
      RTC_DCHECK_GE(microseconds, 0);
      RTC_DCHECK_LT(microseconds, timestamp_impl::kPlusInfinityVal);
      return Timestamp(rtc::dchecked_cast<int64_t>(microseconds));
    }
  }

  template <typename T = int64_t>
  typename std::enable_if<std::is_integral<T>::value, T>::type seconds() const {
    return rtc::dchecked_cast<T>((us() + 500000) / 1000000);
  }
  template <typename T = int64_t>
  typename std::enable_if<std::is_integral<T>::value, T>::type ms() const {
    return rtc::dchecked_cast<T>((us() + 500) / 1000);
  }
  template <typename T = int64_t>
  typename std::enable_if<std::is_integral<T>::value, T>::type us() const {
    RTC_DCHECK(IsFinite());
    return rtc::dchecked_cast<T>(microseconds_);
  }

  template <typename T>
  typename std::enable_if<std::is_floating_point<T>::value, T>::type seconds()
      const {
    return us<T>() * 1e-6;
  }
  template <typename T>
  typename std::enable_if<std::is_floating_point<T>::value, T>::type ms()
      const {
    return us<T>() * 1e-3;
  }
  template <typename T>
  typename std::enable_if<std::is_floating_point<T>::value, T>::type us()
      const {
    if (IsInfinite()) {
      return std::numeric_limits<T>::infinity();
    } else {
      return microseconds_;
    }
  }

  bool IsInfinite() const {
    return microseconds_ == timestamp_impl::kPlusInfinityVal;
  }
  bool IsFinite() const { return !IsInfinite(); }
  TimeDelta operator-(const Timestamp& other) const {
    return TimeDelta::us(us() - other.us());
  }
  Timestamp operator-(const TimeDelta& delta) const {
    return Timestamp::us(us() - delta.us());
  }
  Timestamp operator+(const TimeDelta& delta) const {
    return Timestamp::us(us() + delta.us());
  }
  Timestamp& operator-=(const TimeDelta& other) {
    microseconds_ -= other.us();
    return *this;
  }
  Timestamp& operator+=(const TimeDelta& other) {
    microseconds_ += other.us();
    return *this;
  }
  bool operator==(const Timestamp& other) const {
    return microseconds_ == other.microseconds_;
  }
  bool operator!=(const Timestamp& other) const {
    return microseconds_ != other.microseconds_;
  }
  bool operator<=(const Timestamp& other) const {
    return microseconds_ <= other.microseconds_;
  }
  bool operator>=(const Timestamp& other) const {
    return microseconds_ >= other.microseconds_;
  }
  bool operator>(const Timestamp& other) const {
    return microseconds_ > other.microseconds_;
  }
  bool operator<(const Timestamp& other) const {
    return microseconds_ < other.microseconds_;
  }

 private:
  explicit Timestamp(int64_t us) : microseconds_(us) {}
  int64_t microseconds_;
};

std::string ToString(const Timestamp& value);

}  // namespace webrtc

#endif  // API_UNITS_TIMESTAMP_H_
