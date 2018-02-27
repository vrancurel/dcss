
#include <cstdio>
#include <cstdlib>

#include <nttec/nttec.h>

#include "io_server.h"

namespace io {

  Server::Server() {
    this->daemon = nullptr;
  }

  int Server::callback(void *cls, MHD_Connection *connection, const char *url,
                        const char *method, const char *version,
                        const char *upload_data, size_t *upload_data_size,
                        void **con_cls) {
    std::cerr << "callback\n";
    return 0;
  }
    
  void Server::start_daemon() {
    unsigned int mhd_flags = MHD_NO_FLAG;
    
    this->daemon = MHD_start_daemon(mhd_flags, 0, NULL, NULL,
                                    Server::callback, this,
                                    MHD_OPTION_END);
  }
}
