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
struct mht_node {
  unsigned char digest[20];
};
#define MERKLE_TREE 1
void sha3_update(const unsigned char *input, unsigned int length);
void sha3_final(unsigned char *hash, unsigned int size);
void* sha1(void* message, int message_len, void* digest);

#if 0
void static insert(struct mht_node** tree, const char* message, int message_len) {
  int i = 0;
  unsigned char carry[20];
  unsigned char m[40];
  struct mht_node* node = (struct mht_node*)malloc(sizeof(struct mht_node));
  // sha3_update((unsigned const char*)message,message_len);
  // sha3_final(node->digest,20);
  sha1((void *)message,message_len,node->digest);
  memcpy(carry,node->digest,20);
  for (i=0;i<100;i++) {
    if (tree[i] == NULL) {
      tree[i] = node;
      memcpy(node->digest,carry,20);
      break;
    } else {
      memcpy(m,tree[i]->digest,20);
      memcpy(m+20,carry,20);
      // sha3_update((unsigned const char*)m,40);
      // sha3_final(carry,20);
      sha1(m,40,carry);
      if (tree[i]!=NULL)
        free(tree[i]);
      tree[i]=NULL;
    }
  }
}
#endif
void static insert(const char* message, int message_len) {
  int i = 0;
  unsigned char carry[20];
  unsigned char m[40];
  unsigned char dummy[20];
  int tree[100];
  // sha3_update((unsigned const char*)message,message_len);
  // sha3_final(node->digest,20);
  sha1((void *)message,message_len,dummy);
  //memcpy(carry,dummy,20);
  for (i=0;i<100;i++) {
    if (tree[i] == 0) {
      tree[i] = 1;
      //memcpy(dummy,carry,20);
      break;
    } else {
      //memcpy(m,dummy,20);
      //memcpy(m+20,carry,20);
      // sha3_update((unsigned const char*)m,40);
      // sha3_final(carry,20);
      sha1(m,40,carry);
      tree[i]=0;
    }
  }
}

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
  out_start = cookie->out_index << 5;
  input_start = cookie->in_index[channel] << 5;
  for(i=0;i<32;i++)
    cookie->out_key[out_start+i] = cookie->key_data[channel][input_start+i];
  cookie->out_key_sizes[cookie->out_index] = cookie->key_sizes[channel][cookie->in_index[channel]];
  out_start = cookie->out_index * 100;
  input_start = cookie->in_index[channel] * 100;
  for(i=0;i<100;i++)
    cookie->out_value[out_start+i] = cookie->value_data[channel][input_start+i];
  cookie->out_value_sizes[cookie->out_index] = cookie->value_sizes[channel][cookie->in_index[channel]];
  cookie->out_index++;
}
int onec_my_compare(void* src1, void* src2, int n) {
  return memcmp(src1,src2,n);
}
inline int onec_compare(int i, int s)  {
  int index_i = cookie->in_index[i];
  int index_s = cookie->in_index[s];
  return onec_my_compare(&(cookie->key_data[i][index_i<<5]),&(cookie->key_data[s][index_s<<5]),24);
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
  char* message;
  int message_len;
  /* 
     struct mht_node **current_tree; 
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
   */
  int next;
  for (int i=0;i<file_count;i++)
    onec_readKV(i);
  while (count>0) {
    next = onec_findSmallest(file_count);
    if (next==-1) break;
#if MERKLE_TREE
    message = &cookie->key_data[next][cookie->in_index[next] <<5];
    message_len = cookie->key_sizes[next][cookie->in_index[next]];
    //  current_tree = mht_trees[next];
    //insert(current_tree,message,message_len);
    //  insert(out_tree,message,message_len);
   // bar1("called\n");
    insert(message,message_len);
    insert(message,message_len);
#endif
    onec_writeKV(next);
    if (onec_readKV(next) == 0) {
      count--;
    }
  }
  ocall_1c_flush();
  cookie->out_index=0;
}
