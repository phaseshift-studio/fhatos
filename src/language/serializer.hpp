#ifndef fhatos_serializer_hpp
#define fhatos_serializer_hpp

#include <fhatos.hpp>
//
#include <language/obj.hpp>

namespace fhatos {
  class CharSerializer;

  template<typename SERIALIZER = CharSerializer>
  struct SerialObj {
    const OType type;
    const byte *data;
    const uint16_t length;
    //////////////////
    bool equals(const SerialObj &other) const {
      return (this->type == other.type) &&
             (strcmp(reinterpret_cast<const char *>(this->data), reinterpret_cast<const char *>(other.data)) == 0) &&
             (this->length == other.length);
    }


    string toString() const {
    /*  if (type == REC)
        return SERIALIZER::toRec(*this).toString();
      else*/
        return this->toStr().value();
    }

    static SerialObj interpret(const string &line) {
      if (line[0] == '\"' && line[line.length() - 1] == '\"')
        return SerialObj::fromString(line.substr(1, line.length() - 2)); // might be wrong indices
      else if (line == "true" || line == "false")
        return SerialObj::fromBoolean(line == "true");
      else if (line[line.length() - 1] == 'f') {
        return SerialObj::fromFloat(stof(line.substr(0, line.length() - 1)));
      } else {
        return SerialObj::fromInteger(stoi(line));
      }
    }

    static SerialObj fromBoolean(const bool xboolean) {
      return SERIALIZER::fromBool(Bool(xboolean));
    }

    static SerialObj fromInteger(const int xinteger) {
      return SERIALIZER::fromInt(Int(xinteger));
    }

    static SerialObj fromFloat(const float xfloat) {
      return SERIALIZER::fromReal(Real(xfloat));
    }

    static SerialObj fromString(const string &xstring) {
      return SERIALIZER::fromStr(Str(xstring));
    }

    static SerialObj fromString(const char * &xstring) {
      return SerialObj::fromString(string(xstring, strlen(xstring)));
    }

    /*static SerialObj fromMap(const Map<ObjZ, ObjZ> &xmap) {
      return SERIALIZER::fromRec(Rec(xmap));
    }*/

    Bool toBool() const {
      return SERIALIZER::toBool(*this);
    }

    Int toInt() const {
      return SERIALIZER::toInt(*this);
    }

    Real toReal() const {
      return SERIALIZER::toReal(*this);
    }

