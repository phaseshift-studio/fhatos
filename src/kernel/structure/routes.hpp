#ifndef fhatos__routes_hpp
#define fhatos__routes_hpp

#include <fhatos.hpp>
#include <kernel/structure/machine/device/io/net/wifi/wifi.hpp>
#include <kernel/structure/structure.hpp>

namespace fhatos::kernel {

class Routes {

public:
  Routes(const fURI &id) : __self(id) {}
  String toString() {
    String c;
    for (const auto &child : children) {
      c = c + "\n" + child.resolve(__self).toString();
    }
    //c.trim();
    return c;
  }

  fURI __self;
  List<fURI> children;
};

} // namespace fhatos::kernel

#endif