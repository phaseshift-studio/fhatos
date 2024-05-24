#ifndef fhatos_message_hpp
#define fhatos_message_hpp

#include <fhatos.hpp>
//
#include <structure/furi.hpp>
#include <language/obj.hpp>
#include <language/binary_obj.hpp>

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
    const BinaryObj<> *payload;
    const bool retain;

    //~Message() {
      //if (this->payload)
       // delete this->payload;
   // }

    template<OType type>
    bool is() const {
      return type == this->payload->type();
    }

    bool isQuery(const char *query = "?") const {
      return !this->retain &&
             0 == payload->length() &&
             target.query() == query &&
             !target.equals(source);
    }

    bool isReflexive() const {
      return target.query("").equals(source.query(""));
    }

    string toString() const {
      char temp[100];
      sprintf(temp, "[%s]=%s[retain:%s]=>[%s]", source.toString().c_str(),
              payload->toString().c_str(), FOS_BOOL_STR(retain),
              target.toString().c_str());
      return string(temp);
    };
  };
} // namespace fhatos

#endif
