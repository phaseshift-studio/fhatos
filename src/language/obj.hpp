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

/**
 *  @file   obj.hpp
 *  @brief  Foundation objects of mm-ADT
 *  @author Marko A. Rodriguez
 *  @date   2024-05-01
 ***********************************************/

#ifndef fhatos_obj_hpp
#define fhatos_obj_hpp


#ifndef FL_REAL_TYPE
#define FL_REAL_TYPE float
#include <any>
#endif
#ifndef FL_INT_TYPE
#define FL_INT_TYPE int
#endif

#include <fhatos.hpp>
#include <structure/furi.hpp>
#include <util/uuid.hpp>
#include <utility>
#include <variant>
#ifdef NATIVE
#include <assert.h>
#else
#include <esp_assert.h>
#endif

namespace fhatos {
  /// @brief The base types of mm-ADT
  enum class OType : uint8_t {
    /// The base type of all types is the obj
    OBJ,
    /// A "null" object type used to kill a processing monad
    NOOBJ,
    OBJS,
    /// A boolean mono-type
    BOOL,
    /// An integral number mono-type in Z
    INT,
    /// A real number mono-type in R
    REAL,
    /// A string mono-type denoting a sequence of characters where a "char" is equivalent to str[0]
    STR,
    /// A Uniform Resource Identifier mono-type
    URI,
    /// A list poly-type
    LST,
    /// A key/value pair record poly-type
    REC,
    /// An instruction type denoting an opcode, arguments, and a function
    INST,
    /// A "null" instruction type used to halt a processing monad
    NOINST,
    /// A sequence of instructions denoting a program
    BYTECODE
  };


  class Obj;
  using Objp = ptr<Obj>;
  using fURIp = ptr<fURI>;
  using BObj = Triple<OType, fbyte *, uint32_t>;
  class PtrSerializer;
  class Type;
  using InstFunction = Function<Objp, Objp>;
  using InstArgs = List<ptr<Obj>>;
  using InstOpcode = string;
  using InstValue = Pair<InstArgs, InstFunction>;
  ///
  using Bool = Obj;
  using Boolp = Objp;
  using Int = Obj;
  using Intp = Objp;
  using Uri = Obj;
  using Urip = Objp;
  using Inst = Obj;
  using Instp = Objp;
  using Bytecode = Obj;
  using Bytecodep = Objp;
  using Objs = Obj;
  using Objsp = Objp;


  static const Map<OType, const char *> OTYPE_STR = {{{OType::NOOBJ, "noobj"},
                                                      {OType::NOINST, "noinst"},
                                                      {OType::OBJ, "obj"},
                                                      {OType::OBJS, "objs"},
                                                      {OType::URI, "uri"},
                                                      {OType::BOOL, "bool"},
                                                      {OType::INT, "int"},
                                                      {OType::REAL, "real"},
                                                      {OType::STR, "str"},
                                                      {OType::LST, "lst"},
                                                      {OType::REC, "rec"},
                                                      {OType::INST, "inst"},
                                                      {OType::BYTECODE, "bcode"}}};
  static const Map<string, OType> STR_OTYPE = {{{"noobj", OType::NOOBJ},
                                                {"noinst", OType::NOINST},
                                                {"obj", OType::OBJ},
                                                {"objs", OType::OBJS},
                                                {"uri", OType::URI},
                                                {"bool", OType::BOOL},
                                                {"int", OType::INT},
                                                {"real", OType::REAL},
                                                {"str", OType::STR},
                                                {"lst", OType::LST},
                                                {"rec", OType::REC},
                                                {"inst", OType::INST},
                                                {"bcode", OType::BYTECODE}}};

  static const ptr<fURI> OBJ_FURI = ptr<fURI>(new fURI("/obj"));
  static const ptr<fURI> NOOBJ_FURI = ptr<fURI>(new fURI("/noobj"));
  static const ptr<fURI> TYPE_FURI = ptr<fURI>(new fURI("/type"));
  static const ptr<fURI> BOOL_FURI = ptr<fURI>(new fURI("/bool"));
  static const ptr<fURI> INT_FURI = ptr<fURI>(new fURI("/int"));
  static const ptr<fURI> REAL_FURI = ptr<fURI>(new fURI("/real"));
  static const ptr<fURI> URI_FURI = ptr<fURI>(new fURI("/uri"));
  static const ptr<fURI> STR_FURI = ptr<fURI>(new fURI("/str"));
  static const ptr<fURI> REC_FURI = ptr<fURI>(new fURI("/rec"));
  static const ptr<fURI> INST_FURI = ptr<fURI>(new fURI("/inst"));
  static const ptr<fURI> BCODE_FURI = ptr<fURI>(new fURI("/bcode"));
  static const ptr<fURI> OBJS_FURI = ptr<fURI>(new fURI("/objs"));