    Str toStr() const {
      return SERIALIZER::toStr(*this);
    }
  };

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  class Serializer {
  public:
    template<typename SERIALIZER>
    static SerialObj<SERIALIZER> fromBool(const Bool &xbool) {
      return {.type = OBJ, .data = nullptr, .length = 0};
    }

    template<typename SERIALIZER>
    static SerialObj<SERIALIZER> fromInt(const Int &xint) {
      return {.type = OBJ, .data = nullptr, .length = 0};
    }

    template<typename SERIALIZER>
    static SerialObj<SERIALIZER> fromReal(const Real &xreal) {
      return {.type = OBJ, .data = nullptr, .length = 0};
    }

    template<typename SERIALIZER>
    static SerialObj<SERIALIZER> fromStr(const Str &xstr) {
      return {.type = OBJ, .data = nullptr, .length = 0};
    }

    template<typename SERIALIZER>
    static Bool toBool(const SerialObj<SERIALIZER> &xserial) {
      return Bool(false);
    }

    template<typename SERIALIZER>
    static Int toInt(const SerialObj<SERIALIZER> &xserial) {
      return Int(0);
    }

    template<typename SERIALIZER>
    static Real toReal(const SerialObj<SERIALIZER> &xserial) {
      return Real(0.0f);
    }

    template<typename SERIALIZER>
    static Str toStr(const SerialObj<SERIALIZER> &xserial) {
      return Str("");
    }
  };

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  class CharSerializer : public Serializer {
  public:
    static ObjZ toObj(const SerialObj<CharSerializer> &xobj) {
      switch (xobj.type) {
        case BOOL: return (ObjZ) new Bool(CharSerializer::toBool(xobj));
        case INT: return (ObjZ) new Int(CharSerializer::toInt(xobj));
        case REAL: return (ObjZ) new Real(CharSerializer::toReal(xobj));
        case STR: return (ObjZ) new Str(CharSerializer::toStr(xobj));
       // case REC: return (ObjZ) new Rec(CharSerializer::toRec(xobj));
        default: return (ObjZ) (ObjY) nullptr;
      }
    }

    static SerialObj<> fromBool(const Bool &xbool) {
      return {
        .type = BOOL,
        .data = reinterpret_cast<const byte *>(xbool.value() ? "T" : "F"),
        .length = 1
      };
    }

    static SerialObj<CharSerializer> fromInt(const Int &xint) {
      char temp[15];
      const uint16_t size = sprintf(temp, "%i", xint.value());
      temp[size] = '\0';
      return {
        .type = INT,
        .data = reinterpret_cast<const byte *>(temp),
        .length = size
      };
    }

    static SerialObj<CharSerializer> fromReal(const Real &xreal) {
      char temp[15];
      const uint16_t size = sprintf(temp, "%f", xreal.value());
      temp[size] = '\0';
      return {
        .type = REAL,
        .data = reinterpret_cast<const byte *>(temp),
        .length = size
      };
    }

    static SerialObj<CharSerializer> fromStr(const Str &xstr) {
      char *temp = strdup(xstr.value().c_str());
      temp[xstr.value().length()] = '\0';
      return {
        .type = STR,
        .data = (byte *) temp,
        .length = (uint16_t) (xstr.value().length())
      };
    }

    /*static SerialObj<CharSerializer> fromRec(const Rec &xrec) {
      return {
        .type = REC,
        .data = (byte *) &xrec.value(),
        .length = (uint16_t) sizeof(xrec)
      };
    }*/

    static Bool toBool(const SerialObj<CharSerializer> &xserial) {
      switch (xserial.type) {
        case BOOL:
          return Bool(strcmp((char *) xserial.data, "T") == 0);
        case INT:
          return Bool(xserial.toInt().value() > 0);
        case REAL:
          return Bool(xserial.toReal().value() > 0.0f);
        case STR:
          return Bool(xserial.toStr().value().compare("true") == 0 || xserial.toStr().value().compare("1") == 0);
        default:
          throw std::runtime_error("bad1"); // throw fError("Unknown type: %s", OTYPE_STR.at(xserial.type).c_str());
      }
    }

    static Int toInt(const SerialObj<CharSerializer> &xserial) {
      switch (xserial.type) {
        case BOOL:
          return Int(xserial.toBool().value() ? 1 : 0);
        case INT:
          return Int(atoi((const char *) xserial.data));
        case REAL:
          return Int(static_cast<int>(xserial.toReal().value()));
        case STR:
          return Int(atoi(xserial.toStr().value().c_str()));
        default:
          throw std::runtime_error("bad2"); //fError("Unknown type: %s", OTYPE_STR.at(xserial.type).c_str());
      }
    }

    static Real toReal(const SerialObj<CharSerializer> &xserial) {
      switch (xserial.type) {
        case BOOL:
          return Real(xserial.toBool().value() ? 1.0f : 0.0f);
        case INT:
          return Real(static_cast<float>(xserial.toInt().value()));
        case REAL:
          return Real(static_cast<float>(atof((const char *) xserial.data)));
        case STR:
          return Real(static_cast<float>(atof(xserial.toStr().value().c_str())));
        default:
          throw std::runtime_error("bad3"); //throw fError("Unknown type: %s", OTYPE_STR.at(xserial.type).c_str());
      }
    }

    static Str toStr(const SerialObj<CharSerializer> &xserial) {
      switch (xserial.type) {
        case BOOL: {
          return Str(xserial.toBool().value() ? "true" : "false");
        }
        case INT: {
          char temp[15];
          const uint size = sprintf(temp, "%i", xserial.toInt().value());
          temp[size] = '\0';
          return Str(temp);
        }
        case REAL: {
          char temp[15];
          const uint size = sprintf(temp, "%4f", xserial.toReal().value());
          temp[size] = '\0';
          return Str(string(temp, size));
        }
        case STR: {
          return Str(string(reinterpret_cast<const char *>(xserial.data), xserial.length));
        }
        default:
          throw std::runtime_error("bad4"); // throw fError("Unknown type: %s", OTYPE_STR.at(xserial.type).c_str());
      }
    }


   /* static Rec toRec(const SerialObj<CharSerializer> &xserial) {
      Map<const ObjZ, ObjZ> *map = (Map<const ObjZ, ObjZ> *) xserial.data;
      return Rec(*map);
    }*/
  };
}

#endif
