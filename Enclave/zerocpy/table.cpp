// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
#include "Enclave_t.h"  /* bar*/
#include "Enclave.h"  /* bar1*/

#include "table.h"

#include "cache.h"
#include "options.h"
#include "block.h"
#include "format.h"
#include "two_level_iterator.h"
#include "coding.h"

struct Table::Rep {
  ~Rep() {
    //   delete filter;
    delete [] filter_data;
    delete index_block;
  }

  Options options;
  int status;
  RandomAccessFile* file;
  long private_data;
  uint64_t cache_id;
  // FilterBlockReader* filter;
  const char* filter_data;

  BlockHandle metaindex_handle;  // Handle to metaindex_block: saved from footer
  Block* index_block;
};

int Table::Open_SU(
    int fileIdx,
    uint64_t size,
    Table** table,
    long private_data) {
  *table = NULL;
  if (size < Footer::kEncodedLength) {
    return -1;
  }
 // char footer_space[Footer::kEncodedLength];
  char footer_space[100];
  int length;
  int res = 0;
  ocall_read(&res,size - Footer::kEncodedLength, Footer::kEncodedLength,
      &length, fileIdx, footer_space);
  if (res!=0) return res;

  Footer footer;
  Slice footer_input = Slice(footer_space,length);
  res = footer.DecodeFrom(&footer_input);
  if (res!=0) return res;

  // Read the index block
  BlockContents contents;
  Block* index_block = NULL;
  if (res==0) {
    res = ReadBlockSU(fileIdx, footer.index_handle(), &contents, true, private_data);
    if (res==0) {
      index_block = new Block(contents);
    }
  }

  if (res==0) {
    // We've successfully read the footer and the index block: we're
    // ready to serve requests.
    Rep* rep = new Table::Rep;
 //   rep->file = file;
    rep->metaindex_handle = footer.metaindex_handle();
    rep->index_block = index_block;
    rep->cache_id = 0;
    rep->filter_data = NULL;
    //  rep->filter = NULL;
    rep->private_data = private_data;
    *table = new Table(rep);
    (*table)->ReadMeta(footer);
  } else {
    if (index_block) delete index_block;
  }
 // return res;
  return 0;
}
#if 0
int Table::Open(RandomAccessFile* file,
    int fileIdx,
    uint64_t size,
    Table** table) {
  *table = NULL;
  if (size < Footer::kEncodedLength) {
    return -1;
  }
  char footer_space[Footer::kEncodedLength];
  Slice footer_input;
  // int res = 0;
  int res = file->Read(size - Footer::kEncodedLength, Footer::kEncodedLength,
      &footer_input, footer_space);
  if (res!=0) return res;

  Footer footer;
  res = footer.DecodeFrom(&footer_input);
  if (res!=0) return res;

  // Read the index block
  BlockContents contents;
  Block* index_block = NULL;
  if (res==0) {
    res = ReadBlock(file, fileIdx, footer.index_handle(), &contents, true);
    if (res==0) {
      index_block = new Block(contents);
    }
  }

  if (res==0) {
    // We've successfully read the footer and the index block: we're
    // ready to serve requests.
    Rep* rep = new Table::Rep;
    rep->file = file;
    rep->metaindex_handle = footer.metaindex_handle();
    rep->index_block = index_block;
    rep->cache_id = 0;
    rep->filter_data = NULL;
    //  rep->filter = NULL;
    *table = new Table(rep);
    (*table)->ReadMeta(footer);
  } else {
    if (index_block) delete index_block;
  }

  return res;
}
#endif
void Table::ReadMeta(const Footer& footer) {
  return;
}

Table::~Table() {
  delete rep_;
}

static void DeleteBlock(void* arg, void* ignored) {
  delete reinterpret_cast<Block*>(arg);
}

static void DeleteCachedBlock(const Slice& key, void* value) {
  Block* block = reinterpret_cast<Block*>(value);
  delete block;
}

static void ReleaseBlock(void* arg, void* h) {
  Cache* cache = reinterpret_cast<Cache*>(arg);
  Cache::Handle* handle = reinterpret_cast<Cache::Handle*>(h);
  cache->Release(handle);
}

