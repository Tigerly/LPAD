#include <stdio.h>
#include "table.h"
#include "options.h"
#include "merger.h"
#include "table_builder.h"
#include <stdlib.h>
#include <vector>
#include "Enclave_t.h"
#include "Enclave.h"
using namespace std;
class TwoLevelIterator;
struct g_mem {
  char index_mem1[100000000];
  char index_mem2[100000000];
  char mem1[10000000];
  char mem2[10000000];
  uint64_t  file_size[2];
};
void zc_entry(int file_count,long arg1, long arg2){
  bar1("in zc_entry\n");
  uint64_t* file_size_list = ((struct g_mem *)arg1)->file_size;
  Table **table_list = new Table*[file_count];
  Iterator **iterator_list = new Iterator*[file_count];
  const ReadOptions options;
  Options options1;
  for (int i=0;i<file_count;i++)
    Table::Open_SU(i, file_size_list[i],&table_list[i],arg1);
  for (int i=0;i<file_count;i++)
    iterator_list[i] = table_list[i]->NewIterator(options);

  for (int i=0;i<file_count;i++)
    iterator_list[i]->setFileIdx(i);
  bar1("entering zc_entry\n");
  options1.create_if_missing=1;
  Iterator* resIter = NewMergingIterator(&iterator_list[0] , file_count);
  resIter->SeekToFirst();
  /* build output file */
  TableBuilder* builder = new TableBuilder(options1,arg2);
  for (;resIter->Valid();resIter->Next()) {
    builder->Add(resIter->key(),resIter->value());
  }


  builder->Finish();
  uint64_t file_size = builder->FileSize();
  delete builder;
  return;
}

