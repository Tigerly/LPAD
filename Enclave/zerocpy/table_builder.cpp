// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include "table_builder.h"
#include "Enclave_t.h"
#include "Enclave.h"
#include <assert.h>
#include "options.h"
#include "block_builder.h"
#include "format.h"
#include "coding.h"
#include "crc32c.h"

struct out_mem {
  char data_block[1000000];
  char index_block[1000000000];
  char meta_block[1000];
};


void FindShortestSeparator(
    std::string* start,
    const Slice& limit)  {
  // Find length of common prefix
  size_t min_length = std::min(start->size(), limit.size());
  size_t diff_index = 0;
  while ((diff_index < min_length) &&
      ((*start)[diff_index] == limit[diff_index])) {
    diff_index++;
  }

  if (diff_index >= min_length) {
    // Do not shorten if one string is a prefix of the other
  } else {
    uint8_t diff_byte = static_cast<uint8_t>((*start)[diff_index]);
    if (diff_byte < static_cast<uint8_t>(0xff) &&
        diff_byte + 1 < static_cast<uint8_t>(limit[diff_index])) {
      (*start)[diff_index]++;
      start->resize(diff_index + 1);
    }
  }
}

void FindShortSuccessor(std::string* key) {
  // Find first character that can be incremented
  size_t n = key->size();
  for (size_t i = 0; i < n; i++) {
    const uint8_t byte = (*key)[i];
    if (byte != static_cast<uint8_t>(0xff)) {
      (*key)[i] = byte + 1;
      key->resize(i+1);
      return;
    }
  }
  // *key is a run of 0xffs.  Leave it alone.
}

struct TableBuilder::Rep {
  Options options;
  Options index_block_options;
  WritableFile* file;
  uint64_t offset;
  int status;
  BlockBuilder data_block;
  BlockBuilder index_block;
  long private_data;
  std::string last_key;
  int64_t num_entries;
  bool closed;          // Either Finish() or Abandon() has been called.
  //  FilterBlockBuilder* filter_block;

  // We do not emit the index entry for a block until we have seen the
  // first key for the next data block.  This allows us to use shorter
  // keys in the index block.  For example, consider a block boundary
  // between the keys "the quick brown fox" and "the who".  We can use
  // "the r" as the key for the index block entry since it is >= all
  // entries in the first block and < all entries in subsequent
  // blocks.
  //
  // Invariant: r->pending_index_entry is true only if data_block is empty.
  bool pending_index_entry;
  BlockHandle pending_handle;  // Handle to add to index block

  std::string compressed_output;

  Rep(const Options& opt, WritableFile* f)
    : options(opt),
    index_block_options(opt),
    file(f),
    offset(0),
    data_block(&options),
    index_block(&index_block_options),
    num_entries(0),
    closed(false),
    //   filter_block(opt.filter_policy == NULL ? NULL
    //       : new FilterBlockBuilder(opt.filter_policy)),
    pending_index_entry(false) {
      index_block_options.block_restart_interval = 1;
    }

  Rep(const Options& opt, long p)
    : options(opt),
    index_block_options(opt),
    private_data(p),
    offset(0),
    data_block(&options,0, ((struct out_mem*)p)->data_block),
    index_block(&index_block_options,1,((struct out_mem*)p)->index_block),
    num_entries(0),
    closed(false),
    //   filter_block(opt.filter_policy == NULL ? NULL
    //       : new FilterBlockBuilder(opt.filter_policy)),
    pending_index_entry(false) {
      index_block_options.block_restart_interval = 1;
    }
};

TableBuilder::TableBuilder(const Options& options, WritableFile* file)
  : rep_(new Rep(options, file)) {
    //    if (rep_->filter_block != NULL) {
    //      rep_->filter_block->StartBlock(0);
    //    }
  }
TableBuilder::TableBuilder(const Options& options, long p)
  : rep_(new Rep(options, p)) {
    //    if (rep_->filter_block != NULL) {
    //      rep_->filter_block->StartBlock(0);
    //    }
  }
TableBuilder::~TableBuilder() {
  assert(rep_->closed);  // Catch errors where caller forgot to call Finish()
  //  delete rep_->filter_block;
  delete rep_;
}

int TableBuilder::ChangeOptions(const Options& options) {
  // Note: if more fields are added to Options, update
  // this function to catch changes that should not be allowed to
  // change in the middle of building a Table.

  // Note that any live BlockBuilders point to rep_->options and therefore
  // will automatically pick up the updated options.
  rep_->options = options;
  rep_->index_block_options = options;
  rep_->index_block_options.block_restart_interval = 1;
  return 0;
}

void TableBuilder::Add(const Slice& key, const Slice& value) {
  Rep* r = rep_;
  assert(!r->closed);
  if (!ok()) return;
  if (r->num_entries > 0) {
    assert(key.compare(Slice(r->last_key)) > 0);
  }


  if (r->pending_index_entry) {
    assert(r->data_block.empty());
    FindShortestSeparator(&r->last_key, key);
    std::string handle_encoding;
    r->pending_handle.EncodeTo(&handle_encoding);
    r->index_block.Add(r->last_key, Slice(handle_encoding));
    r->pending_index_entry = false;
  }

  // if (r->filter_block != NULL) {
  //   r->filter_block->AddKey(key);
  //  }

  r->last_key.assign(key.data(), key.size());
  r->num_entries++;
  r->data_block.Add(key, value);
  static int count = 0;

  const size_t estimated_block_size = r->data_block.CurrentSizeEstimate();
  if (estimated_block_size >= r->options.block_size) {
    count++;
    Flush();
  }
}

