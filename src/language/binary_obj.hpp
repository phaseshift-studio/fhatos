#ifndef fhatos_serializer_hpp
#define fhatos_serializer_hpp

#include <fhatos.hpp>
//
#include <language/obj.hpp>

namespace fhatos {
  using binary_obj = Triple<const OType, const byte *, const uint16_t>;
  class CharSerializer;
  class PtrSerializer;

  template<typename SERIALIZER = CharSerializer>
  class BinaryObj : public Obj {
  protected:
    const binary_obj _value;

  public:
    ~BinaryObj() {
      const OType t = std::get<0>(this->_value);
      if (t == INT || t == REAL || t == STR)
        delete std::get<1>(this->_value);
    }

    explicit BinaryObj(const binary_obj bobj) : Obj(std::get<0>(bobj)),
      _value{std::get<0>(bobj), std::get<1>(bobj), std::get<2>(bobj)} {
    };

    explicit BinaryObj(const OType type, const byte *data, const uint16_t length) : Obj(type),
      _value{type, data, length} {
    };

    explicit BinaryObj(const fURI xfuri) : Obj(URI), _value(SERIALIZER::fromUri(xfuri)) {
    }


    explicit BinaryObj(const bool xbool) : Obj(BOOL), _value(SERIALIZER::fromBoolean(xbool)) {
    }


    explicit BinaryObj(const int xint) : Obj(INT), _value(SERIALIZER::fromInteger(xint)) {
    }


    explicit BinaryObj(const float xfloat) : Obj(REAL), _value(SERIALIZER::fromFloat(xfloat)) {
    }

    explicit BinaryObj(const string &xstring) : Obj(STR), _value(SERIALIZER::fromString(xstring)) {
    }

    explicit BinaryObj(const char *xcstr) : Obj(STR), _value(SERIALIZER::fromString(string(xcstr))) {
    }


    static BinaryObj *fromObj(const Obj *obj) {
      switch (obj->type()) {
        case URI: return new BinaryObj<>((binary_obj)SERIALIZER::fromUri(((Uri *) obj)->value()));
        case BOOL: return new BinaryObj<>((binary_obj)SERIALIZER::fromBoolean(((Bool *) obj)->value()));
        case INT: return new BinaryObj<>((binary_obj)SERIALIZER::fromInteger(((Int *) obj)->value()));
        case REAL: return new BinaryObj<>((binary_obj)SERIALIZER::fromFloat(((Real *) obj)->value()));
        case STR: return new BinaryObj<>((binary_obj)SERIALIZER::fromString(((Str *) obj)->value()));
        default: throw std::runtime_error("error on type");
      }
    }

    virtual const binary_obj value() const {
      return _value;
    }

    const byte *data() const {
      return std::get<1>(this->_value);
    }

    uint16_t length() const {
      return std::get<2>(this->_value);
    }

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

    /////////////////
    bool equals(const BinaryObj &other) const {
      return (this->type() == other.type()) &&
             (this->length() == other.length()) &&
             (strcmp(reinterpret_cast<const char *>(this->data()),
                     reinterpret_cast<const char *>(other.data())) == 0);
    }


    virtual const string toString() const {
      switch (this->type()) {
        case BOOL: return toBool().toString();
        case INT: return toInt().toString();
        case REAL: return toReal().toString();
        case STR: return toStr().toString();
        default: throw std::runtime_error("error on type");
      }
    }

