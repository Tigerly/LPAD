#include "Enclave_t.h"  /* bar*/
#include "Enclave.h"  /* bar1*/
#include <string.h>
#define INPUT_BUFFER_SIZE 1024
#define OUTPUT_BUFFER_SIZE 1024

struct enclave_g_arg_t {
  int key_sizes[10][INPUT_BUFFER_SIZE];
  int value_sizes[10][INPUT_BUFFER_SIZE];
  char key_data[10][INPUT_BUFFER_SIZE*32];
  char value_data[10][INPUT_BUFFER_SIZE*100];
  int in_index[10];
  int data_count[10];

  char out_key[32*OUTPUT_BUFFER_SIZE];
  int out_key_sizes[OUTPUT_BUFFER_SIZE];
  char out_value[OUTPUT_BUFFER_SIZE*100];
  int out_value_sizes[OUTPUT_BUFFER_SIZE];
  int out_index;
};

struct enclave_g_arg_t *cookie;



int onec_readKV(int channel) {
  if (cookie->in_index[channel] == INPUT_BUFFER_SIZE || cookie->in_index[channel] == -1) {
    ocall_1c_reload(channel);
    cookie->in_index[channel] = -1;
    if (cookie->data_count[channel] == 0) {
      return 0;
    }
  } else {
    if (cookie->in_index[channel]==cookie->data_count[channel]) {
      cookie->in_index[channel] = -1;
      return 0;
    }
  }
  cookie->in_index[channel]++;
  return 1;
}


int onec_writeKV(int channel) {
  int i=0;
  int out_start=0;
  int input_start=0;
  if (cookie->out_index==OUTPUT_BUFFER_SIZE) {
    ocall_1c_flush();
    cookie->out_index = 0;
  }
  //copy key
  out_start = cookie->out_index << 5;
  input_start = cookie->in_index[channel] << 5;
  for(i=0;i<32;i++)
    cookie->out_key[out_start+i] = cookie->key_data[channel][input_start+i];
  cookie->out_key_sizes[cookie->out_index] = cookie->key_sizes[channel][cookie->in_index[channel]];

  //copy value
  out_start = cookie->out_index * 100;
  input_start = cookie->in_index[channel] * 100;
  for(i=0;i<100;i++)
    cookie->out_value[out_start+i] = cookie->value_data[channel][input_start+i];
  cookie->out_value_sizes[cookie->out_index] = cookie->value_sizes[channel][cookie->in_index[channel]];

  cookie->out_index++;
}

int onec_my_compare(void* src1, void* src2, int n) {
 // int res = 0;
 // for (int i=0;i<n;i++) {
 //   if (*(unsigned char *)src1 == *(unsigned char *)src2) {src1=src1+1;src2=src2+1;}
 //   else if (*(unsigned char *)src1 < *(unsigned char *)src2) return -1;
 //   else return 1;
 // }
  return memcmp(src1,src2,n);
}

inline int onec_compare(int i, int s)  {
  int index_i = cookie->in_index[i];
  int index_s = cookie->in_index[s];
  // Slice  key_i(&key_data[i][index_i<<5], key_sizes[i][index_i]);
  // Slice  key_s(&key_data[s][index_s<<5], key_sizes[s][index_s]);
  return onec_my_compare(&(cookie->key_data[i][index_i<<5]),&(cookie->key_data[s][index_s<<5]),16);
}

int onec_findSmallest(int n_ways) {
  int smallest = -1;
  for (int i=0;i<n_ways;i++) {
    if (cookie->in_index[i]== -1 || cookie->in_index[i] == cookie->data_count[i]) {
      if (onec_readKV(i) == 0)  {
        continue;
      }
    }

    if (smallest == -1) {
      smallest = i;
      continue;
    }
    else {
      if (onec_compare(i,smallest) < 0)
        smallest = i;
    }

  }
  return smallest;
}
void onec_EnclCompact(int file_count, long user_arg)
{
  int i=0;
  int count = file_count;
  cookie = (struct enclave_g_arg_t *)user_arg; 
  
  int next;
  for (int i=0;i<file_count;i++)
    onec_readKV(i);
  while (count>0) {
    next = onec_findSmallest(file_count);
    if (next==-1) break;
    onec_writeKV(next);
    if (onec_readKV(next) == 0) {
      count--;
    }
  }
  ocall_1c_flush();
  cookie->out_index=0;

  //TODO: verify result and signs it
}
