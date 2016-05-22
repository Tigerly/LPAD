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
class MergingIterator;
struct g_mem {
  char index_mem[10][100000000];
  char mem[10][10000000];
  uint64_t  file_size[10];
};

struct mht_node {
  unsigned char digest[20];
};

#define MERKLE_TREE 0
void sha3_update(const unsigned char *input, unsigned int length);
void sha3_final(unsigned char *hash, unsigned int size);
void insert(struct mht_node** tree, const char* message, int message_len) {
  int i = 0;
  unsigned char carry[20];
  unsigned char m[40];
  struct mht_node* node = (struct mht_node*)malloc(sizeof(struct mht_node));
  //  sha1(message,message_len,node->digest);
  sha3_update((unsigned const char*)message,message_len);
  sha3_final(node->digest,20);
  memcpy(carry,node->digest,20);
  for (i=0;i<100;i++) {
    if (tree[i] == NULL) {
      tree[i] = node;
      memcpy(node->digest,carry,20);
      break;
    } else {
      memcpy(m,tree[i]->digest,20);
      memcpy(m+20,carry,20);
      // sha1(m,40,carry);
      sha3_update((unsigned const char*)m,40);
      sha3_final(carry,20);
      if (tree[i]!=NULL)
        free(tree[i]);
      tree[i]=NULL;
    }
  }
}
void zc_entry(int file_count,long arg1, long arg2){
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
  options1.create_if_missing=1;
  Iterator* resIter = NewMergingIterator(&iterator_list[0] , file_count);


  struct mht_node** mht_trees[10];
  struct mht_node** out_tree;
  for (int i=0;i<10;i++) {
    struct mht_node** res = (struct mht_node**)malloc(100*sizeof(struct mht_node*));
    for (int j=0;j<100;j++)
      res[j] = NULL;
    mht_trees[i]=res;
  }
  out_tree = (struct mht_node**)malloc(100*sizeof(struct mht_node*));
  for (int j=0;j<100;j++)
    out_tree[j] = NULL;


  resIter->SeekToFirst();
  /* build output file */
  TableBuilder* builder = new TableBuilder(options1,arg2);
  for (;resIter->Valid();resIter->Next()) {
#if MERKLE_TREE
    insert(mht_trees[resIter->getCurrentIdx()],resIter->key().data(),resIter->key().size());
    insert(out_tree,resIter->key().data(),resIter->key().size());
#endif
    builder->Add(resIter->key(),resIter->value());
  }


  builder->Finish();
  uint64_t file_size = builder->FileSize();
  delete builder;
  return;
}

