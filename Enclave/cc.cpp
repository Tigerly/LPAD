#include "op.h"
#include "Enclave_t.h"  /* bar*/
#include "Enclave.h"  /* bar1*/
#include <string.h>
#include <sgx_thread.h>
#include <map>
#include <algorithm>


static sgx_thread_mutex_t pending_mutex = SGX_THREAD_MUTEX_INITIALIZER;
static sgx_thread_mutex_t time_mutex = SGX_THREAD_MUTEX_INITIALIZER;
static sgx_thread_mutex_t id_mutex = SGX_THREAD_MUTEX_INITIALIZER;

typedef std::map<unsigned long, Op > map_t;
typedef map_t::value_type map_value;
map_t pending_list;
unsigned int g_timer=0;
unsigned int g_id=0;

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
void enclave_preget(unsigned int idd[]) {
  Realtime time = getTime();
  Opid id = allocId();
  Op op(id,time);
  idd[0]=id;
  sgx_thread_mutex_lock(&pending_mutex);
  pending_list.insert(map_value(id,op));
  sgx_thread_mutex_unlock(&pending_mutex);
  bar1("[%ld] pre_r id=%ld\n",time,id);
}

void enclave_postget(char key[],unsigned int id,unsigned long seq, unsigned long tw){
  Realtime time = getTime();
  Op op = pending_list.find(id)->second;
  pending_list.erase(id);
  bar1("[%ld] post_r %ld with %ld key=%s\n",time,seq,tw,key);
}

void enclave_preput(unsigned int idd[]) {
  Realtime time = getTime();
  Opid id = allocId();
  Op op(id,time);
  idd[0] = id;
  sgx_thread_mutex_lock(&pending_mutex);
  pending_list.insert(map_value(id,op));
  sgx_thread_mutex_unlock(&pending_mutex);
  bar1("[%ld] pre_w id=%ld\n",time,id);
}

void enclave_postput(char key[],unsigned int id,unsigned long seq){
  Realtime time = getTime();
  sgx_thread_mutex_lock(&pending_mutex);
  Op op = pending_list.find(id)->second;
  pending_list.erase(id);
  sgx_thread_mutex_unlock(&pending_mutex);
  bar1("[%ld,%ld] post_w %ld key=%s\n",op.getStart(),time,seq,key);
}
