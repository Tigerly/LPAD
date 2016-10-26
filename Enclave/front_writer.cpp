#include "Enclave_t.h"  /* bar*/
#include "Enclave.h"  /* bar1*/
#include <string.h>
#include <sgx_thread.h>
#include <map>
#include <algorithm>
struct hashchain_node {
  unsigned char digest[20];
};
uint64_t last_seq = 0;

static size_t global_counter = 0;

#define DIGEST_SIZE 20
#define KEY_SIZE 16
#define SEQ_SIZE 8
struct hash_chain {
  unsigned char raw_data[DIGEST_SIZE*60000];
  unsigned char imm_data[24*30000];
  int start;
  int tail;
  int imm_start;
  int imm_end;
  int reason;
  unsigned char prev[DIGEST_SIZE];
  int prev_valid;
};
unsigned char reordered_imm_data[24*30000];
struct mht_node {
  unsigned char digest[20];
};
unsigned char buf[DIGEST_SIZE+KEY_SIZE+SEQ_SIZE];
unsigned char ret[DIGEST_SIZE];

uint64_t start_seq = 0;
uint64_t imm_start_seq = 0;
/* temporary*/
char dummy_key[16];
unsigned char dummy_prev_digest[20];
uint64_t dummy_seq=0;
struct mht_node** out_tree = (struct mht_node**)malloc(100*sizeof(struct mht_node*));

void sha3_update(const unsigned char *input, unsigned int length);
void sha3_final(unsigned char *hash, unsigned int size);
void* sha1(void* message, int message_len, void* digest);

void enclave_verify_file(int merkle_height) {
  for (int i=0;i<merkle_height;i++) {
    // sha3_update((unsigned const char*)buf,KEY_SIZE+SEQ_SIZE);
    //  sha3_final(ret,DIGEST_SIZE);
    sha1(buf,KEY_SIZE+SEQ_SIZE,ret);
  }
}
void static add_chain() {
  static int write_count=0;

  int tmp = 7000;//my_chain->tail-my_chain->start;
  int merkle_height=0;
  while (tmp >>= 1) { ++merkle_height; }
  merkle_height++;
  enclave_verify_file(2*merkle_height);
}

void enclave_writer() {
  //  if (seqno == last_seq + 1) {
  //    last_seq = seqno;
  add_chain();

  // add_chain(chain_address, key,key_size,seqno);
  //  } else {
  // abort
  //  }
}
void static build_merkle(struct mht_node** tree, const char* message, int message_len) {
  int i = 0;
  unsigned char carry[20];
  unsigned char m[40];
  struct mht_node* node = (struct mht_node*)malloc(sizeof(struct mht_node));
  //  sha3_update((unsigned const char*)message,message_len);
  //  sha3_final(node->digest,20);
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
      // sha1(m,40,carry);
      //   sha3_update((unsigned const char*)m,40);
      //   sha3_final(carry,20);
      sha1(m,40,carry);
      if (tree[i]!=NULL)
        free(tree[i]);
      tree[i]=NULL;
    }
  }
}


int timeTraverse(long chain, int start, int end){
  int i;
  int status;
  struct hash_chain *my_chain = (struct hash_chain *)chain;
  for (int i=start;i<end;i++) {
    memcpy(buf,dummy_prev_digest,20);
    memcpy(buf,dummy_key,16);
    memcpy(buf,&dummy_seq,8);
    sha3_update((unsigned const char*)buf,KEY_SIZE+SEQ_SIZE);
    sha3_final(ret,DIGEST_SIZE);
    status = memcmp(ret,&my_chain->raw_data[start*20],20);
  } 
  return 1;
}

void enclave_notify(long chain_address) {
}



void enclave_verify(long chain, char key[16], int key_size, uint64_t seqno, int ismem) {
  struct hash_chain *my_chain = (struct hash_chain *)chain;
  int verify_start = 0;
  int iscorrect = 0;
  if (ismem == 1) {
    //  verify_start = my_chain->start+seqno-start_seq;
    //  iscorrect = timetraverse(chain,verify_start,my_chain->tail-1);
    //  if (iscorrect == 0){} // abort();      
    int tmp = 7000;//my_chain->tail-my_chain->start;
    int merkle_height=0;
    while (tmp >>= 1) { ++merkle_height; }
    merkle_height++;
    enclave_verify_file(merkle_height);

  } else if (ismem==2) {
    //   iscorrect = timetraverse(chain,my_chain->imm_start+seqno-imm_start_seq,my_chain->imm_end);
    int tmp = 7000;//my_chain->imm_end-my_chain->imm_start;
    int merkle_height=0;
    while (tmp >>= 1) { ++merkle_height; }
    merkle_height++;
    enclave_verify_file(merkle_height);

  } else {
    /* method 1 - verify over merkle-tree*/
    int tmp = 7000;//my_chain->imm_end-my_chain->imm_start;
    int merkle_height=0;
    while (tmp >>= 1) { ++merkle_height; }
    merkle_height++;
    enclave_verify_file(merkle_height);

    /* method 2 - hash chain verifying*/
    //  iscorrect = timetraverse(chain,my_chain->imm_start,my_chain->imm_end);
    //  if (iscorrect == 0){} //abort();
    //  iscorrect = timetraverse(chain,my_chain->start,my_chain->tail-1);
    //  if (iscorrect == 0){} //abort();
  }
}




void enclave_verify_sim() {
  int tmp = 700;//my_chain->tail-my_chain->start;
  int merkle_height=0;
  while (tmp >>= 1) { ++merkle_height; }
  merkle_height++;
  enclave_verify_file(merkle_height);
}




