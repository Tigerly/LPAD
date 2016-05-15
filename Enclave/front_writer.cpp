#include "Enclave_t.h"  /* bar*/
#include "Enclave.h"  /* bar1*/
#include <string.h>
struct hashchain_node {
  unsigned char digest[20];
};
uint64_t last_seq = 0;

#define DIGEST_SIZE 20
#define MESSAGE_SIZE 32
#define SEQ_SIZE 8
struct hash_chain {
  unsigned char raw_data[DIGEST_SIZE*60000];
  int start;
  int tail;
  int imm_start;
  int imm_end;
  int reason;
  unsigned char prev[DIGEST_SIZE];
  int prev_valid;
};

unsigned char buf[DIGEST_SIZE+MESSAGE_SIZE+SEQ_SIZE];
unsigned char ret[DIGEST_SIZE];

uint64_t start_seq = 0;
uint64_t imm_start_seq = 0;
/* temporary*/
char dummy_key[32];
unsigned char dummy_prev_digest[20];
uint64_t dummy_seq=0;

void sha3_update(const unsigned char *input, unsigned int length);
void sha3_final(unsigned char *hash, unsigned int size);
void static add_chain(long chain_address, const char* message, int message_len, uint64_t seqno) {
  struct hash_chain* my_chain = (struct hash_chain *)chain_address;
  if ((!my_chain->prev_valid) && (my_chain->start == my_chain->tail)) {
    memcpy(buf,message,MESSAGE_SIZE);
    memcpy(buf+MESSAGE_SIZE,&seqno,SEQ_SIZE);
    sha3_update((unsigned const char*)buf,MESSAGE_SIZE+SEQ_SIZE);
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
    memcpy(buf+20,message,32);
    memcpy(buf+52,&seqno,8);
    sha3_update((unsigned const char*)buf,40);
    sha3_final(ret,20);
    memcpy(&my_chain->raw_data[my_chain->tail*20],ret,20);
  }
  my_chain->tail++;
}
void enclave_writer(long chain_address, char key[32], char value[100], int key_size, int value_size, uint64_t seqno) {

  if (seqno == last_seq + 1) {
    last_seq = seqno;
    add_chain(chain_address, key,key_size,seqno);
  } else {
    // abort
  }
}

void enclave_notify(long chain_address) {
  struct hash_chain *my_chain = (struct hash_chain *)chain_address;
  if (my_chain->reason==1) { 
//    bar1("before flush imm_start=%d imm_end=%d\n",my_chain->imm_start,my_chain->imm_end);
    my_chain->imm_start = 0;
    my_chain->imm_end = 0;
//    bar1("after flush imm_start=0 imm_end=0\n");
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
//    bar1("after flip imm_start=%d imm_end=%d start=%d, tail=%d, prev_valid=%d\n",my_chain->imm_start,my_chain->imm_end,my_chain->start,my_chain->tail,my_chain->prev_valid);
  } 
}

int timeTraverse(long chain, int start, int end){
  int i;
 // bar1("time traverse start=%d, end=%d\n",start,end);
  int status;
  struct hash_chain *my_chain = (struct hash_chain *)chain;
  for (int i=start;i<end;i++) {
      memcpy(buf,dummy_prev_digest,20);
      memcpy(buf,dummy_key,32);
      memcpy(buf,&dummy_seq,8);
      sha3_update((unsigned const char*)buf,MESSAGE_SIZE+SEQ_SIZE);
      sha3_final(ret,DIGEST_SIZE);
      status = memcmp(ret,&my_chain->raw_data[start*20],20);
      // if (status!=0) return;
  } 
  return 1;
}

void enclave_verify(long chain, char key[32], int key_size, uint64_t seqno, int isMem) {
  struct hash_chain *my_chain = (struct hash_chain *)chain;
  int verify_start = 0;
  int isCorrect = 0;
  if (isMem) {
 //   bar1("found in mem, start_seq=%lu, target_seq=%lu, start=%d,tail=%d\n",start_seq,seqno,my_chain->start,my_chain->tail);
    verify_start = my_chain->start+seqno-start_seq;
    isCorrect = timeTraverse(chain,verify_start,my_chain->tail-1);
    if (isCorrect == 0){} // abort();      
  } else {
    isCorrect = timeTraverse(chain,verify_start,my_chain->imm_end);
    if (isCorrect == 0){} //abort();
    isCorrect = timeTraverse(chain,my_chain->start,my_chain->tail-1);
    if (isCorrect == 0){} //abort();
  }
}


void enclave_verify_file(int merkle_height) {
//  bar1("verify files height=%d\n",merkle_height);
  for (int i=0;i<merkle_height;i++) {
    sha3_update((unsigned const char*)buf,MESSAGE_SIZE+SEQ_SIZE);
    sha3_final(ret,DIGEST_SIZE);
  }
}