    static BinaryObj interpret(const string &line) {
      if (line[0] == '\"' && line[line.length() - 1] == '\"')
        return BinaryObj(line.substr(1, line.length() - 2)); // might be wrong indices
      else if (strcmp("true", line.c_str()) == 0 || strcmp("false", line.c_str()) == 0)
        return BinaryObj((bool) (strcmp("true", line.c_str()) == 0));
      else if (line[line.length() - 1] == 'f') {
        return BinaryObj(stof(line.substr(0, line.length() - 1)));
      } else {
        return BinaryObj(stoi(line));
      }
    }
  };

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  class Serializer {
  public:
    static binary_obj fromUri(const fURI &xfuri) {
      return {OBJ, nullptr, 0};
    }

    static binary_obj fromBoolean(const bool xbool) {
      return {OBJ, nullptr, 0};
    }

    static binary_obj fromInt(const int xint) {
      return {OBJ, nullptr, 0};
    }

    static binary_obj fromFloat(const float xreal) {
      return {OBJ, nullptr, 0};
    }

    static binary_obj fromString(const string &xstr) {
      return {OBJ, nullptr, 0};
    }

    template<typename SERIALIZER>
    static Uri toUri(const BinaryObj<SERIALIZER> &xserial) {
      return Uri(fURI("/"));
    }

    template<typename SERIALIZER>
    static Bool toBool(const BinaryObj<SERIALIZER> &xserial) {
      return Bool(false);
    }

    template<typename SERIALIZER>
    static Int toInt(const BinaryObj<SERIALIZER> &xserial) {
      return Int(0);
    }

    template<typename SERIALIZER>
    static Real toReal(const BinaryObj<SERIALIZER> &xserial) {
      return Real(0.0f);
    }

    template<typename SERIALIZER>
    static Str toStr(const BinaryObj<SERIALIZER> &xserial) {
      return Str("");
    }
  };

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  /* class PtrSerializer : public Serializer {
   public:
     static binary_obj fromBoolean(const bool xbool) {
       return {
         BOOL,
         new byte(xbool ? '1' : '0'),
         1
       };
     }

     static binary_obj fromInteger(const int xint) {
       return {
         INT,
         (byte *) new int(xint),
         4
       };
     }

     static binary_obj fromFloat(const float xfloat) {
       return {
         REAL,
         (byte *) new string(std::to_string(xfloat)),
         4
       };
     }

     static binary_obj fromString(const string &xstring) {
       return {
         STR,
         (byte *) new string(xstring),
         (uint16_t) (xstring.length())
       };
     }

     static Bool toBool(const BinaryObj<PtrSerializer> &bobj) {
       switch (bobj.type()) {
         case BOOL:
           return Bool(bobj.data()[0] == '1');
         case INT:
           return Bool(bobj.toInt().value() > 0);
         case REAL:
           return Bool(bobj.toReal().value() > 0.0f);
         case STR:
           return Bool(bobj.toStr().value().compare("true") == 0 || bobj.toStr().value().compare("1") == 0);
         default:
           throw std::runtime_error("bad1"); // throw fError("Unknown type: %s", OTYPE_STR.at(xserial.type).c_str());
       }
     }

     static Int toInt(const BinaryObj<PtrSerializer> &bobj) {
       switch (bobj.type()) {
         case BOOL:
           return Int(bobj.toBool().value() ? 1 : 0);
         case INT:
           return Int(std::stoi(std::string((char*)bobj.data())));
         case REAL:
           return Int(static_cast<int>(bobj.toReal().value()));
         case STR:
           return Int(std::stoi(std::string((char*)bobj.data())));
         default:
           throw std::runtime_error("bad2"); //fError("Unknown type: %s", OTYPE_STR.at(xserial.type).c_str());
       }
     }

     static Real toReal(const BinaryObj<PtrSerializer> &bobj) {
       switch (bobj.type()) {
         case BOOL:
           return Real(bobj.toBool().value() ? 1.0f : 0.0f);
         case INT:
           return Real((float) bobj.toInt().value());
         case REAL:
           return Real(std::stof(std::string((char*)bobj.data())));
         case STR:
           return Real(std::stof(std::string((char*)bobj.data())));
         default:
           throw std::runtime_error("bad3"); //throw fError("Unknown type: %s", OTYPE_STR.at(xserial.type).c_str());
       }
     }

     static Str toStr(const BinaryObj<PtrSerializer> &bobj) {
       switch (bobj.type()) {
         case BOOL: {
           return Str(bobj.toBool().value() ? "true" : "false");
         }
         case INT: {
           char temp[15];
           const uint size = sprintf(temp, "%i", bobj.toInt().value());
           temp[size] = '\0';
           return Str(temp);
         }
         case REAL: {
           char temp[15];
           const uint size = sprintf(temp, "%4f", bobj.toReal().value());
           temp[size] = '\0';
           return Str(string(temp, size));
         }
         case STR: {
           return Str(((string *) (bobj.data()))->c_str());
         }
         default:
           throw std::runtime_error("bad4"); // throw fError("Unknown type: %s", OTYPE_STR.at(xserial.type).c_str());
       }
     }
   };*/

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  class CharSerializer : public Serializer {
  public:
    static binary_obj fromUri(const fURI &xfuri) {
      const uint16_t len = xfuri.toString().length();
      char *temp = (char *) malloc(len + 1);
      memccpy(temp, xfuri.toString().c_str(), 0, len);
      temp[len] = '\0';
      return {
        URI,
        (byte *) temp,
        len
      };
    }


    static binary_obj fromBoolean(const bool xbool) {
      return {
        BOOL,
        reinterpret_cast<const byte *>(xbool ? "T" : "F"),
        1
      };
    }

    static binary_obj fromInteger(const int xint) {
      const string str = std::to_string(xint);
      const uint16_t len = str.length();
      char *temp = (char *) malloc(len + 1);
      memccpy(temp, str.c_str(), 0, len);
      temp[len] = '\0';
      return {
        INT,
        (byte *) temp,
        len
      };
    }

    static binary_obj fromFloat(const float xfloat) {
      const string str = std::to_string(xfloat);
      const uint16_t len = str.length();
      char *temp = (char *) malloc(len + 1);
      memccpy(temp, str.c_str(), 0, len);
      temp[len] = '\0';
      return {
        REAL,
        (byte *) temp,
        len
      };
    }

    static binary_obj fromString(const string &xstring) {
      const uint16_t len = xstring.length();
      char *temp = (char *) malloc(len + 1);
      memccpy(temp, xstring.c_str(), 0, len);
      temp[len] = '\0';
      return {
        STR,
        (byte *) temp,
        len
      };
    }

    static Uri toUri(const BinaryObj<CharSerializer> &bobj) {
      switch (bobj.type()) {
        case URI:
          return Uri(fURI("/" + bobj.toString()));;
        case BOOL:
          return Uri(fURI("/" + bobj.toString()));
        case INT:
          return Uri(fURI("/" + bobj.toString()));
        case REAL:
          return Uri(fURI("/" + bobj.toString()));
        case STR:
          return Uri(fURI("/" + bobj.toString()));
        default:
          throw std::runtime_error("bad1"); // throw fError("Unknown type: %s", OTYPE_STR.at(xserial.type).c_str());
      }
    }

    static Bool toBool(const BinaryObj<CharSerializer> &bobj) {
      switch (bobj.type()) {
        case BOOL:
          return Bool(strcmp((char *) bobj.data(), "T") == 0);
        case INT:
          return Bool(bobj.toInt().value() > 0);
        case REAL:
          return Bool(bobj.toReal().value() > 0.0f);
        case STR:
          return Bool(bobj.toStr().value().compare("true") == 0 || bobj.toStr().value().compare("1") == 0);
        default:
          throw std::runtime_error("bad1"); // throw fError("Unknown type: %s", OTYPE_STR.at(xserial.type).c_str());
      }
    }

    static Int toInt(const BinaryObj<CharSerializer> &bobj) {
      switch (bobj.type()) {
        case BOOL:
          return Int(bobj.toBool().value() ? 1 : 0);
        case INT:
          return Int(std::stoi(string((char *) bobj.data(), bobj.length())));
        case REAL:
          return Int(static_cast<int>(bobj.toReal().value()));
        case STR:
          return Int(std::stoi(string((char *) bobj.data(), bobj.length())));
        default:
          throw std::runtime_error("bad2"); //fError("Unknown type: %s", OTYPE_STR.at(xserial.type).c_str());
      }
    }

    static Real toReal(const BinaryObj<CharSerializer> &bobj) {
      switch (bobj.type()) {
        case BOOL:
          return Real(bobj.toBool().value() ? 1.0f : 0.0f);
        case INT:
          return Real((float) (bobj.toInt().value()));
        case REAL:
          return Real(std::stof(string((char *) bobj.data(), bobj.length())));
        case STR:
          return Real(std::stof(string((char *) bobj.data(), bobj.length())));
        default:
          throw std::runtime_error("bad3"); //throw fError("Unknown type: %s", OTYPE_STR.at(xserial.type).c_str());
      }
    }

    static Str toStr(const BinaryObj<CharSerializer> &bobj) {
      switch (bobj.type()) {
        case BOOL:
          return Str(bobj.toBool().value() ? "true" : "false");
        case INT: {
          char temp[15];
          const uint size = sprintf(temp, "%i", bobj.toInt().value());
          temp[size] = '\0';
          return Str(string(temp, size));
        }
        case REAL: {
          char temp[15];
          const uint size = sprintf(temp, "%f", bobj.toReal().value());
          temp[size] = '\0';
          return Str(string(temp, size));
        }
        case STR: {
          return Str(string((char *) (bobj.data()), bobj.length()));
        }
        default:
          throw std::runtime_error("bad4"); // throw fError("Unknown type: %s", OTYPE_STR.at(xserial.type).c_str());
      }
    }
  };
}

#endif
