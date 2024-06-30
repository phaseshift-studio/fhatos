#ifndef fhatos_options_hpp
#define fhatos_options_hpp
#include <memory>

namespace fhatos {
  class Options final : public std::enable_shared_from_this<Options> {
  public:
    uint8_t LOGGING;
    void *ROUTING;
    Ansi<> *PRINTING;
    std::function<void *(void *)> TYPE_FUNCTION;

    explicit Options() {}

    template<typename ROUTER>
    ROUTER *router() {
      if (nullptr == ROUTING)
        throw fError("No router secified in global options\n");
      return (ROUTER *) this->ROUTING;
    }

    /*template<typename ROUTER>
    std::shared_ptr<Options> router(ROUTER router) {
      this->ROUTING = router;
      return this->shared_from_this();
    }*/

    template<typename LOGGER>
    LOGGER logger() {
      return (LOGGER) this->LOGGING;
    }

    /*template<typename LOGGER>
    std::shared_ptr<Options> logger(LOGGER logger) {
      this->LOGGING = logger;
      return this->shared_from_this();
    }*/

    template<typename PRINTER = Ansi<>>
    PRINTER *printer() {
      if (nullptr == PRINTING)
        throw fError("No printer secified in global options\n");
      return (PRINTER *) this->PRINTING;
    }

    /*template<typename PRINTER>
    std::shared_ptr<Options> printer(PRINTER printer) {
      this->PRINTING = printer;
      return this->shared_from_this();
    }*/
  };

  static std::shared_ptr<Options> GLOBAL_OPTIONS = std::shared_ptr<Options>(new Options());

} // namespace fhatos
#endif