void TableBuilder::Flush() {
  Rep* r = rep_;
  assert(!r->closed);
  if (!ok()) return;
  if (r->data_block.empty()) return;
  assert(!r->pending_index_entry);
  WriteBlock(&r->data_block, &r->pending_handle);
  if (ok()) {
    r->pending_index_entry = true;
    ocall_file_flush(&r->status);
  }
  //  if (r->filter_block != NULL) {
  //    r->filter_block->StartBlock(r->offset);
  //  }
}

void TableBuilder::WriteBlock(BlockBuilder* block, BlockHandle* handle) {
  // File format contains a sequence of blocks where each block has:
  //    block_data: uint8[n]
  //    type: uint8
  //    crc: uint32
  assert(ok());
  Rep* r = rep_;
  Slice raw = block->Finish();
  Slice block_contents;
  CompressionType type = r->options.compression;
  // TODO(postrelease): Support more compression options: zlib?
  switch (type) {
    case kNoCompression:
      block_contents = raw;
      break;
#if 0
    case kSnappyCompression: {
                               std::string* compressed = &r->compressed_output;
                               if (port::Snappy_Compress(raw.data(), raw.size(), compressed) &&
                                   compressed->size() < raw.size() - (raw.size() / 8u)) {
                                 block_contents = *compressed;
                               } else {
                                 // Snappy not supported, or compressed less than 12.5%, so just
                                 // store uncompressed form
                                 block_contents = raw;
                                 type = kNoCompression;
                               }
                               break;
                             }
#endif
  }
  WriteRawBlock(block_contents, type, handle,block->getType());
  r->compressed_output.clear();
  block->Reset();
}

void TableBuilder::WriteRawBlock(const Slice& block_contents,
    CompressionType type,
    BlockHandle* handle, int block_type) {
  Rep* r = rep_;
  handle->set_offset(r->offset);
  handle->set_size(block_contents.size());
 // if (block_type==0) bar1("write data block size=%d\n",block_contents.size());
  ocall_append_nospace(&r->status,block_type,block_contents.size());
  if (r->status==0) {
    char trailer[100];
    trailer[0] = type;
    uint32_t crc = crc32c::Value(block_contents.data(), block_contents.size());
    crc = crc32c::Extend(crc, trailer, 1);  // Extend crc to cover block type
    EncodeFixed32(trailer+1, crc32c::Mask(crc));
   // r->status = r->file->Append(Slice(trailer, kBlockTrailerSize));
    ocall_append(&r->status,trailer,kBlockTrailerSize);
    if (r->status==0) {
      r->offset += block_contents.size() + kBlockTrailerSize;
    }
  }
}

int TableBuilder::status() const {
  return rep_->status;
}

int TableBuilder::Finish() {
  Rep* r = rep_;
  Flush();
  assert(!r->closed);
  r->closed = true;

  BlockHandle filter_block_handle, metaindex_block_handle, index_block_handle;

  // Write filter block
  // if (ok() && r->filter_block != NULL) {
  //   WriteRawBlock(r->filter_block->Finish(), kNoCompression,
  //       &filter_block_handle);
  // }

  // Write metaindex block
  if (ok()) {
    BlockBuilder meta_index_block(&r->options,2,((struct out_mem*)(r->private_data))->meta_block);
    //   if (r->filter_block != NULL) {
    // Add mapping from "filter.Name" to location of filter data
    /*    std::string key = "filter.";
          key.append(r->options.filter_policy->Name());
          std::string handle_encoding;
          filter_block_handle.EncodeTo(&handle_encoding);
          meta_index_block.Add(key, handle_encoding);
          }*/

    // TODO(postrelease): Add stats and other meta blocks
    WriteBlock(&meta_index_block, &metaindex_block_handle);
  }

  // Write index block
  if (ok()) {
    if (r->pending_index_entry) {
      FindShortSuccessor(&r->last_key);
      std::string handle_encoding;
      r->pending_handle.EncodeTo(&handle_encoding);
      r->index_block.Add(r->last_key, Slice(handle_encoding));
      r->pending_index_entry = false;
    }
    WriteBlock(&r->index_block, &index_block_handle);
  }

  // Write footer
  if (ok()) {
    Footer footer;
    footer.set_metaindex_handle(metaindex_block_handle);
    footer.set_index_handle(index_block_handle);
    std::string footer_encoding;
    footer.EncodeTo(&footer_encoding);
    ocall_append(&r->status,(char *)footer_encoding.data(),footer_encoding.size());
    if (r->status==0) {
      r->offset += footer_encoding.size();
    }
  }
  return r->status;
}

void TableBuilder::Abandon() {
  Rep* r = rep_;
  assert(!r->closed);
  r->closed = true;
}

uint64_t TableBuilder::NumEntries() const {
  return rep_->num_entries;
}

uint64_t TableBuilder::FileSize() const {
  return rep_->offset;
}

