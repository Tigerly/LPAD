typedef int Crt;
typedef unsigned int Timestamp;
typedef unsigned int Realtime;
typedef unsigned int Opid;
class Op {
  public:
    Op(int key){
      this->key=key;
    }
    Op(int key, int val) {
      this->key = key;
      this->val = val;
    }

    Op(Opid id, Realtime s) {
      this->id = id;
      this->start = s;
    }
/*
    Op(int key, int val, Timestamp ts_att, Timestamp ts_rw, Proof* pf, Realtime start, Realtime end){
      this->key=key;
      this->val=val;
      this->ts_att=ts_att;
      this->ts_rw=ts_rw;
      this->pf=pf;
      this->start = start;
      this->end = end;
    }
*/
    Opid id; 
    void setStart(Realtime start) {this->start=start;}
    void setEnd(Realtime end) {this->end=end;}
    Realtime getStart() {return start;}
    int key;
    int val;
    Timestamp ts_att;
    Timestamp ts_rw;
    Realtime start;
    Realtime end;
//    Proof* pf;
};
