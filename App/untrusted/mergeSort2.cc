// we assume that 
// max key size is 16 byte 
// max value size is 100 byte
// merge from 3 ways
#include "file_system.h"
#include <stdio.h>
#include "table.h"
#include "options.h"
#include "comparator.h"
#include "merger.h"
#include "table_builder.h"
#include "dlfcn.h"
#include "sgx.h"

struct user_defined_args {

  //user data
  int key_sizes[3][16];
  int value_sizes[3][16];
  char key_data[3][256];
  char value_data[3][1600];
  // 16 means either the data array is empty or is all consumed
  int in_index[3];
  int data_count[3];

  char out_key[4096];
  int out_key_sizes[256];
  char out_value[25600];
  int out_value_sizes[256];
  int out_index;
  int n_ways;

  //general purpose args
  unsigned long exit_point;
  void *p_tcs;
};

struct user_defined_args g_arg;
TCS g_tcs;
unsigned long context_pointer_u;
void* enclave_handle;
void* enclave_merge_sort;
void *p_tcs;

Iterator* iter1;
Iterator* iter2;
Iterator* iter3;
WritableFile* outfile;
TableBuilder* outbuilder;

uint64_t file_size;

#define save_context_u1() \
  asm volatile (  \
      "push %%r15\n\t" \
      "push %%r14\n\t" \
      "push %%r13\n\t" \
      "push %%r12\n\t" \
      "push %%rbp\n\t" \
      "push %%rbx\n\t" \
      "push %%r11\n\t" \
      "push %%r10\n\t" \
      "push %%r9\n\t" \
      "push %%r8\n\t" \
      "push %%rax\n\t" \
      "push %%rcx\n\t" \
      "push %%rdx\n\t" \
      "push %%rsi\n\t" \
      "push %%rdi\n\t" \
      "push %%rsp\n\t" \
      "movq %%rsp, %0" \
      : "=m" (context_pointer_u):)

#define reload_context_u1()  \
      asm volatile (  \
          "mov %0, %%rsp\n\t" \
          "pop %%rsp\n\t" \
          "pop %%rdi\n\t" \
          "pop %%rsi\n\t" \
          "pop %%rdx\n\t" \
          "pop %%rcx\n\t" \
          "pop %%rax\n\t" \
          "pop %%r8\n\t" \
          "pop %%r9\n\t" \
          "pop %%r10\n\t" \
          "pop %%r11\n\t" \
          "pop %%rbx\n\t" \
          "pop %%rbp\n\t" \
          "pop %%r12\n\t" \
          "pop %%r13\n\t" \
          "pop %%r14\n\t" \
          "pop %%r15\n\t" \
          :  \
          : "m" (context_pointer_u) \
          )
    void handler() {
      int channel;
      asm volatile("mov %%rcx, %0":"=m" (channel):);
      printf("in handler chann=%d!\n",channel);
      asm volatile("mov %rbp, %rsp"); 
      asm volatile("pop %rbp"); 

      __asm__ __volatile__ (
          "mov %0, %%rbx\n\t" // address of tcs
          "movl $0x02, %%eax\n\t" //EENTER instruction type
          ".byte 0x0f,0x01,0xd7\n\t" //call opcode 01d7
          : 
          : "r" (p_tcs)
          : "eax", "rbx"
          );
    }
void init() {
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

  iter1 = table1->NewIterator(options);
  iter2 = table2->NewIterator(options);
  iter3 = table3->NewIterator(options);
  iter1->SeekToFirst();
  iter2->SeekToFirst();
  iter3->SeekToFirst();

  std::string outname = "000011.ldb";
  NewWritableFile(outname,&outfile);
  Options opt;
  opt.create_if_missing = 1;
  outbuilder = new TableBuilder(opt,outfile);

  g_arg.in_index[0] = -1;
  g_arg.in_index[1] = -1;
  g_arg.in_index[2] = -1;

}
#if 0
void ocall_loadData() {
  
}

int load_data_from_outside(int channel) {
  Iterator* iter;
  int index = 0;
  int i = 0;
  int key_size = 0;
  int value_size = 0;
  int start = 0;
  if (channel==0) iter = iter1;
  else if (channel==1) iter = iter2;
  else iter = iter3;
  while (iter->Valid() && index<16) {
    key_size = iter->key().size();
    value_size = iter->value().size();
    key_sizes[channel][index] = key_size;
    value_sizes[channel][index] = value_size;
    start = index * 16;
    for (i=0;i<16 && i< key_size;i++) 
      key_data[channel][start+i] = iter->key().data()[i];
    start = index * 100;
    for (i=0;i<100 && i<value_size;i++) 
      value_data[channel][start+i] = iter->value().data()[i];

    index++;
    iter->Next();
  }
  return index; 
}

