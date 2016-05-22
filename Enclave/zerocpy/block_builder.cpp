// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// BlockBuilder generates blocks where keys are prefix-compressed:
//
// When we store a key, we drop the prefix shared with the previous
// string.  This helps reduce the space requirement significantly.
// Furthermore, once every K keys, we do not apply the prefix
// compression and store the entire key.  We call this a "restart
// point".  The tail end of the block stores the offsets of all of the
// restart points, and can be used to do a binary search when looking
// for a particular key.  Values are stored as-is (without compression)
// immediately following the corresponding key.
//
// An entry for a particular key-value pair has the form:
//     shared_bytes: varint32
//     unshared_bytes: varint32
//     value_length: varint32
//     key_delta: char[unshared_bytes]
//     value: char[value_length]
// shared_bytes == 0 for restart points.
//
// The trailer of the block has the form:
//     restarts: uint32[num_restarts]
//     num_restarts: uint32
// restarts[i] contains the offset within the block of the ith restart point.

#include "block_builder.h"
#include "Enclave_t.h"
#include "Enclave.h"
#include <algorithm>
#include <assert.h>
//#include "comparator.h"
#include "table_builder.h"
#include "coding.h"
#define CHAR_BUFFER 1

static uint32_t g_mem[30000000];
//static uint32_t g_mem[1000000];

BlockBuilder::BlockBuilder(const Options* options, int i, char* buf)
    : options_(options),
      restarts_(),
      counter_(0),
      block_type(i),
      b_SU(buf),
      buffer_pointer(0),
      finished_(false) {
  assert(options->block_restart_interval >= 1);
  restarts_.push_back(0);       // First restart point is at offset 0
  if (block_type==1) {
      restarts_index = g_mem;
   //  restarts_index = (uint32_t *)malloc(1000000*sizeof(uint32_t));
      restarts_pointer = 1;
      restarts_index[0] = 0;
  }
}

BlockBuilder::BlockBuilder(const Options* options)
    : options_(options),
      restarts_(),
      counter_(0),
      finished_(false) {
  assert(options->block_restart_interval >= 1);
  restarts_.push_back(0);       // First restart point is at offset 0
}
void BlockBuilder::Reset() {
//CHAR_BUFFER
  buffer_pointer = 0;
//  buffer_.clear();
  restarts_.clear();
  restarts_.push_back(0);       // First restart point is at offset 0
  if (block_type == 1) {
    restarts_index[0] = 0;
    restarts_pointer = 1;
  }
  counter_ = 0;
  finished_ = false;
  last_key_.clear();
}

size_t BlockBuilder::CurrentSizeEstimate() const {
  if (block_type==1)
  return (buffer_pointer +                        // Raw data buffer
          restarts_pointer * sizeof(uint32_t) +   // Restart array
          sizeof(uint32_t));                      // Restart array length
  return (buffer_pointer +                        // Raw data buffer
          restarts_.size() * sizeof(uint32_t) +   // Restart array
          sizeof(uint32_t));                      // Restart array length
//char buffer
// return (buffer_.size() +                        // Raw data buffer
 //         restarts_.size() * sizeof(uint32_t) +   // Restart array
  //        sizeof(uint32_t));                      // Restart array length
}

Slice BlockBuilder::Finish() {
  if (block_type == 1) {
 //   bar1("restarts_size=%u\n",restarts_pointer);
    for (size_t i = 0; i < restarts_pointer; i++) {
      PutFixed32_SU(b_SU+buffer_pointer,restarts_index[i]);
      buffer_pointer+=4;
    }

  }
  else{
    // Append restart array
    for (size_t i = 0; i < restarts_.size(); i++) {
      PutFixed32_SU(b_SU+buffer_pointer,restarts_[i]);
      buffer_pointer+=4;
      //char buffer
      //  PutFixed32(&buffer_, restarts_[i]);

    }
  }
#if CHAR_BUFFER
  if (block_type == 1) {
    PutFixed32_SU(b_SU+buffer_pointer, restarts_pointer);
    buffer_pointer+=4;
  } else {
    PutFixed32_SU(b_SU+buffer_pointer, restarts_.size());
    buffer_pointer+=4;
  }
#else
  PutFixed32(&buffer_, restarts_.size());
#endif
  finished_ = true;
#if CHAR_BUFFER
  return Slice(b_SU,buffer_pointer);
#else
  return Slice(buffer_);
#endif
}

void BlockBuilder::Add(const Slice& key, const Slice& value) {
  Slice last_key_piece(last_key_);
  assert(!finished_);
  assert(counter_ <= options_->block_restart_interval);
#if CHAR_BUFFER
  assert(buffer_pointer==0 // No values yet?
         || key.compare(last_key_piece) > 0);
#else
  assert(buffer_.empty() // No values yet?
         || key.compare(last_key_piece) > 0);
#endif
  size_t shared = 0;
  if (counter_ < options_->block_restart_interval) {
    // See how much sharing to do with previous string
    const size_t min_length = std::min(last_key_piece.size(), key.size());
    while ((shared < min_length) && (last_key_piece[shared] == key[shared])) {
      shared++;
    }
  } else {
    // Restart compression
#if CHAR_BUFFER
    if (block_type == 1) {
      restarts_index[restarts_pointer] = buffer_pointer;
      restarts_pointer++;
    }
    else {
      restarts_.push_back(buffer_pointer);
    }
#else
     restarts_.push_back(buffer_.size());
#endif
    counter_ = 0;
  }
  const size_t non_shared = key.size() - shared;
  // Add "<shared><non_shared><value_size>" to buffer_
#if CHAR_BUFFER
  buffer_pointer+=PutVarint32_SU(b_SU+buffer_pointer, shared);
  buffer_pointer+=PutVarint32_SU(b_SU+buffer_pointer, non_shared);
  buffer_pointer+=PutVarint32_SU(b_SU+buffer_pointer, value.size());
  memcpy(b_SU+buffer_pointer,key.data()+shared,non_shared);
  buffer_pointer+=non_shared;
  memcpy(b_SU+buffer_pointer,value.data(),value.size());
  buffer_pointer+=value.size();
#else 
  PutVarint32(&buffer_, shared);
  PutVarint32(&buffer_, non_shared);
  PutVarint32(&buffer_, value.size());

  // Add string delta to buffer_ followed by value
  buffer_.append(key.data() + shared, non_shared);
  buffer_.append(value.data(), value.size());
#endif

  // Update state
  last_key_.resize(shared);
  last_key_.append(key.data() + shared, non_shared);
  assert(Slice(last_key_) == key);
  counter_++;
}

void BlockBuilder::setType(int i) {block_type=i;}
int BlockBuilder::getType() {return block_type;}

