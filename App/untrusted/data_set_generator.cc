#include "file_system.h"
#include <stdio.h>
#include "table.h"
#include "options.h"
#include "comparator.h"
#include "merger.h"
#include "table_builder.h"
#include "dlfcn.h"
#include "sgx.h"
#include <stdlib.h>
void increase_key_by_twenty(Slice **key, Slice **value, char* key_data, char* value_data)
{
  int mean;
  int i = 14;
  mean = (unsigned char)key_data[15];
  if (mean<108) {
      key_data[15] = key_data[15] + 20;
  } else {
      key_data[15] = mean+20-128;
      while (i>=0) {
        if (key_data[i]!=127) {key_data[i]++;break;}
        else {key_data[i]=0;i--;}
      }
  }
//  for (i=0;i<3;i++)
//    printf("key_data[%d]=%d ",i,key_data[i]);
 // printf("\n");
  *key = new Slice(key_data,16);
  *value = new Slice(value_data,100);
}
int main(int argc, char* argv[]){
  char* file_name = argv[1];
  int order_no = atoi(argv[2]);
  int num_million = atoi(argv[3]);
 // printf("order_no=%d and num_million=%d\n",order_no,num_million);
  Slice *key;
  Slice *value;
  char key_data[16];
  char value_data[100];
  memset(key_data,0,16);
  memset(value_data,0,100);
  key_data[15] = order_no;
  value_data[0] = 'b';
  key = new Slice(key_data,16);
  value = new Slice(value_data,100);
  /* build output file */
  std::string outname = file_name;
  WritableFile* outfile;
  NewWritableFile(outname,&outfile);
  Options opt;
  opt.create_if_missing=1;
  TableBuilder* builder = new TableBuilder(opt,outfile);
  for (int i=0;i<100000*num_million;i++) {
     builder->Add(*key,*value);
     delete key;
     delete value;
    increase_key_by_twenty(&key,&value,key_data,value_data);
  }


  builder->Finish();
  uint64_t file_size = builder->FileSize();
  delete builder;
  outfile->Sync();
  outfile->Close();
  delete outfile;
  printf("outfile file size=%llu\n",file_size);
#if 0
  /* verify */
  const ReadOptions options;
  std::string readname = "01.ldb";
  RandomAccessFile* readfile = NULL;
  Table *readtable = NULL;
  NewRandomAccessFile(readname,&readfile);
  Table::Open(readfile,file_size,&readtable);
  Iterator* readiter = readtable->NewIterator(options);
  readiter->SeekToFirst();
  printf("==========================out file  data=======================\n");
  for (;readiter->Valid();readiter->Next()) {
    Slice key = readiter->key();
    printf("key=%s and value=%s\n",key.data(),readiter->value().data());
  }
  delete readfile;
#endif 
  return 0;
}

