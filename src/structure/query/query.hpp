#ifndef fhatos_query_hpp
#define fhatos_query_hpp

#include <fhatos.hpp>
#include <structure/furi.hpp>

namespace fhatos {
  class Query {
  protected:
    const fURI *furi;

  public:
    explicit Query(const fURI *furi) : furi(furi) {
    };
  };
}

#endif
