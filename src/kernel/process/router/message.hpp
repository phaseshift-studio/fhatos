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
enum MType { OBJ = 0, BOOL = 1, INT = 2, REAL = 3, STR = 4, LST = 5, REC = 6 };
const Map<MType, String> MTYPE_NAMES = {{{OBJ, F("obj")},
                                         {BOOL, F("bool")},
                                         {INT, F("int")},
                                         {REAL, F("real")},
                                         {STR, F("str")},
                                         {LST, F("lst")},
                                         {REC, F("rec")}}};
struct Payload {
  const MType type;
  const byte * data;
  const uint length;
  //////////////////
  const bool equals(const Payload &other) const {
    return (this->type == other.type) &&
           (strcmp((char *)this->data, (char *)other.data) == 0) &&
           (this->length == other.length);
  }
  //////////////////
  const bool toBool() const {
    switch (type) {
    case BOOL:
      return strcmp((char *)this->data, "T") == 0;
    case INT:
      return this->toInt() > 0;
    case REAL:
      return this->toFloat() > 0.0f;
    case STR:
      return this->toString().equals("true") || this->toString().equals("1");
    default:
      throw fError("Unknown type: %s", MTYPE_NAMES.at(this->type).c_str());
    }
  }
  const int toInt() const {
    switch (type) {
    case BOOL:
      return this->toBool() ? 1 : 0;
    case INT:
      return (int)atoi((const char *)this->data);
    case REAL:
      return (int)this->toFloat();
    case STR:
      return this->toString().toInt();
    default:
      throw fError("Unknown type: %s", MTYPE_NAMES.at(this->type).c_str());
    }
  }

  const float toFloat() const {
    switch (type) {
    case BOOL:
      return this->toBool() ? 1.0f : 0.0f;
    case INT:
      return (float)this->toInt();
    case REAL:
      return (float)atof((const char *)this->data);
      //(float)String((const char *)this->data).toFloat();
    case STR:
      return this->toString().toFloat();
    default:
      throw fError("Unknown type: %s", MTYPE_NAMES.at(this->type).c_str());
    }
  }

  const String toString() const {
    switch (type) {
    case BOOL:
      return String(this->toBool() ? "true" : "false");
    case INT:
      return String(this->toInt());
    case REAL: {
      char temp[15];
      uint size = sprintf(temp, "%f", this->toFloat());
      temp[size] = '\0';
      return String(temp);
    }
    case STR: {
      return String((char *)this->data);
    }
    default:
      throw fError("Unknown type: %s", MTYPE_NAMES.at(this->type).c_str());
    }
  }

  static const Payload fromBool(const bool xbool) {
    return {.type = BOOL,
            .data = (byte *)(xbool ? "T" : "F"),
            .length = 1};
  }

  static const Payload fromInt(const int xint) {
    char temp[15];
    uint size = sprintf(temp, "%i", xint);
    temp[size] = '\0';
    return {.type = INT,
            .data = (byte *)temp,
            .length = size};
  }

  static const Payload fromFloat(const float xfloat) {
    char temp[15];
    uint size = sprintf(temp, "%.4f", xfloat);
    temp[size] = '\0';
    return {.type = REAL,
            .data = (byte *)temp,
            .length = size};
  }

  static const Payload fromString(const String &xstring) {
    char *temp = strdup(xstring.c_str());
    temp[xstring.length()] = '\0';
    return {.type = STR,
            .data = (byte *)temp,
            .length = xstring.length()};
  }

  static const Payload interpret(const String &line) {
    if (line.startsWith("\"") && line.endsWith("\""))
      return Payload::fromString(line.substring(1, line.length() - 1));
    else if (line.equals("true") || line.equals("false"))
      return Payload::fromBool(line.equals("true"));
    else if (line.endsWith("f")) {
      char *c = strdup(line.substring(0, line.length() - 1).c_str());
      c[line.length() - 1] = '\0';
      const Payload p = Payload::fromFloat((float)atof(c));
      delete c;
      return p;
    } else {
      char *c = strdup(line.c_str());
      c[line.length()] = '\0';
      const Payload p = Payload::fromInt((int)atoi(c));
      delete c;
      return p;
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

  template <MType type> const bool is() const {
    return type == this->payload.type;
  }

  const String toString() const {
    char temp[100];
    sprintf(temp, "[%s]=%s[retain:%s]=>[%s]", source.toString().c_str(),
            payload.toString().c_str(), FP_BOOL_STR(retain),
            target.toString().c_str());
    const String s(temp);
    delete &temp;
    return s;
  };
};
} // namespace fhatos::kernel

#endif