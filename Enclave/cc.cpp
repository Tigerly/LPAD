#include "op.h"
#include "Enclave_t.h"  /* bar*/
#include "Enclave.h"  /* bar1*/
#include <string.h>
#include <sgx_thread.h>
#include <map>
#include <algorithm>
#include <vector>


static sgx_thread_mutex_t g_mutex = SGX_THREAD_MUTEX_INITIALIZER;
static sgx_thread_mutex_t time_mutex = SGX_THREAD_MUTEX_INITIALIZER;
static sgx_thread_mutex_t id_mutex = SGX_THREAD_MUTEX_INITIALIZER;

typedef std::map<unsigned long, Op > map_t;
typedef std::vector<Op > vector_t;
typedef map_t::value_type map_value;
map_t pending_list;
map_t completed_list;
unsigned int g_timer=0;
unsigned int g_id=0;
HistoryW history;

Realtime getTime() {
  Realtime local;
  sgx_thread_mutex_lock(&time_mutex);
  g_timer++;
  local=g_timer;
  sgx_thread_mutex_unlock(&time_mutex);
  return local;
}

Opid allocId() {
  Opid local;
  sgx_thread_mutex_lock(&id_mutex);
  g_id++;
  local=g_id;
  sgx_thread_mutex_unlock(&id_mutex);
  return local;
}

void enclave_preget(unsigned int idd[]) {
  Realtime time = getTime();
  Opid id = allocId();
  Op op(id,time);
  idd[0]=id;
  sgx_thread_mutex_lock(&g_mutex);
  op.setTs(history.latest().getTs());
  pending_list.insert(map_value(id,op));
  sgx_thread_mutex_unlock(&g_mutex);
  //bar1("[%ld] pre_r id=%ld\n",time,id);
}

void nmt(Op op, HistoryW& his) {
}

void enclave_postget(char key[],unsigned int id,unsigned long seq, unsigned long tw){
  Realtime time = getTime();
  sgx_thread_mutex_lock(&g_mutex);
  Op op = pending_list.find(id)->second;
  pending_list.erase(id);
  op.setTRW(tw);
  nmt(op,history);
  sgx_thread_mutex_unlock(&g_mutex);
  //bar1("[%ld] post_r %ld with %ld key=%s\n",time,seq,tw,key);
}

void enclave_preput(unsigned int idd[]) {
  Realtime time = getTime();
  Opid id = allocId();
  Op op(id,time);
  idd[0] = id;
  sgx_thread_mutex_lock(&g_mutex);
  pending_list.insert(map_value(id,op));
  sgx_thread_mutex_unlock(&g_mutex);
  //bar1("[%ld] pre_w id=%ld\n",time,id);
}

void truncate(map_t& comp,HistoryW& his, vector_t& acl) {
  Timestamp cur = his.latest().getTs()+1;
  while (comp.find(cur)!=comp.end()) {
      acl.push_back(comp.find(cur)->second);
      comp.erase(cur);
      cur++;
  }
}

bool check(vector_t& acl) {
  if (acl.size()==1) return true;
  for (int i=0;i<acl.size();i++) {
      for (int j=i+1;j<acl.size();i++) {
          if (acl[j].getEnd()<=acl[i].getStart()) {return false;}
      }
  }
  return true;
}

bool try_merge(HistoryW& his, vector_t& acl) {
  if (acl[0].getEnd() < his.latest().getStart()) return false;
  for(int i=0;i<acl.size();i++)
    his.update(acl[i]);
  return true;
}

void enclave_postput(char key[],unsigned int id,unsigned long seq){
  Realtime time = getTime();
  sgx_thread_mutex_lock(&g_mutex);
  Op op = pending_list.find(id)->second;
  op.setEnd(time);
  op.setTs(seq);
  pending_list.erase(id);
  completed_list.insert(map_value(id,op));

  vector_t acl;
  //truncate
  truncate(completed_list,history,acl);
  if (!check(acl)) //abort
  //merge
  if (!try_merge(history,acl)) //abort
  
  sgx_thread_mutex_unlock(&g_mutex);
  //bar1("[%ld,%ld] post_w %ld key=%s\n",op.getStart(),time,seq,key);
}
