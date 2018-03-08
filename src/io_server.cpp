
#include <cstdio>
#include <cstdlib>

#include <nttec/nttec.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include "io_server.h"

namespace io {

  Server::Server() {
    this->daemon = nullptr;
  }

  /**
   * Get the I/O Server port
   *
   * return the port number or 0 on failure
   */
  uint16_t Server::get_port()
  {
    const union MHD_DaemonInfo *dinfo;

    if (this->daemon == nullptr) {
      std::cerr << "start daemon first\n";
      return 0;
    }
        
    dinfo = MHD_get_daemon_info(this->daemon, MHD_DAEMON_INFO_LISTEN_FD);

    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    if (getsockname(dinfo->listen_fd, reinterpret_cast<struct sockaddr *>(&sin), &len) == -1) {
      std::cerr << "getsockname error\n";
      return 0;
    }

    return ntohs(sin.sin_port);
  }
  
  int Server::callback(void *, MHD_Connection *, const char *,
                       const char *, const char *,
                       const char *, size_t *,
                       void **) {
    std::cerr << "callback\n";
    return 0;
  }
    
  void Server::start_daemon() {
    unsigned int mhd_flags = MHD_NO_FLAG;
    uint16_t port = 0;

    this->daemon = MHD_start_daemon(mhd_flags, port, NULL, NULL,
                                    Server::callback, this,
                                    MHD_OPTION_END);
  }
}
