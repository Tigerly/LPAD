#include "Enclave_t.h"  /* bar*/
#include "Enclave.h"  /* bar1*/
#include <string.h>
struct hashchain_node {
  unsigned char digest[20];
};
uint64_t last_seq = 0;

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
void static add_chain(long chain_address, const char* message, int message_len, uint64_t seqno) {
  struct hash_chain* my_chain = (struct hash_chain *)chain_address;
  if ((!my_chain->prev_valid) && (my_chain->start == my_chain->tail)) {
    memcpy(buf,message,KEY_SIZE);
    memcpy(buf+KEY_SIZE,&seqno,SEQ_SIZE);
    sha3_update((unsigned const char*)buf,KEY_SIZE+SEQ_SIZE);
    sha3_final(ret,DIGEST_SIZE);
    memcpy(&my_chain->raw_data[my_chain->tail*DIGEST_SIZE],ret,DIGEST_SIZE);
    start_seq = seqno;
  }
  else {
    if (my_chain->prev_valid) {
      memcpy(buf,my_chain->prev,20);
      start_seq = seqno;
      my_chain->prev_valid = 0;
    }
    else 
      memcpy(buf,&my_chain->raw_data[(my_chain->tail-1)*20],20);
    memcpy(buf+DIGEST_SIZE,message,16);
    memcpy(buf+DIGEST_SIZE+KEY_SIZE,&seqno,8);
    sha3_update((unsigned const char*)buf,40);
    sha3_final(ret,20);
    memcpy(&my_chain->raw_data[my_chain->tail*20],ret,20);
  }
  my_chain->tail++;
}
void enclave_writer(long chain_address, char key[16], char value[100], int key_size, int value_size, uint64_t seqno) {
  if (seqno == last_seq + 1) {
    last_seq = seqno;
    add_chain(chain_address, key,key_size,seqno);
  } else {
    // abort
  }
}
void static build_merkle(struct mht_node** tree, const char* message, int message_len) {
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


int timeTraverse(long chain, int start, int end){
  int i;
  // bar1("time traverse start=%d, end=%d\n",start,end);
  int status;
  struct hash_chain *my_chain = (struct hash_chain *)chain;
  for (int i=start;i<end;i++) {
    memcpy(buf,dummy_prev_digest,20);
    memcpy(buf,dummy_key,16);
    memcpy(buf,&dummy_seq,8);
    sha3_update((unsigned const char*)buf,KEY_SIZE+SEQ_SIZE);
    sha3_final(ret,DIGEST_SIZE);
    status = memcmp(ret,&my_chain->raw_data[start*20],20);
    // if (status!=0) return;
  } 
  return 1;
}

void enclave_notify(long chain_address) {
  struct hash_chain *my_chain = (struct hash_chain *)chain_address;
  if (my_chain->reason==1) { 
    //    bar1("before flush imm_start=%d imm_end=%d\n",my_chain->imm_start,my_chain->imm_end);
    my_chain->imm_start = 0;
    my_chain->imm_end = 0;
    //bar1("after flush imm_start=0 imm_end=0\n");
  }
  else {
    //    bar1("before flip imm_start=%d imm_end=%d start=%d, tail=%d, prev_valid=%d\n",my_chain->imm_start,my_chain->imm_end,my_chain->start,my_chain->tail,my_chain->prev_valid);
    my_chain->imm_start = my_chain->start;
    my_chain->imm_end = my_chain->tail-1;
    memcpy(my_chain->prev,&my_chain->raw_data[my_chain->imm_end],20);
    my_chain->prev_valid = 1;
    my_chain->start = 30000-my_chain->start;
    my_chain->tail = my_chain->start;
    imm_start_seq = start_seq;
    /* reorder immutable memtable to time-ordered*/
   
   uint64_t tag;
    uint64_t min;
    uint64_t max;
    memcpy(&tag,my_chain->imm_data+16,8);
    min = max = tag>>8;
    int pos;
    int j=0;
    for (int i=0;i<my_chain->imm_end-my_chain->imm_start+1;i++) {
      memcpy(&tag,my_chain->imm_data+j+16,8);
      tag = tag>>8;
      if (tag<min) min=tag;
      if (tag>max) max=tag;
      j+=24;
    // pos = tag - imm_start_seq;
      pos = 0;
      memcpy(reordered_imm_data+pos*24,my_chain->imm_data+j,24);
    }
    /* time traverse to verify*/
    int isCorrect=0;
    isCorrect = timeTraverse(chain_address,my_chain->imm_start,my_chain->imm_end);
    if (isCorrect==0) {}//abort();

    /* build merkle tree*/
    int k=0;
    for (int i=0;i<my_chain->imm_end-my_chain->imm_start+1;i++) {
      build_merkle(out_tree,(const char *)my_chain->imm_data+k,24);
      k+=24;
    }
  //  bar1("after flip max=%lu and min=%lu\n",max,min);
  //  bar1("after flip imm_start=%d imm_end=%d start=%d, tail=%d, prev_valid=%d, imm_start_seq=%lu\n",my_chain->imm_start,my_chain->imm_end,my_chain->start,my_chain->tail,my_chain->prev_valid,imm_start_seq);
  } 
}

void enclave_verify_file(int merkle_height) {
  for (int i=0;i<merkle_height;i++) {
    sha3_update((unsigned const char*)buf,KEY_SIZE+SEQ_SIZE);
    sha3_final(ret,DIGEST_SIZE);
  }
}

void enclave_verify(long chain, char key[16], int key_size, uint64_t seqno, int isMem) {
  struct hash_chain *my_chain = (struct hash_chain *)chain;
  int verify_start = 0;
  int isCorrect = 0;
  if (isMem == 1) {
    //   bar1("found in mem, start_seq=%lu, target_seq=%lu, start=%d,tail=%d\n",start_seq,seqno,my_chain->start,my_chain->tail);
    //  verify_start = my_chain->start+seqno-start_seq;
    //  isCorrect = timeTraverse(chain,verify_start,my_chain->tail-1);
    //  if (isCorrect == 0){} // abort();      
    int tmp = my_chain->tail-my_chain->start;
    int merkle_height=0;
    while (tmp >>= 1) { ++merkle_height; }
    merkle_height++;
    enclave_verify_file(2*merkle_height);

  } else if (isMem==2) {
    // bar1("verifiy partial imm seqno=%lu imm_start_seq=%lu\n",seqno,imm_start_seq);
    //   isCorrect = timeTraverse(chain,my_chain->imm_start+seqno-imm_start_seq,my_chain->imm_end);
    int tmp = my_chain->imm_end-my_chain->imm_start;
    int merkle_height=0;
    while (tmp >>= 1) { ++merkle_height; }
    merkle_height++;
    enclave_verify_file(2*merkle_height);

  } else {
    //        bar1("verify entire imm\n");
    /* method 1 - verify over merkle-tree*/
    int tmp = my_chain->imm_end-my_chain->imm_start;
    int merkle_height=0;
    while (tmp >>= 1) { ++merkle_height; }
    merkle_height++;
    enclave_verify_file(2*merkle_height);

    /* method 2 - hash chain verifying*/
    //  isCorrect = timeTraverse(chain,my_chain->imm_start,my_chain->imm_end);
    //  if (isCorrect == 0){} //abort();
    //  isCorrect = timeTraverse(chain,my_chain->start,my_chain->tail-1);
    //  if (isCorrect == 0){} //abort();
  }
}



