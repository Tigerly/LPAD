#include "op.h"
#include "Enclave_t.h"  /* bar*/
#include "Enclave.h"  /* bar1*/
#include <string.h>
#include <sgx_thread.h>
#include <map>
#include <algorithm>


static sgx_thread_mutex_t global_mutex = SGX_THREAD_MUTEX_INITIALIZER;

typedef std::map<int,int > map_t;
typedef map_t::value_type map_value;
map_t global_map;
unsigned int g_counter=0;

Realtime getTime() {
  Realtime local;
  sgx_thread_mutex_lock(&global_mutex);
  g_counter++;
  local=g_counter;
  sgx_thread_mutex_unlock(&global_mutex);
  return local;
}
/*
class store_wrapper{
  Store store;
  Att cPut(key,val){
    prePut(key,val);
    att(tsw)=store.dPut(key,val);
    return postPut(key,val,att(tsw)); 7 }
  Crt cGet(key){
    preGet(<key>);
    <key,val>,pf(tsrw,tsr*)=store.dGet(key);
    return postGet(<key>,pf(tsrw,tsr*));
  }

  mutex State pending_wr, completed_wr,history_w;

  void prePut(<key,val>){
    pending_wr.add(<key,val,start_rt=now()>);
  }
  boolean postPut(<key,val>,att(tsw)){
    <key,val,start_rt>=pending_wr.remove();
    completed_wr.addW(<key,val,start_rt,end_rt=now(),tsw>)
      ;
    ac1 = completed_wr.tryTruncate();
    if(ac1 != NULL){
      assertC(ac1,history_w);
      for(Write w in ac1.trim())
        history_w.put(w);
    }
  }
  void preGet(key){
    pending_wr.add(<key,start_rt=now()>);
  }
  boolean postGet(r<key,val,tsrw,tsr,pf(tsrw,tsr*)>){
    r<key,start_rt>=pending_wr.remove();
    if(r.tsr <= history_w.latest());
    assertL2(r<key,val,tsr,tsrw,pf(tsrw,tsr*)>,history_w
        );
    else
      completed_wr.addR(<key,val,start_rt,end_rt=now(),tsr
          ,tsrw>);  }
}
void assertC(ac1,history_w){
  o0<key,val,ts> = history_w.latest();
  o1<key,val,ts> = ac1.oldest();
  do{
    assertL1pairwise(o0<key,val,ts>,o1<key,val,ts>);  o0=o1; o1=o1.next(ac1);
  } while (o1 != NULL)
  for (read r in ac1)
    assertL2(r<key,val,tsr,tsrw,pf(tsrw,tsr*)>,history_w); 12 }
void assertL2(r<key,val,tsr,tsrw,pf(tsrw,tsr*)>,history_w)
{
*/
void enclave_preget() {
//  sgx_thread_mutex_lock(&global_mutex);
//  global_map.insert(map_value(0,0));
//  sgx_thread_mutex_unlock(&global_mutex);
}

void enclave_postget(unsigned long seq, unsigned long tw){
//  sgx_thread_mutex_lock(&global_mutex);
//  global_map.find(0);
//  sgx_thread_mutex_unlock(&global_mutex);
    bar1("r %ld with %ld\n",seq,tw);
}

void enclave_preput() {
//  sgx_thread_mutex_lock(&global_mutex);
//  global_map.insert(map_value(0,0));
//  sgx_thread_mutex_unlock(&global_mutex);
}

void enclave_postput(unsigned long seq){
//  sgx_thread_mutex_lock(&global_mutex);
//  global_map.find(0);
//  sgx_thread_mutex_unlock(&global_mutex);
    bar1("w %ld\n",seq);
}
