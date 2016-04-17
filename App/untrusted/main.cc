#include "file_system.h"
#include <stdio.h>
#include "table.h"
#include "options.h"
#include "comparator.h"
#include "merger.h"
#include "table_builder.h"
#include "dlfcn.h"
#include "sgx.h"

struct element {
  Slice *key;
  Slice *value;
};

struct element_list {
  struct element** elements_;
  int index;
  int size;
};

struct merge_sort_args {
  struct element_list** input;
  int value;
  struct element_list* output;
};

typedef void (*merge_sort_t)(struct element_list **, int, struct element_list *);
int main(int argc, char* argv[]){

  /* fetach data from file 1,2,3 */
  RandomAccessFile* file1 = NULL;
  RandomAccessFile* file2 = NULL;
  RandomAccessFile* file3 = NULL;
  std::string fname1 = "000005.ldb";
  std::string fname2 = "000007.ldb";
  std::string fname3 = "000009.ldb";
  Table *table1 = NULL;
  Table *table2 = NULL;
  Table *table3 = NULL;
  const ReadOptions options;
  NewRandomAccessFile(fname1,&file1);
  NewRandomAccessFile(fname2,&file2);
  NewRandomAccessFile(fname3,&file3);
  Table::Open(file1,131,&table1);
  Table::Open(file2,131,&table2);
  Table::Open(file3,131,&table3);

  Iterator* iter1 = table1->NewIterator(options);
  Iterator* iter2 = table2->NewIterator(options);
  Iterator* iter3 = table3->NewIterator(options);

  struct element_list list_a;
  struct element_list list_b;
  struct element_list list_c;

  int i = 0;
  iter1->SeekToFirst();
  int size_a = 0;
  for (;iter1->Valid();iter1->Next()) 
    size_a++;
  list_a.index = 0;
  list_a.elements_ = new element*[size_a];
  list_a.size = size_a;
  iter1->SeekToFirst();
  printf("==========================FILE1's data=======================\n");
  for (;iter1->Valid();iter1->Next()) {
    Slice key = iter1->key();
    Slice value = iter1->value();
    printf("key=%s and value=%s\n",key.data(),value.data());
    element* myElement1 = new element;
    char* data = new char[key.size()];
    memcpy(data,key.data(),key.size());
    Slice *key1 = new Slice(data,key.size());
    data = new char[value.size()];
    memcpy(data,value.data(),value.size());
    Slice *value1 = new Slice(data,value.size());
    myElement1->key = key1; 
    myElement1->value = value1; 
    list_a.elements_[i++] = myElement1;
  }

  i = 0;
  iter2->SeekToFirst();
  int size_b = 0;
  for (;iter2->Valid();iter2->Next()) 
    size_b++;
  list_b.index = 0;
  list_b.elements_ = new element*[size_b];
  list_b.size = size_b;
  iter2->SeekToFirst();
  printf("==========================FILE2's data=======================\n");
  for (;iter2->Valid();iter2->Next()) {
    Slice key = iter2->key();
    Slice value = iter2->value();
    printf("key=%s and value=%s\n",key.data(),value.data());
    element* myElement2 = new element;
    char* data = new char[key.size()];
    memcpy(data,key.data(),key.size());
    Slice *key1 = new Slice(data,key.size());
    data = new char[value.size()];
    memcpy(data,value.data(),value.size());
    Slice *value1 = new Slice(data,value.size());
    myElement2->key = key1; 
    myElement2->value = value1; 
    list_b.elements_[i++] = myElement2;
  }

  i = 0;
  iter3->SeekToFirst();
  int size_c = 0;
  for (;iter3->Valid();iter3->Next()) 
    size_c++;
  list_c.index = 0;
  list_c.elements_ = new element*[size_c];
  list_c.size = size_c;
  iter3->SeekToFirst();
  printf("=========================FILE3's data=======================\n");
  for (;iter3->Valid();iter3->Next()) {
    Slice key = iter3->key();
    Slice value = iter3->value();
    printf("key=%s and value=%s\n",key.data(),value.data());
    element* myElement3 = new element;
    char* data = new char[key.size()];
    memcpy(data,key.data(),key.size());
    Slice *key1 = new Slice(data,key.size());
    data = new char[value.size()];
    memcpy(data,value.data(),value.size());
    Slice *value1 = new Slice(data,value.size());
    myElement3->key = key1; 
    myElement3->value = value1; 
    list_c.elements_[i++] = myElement3;
  }


  /* universal n-way merge sort */
  struct element_list** input = new element_list*[3];
  struct element_list output;
  input[0] = &list_a;
  input[1] = &list_b;
  input[2] = &list_c;
  void* enclave_handle = dlopen("./bin/merge_enclave.so",RTLD_LAZY);
  void* enclave_merge_sort = dlsym(enclave_handle,"merge_sort");
 // (*(merge_sort_t)enclave_merge_sort)(input,3,&output);

  /* we are going to enter  enclave */
  struct merge_sort_args arg;
  arg.input = input;
  arg.output = &output;
  arg.value = 3;
  TCS tcs;
  tcs.saved = false;
  tcs.oentry = (unsigned long)enclave_merge_sort;
  void *p_tcs = &tcs;

  __asm__ __volatile__ (
      "mov %2, %%rdx\n\t" // App-level argument
      "mov %1, %%rbx\n\t" // address of tcs
      "movl $0x02, %%eax\n\t" //EENTER instruction type
      ".byte 0x0f,0x01,0xd7\n\t" //call opcode 01d7
      "mov %%rbx, %0"
      : "=r" (p_tcs)
      : "r" (p_tcs), "r"(&arg)
      : "eax", "rbx", "rdx"
      );
#if 1
  /* build output file */
  std::string outname = "000011.ldb";
  WritableFile* outfile;
  NewWritableFile(outname,&outfile);
  Options opt;
  opt.create_if_missing=1;
  TableBuilder* builder = new TableBuilder(opt,outfile);
  for (int i=0;i<output.size;i++)
    builder->Add(*output.elements_[i]->key,*output.elements_[i]->value);


  builder->Finish();
  uint64_t file_size = builder->FileSize();
  delete builder;
  outfile->Sync();
  outfile->Close();
  delete outfile;
  delete file1;
  delete file2;
  delete file3;
  delete table1;
  delete table2;
  delete table3;

  /* verify */
  std::string readname = "000011.ldb";
  RandomAccessFile* readfile = NULL;
  Table *readtable = NULL;
  NewRandomAccessFile(readname,&readfile);
  Table::Open(readfile,file_size,&readtable);
  Iterator* readiter = readtable->NewIterator(options);
  readiter->SeekToFirst();
  printf("==========================out file  data=======================\n");
  for (;readiter->Valid();readiter->Next()) {
    Slice key = readiter->key();
    printf("key=%s and value=%s\n",key.data(),readiter->value().data());
  }
  delete readfile;
#endif 
  return 0;
}