void flush_data() {
  Slice* key;
  Slice* value;
  int i;
  for(i=0;i<out_index;i++) {
      key = new Slice(&out_key[i*16],out_key_sizes[i]);
      value = new Slice(&out_value[i*100],out_value_sizes[i]);
      outbuilder->Add(*key,*value);
  }  
  outbuilder->Finish();
  outfile->Sync();
  outfile->Close();
  file_size = outbuilder->FileSize();
}

// channel = 0,1 or 2
// return pointer to the heap data
int readKV(int channel) {
  if (in_index[channel] == 16 || in_index[channel] == -1) {
    int count = load_data_from_outside(channel);
    in_index[channel] = -1;
    data_count[channel] = count;
    if (count == 0) {
      return 0;
    } 
  } else {
    if (in_index[channel]==data_count[channel]) {
      in_index[channel] = -1;
      return 0;
    } 
  }
  in_index[channel]++;
  return 1;
}


int writeKV(int channel) {
  int i=0;
  int out_start=0;
  int input_start=0;
  if (out_index==256) {
    // flush_data();
    out_index = 0;
  }
  //copy key
  out_start = out_index << 4;
  input_start = in_index[channel] << 4;
  for(i=0;i<16;i++)
    out_key[out_start+i] = key_data[channel][input_start+i];
  out_key_sizes[out_index] = key_sizes[channel][in_index[channel]];

  //copy value
  out_start = out_index * 100;
  input_start = in_index[channel] * 100;
  for(i=0;i<100;i++)
    out_value[out_start+i] = value_data[channel][input_start+i];
  out_value_sizes[out_index] = value_sizes[channel][in_index[channel]];

  out_index++;
}

inline int compare(int i, int s)  {
  int index_i = in_index[i];
  int index_s = in_index[s];
  const size_t min_len = (key_sizes[i][index_i] < key_sizes[s][index_s]) ?
    key_sizes[i][index_i] : key_sizes[s][index_s];
  int r=0;
  if (key_data[i][index_i<<4] < key_data[s][index_s<<4]) 
    r = -1;
  else if (key_data[i][index_i<<4] == key_data[s][index_s<<4]) 
    r = 0;
  else 
    r =1;
  if (r == 0) {
    if (key_sizes[i][index_i] < key_sizes[s][index_s]) r = -1;
    else if (key_sizes[i][index_i] > key_sizes[s][index_s]) r = +1;
  }
  return r;
}
int findSmallest(int n_ways) {
  int smallest = -1;
  for (int i=0;i<n_ways;i++) {
    if (in_index[i]== -1 || in_index[i] == data_count[i]) continue;
    else {
      if (smallest == -1) {
        smallest = i;
        continue;
      }
      else {
        if (compare(i,smallest) < 0)
          smallest = i;
      }
    }
  }
  return smallest;
}

void merge_sort(int n_ways) {
  int next = 0;
  int i;
  int count = n_ways;

  //read data
  for (int i=0;i<n_ways;i++)
    readKV(i);

  while (count>0) {
    next = findSmallest(n_ways);
    if (next==-1) break;
    writeKV(next);
    if (readKV(next) == 0) {
      count--;
    } 
  }
}
#endif
int main() {
  init();

  enclave_handle = dlopen("./bin/merge_enclave.so",RTLD_LAZY);
  enclave_merge_sort = dlsym(enclave_handle,"enclave_entry_point");

  //preparing arguments to enter enclave */
  g_arg.n_ways = 3;
  g_arg.exit_point = (unsigned long)handler;
  g_tcs.saved = false;
  g_tcs.oentry = (unsigned long)enclave_merge_sort;
  g_arg.p_tcs = &g_tcs;
  p_tcs = &g_tcs;

  printf("the address of enclave handle=%p\n",enclave_handle);
  printf("the address of enclave_merge_sort=%p\n",enclave_merge_sort);
  printf("the address of tcs=%p\n",g_arg.p_tcs);
  printf("the address of tcs=%p\n",&g_arg);

  save_context_u1();
  __asm__ __volatile__ (
      "mov %2, %%rdx\n\t" // App-level argument
      "mov %1, %%rbx\n\t" // address of tcs
      "movl $0x02, %%eax\n\t" //EENTER instruction type
      ".byte 0x0f,0x01,0xd7\n\t" //call opcode 01d7
      "mov %%rbx, %0"
      : "=r" (p_tcs)
      : "r" (p_tcs), "r"(&g_arg)
      : "eax", "rbx", "rdx"
      );

  reload_context_u1();


  printf("back to main\n");
  return 0;

//  flush_data();
#if 0
   /* verify */
  const ReadOptions options;
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
#endif
}
