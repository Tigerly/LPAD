#include <stdarg.h>
#include <stdio.h>      /* vsnprintf */

#include <string.h>
#include <sgx_cpuid.h>

#include "sgx_trts.h"
#include "Enclave.h"
#include "Enclave_t.h"  /* bar*/
#include <unistd.h>

/* 
 * printf: 
 *   Invokes OCALL to display the enclave buffer to the terminal.
 */
void bar1(const char *fmt, ...)
{
  char buf[BUFSIZ] = {'\0'};
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(buf, BUFSIZ, fmt, ap);
  va_end(ap);
  ocall_bar(buf);
}

/* ecall_foo:
 *   Uses malloc/free to allocate/free trusted memory.
 */
extern void EnclCompact(int file_count);
extern void eextrac_EnclCompact(int file_count);
extern void onec_EnclCompact(int file_count,long user_arg);
extern void zc_entry(int file_count,long user_arg1, long user_arg2);
int ecall_foo(int i, long arg1, long arg2)
{
//  EnclCompact(i);
//  eextrac_EnclCompact(i);
  onec_EnclCompact(i,arg1);
//  zc_entry(i,arg1,arg2);
  return 3;
}
/* ecall_sgx_cpuid:
 *   Uses sgx_cpuid to get CPU features and types.
 */
void ecall_sgx_cpuid(int cpuinfo[4], int leaf)
{
  sgx_status_t ret = sgx_cpuid(cpuinfo, leaf);
  if (ret != SGX_SUCCESS)
    abort();
}

void enclave_preput(unsigned int id[]);
void enclave_postput(char key[],unsigned int id,unsigned long seq);
void enclave_preget(unsigned int id[]);
void enclave_postget(char key[],unsigned int id, unsigned long seq, unsigned long tw, int pf[], int pf_index);
void enclave_writer();
void enclave_notify(long chain);
void enclave_verify_file(int merkle_height);
void enclave_verify_sim();
void enclave_verify(long chain, char key[16], int key_size, uint64_t seqno, int isMem);
void ecall_writer() {
  enclave_writer();
}

void ecall_verify(long chain, char key[16], int key_size, uint64_t seqno, int isMem) {
  enclave_verify(chain,key,key_size,seqno,isMem);
}

void ecall_verify_sim() {
  enclave_verify_sim();
}
void ecall_notify(long chain) {
  enclave_notify(chain);
}

void ecall_verify_file(int merkle_height) {
  enclave_verify_file(merkle_height);
}

void ecall_preget(unsigned int id[]){
  enclave_preget(id);
}

void ecall_postget(char key[],unsigned int id,unsigned long seq,unsigned long tw, int pf[], int pf_index){
  enclave_postget(key,id,seq,tw,pf,pf_index);
}

void ecall_preput(unsigned int id[]){
  enclave_preput(id);
}

void ecall_postput(char key[],unsigned int id,unsigned long seq){
  enclave_postput(key,id,seq);
}
