// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MEMORY_SHARED_MEMORY_MAPPING_H_
#define BASE_MEMORY_SHARED_MEMORY_MAPPING_H_

#include <cstddef>

#include "base/macros.h"
#include "base/unguessable_token.h"

namespace base {

namespace subtle {
class PlatformSharedMemoryRegion;
}  // namespace subtle

// Base class for scoped handles to a shared memory mapping created from a
// shared memory region. Created shared memory mappings remain valid even if the
// creator region is transferred or destroyed.
//
// Each mapping has an UnguessableToken that identifies the shared memory region
// it was created from. This is used for memory metrics, to avoid overcounting
// shared memory.
class BASE_EXPORT SharedMemoryMapping {
 public:
  // Default constructor initializes an invalid instance.
  SharedMemoryMapping();

  // Move operations are allowed.
  SharedMemoryMapping(SharedMemoryMapping&& mapping);
  SharedMemoryMapping& operator=(SharedMemoryMapping&& mapping);

  // Unmaps the region if the mapping is valid.
  virtual ~SharedMemoryMapping();

  // Returns true iff the mapping is valid. False means there is no
  // corresponding area of memory.
  bool IsValid() const { return memory_ != nullptr; }

  // Returns the logical size of the mapping in bytes. This is precisely the
  // size requested by whoever created the mapping, and it is always less than
  // or equal to |mapped_size()|. This is undefined for invalid instances.
  size_t size() const {
    DCHECK(IsValid());
    return size_;
  }

  // Returns the actual size of the mapping in bytes. This is always at least
  // as large as |size()| but may be larger due to platform mapping alignment
  // constraints. This is undefined for invalid instances.
  size_t mapped_size() const {
    DCHECK(IsValid());
    return mapped_size_;
  }

  // Returns 128-bit GUID of the region this mapping belongs to.
  const UnguessableToken& guid() const {
    DCHECK(IsValid());
    return guid_;
  }

 protected:
  SharedMemoryMapping(void* address,
                      size_t size,
                      size_t mapped_size,
                      const UnguessableToken& guid);
  void* raw_memory_ptr() const { return memory_; }

 private:
  friend class SharedMemoryTracker;

  void Unmap();

  void* memory_ = nullptr;
  size_t size_ = 0;
  size_t mapped_size_ = 0;
  UnguessableToken guid_;

  DISALLOW_COPY_AND_ASSIGN(SharedMemoryMapping);
};

// Class modeling a read-only mapping of a shared memory region into the
// current process' address space. This is created by ReadOnlySharedMemoryRegion
// instances.
class BASE_EXPORT ReadOnlySharedMemoryMapping : public SharedMemoryMapping {
 public:
  // Default constructor initializes an invalid instance.
  ReadOnlySharedMemoryMapping();

  // Move operations are allowed.
  ReadOnlySharedMemoryMapping(ReadOnlySharedMemoryMapping&&);
  ReadOnlySharedMemoryMapping& operator=(ReadOnlySharedMemoryMapping&&);

  // Returns the base address of the mapping. This is read-only memory. This is
  // page-aligned. This is nullptr for invalid instances.
  const void* memory() const { return raw_memory_ptr(); }

 private:
  friend class ReadOnlySharedMemoryRegion;
  ReadOnlySharedMemoryMapping(void* address,
                              size_t size,
                              size_t mapped_size,
                              const UnguessableToken& guid);

  DISALLOW_COPY_AND_ASSIGN(ReadOnlySharedMemoryMapping);
};

// Class modeling a writable mapping of a shared memory region into the
// current process' address space. This is created by *SharedMemoryRegion
// instances.
class BASE_EXPORT WritableSharedMemoryMapping : public SharedMemoryMapping {
 public:
  // Default constructor initializes an invalid instance.
  WritableSharedMemoryMapping();

  // Move operations are allowed.
  WritableSharedMemoryMapping(WritableSharedMemoryMapping&&);
  WritableSharedMemoryMapping& operator=(WritableSharedMemoryMapping&&);

  // Returns the base address of the mapping. This is writable memory. This is
  // page-aligned. This is nullptr for invalid instances.
  void* memory() const { return raw_memory_ptr(); }

 private:
  friend WritableSharedMemoryMapping MapAtForTesting(
      subtle::PlatformSharedMemoryRegion* region,
      off_t offset,
      size_t size);
  friend class ReadOnlySharedMemoryRegion;
  friend class WritableSharedMemoryRegion;
  friend class UnsafeSharedMemoryRegion;
  WritableSharedMemoryMapping(void* address,
                              size_t size,
                              size_t mapped_size,
                              const UnguessableToken& guid);

  DISALLOW_COPY_AND_ASSIGN(WritableSharedMemoryMapping);
};

}  // namespace base

#endif  // BASE_MEMORY_SHARED_MEMORY_MAPPING_H_
