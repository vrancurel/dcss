
#include "ntl.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <streambuf>

template class FECFNTRS<uint32_t>;

int vflag = 1;

struct membuf: std::streambuf {
  membuf(char const* base, size_t size) {
    char* p(const_cast<char*>(base));
    this->setg(p, p, p + size);
  }
};

struct imemstream: virtual membuf, std::istream {
  imemstream(char const* base, size_t size)
    : membuf(base, size)
    , std::istream(static_cast<std::streambuf*>(this)) {
  }
};

/** 
 * This function:
 * 1) compute the ideal mmap size
 * 2) mmaps the file
 * 3) create proper input and output streams
 * 
 * @param filename 
 * @param fec 
 */
int create_coding_files(const char *filename, FEC<uint32_t> *fec)
{
  void *addr;
  char *tmp_addr;
  int fd;
  struct stat sb;
  size_t length, data_size;
  off_t offset;
  char cfilename[1024];
  std::vector<std::istream*> d_files(fec->n_data, nullptr);
  std::vector<std::ostream*> c_files(fec->n_outputs, nullptr);
  std::vector<std::ostream*> c_props_files(fec->n_outputs, nullptr);
  std::vector<KeyValue*> c_props(fec->n_outputs, nullptr);

  fd = open(filename, O_RDONLY);
  if (fd == -1) {
    std::cerr << "opening " << filename << " failed\n";
    return -1;
  }
  
  if (fstat(fd, &sb) == -1) {
    std::cerr << "fstat " << filename << " failed\n";
    return -1;
  }

  length = sb.st_size;
  
  data_size = length / fec->n_data;
  if (length % fec->n_data != 0) {
    // FIXME
    std::cerr << "file size is not multiple of n_data\n";
    return -1;
  }
  std::cerr << "data_size " << data_size << "\n";

  addr = mmap(NULL, length, PROT_READ,
              MAP_PRIVATE, fd, 0);
  if (addr == (void *) -1) {
    std::cerr << "mmap failed\n";
    exit(1);
  }
  std::cerr << "length " << length << " addr " << addr << "\n";

  //do it
  offset = 0;
  for (int i = 0; i < fec->n_data; i++) {
    if (vflag)
      std::cerr << "create: opening data " << i << " offset " << offset << "\n";
    tmp_addr = ((char *) addr) + offset;
    d_files[i] = new imemstream(tmp_addr, data_size);
    offset += data_size;
  }
  
  for (int i = 0; i < fec->n_outputs; i++) {
    snprintf(cfilename, sizeof (cfilename), "%s.c%d", filename, i);
    if (vflag)
      std::cerr<< "create: opening coding for writing " << filename << "\n";
    c_files[i] = new std::ofstream(cfilename);
    snprintf(cfilename, sizeof (cfilename), "%s.c%d.props", filename, i);
    if (vflag)
      std::cerr<< "create: opening coding props for writing " <<
        filename << "\n";
    c_props_files[i] = new std::ofstream(cfilename);
    c_props[i] = new KeyValue();
  }

  fec->encode_bufs(d_files, c_files, c_props);
  
  if (munmap(addr, length) != 0) {
    std::cerr << "munmap " << filename << " failed\n";
    exit(1);
  }
  
  close(fd);
  
  return 0;
}

void do_put(const char *filename, int n_data, int n_parities)
{
  FECFNTRS<uint32_t> *fec;

  fec = new FECFNTRS<uint32_t>(2, n_data, n_parities);
  if (create_coding_files(filename, fec) != 0) {
    return ;
  }
}
