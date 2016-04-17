#include "Enclave_t.h"  /* bar*/
#include "Enclave.h"  /* bar1*/

#define INPUT_BUFFER_SIZE 1024
#define OUTPUT_BUFFER_SIZE 1024
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



int readKV(int channel) {
  if (in_index[channel] == INPUT_BUFFER_SIZE || in_index[channel] == -1) {
    bar1("in readKV and try to reload\n");
    ocall_reload(key_sizes[channel],value_sizes[channel],key_data[channel],value_data[channel],&data_count[channel],channel);
    in_index[channel] = -1;
    if (data_count[channel] == 0) {
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
  if (out_index==OUTPUT_BUFFER_SIZE) {
    bar1("in writeKV and try to flush\n");
    ocall_flush(out_key_sizes,out_value_sizes,out_key,out_value,out_index);
    out_index = 0;
  }
  //copy key
  out_start = out_index << 5;
  input_start = in_index[channel] << 5;
  for(i=0;i<32;i++)
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

int my_compare(void* src1, void* src2, int n) {
  int res = 0;
  for (int i=0;i<n;i++) {
    if (*(unsigned char *)src1 == *(unsigned char *)src2) {src1=src1+1;src2=src2+1;}
    else if (*(unsigned char *)src1 < *(unsigned char *)src2) return -1;
    else return 1;
  }
}

inline int compare(int i, int s)  {
  int index_i = in_index[i];
  int index_s = in_index[s];
  // Slice  key_i(&key_data[i][index_i<<5], key_sizes[i][index_i]);
  // Slice  key_s(&key_data[s][index_s<<5], key_sizes[s][index_s]);
  return my_compare(&key_data[i][index_i<<5],&key_data[s][index_s<<5],16);
}

int findSmallest(int n_ways) {
  int smallest = -1;
  for (int i=0;i<n_ways;i++) {
    if (in_index[i]== -1 || in_index[i] == data_count[i]) {
      if (readKV(i) == 0)  {
        continue;
      }
    }

    if (smallest == -1) {
      smallest = i;
      continue;
    }
    else {
      if (compare(i,smallest) < 0)
        smallest = i;
    }

  }
  return smallest;
}
void EnclCompact(int file_count)
{
  int i=0;
  int count = file_count;
  bar1("in enclcompact and file_count=%d\n",count);
  int next;
  for (int i=0;i<file_count;i++)
    readKV(i);
  while (count>0) {
    next = findSmallest(file_count);
    if (next==-1) break;
    writeKV(next);
    if (readKV(next) == 0) {
      count--;
    }
  }
  bar1("the last flush in enclave\n");
  ocall_flush(out_key_sizes,out_value_sizes,out_key,out_value,out_index);
  out_index=0;

  //TODO: verify result and signs it
}
