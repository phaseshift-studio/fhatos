/*******************************************************************************
FhatOS: A Distributed Operating System
  Copyright (c) 2024 PhaseShift Studio, LLC

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#ifdef NATIVE
#include "../inet.hpp"
#include <arpa/inet.h>
#include <avahi-client/client.h>
#include <avahi-client/publish.h>
#include <avahi-common/error.h>
#include <avahi-common/malloc.h>
#include <avahi-common/thread-watch.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

namespace fhatos {
  ID ip_addr(const ID &name) {
    ifreq ifr{};
    const int fd = socket(AF_INET, SOCK_DGRAM, 0);
    ifr.ifr_addr.sa_family = AF_INET;
    memcpy(ifr.ifr_name, name.toString().c_str(), IFNAMSIZ - 1);
    ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);
    return ID(string(inet_ntoa(reinterpret_cast<sockaddr_in *>(&ifr.ifr_addr)->sin_addr)).c_str());
  }

  ID Inet::mdns() {
    int error;
    static AvahiThreadedPoll *poll = avahi_threaded_poll_new();
    AvahiClient *client =
        avahi_client_new(avahi_threaded_poll_get(poll), AVAHI_CLIENT_NO_FAIL, nullptr, nullptr, &error);
    if(!client)
      throw fError("!rfailed !yto locate mdns!!: [error code: %d]", error);
    ID name = avahi_client_get_host_name(client);
    //avahi_threaded_poll_quit(poll);
    //avahi_threaded_poll_free(poll);
    avahi_client_free(client);
    return name;
  }

  Rec_p Inet::ip_addresses() {
    ifaddrs *addrs;
    getifaddrs(&addrs);
    const Rec_p ifaces = Obj::to_rec();
    for(const ifaddrs *addr = addrs; addr != nullptr; addr = addr->ifa_next) {
      if(addr->ifa_addr && addr->ifa_addr->sa_family == AF_PACKET) {
        ifaces->rec_set(vri(addr->ifa_name), vri(ip_addr(addr->ifa_name)));
      }
    }
    freeifaddrs(addrs);
    return ifaces;
  }
} // namespace fhatos
#endif
