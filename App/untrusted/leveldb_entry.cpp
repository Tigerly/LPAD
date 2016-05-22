#include "file_system.h"
#include <stdio.h>
#include "table.h"
#include "options.h"
#include "comparator.h"
#include "merger.h"
#include "table_builder.h"
#include "dlfcn.h"
#include <time.h>
#include <stdlib.h>
#include <vector>

using namespace std;
#define INPUT_BUFFER_SIZE 1024
#define OUTPUT_BUFFER_SIZE 1024
TableBuilder* builder;
Iterator **g_list;
WritableFile* outfile;

struct g_arg_t {
  int key_sizes[10][INPUT_BUFFER_SIZE];
  int value_sizes[10][INPUT_BUFFER_SIZE];
  char key_data[10][INPUT_BUFFER_SIZE*32];
  char value_data[10][INPUT_BUFFER_SIZE*100];
  int in_index[10] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
  int data_count[10];

  char out_key[32*OUTPUT_BUFFER_SIZE];
  int out_key_sizes[OUTPUT_BUFFER_SIZE];
  char out_value[OUTPUT_BUFFER_SIZE*100];
  int out_value_sizes[OUTPUT_BUFFER_SIZE];
  int out_index=0;
};

struct g_arg_t my_arg;


void do_flush(int key_sizes[], int value_sizes[], 
    char key[],char value[], int length) {
  int i=0;
  for (i=0;i<length;i++) {
    Slice key_slice(&key[i<<5],key_sizes[i]);
    Slice value_slice(&value[i*100],value_sizes[i]);
    builder->Add(key_slice,value_slice);
  }
}


void do_reload(int key_sizes[],int value_sizes[],
    char key[], char value[], int length[], int channel){
  Iterator* iter;
  int index = 0;
  int i = 0;
  int key_size = 0;
  int value_size = 0;
  int start = 0;
  iter = g_list[channel];
  while (iter->Valid() && index<INPUT_BUFFER_SIZE) {
    key_size = iter->key().size();
    value_size = iter->value().size();
    key_sizes[index] = key_size;
    value_sizes[index] = value_size;
    start = index * 32;
    for (i=0;i<32 && i< key_size;i++)
      key[start+i] = iter->key().data()[i];
    start = index * 100;
    for (i=0;i<100 && i<value_size;i++)
      value[start+i] = iter->value().data()[i];

    index++;
    iter->Next();
  }
  length[0] = index;
}

void do_flush_eextrac(int key_size, int value_size, 
    char key[],char value[]) {
  Slice key_slice(key,key_size);
  Slice value_slice(value,value_size);
  builder->Add(key_slice,value_slice);
}


void do_reload_eextrac(int key_size[],int value_size[],
    char key[], char value[], int valid[], int fileIdx){
  Iterator* iter;
  int i = 0;
  int ks = 0;
  int vs = 0;
  iter = g_list[fileIdx];
  if (iter->Valid()) {
    valid[0] = 1;
    ks = iter->key().size();
    vs = iter->value().size();
    key_size[0] = ks;
    value_size[0] = vs; 
    for (i=0;i< ks;i++)
      key[i] = iter->key().data()[i];
    for (i=0;i<vs;i++)
      value[i] = iter->value().data()[i];
    iter->Next();
  } else {
    valid[0] = 0;
  }
}

void do_flush_1c() {
  int i=0;
  for (i=0;i<my_arg.out_index;i++) {
    Slice key(&(my_arg.out_key[i<<5]),my_arg.out_key_sizes[i]);
    Slice value(&(my_arg.out_value[i*100]),my_arg.out_value_sizes[i]);
    builder->Add(key,value);
  }
}

void do_reload_1c(int channel) {
  Iterator* iter;
  int index = 0;
  int i = 0;
  int key_size = 0;
  int value_size = 0;
  int start = 0;
  iter = g_list[channel];
  while (iter->Valid() && index<INPUT_BUFFER_SIZE) {
    key_size = iter->key().size();
    value_size = iter->value().size();
    my_arg.key_sizes[channel][index] = key_size;
    my_arg.value_sizes[channel][index] = value_size;
    start = index * 32;
    for (i=0;i<32 && i< key_size;i++)
      my_arg.key_data[channel][start+i] = iter->key().data()[i];
    start = index * 100;
    for (i=0;i<100 && i<value_size;i++)
      my_arg.value_data[channel][start+i] = iter->value().data()[i];

    index++;
    iter->Next();
  }
  my_arg.data_count[channel] = index;
}


int su_prepare(int argc, char* argv[], int *r, long *user_arg){
  int file_count = atoi(argv[1]);
  *r = file_count;
  *user_arg = (long)(&my_arg);
  uint64_t* file_size_list = new uint64_t[file_count];
  RandomAccessFile** file_list = new RandomAccessFile*[file_count];
  vector<std::string> file_name_list;
  Table **table_list = new Table*[file_count];
  g_list = new Iterator*[file_count];
  for (int i=0;i<file_count;i++)
    file_name_list.push_back(argv[2+i]);
  for (int i=0;i<file_count;i++)
    printf("file_name=%s\n",file_name_list[i].c_str());
  for (int i=0;i<file_count;i++)
    file_size_list[i] = atoll(argv[2+file_count+i]);
  for (int i=0;i<file_count;i++)
    printf("file_size=%llu\n",file_size_list[i]);
  const ReadOptions options;
  Options options1;
  for (int i=0;i<file_count;i++)
    NewRandomAccessFile(file_name_list[i],&file_list[i]);
  for (int i=0;i<file_count;i++)
    Table::Open(file_list[i],file_size_list[i],&table_list[i]);
  for (int i=0;i<file_count;i++) {
    g_list[i] = table_list[i]->NewIterator(options);
    g_list[i]->SeekToFirst();
  }
  options1.create_if_missing=1;
  /* build output file */
  std::string outname = "output.ldb";
  NewWritableFile(outname,&outfile);
  builder = new TableBuilder(options1,outfile);

  return 0;
}


void su_cleanup() {
  builder->Finish();
  uint64_t file_size = builder->FileSize();
  delete builder;
  outfile->Sync();
  outfile->Close();
  printf("outfile size=%lu\n",file_size);
  delete outfile;
}

