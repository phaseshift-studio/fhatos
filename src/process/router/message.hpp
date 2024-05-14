#ifndef fhatos_message_hpp
#define fhatos_message_hpp

#include <fhatos.hpp>
//
#include <structure/furi.hpp>
#include <language/obj.hpp>
#include <language/serializer.hpp>

#define RETAIN_MESSAGE true
#define TRANSIENT_MESSAGE false

namespace fhatos {
  //////////////////////////////////////////////
  /////////////// MESSAGE STRUCT ///////////////
  //////////////////////////////////////////////
  struct Message {
  public:
    const ID source;
    const ID target;
    const SerialObj<> payload;
    const bool retain;

    template<OType type>
    const bool is() const {
      return type == this->payload.type();
    }

    const String toString() const {
      char temp[100];
      sprintf(temp, "[%s]=%s[retain:%s]=>[%s]", source.toString().c_str(),
              payload.toStr().toString().c_str(), FOS_BOOL_STR(retain),
              target.toString().c_str());
      return String(temp);
    };
  };
} // namespace fhatos

#endif