  enum class IType : uint8_t {
    NOINST = 0,
    ONE_TO_ONE,
    ONE_TO_MANY,
    MANY_TO_ONE,
    MANY_TO_MANY,
  }; // TYPE
  static const Map<IType, const char *> ITYPE_STR = {{
      {IType::NOINST, "0->0 (null)"},
      {IType::ONE_TO_ONE, "f(x)->y (map)"},
      {IType::ONE_TO_MANY, "f(x)->y* (flatmap)"},
      {IType::MANY_TO_ONE, "f(x*)->y (reduce)"},
      {IType::MANY_TO_MANY, "f(x*)->y* (barrier)"},
  }};

  //////////////////////////////////////////////////
  ////////////////////// OBJ //////////////////////
  /////////////////////////////////////////////////
  /// An mm-ADT abstract object from which all other types derive
  class Obj : public std::enable_shared_from_this<Obj> {
  protected:
    Any _value;
    const ptr<fURI> _furi;
    //////////////////////////////////////////
    struct obj_hash {
      size_t operator()(const Objp &obj) const {
        return static_cast<std::string::value_type>(obj->o_domain()) ^ obj->_furi->toString().size() ^
               obj->toString().length() ^ obj->toString()[0];
      }
    };

    struct obj_equal_to : std::binary_function<Objp &, Objp &, bool> {
      bool operator()(const Objp &a, const Objp &b) const { return *a == *b; }
    };
    template<typename K = ptr<Obj>, typename V = ptr<Obj>, typename H = obj_hash, typename Q = obj_equal_to>
    using RecMap = OrderedMap<K, V, H, Q>;


  public:
    virtual ~Obj() = default;
    explicit Obj(const Any &value, const ptr<fURI> &furi) : _value(value), _furi(furi) {}

    /////
    //////////////////////////////////////////////////////////////
    //// IMPLICIT CONVERSIONS (FOR NATIVE C++ CONSTRUCTIONS) ////
    //////////////////////////////////////////////////////////////
    template<class T, class = typename std::enable_if_t<std::is_same_v<bool, T>>>
    Obj(const T xbool) : _value(xbool), _furi(BOOL_FURI) {}
    Obj(const FL_INT_TYPE xint) : _value(xint), _furi(INT_FURI) {}
    Obj(const FL_REAL_TYPE xreal) : _value(xreal), _furi(REAL_FURI) {}
    Obj(const fURI xuri) : _value(xuri), _furi(URI_FURI) {}
    Obj(const char *xstr) : _value(xstr), _furi(STR_FURI) {}
    Obj(const string xstr) : _value(xstr), _furi(STR_FURI) {}
    Obj(const std::initializer_list<Pair<const Obj, Obj>> &xrec) : _value(RecMap<>({})), _furi(REC_FURI) {
      RecMap<> map = this->rec_value();
      for (const Pair<const Obj, Obj> &pair: xrec) {
        map.emplace(share(pair.first), share(pair.second));
      }
      this->_value = map;
    }
    Obj(const List<Obj> bcode) : _value(bcode), _furi(BCODE_FURI) {
      List<Objp> list = this->bcode_value();
      for (const Obj obj: bcode) {
        list.push_back(share(obj));
      }
      this->_value = list;
    }
    //////////////////////////////////////////////////////////////
    const OType o_domain() const { return STR_OTYPE.at(this->_furi->path(0, 1)); }
    const OType o_range() const { return STR_OTYPE.at(this->_furi->path(0, 1)); } // TODO
    const fURIp range() const { return this->_furi; }
    const fURIp domain() const { return this->_furi; }
    const ID id() const { return *this->_furi; }
    template<typename _VALUE>
    const _VALUE value() const {
      return std::any_cast<_VALUE>(this->_value);
    }
    const bool bool_value() const {
      assert(OType::BOOL == o_range());
      return this->value<bool>();
    }
    const FL_INT_TYPE int_value() const {
      assert(OType::INT == o_range());
      return this->value<FL_INT_TYPE>();
    }
    const FL_REAL_TYPE real_value() const {
      assert(OType::REAL == o_range());
      return this->value<FL_REAL_TYPE>();
    }
    const fURIp uri_value() const {
      assert(OType::URI == o_range());
      return this->value<ptr<fURI>>();
    }
    const string str_value() const {
      assert(OType::STR == o_range());
      return this->value<string>();
    }
    RecMap<> rec_value() const {
      assert(OType::REC == o_range());
      return this->value<RecMap<>>();
    }
    Objp get(const Objp &key) const {
      assert(OType::REC == o_range());
      return this->rec_value().count(key) ? this->rec_value().at(key) : Obj::to_noobj();
    }
    void set(Objp &key, const Objp &val) {
      assert(OType::REC == o_range());
      this->rec_value().emplace(key, val);
    }
    const InstValue inst_value() const {
      assert(OType::INST == o_range());
      return this->value<InstValue>();
    }
    const string inst_op() const {
      assert(OType::INST == o_range());
      return this->_furi->lastSegment();
    }
    const List<Objp> inst_args() const {
      assert(OType::INST == o_range());
      return this->inst_value().first;
    }
    Objp inst_arg(const uint8_t index) const {
      assert(OType::INST == o_range());
      return this->inst_value().first.at(index);
    }

