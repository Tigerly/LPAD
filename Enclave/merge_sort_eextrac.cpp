#include "Enclave_t.h"  /* bar*/
#include "Enclave.h"  /* bar1*/
#include <string.h>
int ee_key_sizes[10];
int ee_value_sizes[10];
char ee_key_data[10][32];
char ee_value_data[10][100];
int valid[10];


int eextrac_readKV(int channel) {
  ocall_eextrac_nextKey(&ee_key_sizes[channel],&ee_value_sizes[channel],
      ee_key_data[channel],ee_value_data[channel],
      &valid[channel],channel);
  if (valid[channel]) return 1;
  else return 0;
}


void eextrac_writeKV(int channel) {
    ocall_eextrac_flush(ee_key_sizes[channel],ee_value_sizes[channel],
        ee_key_data[channel],ee_value_data[channel]);
}

int eextrac_my_compare(void* src1, void* src2, int n) {
//  int res = 0;
//  for (int i=0;i<n;i++) {
//    if (*(unsigned char *)src1 == *(unsigned char *)src2) {src1=src1+1;src2=src2+1;}
 //   else if (*(unsigned char *)src1 < *(unsigned char *)src2) return -1;
  //  else return 1;
//  }
  return memcmp(src1,src2,n);
}

inline int eextrac_compare(int i, int s)  {
  // Slice  key_i(&ee_key_data[i][index_i<<5], ee_key_sizes[i][index_i]);
  // Slice  key_s(&ee_key_data[s][index_s<<5], ee_key_sizes[s][index_s]);
  return eextrac_my_compare(ee_key_data[i],ee_key_data[s],24);
}

int eextrac_findSmallest(int n_ways) {
  int smallest = -1;
  for (int i=0;i<n_ways;i++) {
    if (!valid[i]) continue;

    if (smallest == -1) {
      smallest = i;
      continue;
    }
    else {
      if (eextrac_compare(i,smallest) < 0)
        smallest = i;
    }

  }
  return smallest;
}
void eextrac_EnclCompact(int file_count)
{
  int i=0;
  int count = file_count;
  int next;
  for (int i=0;i<file_count;i++)
    eextrac_readKV(i);
  while (count>0) {
    next = eextrac_findSmallest(file_count);
    if (next==-1) break;
    eextrac_writeKV(next);
    if (eextrac_readKV(next) == 0) {
      count--;
    }
  }
  //TODO: verify result and signs it
}
