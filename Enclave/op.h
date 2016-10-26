typedef int Crt;
typedef unsigned int Timestamp;
typedef unsigned int Realtime;
class Op {
  public:
    Op(int key){
      this->key=key;
    }
    Op(int key, int val) {
      this->key = key;
      this->val = val;
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
    void setStart(Realtime start) {this->start=start;}
    void setEnd(Realtime end) {this->end=end;}
    int key = -1;
    int val = -1;
    Timestamp ts_att = -1;
    Timestamp ts_rw = -1;
    Realtime start = -1;
    Realtime end = -1;
//    Proof* pf;
};