    const InstFunction inst_f() const {
      assert(OType::INST == o_range());
      return this->inst_value().second;
    }
    List<Objp> bcode_value() const {
      assert(OType::BYTECODE == o_range());
      return this->value<List<Objp>>();
    }


    const string toString() const {
      string objString;
      switch (this->o_range()) {
        case OType::BOOL:
          objString = this->bool_value() ? "true" : "false";
          break;
        case OType::INT:
          objString = std::to_string(this->int_value());
          break;
        case OType::REAL:
          objString = std::to_string(this->real_value());
          break;
        case OType::URI:
          objString = this->uri_value()->toString();
          break;
        case OType::STR:
          objString = "'" + this->str_value() + "'";
          break;
        case OType::REC: {
          string t = "!m[!!";
          for (const auto &[k, v]: this->rec_value()) {
            t += k->toString() + "!m=>!!" + v->toString() + ",";
          }
          t[t.length() - 1] = '!';
          objString = t.append("m]!!");
          break;
        }
        case OType::INST: {
          string t = "";
          for (const auto &arg: this->inst_value().first) {
            t += arg->toString() + ",";
          }
          objString = t.substr(0, t.length() - 1);
          break;
        }
        case OType::BYTECODE: {
          string s;
          for (const auto &inst: this->bcode_value()) {
            s.append(inst->toString() + '.');
          }
          objString = s.substr(0, s.length() - 1);
          break;
        }
        case OType::NOOBJ: {
          objString = "!bØ!!";
          break;
        }
        default:
          throw fError("Unknown obj type in toString(): %s\n", OTYPE_STR.at(this->o_range()));
      }
      return this->_furi->pathLength() > 1
                 ? (this->_furi->user()->empty() ? "" : ("!b" + this->_furi->user().value() + "!g@!b/!!")) +
                       ("!b" + this->_furi->lastSegment() + "!g[!!" + objString + "!g]!!")
                 : (this->_furi->user()->empty() ? "" : ("!b" + this->_furi->user().value() + "!g@!!")) + objString;
    }
    int compare(const Obj &other) const { return this->toString().compare(other.toString()); }
    bool operator&&(const Obj &other) const { return this->bool_value() && other.bool_value(); }
    bool operator||(const Obj &other) const { return this->bool_value() || other.bool_value(); }
    bool operator<(const Obj &other) const { return this->int_value() < other.int_value(); }
    bool operator>(const Obj &other) const { return this->int_value() > other.int_value(); }
    bool operator<=(const Obj &other) const { return this->int_value() <= other.int_value(); }
    bool operator>=(const Obj &other) const { return this->int_value() >= other.int_value(); }
    Obj operator*(const Obj &other) const { return Obj(this->int_value() * other.int_value(), this->_furi); }
    Obj operator+(const Obj &other) const { return Obj(this->int_value() + other.int_value(), this->_furi); }
    Obj operator%(const Obj &other) const { return Obj(this->int_value() % other.int_value(), this->_furi); }
    bool operator!=(const Obj &other) const { return !(*this == other); }
    bool operator==(const Obj &other) const {
      if (!this->_furi->equals(*other._furi))
        return false;
      switch (this->o_range()) {
        case OType::BOOL:
          return this->bool_value() == other.bool_value();
        case OType::INT:
          return this->int_value() == other.int_value();
        case OType::REAL:
          return this->real_value() == other.real_value();
        case OType::URI:
          return this->uri_value()->equals(*other.uri_value());
        case OType::STR:
          return this->str_value() == other.str_value();
        case OType::REC: {
          auto itB = other.rec_value().begin();
          for (auto itA = this->rec_value().begin(); itA != this->rec_value().end(); ++itA) {
            if (*itA->first != *itB->first || *itA->second != *itB->second)
              return false;
            ++itB;
          }
          return true;
        }
        default:
          throw fError("Unknown obj type in ==: %s\n", OTYPE_STR.at(this->o_range()));
      }
    }
    bool isNoObj() const { return this->o_range() == OType::NOOBJ; }
    bool isBytecode() { return false; }
    Objp apply(const Objp &lhs) {
      switch (this->o_range()) {
        case OType::BOOL:
          return shared_from_this();
        case OType::INT:
          return shared_from_this();
        case OType::REAL:
          return shared_from_this();
        case OType::URI:
          return shared_from_this();
        case OType::STR:
          return shared_from_this();
        case OType::INST:
          return this->inst_f()(lhs);
        case OType::BYTECODE: {
          ptr<Obj> currentObj = lhs;
          for (const Instp &currentInst: this->bcode_value()) {
            if (currentInst->isNoObj() || currentObj->isNoObj())
              break;
            LOG(DEBUG, "Applying %s => %s\n", currentObj->toString().c_str(), currentInst->toString().c_str());
            currentObj = currentInst->apply(currentObj);
          }
          return currentObj; //(currentObj->type() == OType::URI) ? relativeUri((ptr<Uri>currentObj) : currentObj;
        }
        case OType::NOOBJ:
          return Obj::to_noobj();
        default:
          throw fError("Unknown obj type in apply(): %s\n", OTYPE_STR.at(this->o_range()));
      }
    }
    const Objp
    split(const Any &newValue,
          const std::variant<ptr<fURI>, const char *> &newType = std::variant<ptr<fURI>, const char *>(nullptr)) const {
      return share(Obj(newValue, std::variant<ptr<fURI>, const char *>(nullptr) == newType
                                     ? this->_furi
                                     : (std::holds_alternative<const char *>(newType)
                                            ? ptr<fURI>(new fURI(std::get<const char *>(newType)))
                                            : std::get<ptr<fURI>>(newType))));
    }

