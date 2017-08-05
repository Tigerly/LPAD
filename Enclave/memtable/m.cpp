// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include "Enclave_t.h"
#include "Enclave.h"
#include "arena.h"
#include "skiplist.h"
//#include "port_posix.h"
#include "comparator.h"
#include "m.h"


class MemTable* mem;
struct arg_mem {
  SequenceNumber s;
  ValueType type;
  Slice key;
  Slice value;
};

struct arg_out_mem {
  Slice userkey;
  Slice memkey;
  std::string* value;
  Status* s;
};

void enclave_memtable_create() {
  InternalKeyComparator comp(BytewiseComparator());
  mem = new MemTable(comp); 
}

void enclave_memtable_add(long arg) {
  struct arg_mem* thearg = (struct arg_mem*)arg;
//  mem->Add(thearg->s,thearg->type,thearg->key,thearg->value);
}

void enclave_memtable_get(long arg) {
  struct arg_out_mem* thearg = (struct arg_out_mem*)arg;
 mem->SUGet(thearg->memkey,thearg->userkey,thearg->value,thearg->s);
}
