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
  class BinaryObj : public Obj<binary_obj> {
  public:
    explicit BinaryObj(const OType type, const byte *data, const uint16_t length) : Obj({type, data, length}) {
    };

    explicit BinaryObj(const bool xbool) : Obj(SERIALIZER::fromBoolean(xbool)) {
    }


    explicit BinaryObj(const int xint) : Obj(SERIALIZER::fromInteger(xint)) {
    }


    explicit BinaryObj(const float xfloat) : Obj(SERIALIZER::fromFloat(xfloat)) {
    }

    explicit BinaryObj(const string &xstring) : Obj(SERIALIZER::fromString(xstring)) {
    }


    virtual OType type() const override {
      return std::get<0>(this->_value);
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
    virtual bool equals(const BinaryObj<> &other) const {
      return (this->type() == other.type()) &&
             (this->length() == other.length()) &&
             (strcmp((const char *) this->data(),
                     (const char *) (other.data())) == 0);
    }


    virtual string toString() const override {
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
        return BinaryObj((string) line.substr(1, line.length() - 2)); // might be wrong indices
      else if (line == "true" || line == "false")
        return BinaryObj((bool) (line == "true"));
      else if (line[line.length() - 1] == 'f') {
        return BinaryObj((float) stof(line.substr(0, line.length() - 1)));
      } else {
        return BinaryObj((int) stoi(line));
      }
    }
  };

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  class Serializer {
  public:
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

  class PtrSerializer : public Serializer {
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
          return Int(*((int *) (bobj.data())));
        case REAL:
          return Int(static_cast<int>(bobj.toReal().value()));
        case STR:
          return Int(atoi(bobj.toStr().value().c_str()));
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
          return Real(atof(((string *) (bobj.data()))->c_str()));
        case STR:
          return Real(atof(((string *) (bobj.data()))->c_str()));
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
  };

  class CharSerializer : public Serializer {
  public:
    static binary_obj fromBoolean(const bool xbool) {
      return {
        BOOL,
        reinterpret_cast<const byte *>(xbool ? "T" : "F"),
        1
      };
    }

    static binary_obj fromInteger(const int xint) {
      char temp[15];
      const uint16_t size = sprintf(temp, "%i", xint);
      temp[size] = '\0';
      return {
        INT,
        reinterpret_cast<const byte *>(temp),
        size
      };
    }

    static binary_obj fromFloat(const float xfloat) {
      const string* temp =  new string(std::to_string(xfloat));
      return {
        REAL,
        (byte*)strdup(temp->c_str()),
        temp->size()
      };
    }

    static binary_obj fromString(const string &xstring) {
      char *temp = strdup(xstring.c_str());
      temp[xstring.length()] = '\0';
      return {
        STR,
        (byte *) temp,
        (uint16_t) (xstring.length())
      };
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
          return Int(atoi((const char *) bobj.data()));
        case REAL:
          return Int(static_cast<int>(bobj.toReal().value()));
        case STR:
          return Int(atoi(bobj.toStr().value().c_str()));
        default:
          throw std::runtime_error("bad2"); //fError("Unknown type: %s", OTYPE_STR.at(xserial.type).c_str());
      }
    }

    static Real toReal(const BinaryObj<CharSerializer> &bobj) {
      switch (bobj.type()) {
        case BOOL:
          return Real(bobj.toBool().value() ? 1.0f : 0.0f);
        case INT:
          return Real(static_cast<float>(bobj.toInt().value()));
        case REAL:
          return Real(static_cast<float>(atof((const char *) bobj.data())));
        case STR:
          return Real(static_cast<float>(atof(bobj.toStr().value().c_str())));
        default:
          throw std::runtime_error("bad3"); //throw fError("Unknown type: %s", OTYPE_STR.at(xserial.type).c_str());
      }
    }

    static Str toStr(const BinaryObj<CharSerializer> &bobj) {
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
          return Str(string(reinterpret_cast<const char *>(bobj.data()), bobj.length()));
        }
        default:
          throw std::runtime_error("bad4"); // throw fError("Unknown type: %s", OTYPE_STR.at(xserial.type).c_str());
      }
    }
  };
}

#endif
