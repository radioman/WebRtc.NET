// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_FUCHSIA_SCOPED_ZX_HANDLE_H_
#define BASE_FUCHSIA_SCOPED_ZX_HANDLE_H_

#include <lib/zx/handle.h>

namespace base {

// TODO(852541): Temporary shim to implement the old ScopedGeneric based
// container as a native zx::handle. Remove this once all callers have been
// migrated to use the libzx containers.
using ScopedZxHandle = zx::handle;

}  // namespace base

#endif  // BASE_FUCHSIA_SCOPED_ZX_HANDLE_H_
