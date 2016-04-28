#include "file_system.h"
#include "slice.h"

int RandomAccessFile::Read(uint64_t offset, size_t n, Slice* result,
    char* scratch) {
  ssize_t r = pread(fd_, scratch, n, static_cast<off_t>(offset));
  *result = Slice(scratch, (r < 0) ? 0 : r);
  if (r < 0) {
    // An error: return a non-ok status
    return -1;
  }
  return 0;
}

int WritableFile::Append(const Slice& data) {
  size_t r = fwrite_unlocked(data.data(), 1, data.size(), file_);
  if (r != data.size()) {
    return -1;
  }
  return 0;
}

int WritableFile::Close() {
  int result;
  if (fclose(file_) != 0) {
    result = -1;
  }
  file_ = NULL;
  return result;
}

int WritableFile::Flush() {
  if (fflush_unlocked(file_) != 0) {
    return -1;
  }
  return 0;
}

int WritableFile::Sync() {
  // Ensure new files referred to by the manifest are in the filesystem.
  int s = 0;
  if (fflush_unlocked(file_) != 0 ||
      fdatasync(fileno(file_)) != 0) {
    s = -1;
  }
  return s;
}

int NewRandomAccessFile(const std::string& fname,
    RandomAccessFile** result) {
  *result = NULL;
  int fd = open(fname.c_str(), O_RDONLY);
  if (fd < 0) {
    return -1;
  } else {
    *result = new RandomAccessFile(fname, fd);
  }
  return 0;
}


int NewWritableFile(const std::string& fname,
    WritableFile** result) {
  int s;
  FILE* f = fopen(fname.c_str(), "w");
  if (f == NULL) {
    *result = NULL;
    s = -1;
  } else {
    *result = new WritableFile(fname, f);
  }
  return s;
}
int DeleteFile(const std::string& fname) {
  int result;
  if (unlink(fname.c_str()) != 0) {
    result = -1;
  }
  return result;
}