    Objp as(const fURIp &furi) const { return share(Obj(this->_value, furi)); }

    const Instp nextInst(Instp currentInst) const {
      if (currentInst->isNoObj())
        return currentInst;
      bool found = false;
      for (const auto &inst: this->bcode_value()) {
        if (found)
          return inst;
        if (inst == currentInst)
          found = true;
      }
      return Obj::to_noobj();
    }

    /// STATIC TYPE CONSTRAINED CONSTRUCTORS
    static Objp to_noobj() { return share(Obj(nullptr, NOOBJ_FURI)); }

    static Objp to_bool(const bool value, const fURIp &furi = BOOL_FURI) {
      assert(furi->path(0, 1) == STR_OTYPE.at(OType::BOOL));
      return share(Obj(value, furi));
    }

    static Objp to_int(const bool value, const fURIp &furi = BOOL_FURI) {
      assert(furi->path(0, 1) == STR_OTYPE.at(OType::INT));
      return share(Obj(value, furi));
    }

    static Objp to_str(const string value, const fURIp &furi = STR_FURI) {
      assert(furi->path(0, 1) == STR_OTYPE.at(OType::STR));
      return share(Obj(value, furi));
    }

    static Objp to_str(const char *value, const fURIp &furi = STR_FURI) {
      assert(furi->path(0, 1) == STR_OTYPE.at(OType::STR));
      return share(Obj(string(value), furi));
    }

