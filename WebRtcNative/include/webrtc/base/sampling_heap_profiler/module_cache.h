// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_SAMPLING_HEAP_PROFILER_MODULE_CACHE_H_
#define BASE_SAMPLING_HEAP_PROFILER_MODULE_CACHE_H_

#include <map>

#include "base/profiler/stack_sampling_profiler.h"

namespace base {

class BASE_EXPORT ModuleCache {
 public:
  using Module = StackSamplingProfiler::InternalModule;

  ModuleCache();
  ~ModuleCache();

  const Module& GetModuleForAddress(uintptr_t address);

 private:
  std::map<uintptr_t, Module> modules_cache_map_;
};

}  // namespace base

#endif  // BASE_SAMPLING_HEAP_PROFILER_MODULE_CACHE_H_
