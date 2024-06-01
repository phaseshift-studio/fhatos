/*******************************************************************************
 FhatOS: A Distributed Operating System
 Copyright (c) 2024 PhaseShift Studio, LLC

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Affero General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Affero General Public License for more details.

 You should have received a copy of the GNU Affero General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#ifndef fhatos_serializer_hpp
#define fhatos_serializer_hpp

#include <fhatos.hpp>
//
#include <language/obj.hpp>

namespace fhatos {
  using binary_obj = Triple<const OType, const fbyte *, const uint16_t>;
  class CharSerializer;
  class PtrSerializer;

  template<typename SERIALIZER = CharSerializer>
  class BinaryObj : public Obj {
  protected:
    const binary_obj _value;

  public:
    ~BinaryObj() {
      const OType t = std::get<0>(this->_value);
      if (t == OType::INT || t == OType::REAL || t == OType::STR)
        delete std::get<1>(this->_value);
    }

    explicit BinaryObj(const binary_obj bobj) : Obj(std::get<0>(bobj)), _value(bobj) {
    };

    explicit BinaryObj(const OType type, const fbyte *data, const uint16_t length) : Obj(type),
      _value{type, data, length} {
    };

    explicit BinaryObj(const fURI xfuri) : Obj(OType::URI), _value(SERIALIZER::fromUri(xfuri)) {
    }


    explicit BinaryObj(const bool xbool) : Obj(OType::BOOL), _value(SERIALIZER::fromBoolean(xbool)) {
    }


    explicit BinaryObj(const int xint) : Obj(OType::INT), _value(SERIALIZER::fromInteger(xint)) {
    }


    explicit BinaryObj(const float xfloat) : Obj(OType::REAL), _value(SERIALIZER::fromFloat(xfloat)) {
    }

    explicit BinaryObj(const string &xstring) : Obj(OType::STR), _value(SERIALIZER::fromString(xstring)) {
    }

    explicit BinaryObj(const char *xcstr) : Obj(OType::STR), _value(SERIALIZER::fromString(string(xcstr))) {
    }


    static BinaryObj *fromObj(const Obj *obj) {
      LOG(DEBUG, "Creating BinaryObj from !b%s!! [!y%s!!]\n", obj->toString().c_str(), OTYPE_STR.at(obj->type()));
      switch (obj->type()) {
        case OType::URI: return new BinaryObj<>((binary_obj) SERIALIZER::fromUri(((Uri *) obj)->value()));
        case OType::BOOL: return new BinaryObj<>((binary_obj) SERIALIZER::fromBoolean(((Bool *) obj)->value()));
        case OType::INT: return new BinaryObj<>((binary_obj) SERIALIZER::fromInteger(((Int *) obj)->value()));
        case OType::REAL: return new BinaryObj<>((binary_obj) SERIALIZER::fromFloat(((Real *) obj)->value()));
        case OType::STR: return new BinaryObj<>((binary_obj) SERIALIZER::fromString(((Str *) obj)->value()));
        case OType::REC: return new BinaryObj<>((binary_obj) SERIALIZER::fromMap(*((Rec *) obj)->value()));
        default: throw fError("error on type: %s", OTYPE_STR.at(obj->type()));
      }
    }

    virtual const binary_obj value() const {
      return _value;
    }

    const fbyte *data() const {
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

    Uri toUri() const {
      return SERIALIZER::toUri(*this);
    }

    Rec toRec() const {
      return SERIALIZER::toRec(*this);
    }

    Obj *toObj() const {
      switch (this->type()) {
        case OType::BOOL: return new Bool(toBool());
        case OType::INT: return new Int(toInt());
        case OType::REAL: return new Real(toReal());
        case OType::STR: return new Str(toStr());
        case OType::URI: return new Uri(toUri());
        //case OType::REC: return &toRec();
        default: throw new fError("Unsupported conversion for %s\n", OTYPE_STR.at(this->type()));
      }
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
        case OType::BOOL: return toBool().toString();
        case OType::INT: return toInt().toString();
        case OType::REAL: return toReal().toString();
        case OType::STR: return toStr().toString();
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
      return {OType::OBJ, nullptr, 0};
    }

    static binary_obj fromBoolean(const bool xbool) {
      return {OType::OBJ, nullptr, 0};
    }

    static binary_obj fromInt(const int xint) {
      return {OType::OBJ, nullptr, 0};
    }

    static binary_obj fromFloat(const float xreal) {
      return {OType::OBJ, nullptr, 0};
    }

    static binary_obj fromString(const string &xstr) {
      return {OType::OBJ, nullptr, 0};
    }

    static binary_obj fromMap(const RecMap<Obj*,Obj*> &xmap) {
      return {OType::OBJ, nullptr, 0};
    }

    template<typename SERIALIZER>
    static Uri toUri(const BinaryObj<SERIALIZER> &xserial) {
      return Uri(fURI("/"));
    }

    template<typename SERIALIZER>
    static Rec toRec(const BinaryObj<SERIALIZER> &xmap) {
      return Rec({});
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
        OType::URI,
        (fbyte *) temp,
        len
      };
    }


    static binary_obj fromBoolean(const bool xbool) {
      return {
        OType::BOOL,
        reinterpret_cast<const fbyte *>(xbool ? "T" : "F"),
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
        OType::INT,
        (fbyte *) temp,
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
        OType::REAL,
        (fbyte *) temp,
        len
      };
    }

    static binary_obj fromString(const string &xstring) {
      const uint16_t len = xstring.length();
      char *temp = (char *) malloc(len + 1);
      memccpy(temp, xstring.c_str(), 0, len);
      temp[len] = '\0';
      return {
        OType::STR,
        (fbyte *) temp,
        len
      };
    }

    static binary_obj fromMap(const RecMap<Obj *, Obj *> &xmap) {
      fbyte *bytes = static_cast<fbyte *>(malloc(sizeof(xmap)));
      memcpy(bytes, &xmap, sizeof(xmap));
      return binary_obj{OType::REC, bytes, sizeof(xmap)};
    }

    static Rec toRec(const BinaryObj<CharSerializer> &bobj) {
      switch (bobj.type()) {
        case OType::REC:
          return *((Rec *) bobj.data());
        default: throw std::runtime_error("bad7");
        // throw fError("Unknown type: %s", OTYPE_STR.at(xserial.type).c_str());
      }
    }

    static Uri toUri(const BinaryObj<CharSerializer> &bobj) {
      switch (bobj.type()) {
        case OType::URI:
          return Uri(fURI("/" + bobj.toString()));;
        case OType::BOOL:
          return Uri(fURI("/" + bobj.toString()));
        case OType::INT:
          return Uri(fURI("/" + bobj.toString()));
        case OType::REAL:
          return Uri(fURI("/" + bobj.toString()));
        case OType::STR:
          return Uri(fURI("/" + bobj.toString()));
        default:
          throw std::runtime_error("bad1"); // throw fError("Unknown type: %s", OTYPE_STR.at(xserial.type).c_str());
      }
    }

    static Bool toBool(const BinaryObj<CharSerializer> &bobj) {
      switch (bobj.type()) {
        case OType::BOOL:
          return Bool(strcmp((char *) bobj.data(), "T") == 0);
        case OType::INT:
          return Bool(bobj.toInt().value() > 0);
        case OType::REAL:
          return Bool(bobj.toReal().value() > 0.0f);
        case OType::STR:
          return Bool(bobj.toStr().value().compare("true") == 0 || bobj.toStr().value().compare("1") == 0);
        default:
          throw std::runtime_error("bad1"); // throw fError("Unknown type: %s", OTYPE_STR.at(xserial.type));
      }
    }

    static Int toInt(const BinaryObj<CharSerializer> &bobj) {
      switch (bobj.type()) {
        case OType::BOOL:
          return Int(bobj.toBool().value() ? 1 : 0);
        case OType::INT:
          return Int(std::stoi(string((char *) bobj.data(), bobj.length())));
        case OType::REAL:
          return Int(static_cast<int>(bobj.toReal().value()));
        case OType::STR:
          return Int(std::stoi(string((char *) bobj.data(), bobj.length())));
        default:
          throw std::runtime_error("bad2"); //fError("Unknown type: %s", OTYPE_STR.at(xserial.type));
      }
    }

    static Real toReal(const BinaryObj<CharSerializer> &bobj) {
      switch (bobj.type()) {
        case OType::BOOL:
          return Real(bobj.toBool().value() ? 1.0f : 0.0f);
        case OType::INT:
          return Real((float) (bobj.toInt().value()));
        case OType::REAL:
          return Real(std::stof(string((char *) bobj.data(), bobj.length())));
        case OType::STR:
          return Real(std::stof(string((char *) bobj.data(), bobj.length())));
        default:
          throw std::runtime_error("bad3"); //throw fError("Unknown type: %s", OTYPE_STR.at(xserial.type).c_str());
      }
    }

    static Str toStr(const BinaryObj<CharSerializer> &bobj) {
      switch (bobj.type()) {
        case OType::BOOL:
          return Str(bobj.toBool().value() ? "true" : "false");
        case OType::INT: {
          char temp[15];
          const uint size = sprintf(temp, "%i", bobj.toInt().value());
          temp[size] = '\0';
          return Str(string(temp, size));
        }
        case OType::REAL: {
          char temp[15];
          const uint size = sprintf(temp, "%f", bobj.toReal().value());
          temp[size] = '\0';
          return Str(string(temp, size));
        }
        case OType::STR: {
          return Str(string((char *) (bobj.data()), bobj.length()));
        }
        default:
          throw std::runtime_error("bad4"); // throw fError("Unknown type: %s", OTYPE_STR.at(xserial.type).c_str());
      }
    }
  };
}

#endif
