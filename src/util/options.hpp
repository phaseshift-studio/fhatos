#ifndef fhatos_options_hpp
#define fhatos_options_hpp
#include <memory>

namespace fhatos {
  class Options final : public std::enable_shared_from_this<Options> {
  public:
    uint8_t LOGGING;
    void *ROUTING;
    Ansi<> *PRINTING;

    explicit Options() {}

    template<typename ROUTER>
    ROUTER *router() {
      if (nullptr == ROUTING)
        throw fError("No router secified in global options\n");
      return (ROUTER *) this->ROUTING;
    }

    template<typename LOGGER>
    LOGGER logger() {
      return (LOGGER) this->LOGGING;
    }

    template<typename PRINTER = Ansi<>>
    PRINTER *printer() {
      if (nullptr == PRINTING)
        throw fError("No printer secified in global options\n");
      return (PRINTER *) this->PRINTING;
    }
  };

  static std::shared_ptr<Options> GLOBAL_OPTIONS = std::shared_ptr<Options>(new Options());

} // namespace fhatos
#endif
