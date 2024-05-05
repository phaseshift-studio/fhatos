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
      return HIGH == data[0];
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
    case INT: {
      int integer = 0;
      integer = (integer << 8) + this->data[3];
      integer = (integer << 8) + this->data[2];
      integer = (integer << 8) + this->data[1];
      integer = (integer << 8) + this->data[0];
      return integer;
    }
    case STR:
      return this->toString().toInt();
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