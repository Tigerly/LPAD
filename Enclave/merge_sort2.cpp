#include "Enclave_t.h"  /* bar*/
#include "Enclave.h"  /* bar1*/

int* file1;
int* file2;
int* outfile;
int file1ii=0;
int file2ii=0;
int outIndex=0;

//return true if eof
bool readKV(int fileIdx, int* nextKey)
{
  //TODO REMOVE
  if(fileIdx == 1){
    if(file1ii > 9) return true;
    else {
      *nextKey = file1[file1ii];
    }
  } else {
    if(file2ii > 9) return true;
    else {
      *nextKey = file2[file2ii];
    }
  }
  return false;
}

bool writeKV(int k, bool endOfFile)
{
  outfile[outIndex++] = k;
}

void moveNext(int i) {
  if (i==1) file1ii++;
  else file2ii++;
}

void EnclCompact(int* input1, int* input2, int* output)
{
  file1 = input1;
  file2 = input2;
  outfile = output;
  int i1 = 0, i2 = 0;
  bool endOfFile = false;
  int streamEnded = 0;

  do {
    if (readKV(1,&i1)) streamEnded |= 1;
    if (readKV(2,&i2)) streamEnded |= 2;
    if (streamEnded) break;
    if(i1 < i2){
      writeKV(i1,false);
      moveNext(1);
    }
    else {
      writeKV(i2,false);
      moveNext(2);
    }
  } while (true);

  if(streamEnded == 1 || streamEnded == 2){
    int i =0;
    int stream = 3-streamEnded;
    do {
      if(readKV(stream,&i)) break;
      else {
        writeKV(i,false);
        moveNext(stream);
      }
    } while (true);
  }

  //TODO: verify result and signs it
}
