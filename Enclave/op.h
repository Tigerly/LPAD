typedef int Crt;
typedef unsigned int Timestamp;
typedef unsigned int Realtime;
typedef unsigned int Opid;
class Op {
  public:
    Op(){}
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
    void setTs(Timestamp ts) {this->ts_att=ts;}
    Realtime getStart() {return start;}
    Realtime getEnd() {return end;}
    Timestamp getTs() {return ts_att;}
    int key;
    int val;
    Timestamp ts_att;
    Timestamp ts_rw;
    Realtime start;
    Realtime end;
    //    Proof* pf;
};

class HistoryW {
public:
  HistoryW() {
 //   current = new Op(-1,-1,0,-1,NULL,0,0);
    current.setTs(0);
    current.setStart(0);
    current.setEnd(0);
  }
  Op latest(){return current;}
  void update(Op op) {
    this->current=op;
  }
private:
  Op current;
};
