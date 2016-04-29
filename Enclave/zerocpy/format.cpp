// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include "format.h"
#include "Enclave_t.h"
#include "Enclave.h"
//#include "port.h"
#include "block.h"
#include "coding.h"
//#include "crc32c.h"
//#include "file_system.h"
#include "options.h"
#include <stdio.h>
struct g_mem {
  char index_mem[10][100000000];
  char mem[10][10000000];
  uint64_t  file_size[2];
};

void BlockHandle::EncodeTo(std::string* dst) const {
  // Sanity check that all fields have been set
  assert(offset_ != ~static_cast<uint64_t>(0));
  assert(size_ != ~static_cast<uint64_t>(0));
  PutVarint64(dst, offset_);
  PutVarint64(dst, size_);
}

int BlockHandle::DecodeFrom(Slice* input) {
  if (GetVarint64(input, &offset_) &&
      GetVarint64(input, &size_)) {
    return 0;
  } else {
    return -1;
  }
}

void Footer::EncodeTo(std::string* dst) const {
#ifndef NDEBUG
  const size_t original_size = dst->size();
#endif
  metaindex_handle_.EncodeTo(dst);
  index_handle_.EncodeTo(dst);
  dst->resize(2 * BlockHandle::kMaxEncodedLength);  // Padding
  PutFixed32(dst, static_cast<uint32_t>(kTableMagicNumber & 0xffffffffu));
  PutFixed32(dst, static_cast<uint32_t>(kTableMagicNumber >> 32));
  assert(dst->size() == original_size + kEncodedLength);
}

int Footer::DecodeFrom(Slice* input) {
  const char* magic_ptr = input->data() + kEncodedLength - 8;
  const uint32_t magic_lo = DecodeFixed32(magic_ptr);
  const uint32_t magic_hi = DecodeFixed32(magic_ptr + 4);
  const uint64_t magic = ((static_cast<uint64_t>(magic_hi) << 32) |
      (static_cast<uint64_t>(magic_lo)));
  if (magic != kTableMagicNumber) {
    return -1;
  }

  int result = metaindex_handle_.DecodeFrom(input);
  if (result==0) {
    result = index_handle_.DecodeFrom(input);
  }
  if (result==0) {
    // We skip over any leftover data (just padding for now) in "input"
    const char* end = magic_ptr + 8;
    *input = Slice(end, input->data() + input->size() - end);
  }
  return result;
}
int ReadBlockSU(int fileIdx,
    const BlockHandle& handle,
    BlockContents* result,
    bool isIndex, long private_data) {
  result->data = Slice();
  result->cachable = false;
  result->heap_allocated = false;

  // Read the block contents as well as the type/crc footer.
  // See table_builder.cc for the code that built this structure.
  size_t n = static_cast<size_t>(handle.size());
  int res = 0;
  char* mem;
  int length;
  if (isIndex) { 
      mem = ((struct g_mem*)private_data)->index_mem[fileIdx];
  } else {
      mem = ((struct g_mem*)private_data)->mem[fileIdx];
  }
  ocall_read_nospace(&res, handle.offset(), n + kBlockTrailerSize, &length, fileIdx, isIndex);
  Slice contents = Slice(mem,length);
  if (res!=0) {
    // delete[] buf;
    return res;
  }
  if (contents.size() != n + kBlockTrailerSize) {
    // delete[] buf;
    return -1;
  }

  // Check the crc of the type and the block contents
  const char* data = contents.data();    // Pointer to where Read put the data

  switch (data[n]) {
    case kNoCompression:
      if (data != mem) {
        // File implementation gave us pointer to some other data.
        // Use it directly under the assumption that it will be live
        // while the file is open.
        // delete[] buf;
        result->data = Slice(data, n);
        result->heap_allocated = false;
        result->cachable = false;  // Do not double-cache
      } else {
        result->data = Slice(mem, n);
        result->heap_allocated = false;
        result->cachable = false;
      }

      // Ok
      break;
      /*
         case kSnappyCompression: {
         size_t ulength = 0;
         if (!port::Snappy_GetUncompressedLength(data, n, &ulength)) {
         delete[] buf;
         return -1;
         }
         char* ubuf = new char[ulength];
         if (!port::Snappy_Uncompress(data, n, ubuf)) {
         delete[] buf;
         delete[] ubuf;
         return -1;
         }
         delete[] buf;
         result->data = Slice(ubuf, ulength);
         result->heap_allocated = true;
         result->cachable = true;
         break;
         }
       */
    default:
      // delete[] buf;
      return -1;
  }

  return 0;
}

#if 0
int ReadBlock(RandomAccessFile* file,
    int fileIdx,
    const BlockHandle& handle,
    BlockContents* result,
    bool isIndex) {
  result->data = Slice();
  result->cachable = false;
  result->heap_allocated = false;

  // Read the block contents as well as the type/crc footer.
  // See table_builder.cc for the code that built this structure.
  size_t n = static_cast<size_t>(handle.size());
  char* buf = new char[n + kBlockTrailerSize];
  Slice contents;
  int res = 0;
  res  = file->Read(handle.offset(), n + kBlockTrailerSize, &contents,buf);
  if (res!=0) {
    // delete[] buf;
    return res;
  }
  if (contents.size() != n + kBlockTrailerSize) {
    // delete[] buf;
    return -1;
  }

  // Check the crc of the type and the block contents
  const char* data = contents.data();    // Pointer to where Read put the data

  switch (data[n]) {
    case kNoCompression:
      if (data != buf) {
        // File implementation gave us pointer to some other data.
        // Use it directly under the assumption that it will be live
        // while the file is open.
        // delete[] buf;
        result->data = Slice(data, n);
        result->heap_allocated = false;
        result->cachable = false;  // Do not double-cache
      } else {
        result->data = Slice(buf, n);
        result->heap_allocated = true;
        result->cachable = false;
      }

      // Ok
      break;
      /*
         case kSnappyCompression: {
         size_t ulength = 0;
         if (!port::Snappy_GetUncompressedLength(data, n, &ulength)) {
         delete[] buf;
         return -1;
         }
         char* ubuf = new char[ulength];
         if (!port::Snappy_Uncompress(data, n, ubuf)) {
         delete[] buf;
         delete[] ubuf;
         return -1;
         }
         delete[] buf;
         result->data = Slice(ubuf, ulength);
         result->heap_allocated = true;
         result->cachable = true;
         break;
         }
       */
    default:
      // delete[] buf;
      return -1;
  }

  return 0;
}
#endif
