#include "file_system.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <vector>
struct g_mem {
  char index_mem[10][100000000];
  char mem[10][10000000];
  uint64_t  file_size[10];
};
struct g_mem my_mem;

struct out_mem {
  char data_block[1000000];
  char index_block[1000000000];
  char meta_block[1000];
};

struct out_mem my_out_mem;


using namespace std;
zcRandomAccessFile** file_list;
zcWritableFile* zcOutfile;

int do_read(uint64_t offset, size_t size, int length[1], int fileIdx, char space[100]) {
  return file_list[fileIdx]->ReadSU(offset,size,length,space);
}
int do_read_nospace(uint64_t offset, size_t size, int length[1], int fileIdx, int isIndex) {
  char* mem;
  if (isIndex) { 
      mem = my_mem.index_mem[fileIdx];
  } else {
      mem = my_mem.mem[fileIdx];
  }
  return file_list[fileIdx]->ReadSU(offset,size,length,mem);
}

int do_append(char* buf, size_t size) {
  return zcOutfile->Append(Slice(buf,size)); 
}
int do_append_nospace(int block_type, size_t size) {
  char* buf;
  if (block_type==0) buf=my_out_mem.data_block; 
  else if (block_type==1) buf=my_out_mem.index_block; 
  else buf=my_out_mem.meta_block; 
  return zcOutfile->Append(Slice(buf,size)); 
}
int do_file_flush() {
  return zcOutfile->Flush();
}
int su_prepare_zc(int argc, char* argv[], int* r, long* arg1, long* arg2){
  int file_count = atoi(argv[1]);
  *r = file_count;
  *arg1 = (long)&my_mem;
  *arg2 = (long)&my_out_mem;
  uint64_t* file_size_list = my_mem.file_size;
  file_list = new zcRandomAccessFile*[file_count];
  vector<std::string> file_name_list;
  for (int i=0;i<file_count;i++)
    file_name_list.push_back(argv[2+i]);
  for (int i=0;i<file_count;i++)
    printf("file_name=%s\n",file_name_list[i].c_str());
  for (int i=0;i<file_count;i++)
    file_size_list[i] = atoll(argv[2+file_count+i]);
  for (int i=0;i<file_count;i++)
    printf("file_size=%llu\n",file_size_list[i]);
  for (int i=0;i<file_count;i++)
    zcNewRandomAccessFile(file_name_list[i],&file_list[i]);


  /* build output file */
  std::string outname = "/home/ju/test_data/output.ldb";
  zcNewWritableFile(outname,&zcOutfile);
  
#if 0
  /* verify */
  std::string readname = "000011.ldb";
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

void su_cleanup_zc() {
  zcOutfile->Sync();
  zcOutfile->Close();
  delete zcOutfile;
}

