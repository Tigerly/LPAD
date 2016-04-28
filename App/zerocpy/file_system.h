#ifndef __FILE_SYSTEM__
#define __FILE_SYSTEM__
#include "slice.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>

class zcRandomAccessFile {
  public:
    zcRandomAccessFile(const std::string& fname, int fd)
      : filename_(fname), fd_(fd) { }
    ~zcRandomAccessFile(){close(fd_);}

    // Read up to "n" bytes from the file starting at "offset".
    // "scratch[0..n-1]" may be written by this routine.  Sets "*result"
    // to the data that was read (including if fewer than "n" bytes were
    // successfully read).  May set "*result" to point at data in
    // "scratch[0..n-1]", so "scratch[0..n-1]" must be live when
    // "*result" is used.  If an error was encountered, returns a non-OK
    // status.
    //
    // Safe for concurrent use by multiple threads.
    int Read(uint64_t offset, size_t n, Slice* result,
        char* scratch);
    int ReadSU(uint64_t offset, size_t n, int* length,
        char* scratch);

  private:
    std::string filename_;
    int fd_;
};

class zcWritableFile  {
  private:
    std::string filename_;
    FILE* file_;

  public:
    zcWritableFile(const std::string& fname, FILE* f)
      : filename_(fname), file_(f) { }

    ~zcWritableFile() {
      if (file_ != NULL) {
        // Ignoring any potential errors
        fclose(file_);
      }
    }

    int Append(const Slice& data);

    int Close();

    int Flush();

    virtual int Sync();
};


int zcNewRandomAccessFile(const std::string& fname,
    zcRandomAccessFile** result);
int zcNewWritableFile(const std::string& fname,
    zcWritableFile** result);
int zcDeleteFile(const std::string& fname);

#endif
