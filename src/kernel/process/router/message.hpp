#ifndef fhatos_kernel__message_hpp
#define fhatos_kernel__message_hpp

#include <fhatos.hpp>
//
#include <kernel/furi.hpp>

#define RETAIN_MESSAGE true
#define TRANSIENT_MESSAGE false

namespace fhatos::kernel {

//////////////////////////////////////////////
/////////////// PAYLOAD STRUCT ///////////////
//////////////////////////////////////////////
enum MType { OBJ, BOOL, INT, REAL, STR, LST, REC };
struct Payload {
  const MType type;
  const byte *data;
  const uint length;
  //////////////////
  const bool toBool() const {
    switch (type) {
    case BOOL:
      return atoi((const char *)this->data) == 1;
    case INT:
      return this->toInt() > 0;
    case STR:
      return this->toString().equals("true") || this->toString().equals("1");
    default:
      throw fError("error");
    }
  }
  const int toInt() const {
    switch (type) {
    case BOOL:
      return this->toBool() ? 1 : 0;
    case INT:
      return atoi((const char *)this->data);
    case STR:
      return this->toString().toInt();
    default:
      throw fError("error");
    }
  }
  const String toString() const {
    switch (type) {
    case BOOL:
      return String(this->toBool() ? "true" : "false");
    case INT:
      return String(this->toInt());
    case STR:
      return String(this->data, this->length);
    default:
      throw fError("Unknown type: %i", this->type);
    }
  }

  static const Payload fromBool(const bool xbool) {
    char temp[2];
    itoa(xbool ? 1 : 0, temp, 2);
    return {.type = BOOL, .data = (const byte *)temp, .length = strlen(temp)};
  }

  static const Payload fromInt(const int xint) {
    char temp[10];
    itoa(xint, temp, 10);
    return {.type = INT, .data = (const byte *)temp, .length = strlen(temp)};
  }

  static const Payload fromString(const String xstring) {
    return {.type = STR,
            .data = (const byte *)strdup(xstring.c_str()),
            .length = xstring.length()};
  }
};

//////////////////////////////////////////////
/////////////// MESSAGE STRUCT ///////////////
//////////////////////////////////////////////
struct Message {
public:
  const ID source;
  const ID target;
  const Payload payload;
  const bool retain;

  /*Message(ID source, ID target, PAYLOAD payload, const bool retain = false)
      : source(std::move(source)), target(std::move(target)),
        payload(std::move(payload)), retain(retain) {};*/

  const String toString() const {
    char temp[100];
    sprintf(temp, "[%s]=%s[retain:%s]=>[%s]", source.toString().c_str(),
            payload.toString().c_str(), FP_BOOL_STR(retain),
            target.toString().c_str());
    return temp;
  };
};
} // namespace fhatos::kernel

#endif