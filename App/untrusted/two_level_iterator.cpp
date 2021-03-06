// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include "two_level_iterator.h"

#include "table.h"
#include "block.h"
#include "format.h"
#include "iterator_wrapper.h"


namespace {

typedef Iterator* (*BlockFunction)(void*, const ReadOptions&, const Slice&);

class TwoLevelIterator: public Iterator {
 public:
  TwoLevelIterator(
    Iterator* index_iter,
    BlockFunction block_function,
    void* arg,
    const ReadOptions& options);

  virtual ~TwoLevelIterator();

  virtual void Seek(const Slice& target);
  virtual void SeekToFirst();
  virtual void SeekToLast();
  virtual void Next();
  virtual void Prev();

  virtual bool isEOB();
  virtual bool isIOAcked();
  virtual void ackIO();
  virtual void SU_Next();

  virtual bool Valid() const {
    return data_iter_.Valid();
  }
  virtual Slice key() const {
    assert(Valid());
    return data_iter_.key();
  }
  virtual Slice value() const {
    assert(Valid());
    return data_iter_.value();
  }
  virtual int status() const {
    // It'd be nice if status() returned a const Status& instead of a Status
    if (index_iter_.status()!=0) {
      return index_iter_.status();
    } else if (data_iter_.iter() != NULL && data_iter_.status()!=0) {
      return data_iter_.status();
    } else {
      return status_;
    }
  }

 private:
  void SaveError(int error) {
    if (status_==0 && error!=0) status_ = error;
  }
  void SkipEmptyDataBlocksForward();
  void SU_SkipEmptyDataBlocksForward();
  void SkipEmptyDataBlocksBackward();
  void SetDataIterator(Iterator* data_iter);
  void InitDataBlock();

  BlockFunction block_function_;
  void* arg_;
  const ReadOptions options_;
  int status_;
  IteratorWrapper index_iter_;
  IteratorWrapper data_iter_; // May be NULL
  // If data_iter_ is non-NULL, then "data_block_handle_" holds the
  // "index_value" passed to block_function_ to create the data_iter_.
  std::string data_block_handle_;

  bool EOB_;
  bool IOAcked_;
};

TwoLevelIterator::TwoLevelIterator(
    Iterator* index_iter,
    BlockFunction block_function,
    void* arg,
    const ReadOptions& options)
    : block_function_(block_function),
      arg_(arg),
      options_(options),
      index_iter_(index_iter),
      data_iter_(NULL) ,
      EOB_(false) ,
      IOAcked_(false) {
}

TwoLevelIterator::~TwoLevelIterator() {
}
void TwoLevelIterator::ackIO() {
  IOAcked_ = true;
}

bool TwoLevelIterator::isEOB() {
  return EOB_;
}

bool TwoLevelIterator::isIOAcked() {
  return IOAcked_;
}
void TwoLevelIterator::Seek(const Slice& target) {
  index_iter_.Seek(target);
  InitDataBlock();
  if (data_iter_.iter() != NULL) data_iter_.Seek(target);
  SkipEmptyDataBlocksForward();
}

void TwoLevelIterator::SeekToFirst() {
  index_iter_.SeekToFirst();
  InitDataBlock();
  if (data_iter_.iter() != NULL) data_iter_.SeekToFirst();
  SkipEmptyDataBlocksForward();
}

void TwoLevelIterator::SeekToLast() {
  index_iter_.SeekToLast();
  InitDataBlock();
  if (data_iter_.iter() != NULL) data_iter_.SeekToLast();
  SkipEmptyDataBlocksBackward();
}

void TwoLevelIterator::SU_Next() {
  if (IOAcked_) {
    SU_SkipEmptyDataBlocksForward();
    return;
  }
  assert(Valid());
  data_iter_.Next();
  SU_SkipEmptyDataBlocksForward();
}

void TwoLevelIterator::SU_SkipEmptyDataBlocksForward() {
  while (data_iter_.iter() == NULL || !data_iter_.Valid()) {
    // Move to next block
    if (!index_iter_.Valid()) {
      SetDataIterator(NULL);
      return;
    }
    if (IOAcked_) {
      IOAcked_ = false;
      EOB_ = false;
    } else {
      EOB_ = true;
      return;
    }
    index_iter_.Next();
    InitDataBlock();
    if (data_iter_.iter() != NULL) data_iter_.SeekToFirst();
  }
}
void TwoLevelIterator::Next() {
  assert(Valid());
  data_iter_.Next();
  SkipEmptyDataBlocksForward();
}


void TwoLevelIterator::Prev() {
  assert(Valid());
  data_iter_.Prev();
  SkipEmptyDataBlocksBackward();
}



void TwoLevelIterator::SkipEmptyDataBlocksForward() {
  while (data_iter_.iter() == NULL || !data_iter_.Valid()) {
    // Move to next block
    if (!index_iter_.Valid()) {
      SetDataIterator(NULL);
      return;
    }
    index_iter_.Next();
    InitDataBlock();
    if (data_iter_.iter() != NULL) data_iter_.SeekToFirst();
  }
}

void TwoLevelIterator::SkipEmptyDataBlocksBackward() {
  while (data_iter_.iter() == NULL || !data_iter_.Valid()) {
    // Move to next block
    if (!index_iter_.Valid()) {
      SetDataIterator(NULL);
      return;
    }
    index_iter_.Prev();
    InitDataBlock();
    if (data_iter_.iter() != NULL) data_iter_.SeekToLast();
  }
}

void TwoLevelIterator::SetDataIterator(Iterator* data_iter) {
  if (data_iter_.iter() != NULL) SaveError(data_iter_.status());
  data_iter_.Set(data_iter);
}

void TwoLevelIterator::InitDataBlock() {
  if (!index_iter_.Valid()) {
    SetDataIterator(NULL);
  } else {
    Slice handle = index_iter_.value();
    if (data_iter_.iter() != NULL && handle.compare(data_block_handle_) == 0) {
      // data_iter_ is already constructed with this iterator, so
      // no need to change anything
    } else {
      Iterator* iter = (*block_function_)(arg_, options_, handle);
      data_block_handle_.assign(handle.data(), handle.size());
      SetDataIterator(iter);
    }
  }
}

}  // namespace

Iterator* NewTwoLevelIterator(
    Iterator* index_iter,
    BlockFunction block_function,
    void* arg,
    const ReadOptions& options) {
  return new TwoLevelIterator(index_iter, block_function, arg, options);
}