// Convert an index iterator value (i.e., an encoded BlockHandle)
// into an iterator over the contents of the corresponding block.
Iterator* Table::BlockReader(void* arg,
    int fileIdx,
    const ReadOptions& options,
    const Slice& index_value) {
  Table* table = reinterpret_cast<Table*>(arg);
  Cache* block_cache = table->rep_->options.block_cache;
  Block* block = NULL;
  Cache::Handle* cache_handle = NULL;

  BlockHandle handle;
  Slice input = index_value;
  int res = handle.DecodeFrom(&input);
  // We intentionally allow extra stuff in index_value so that we
  // can add more features in the future.

  if (res==0) {
    BlockContents contents;
    if (block_cache != NULL) {
      char cache_key_buffer[16];
      EncodeFixed64(cache_key_buffer, table->rep_->cache_id);
      EncodeFixed64(cache_key_buffer+8, handle.offset());
      Slice key(cache_key_buffer, sizeof(cache_key_buffer));
      cache_handle = block_cache->Lookup(key);
      if (cache_handle != NULL) {
        block = reinterpret_cast<Block*>(block_cache->Value(cache_handle));
      } else {
        res = ReadBlockSU(fileIdx, handle, &contents, false, table->rep_->private_data);
        if (res==0) {
          block = new Block(contents);
          if (contents.cachable && options.fill_cache) {
            cache_handle = block_cache->Insert(
                key, block, block->size(), &DeleteCachedBlock);
          }
        }
      }
    } else {
      res = ReadBlockSU(fileIdx, handle, &contents, false, table->rep_->private_data);
      if (res==0) {
        block = new Block(contents);
      }
    }
  }

  Iterator* iter;
  if (block != NULL) {
    iter = block->NewIterator();
    if (cache_handle == NULL) {
      iter->RegisterCleanup(&DeleteBlock, block, NULL);
    } else {
      iter->RegisterCleanup(&ReleaseBlock, block_cache, cache_handle);
    }
  } else {
    iter = NewErrorIterator();
  }
  return iter;
}

Iterator* Table::NewIterator(const ReadOptions& options) const {
  return NewTwoLevelIterator(
      rep_->index_block->NewIterator(),
      &Table::BlockReader, const_cast<Table*>(this), options);
}
/*
   int Table::InternalGet(const ReadOptions& options, const Slice& k,
   void* arg,
   void (*saver)(void*, const Slice&, const Slice&)) {
   int res;
   Iterator* iiter = rep_->index_block->NewIterator();
   iiter->Seek(k);
   if (iiter->Valid()) {
   Slice handle_value = iiter->value();
   FilterBlockReader* filter = rep_->filter;
   BlockHandle handle;
   if (filter != NULL &&
   (handle.DecodeFrom(&handle_value)==0) &&
   !filter->KeyMayMatch(handle.offset(), k)) {
// Not found
} else {
Iterator* block_iter = BlockReader(this, options, iiter->value());
block_iter->Seek(k);
if (block_iter->Valid()) {
(*saver)(arg, block_iter->key(), block_iter->value());
}
res = block_iter->status();
delete block_iter;
}
}
if (res==0) {
res = iiter->status();
}
delete iiter;
return res;
}
 */

uint64_t Table::ApproximateOffsetOf(const Slice& key) const {
  Iterator* index_iter =
    rep_->index_block->NewIterator();
  index_iter->Seek(key);
  uint64_t result;
  if (index_iter->Valid()) {
    BlockHandle handle;
    Slice input = index_iter->value();
    int res = handle.DecodeFrom(&input);
    if (res==0) {
      result = handle.offset();
    } else {
      // Strange: we can't decode the block handle in the index block.
      // We'll just return the offset of the metaindex block, which is
      // close to the whole file size for this case.
      result = rep_->metaindex_handle.offset();
    }
  } else {
    // key is past the last key in the file.  Approximate the offset
    // by returning the offset of the metaindex block (which is
    // right near the end of the file).
    result = rep_->metaindex_handle.offset();
  }
  delete index_iter;
  return result;
}

