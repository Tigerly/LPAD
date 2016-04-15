#include "Enclave_t.h"  /* bar*/
#include "Enclave.h"  /* bar1*/

int file1ii=0;
int file2ii=0;

bool readKV(int fileIdx, int* nextKey)
{
  //TODO REMOVE
  if(fileIdx == 1){
    if(++file1ii > 3) return true;
    else {
      *nextKey = file1ii+1;
      return false;
    }
  } else {
    if(++file2ii > 3) return true;
    else {
      *nextKey = file2ii+1;
      return false;
    }
  }
}

bool writeKV(int k, bool endOfFile)
{
//TODO: REMOVE
bar1("haha\n");
}

void EnclCompact(void)
{
    int input1 = 0, input2 = 0;
    int output = 0;
    bool endOfFile;
    int streamEnded;

    do {
        if(input1 < input2){
            output = input1;
            endOfFile = readKV(1,&input1);
            if(endOfFile == true) streamEnded = 1;
        }
        else {
            output = input2;
            endOfFile = readKV(2,&input2);
            if(endOfFile == true) streamEnded = 2;
        }
        writeKV(output, false);
    } while (!endOfFile);

    if(streamEnded == 1){
        //TODO: appendFile1
    } else {
        //TODO: appendFile2
    }

    //TODO: verify result and signs it
}