    static Objp to_uri(const fURIp &value, const fURIp &furi = URI_FURI) {
      assert(furi->path(0, 1) == STR_OTYPE.at(OType::URI));
      return share(Obj(value, furi));
    }

    static Objp to_uri(const char *value, const fURIp &furi = URI_FURI) {
      assert(furi->path(0, 1) == STR_OTYPE.at(OType::URI));
      return share(Obj(value, furi));
    }

    static Objp to_inst(const InstValue &value, const fURIp &furi = INST_FURI) {
      assert(furi->path(0, 1) == STR_OTYPE.at(OType::INST));
      return share(Obj(value, furi));
    }

    static Objp to_inst(const string opcode, const List<Objp> &args, const InstFunction &function,
                        const fURIp &furi = nullptr) {
      const fURIp fix = !furi ? share(fURI(string("/inst/") + opcode)) : furi;
      assert(fix->path(0, 1) == STR_OTYPE.at(OType::INST));
      return share(Obj(std::make_pair(args, function), fix));
    }

    static Objp to_bcode(const List<Objp> &insts, const fURIp &furi = BCODE_FURI) {
      assert(furi->path(0, 1) == STR_OTYPE.at(OType::BYTECODE));
      return share(Obj(insts, furi));
    }

    template<typename SERIALIZER = PtrSerializer>
    const ptr<BObj> serialize() const {
      return SERIALIZER::singleton()->serialize(this);
    }

    /*template<typename SERIALIZER = PtrSerializer, typename _OBJ>
    static const ptr<_OBJ> deserialize(const BObj *bobj) {
      return SERIALIZER::singleton()->deserialize(this);
    }*/

    static List<Objp> cast(const List<Obj> &list) {
      List<Objp> newList = List<Objp>();
      for (const auto &obj: list) {
        newList.push_back(share(Obj(obj)));
      }
      return newList;
    }
  };


} // namespace fhatos


// namespace fhatos
///////////////////////////////////////////////////
////////////////////// TYPE //////////////////////
//////////////////////////////////////////////////
/// An mm-ADT type encoded in a URI
/*class Type : public Obj {
public:
  explicit Type(const ptr<fURI> &furi) : Obj(furi, TYPE_FURI) {}
  ptr<fURI> v_furi() const { return std::any_cast<ptr<fURI>>(std::get<0>(this->_var)); }
  OType instanceOType() { return STR_OTYPE.at(this->v_furi()->path(0, 1)); }
  string variable() const { return this->v_furi()->user().value_or(""); }
  string name() const { return this->v_furi()->path(this->v_furi()->pathLength() - 1); }
  string stype() const { return this->v_furi()->path(); }
  string location() const { return this->v_furi()->host(); }
  bool mutating() const { return this->variable()[0] == '~'; }
  string objString(const string &objString) const {
    return this->v_furi()->pathLength() > 1
               ? (this->variable().empty() ? "" : ("!b" + this->variable() + "!g@!b/!!")) +
                     ("!b" + this->name() + "!g[!!" + objString + "!g]!!")
               : (this->variable().empty() ? "" : ("!b" + this->variable() + "!g@!!")) + objString;
  }
  string toString() const override { return this->v_furi()->toString(); }
  static ptr<fURI> obj_t(const OType &otype, const char *utype) {
    string typeToken = string(utype);
    bool hasAuthority = typeToken.find('@') != std::string::npos;
    bool hasSlash = typeToken.starts_with("/");
    fURI temp = fURI(hasAuthority ? typeToken : hasSlash ? typeToken : "/" + typeToken);
    fURI temp2 = fURI(string("/") + OTYPE_STR.at(otype) + (temp.path().empty() ? "" : ("/" + temp.path())));
    fURI temp3 = temp2.authority(temp.authority());
    return ptr<fURI>(new fURI(temp3));
  }
  static ptr<fURI> obj_t(const OType &otype, const ptr<fURI> &utype) {
    return Type::obj_t(otype, utype.get() ? utype->toString().c_str() : "");
  }
};*/


#endif
