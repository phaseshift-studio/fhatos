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
    BCODE
  };


  class Obj;
  using Obj_p = ptr<Obj>;
  using fURI_p = ptr<fURI>;
  using Bool = Obj;
  using Bool_p = Obj_p;
  using Int = Obj;
  using Int_p = Obj_p;
  using Real = Obj;
  using Real_p = Obj_p;
  using Uri = Obj;
  using Uri_p = Obj_p;
  using Str = Obj;
  using Str_p = Obj_p;
  using Rec = Obj;
  using Rec_p = Obj_p;
  using Inst = Obj;
  using Inst_p = Obj_p;
  using BCode = Obj;
  using BCode_p = Obj_p;
  using Objs = Obj;
  using Objs_p = Obj_p;
  //
  using BObj = Triple<OType, fbyte *, uint32_t>;
  class PtrSerializer;
  class Type;
  // Inst structures
  using InstFunction = Function<Obj_p, Obj_p>;
  using InstArgs = List<ptr<Obj>>;
  using InstOpcode = string;
  using InstValue = Pair<InstArgs, InstFunction>;


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
                                                      {OType::BCODE, "bcode"}}};
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
                                                {"bcode", OType::BCODE}}};

  static const fURI_p OBJ_FURI = fURI_p(new fURI("/obj"));
  static const fURI_p NOOBJ_FURI = fURI_p(new fURI("/noobj"));
  static const fURI_p TYPE_FURI = fURI_p(new fURI("/type"));
  static const fURI_p BOOL_FURI = fURI_p(new fURI("/bool"));
  static const fURI_p INT_FURI = fURI_p(new fURI("/int"));
  static const fURI_p REAL_FURI = fURI_p(new fURI("/real"));
  static const fURI_p URI_FURI = fURI_p(new fURI("/uri"));
  static const fURI_p STR_FURI = fURI_p(new fURI("/str"));
  static const fURI_p REC_FURI = fURI_p(new fURI("/rec"));
  static const fURI_p INST_FURI = fURI_p(new fURI("/inst"));
  static const fURI_p BCODE_FURI = fURI_p(new fURI("/bcode"));
  static const fURI_p OBJS_FURI = fURI_p(new fURI("/objs"));

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
    ptr<fURI> _furi;
    //////////////////////////////////////////
    struct obj_hash {
      size_t operator()(const Obj_p &obj) const {
        return static_cast<std::string::value_type>(obj->o_domain()) ^ obj->_furi->toString().size() ^
               obj->toString().length() ^ obj->toString()[0];
      }
    };

    struct obj_equal_to : std::binary_function<Obj_p &, Obj_p &, bool> {
      bool operator()(const Obj_p &a, const Obj_p &b) const { return *a == *b; }
    };


  public:
    static fURI_p RESOLVE(fURI_p base, fURI_p furi) {
      if (base->equals(*furi))
        return base;
      else {
        return share(base->resolve(furi->toString().c_str()));
      }
    }

    template<typename K = ptr<Obj>, typename V = ptr<Obj>, typename H = obj_hash, typename Q = obj_equal_to>
    using RecMap = OrderedMap<K, V, H, Q>;
    virtual ~Obj() = default;
    explicit Obj(const Any &value, const fURI_p &furi) : _value(value), _furi(furi) {}
    /////
    //////////////////////////////////////////////////////////////
    //// IMPLICIT CONVERSIONS (FOR NATIVE C++ CONSTRUCTIONS) ////
    //////////////////////////////////////////////////////////////
    template<class T, class = typename std::enable_if_t<std::is_same_v<bool, T>>>
    Obj(const T xbool, const char *furi = BOOL_FURI->toString().c_str()) :
        Obj(Any(xbool), RESOLVE(BOOL_FURI, share(fURI(furi)))) {}
    Obj(const FL_INT_TYPE xint, const char *furi = INT_FURI->toString().c_str()) :
        Obj(Any(xint), RESOLVE(INT_FURI, share(fURI(furi)))) {}
    Obj(const FL_REAL_TYPE xreal, const char *furi = REAL_FURI->toString().c_str()) :
        Obj(Any(xreal), RESOLVE(REAL_FURI, share(fURI(furi)))) {}
    Obj(const fURI &xuri, const char *furi = URI_FURI->toString().c_str()) :
        Obj(Any(xuri), RESOLVE(URI_FURI, share(fURI(furi)))) {}
    Obj(const char *xstr, const char *furi = STR_FURI->toString().c_str()) :
        Obj(Any(string(xstr)), RESOLVE(STR_FURI, share(fURI(furi)))) {}
    Obj(const string &xstr, const char *furi = STR_FURI->toString().c_str()) :
        Obj(Any(xstr), RESOLVE(STR_FURI, share(fURI(furi)))) {}
    Obj(const std::initializer_list<Pair<const Obj, Obj>> &xrec, const char *furi = REC_FURI->toString().c_str()) :
        Obj(Any(xrec), RESOLVE(REC_FURI, share(fURI(furi)))) {
      RecMap<> map = RecMap<>();
      for (const auto &pair: xrec) {
        map.insert(make_pair(share(pair.first), share(pair.second)));
      }
      this->_value = std::move(map);
    }
    Obj(const List<Inst> &bcode, const char *furi = BCODE_FURI->toString().c_str()) :
        Obj(Any(bcode), RESOLVE(BCODE_FURI, share(fURI(furi)))) {
      List<Obj_p> list = this->bcode_value();
      for (const auto &obj: bcode) {
        list.push_back(share(obj));
      }
      this->_value = list;
    }
    Obj(const List<Inst_p> &bcode, const char *furi = BCODE_FURI->toString().c_str()) :
        Obj(Any(bcode), RESOLVE(BCODE_FURI, share(fURI(furi)))) {}
    //////////////////////////////////////////////////////////////
    const OType o_domain() const { return STR_OTYPE.at(this->_furi->path(0, 1)); }
    const OType o_range() const { return STR_OTYPE.at(this->_furi->path(0, 1)); } // TODO
    const fURI_p range() const { return this->_furi; }
    const fURI_p domain() const { return this->_furi; }
    const fURI_p id() const { return this->_furi; }
    template<typename VALUE>
    const VALUE value() const {
      return std::any_cast<VALUE>(this->_value);
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
    const fURI uri_value() const {
      assert(OType::URI == o_range());
      return this->value<fURI>();
    }
    const string str_value() const {
      assert(OType::STR == o_range());
      return this->value<string>();
    }
    RecMap<> rec_value() const {
      assert(OType::REC == o_range());
      return this->value<RecMap<>>();
    }
    Obj_p rec_get(const Obj_p &key) const {
      assert(OType::REC == o_range());
      return this->rec_value().count(key) ? this->rec_value().at(key) : Obj::to_noobj();
    }
    void rec_set(Obj_p &key, const Obj_p &val) const {
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
    const List<Obj_p> inst_args() const {
      assert(OType::INST == o_range());
      return this->inst_value().first;
    }
    Obj_p inst_arg(const uint8_t index) const {
      assert(OType::INST == o_range());
      return this->inst_value().first.at(index);
    }

    const InstFunction inst_f() const {
      assert(OType::INST == o_range());
      return this->inst_value().second;
    }
    List<Obj_p> bcode_value() const {
      assert(OType::BCODE == o_range());
      return this->value<List<Obj_p>>();
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
          objString = this->uri_value().toString();
          break;
        case OType::STR:
          objString = "!m'!!" + this->str_value() + "!m'!!";
          break;
        case OType::REC: {
          objString = "!m[!!";
          bool first = true;
          for (const auto &[k, v]: this->rec_value()) {
            if (first) {
              first = false;
            } else {
              objString += "!m,!!";
            }
            objString += k->toString() + "!m=>!!" + v->toString();
          }
          objString += "!m]!!";
          break;
        }
        case OType::INST: {
          bool first = true;
          for (const auto &arg: this->inst_value().first) {
            if (first) {
              first = false;
            } else {
              objString += "!m,!!";
            }
            objString += arg->toString();
          }
          break;
        }
        case OType::BCODE: {
          if (this->bcode_value().empty())
            objString = "_";
          else {
            bool first = true;
            for (const auto &inst: this->bcode_value()) {
              if (first) {
                first = false;
              } else {
                objString += "!m.!!";
              }
              objString += inst->toString();
            }
          }
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
    int compare(const Obj &rhs) const { return this->toString().compare(rhs.toString()); }
    bool operator&&(const Obj &rhs) const { return this->bool_value() && rhs.bool_value(); }
    bool operator||(const Obj &rhs) const { return this->bool_value() || rhs.bool_value(); }
    bool operator<(const Obj &rhs) const { return this->int_value() < rhs.int_value(); }
    bool operator>(const Obj &rhs) const { return this->int_value() > rhs.int_value(); }
    bool operator<=(const Obj &rhs) const { return this->int_value() <= rhs.int_value(); }
    bool operator>=(const Obj &rhs) const { return this->int_value() >= rhs.int_value(); }
    Obj operator*(const Obj &rhs) const {
      switch (this->o_range()) {
        case OType::BOOL:
          return Obj(this->bool_value() && rhs.bool_value(), this->id());
        case OType::INT:
          return Obj(this->int_value() * rhs.int_value(), this->id());
        case OType::REAL:
          return Obj(this->real_value() * rhs.real_value(), this->id());
        // case OType::URI:
        //   return Obj(this->uri_value().extend(rhs.uri_value().path().c_str()), this->id());
        // case OType::STR:
        //   return Obj(this->str_value() + rhs.str_value(), this->id());
        case OType::REC: {
          auto map = RecMap<>();
          auto itB = rhs.rec_value().begin();
          for (auto itA = this->rec_value().begin(); itA != this->rec_value().end(); ++itA) {
            map.insert(std::make_pair(share(Obj((*itA->first) * (*itB->first), itA->first->id())),
                                      share(Obj((*itA->second) * (*itB->second), itA->second->id()))));
            ++itB;
          }
          return Obj(map, this->id());
        }
        default:
          throw fError("Unknown obj type in +: %s\n", OTYPE_STR.at(this->o_range()));
      }
    }
    Obj operator+(const Obj &rhs) const {
      switch (this->o_range()) {
        case OType::BOOL:
          return Obj(this->bool_value() || rhs.bool_value(), this->id());
        case OType::INT:
          return Obj(this->int_value() + rhs.int_value(), this->id());
        case OType::REAL:
          return Obj(this->real_value() + rhs.real_value(), this->id());
        case OType::URI:
          return Obj(this->uri_value().extend(rhs.uri_value().path().c_str()), this->id());
        case OType::STR:
          return Obj(this->str_value() + rhs.str_value(), this->id());
        case OType::REC: {
          RecMap<> map = RecMap<>();
          for (const auto &pair: this->rec_value()) {
            map.insert(pair);
          }
          for (const auto &pair: rhs.rec_value()) {
            map.insert(pair);
          }
          return Obj(map, this->id());
        }
        default:
          throw fError("Unknown obj type in +: %s\n", OTYPE_STR.at(this->o_range()));
      }
    }
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
          return this->uri_value() == other.uri_value();
        case OType::STR:
          return this->str_value() == other.str_value();
        case OType::REC: {
          auto pairsA = this->rec_value();
          auto pairsB = other.rec_value();
          // if (pairsA.size() != pairsB.size())
          //  return false;
          auto itB = pairsB.begin();
          for (const auto &itA: pairsA) {
            if (*itA.first != *itB->first || *itA.second != *itB->second)
              return false;
            ++itB;
          }
          return true;
        }
        case OType::INST: {
          if (other.inst_op() != this->inst_op())
            return false;
          auto argsA = this->inst_args();
          auto argsB = other.inst_args();
          if (argsA.size() != argsB.size())
            return false;
          auto itB = argsB.begin();
          for (const auto &itA: argsA) {
            if (*itA != **itB)
              return false;
            ++itB;
          }
          return true;
        }
        case OType::BCODE: {
          auto instsA = this->bcode_value();
          auto instsB = other.bcode_value();
          if (instsA.size() != instsB.size())
            return false;
          auto itB = instsB.begin();
          for (const auto &itA: instsA) {
            if (*itA != **itB)
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
    bool isBool() const { return this->o_range() == OType::BOOL; }
    bool isInt() const { return this->o_range() == OType::INT; }
    bool isReal() const { return this->o_range() == OType::REAL; }
    bool isUri() const { return this->o_range() == OType::URI; }
    bool isStr() const { return this->o_range() == OType::STR; }
    bool isRec() const { return this->o_range() == OType::REC; }
    bool isInst() const { return this->o_range() == OType::INST; }
    bool isBytecode() const { return this->o_range() == OType::BCODE; }
    Obj_p apply(const Obj_p &lhs) {
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
        case OType::REC:
          return shared_from_this();
        case OType::INST:
          return this->inst_f()(lhs);
        case OType::BCODE: {
          ptr<Obj> currentObj = lhs;
          for (const Inst_p &currentInst: this->bcode_value()) {
            if (currentInst->isNoObj() || currentObj->isNoObj())
              break;
            // LOG(DEBUG, "Applying %s => %s\n", currentObj->toString().c_str(), currentInst->toString().c_str());
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
    const Obj_p
    split(const Any &newValue,
          const std::variant<ptr<fURI>, const char *> &newType = std::variant<ptr<fURI>, const char *>(nullptr)) const {
      const Obj temp = Obj(newValue, this->_furi);
      if (nullptr != std::get<const char *>(newType)) {
        return temp.as(std::holds_alternative<const char *>(newType)
                           ? ptr<fURI>(new fURI(std::get<const char *>(newType)))
                           : std::get<ptr<fURI>>(newType));
      }
      return share(temp);
    }

    Obj_p as(const fURI_p &furi) const {
      return share(Obj(this->_value, share(this->_furi->resolve(furi->toString().c_str()))));
    }
    Obj_p as(const char *furi) const { return this->as(share(fURI(furi))); }

    const Inst_p nextInst(Inst_p currentInst) const {
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
    static Obj_p to_noobj() { return share(Obj(nullptr, NOOBJ_FURI)); }

    static Obj_p to_bool(const bool value, const fURI_p &furi = BOOL_FURI) {
      assert(furi->path(0, 1) == OTYPE_STR.at(OType::BOOL));
      return share(Obj(value, furi));
    }

    static Obj_p to_int(const FL_INT_TYPE value, const fURI_p &furi = INT_FURI) {
      assert(furi->path(0, 1) == OTYPE_STR.at(OType::INT));
      return share(Obj(value, furi));
    }

    static Obj_p to_real(const FL_REAL_TYPE value, const fURI_p &furi = REAL_FURI) {
      assert(furi->path(0, 1) == OTYPE_STR.at(OType::REAL));
      return share(Obj(value, furi));
    }

    static Obj_p to_str(const string &value, const fURI_p &furi = STR_FURI) {
      assert(furi->path(0, 1) == OTYPE_STR.at(OType::STR));
      return share(Obj(value, furi));
    }

    static Obj_p to_str(const char *value, const fURI_p &furi = STR_FURI) {
      assert(furi->path(0, 1) == OTYPE_STR.at(OType::STR));
      return share(Obj(string(value), furi));
    }

    static Obj_p to_uri(const fURI &value, const fURI_p &furi = URI_FURI) {
      assert(furi->path(0, 1) == OTYPE_STR.at(OType::URI));
      return share(Obj(value, furi));
    }

    static Obj_p to_uri(const char *value, const fURI_p &furi = URI_FURI) {
      assert(furi->path(0, 1) == OTYPE_STR.at(OType::URI));
      return share(Obj(value, furi));
    }

    static Obj_p to_rec(const RecMap<> &map, const fURI_p &furi = REC_FURI) {
      assert(furi->path(0, 1) == OTYPE_STR.at(OType::REC));
      return share(Obj(map, furi));
    }

    static Obj_p to_rec(const std::initializer_list<Pair<const Obj, Obj>> &xrec, const fURI_p &furi = REC_FURI) {
      assert(furi->path(0, 1) == OTYPE_STR.at(OType::REC));
      RecMap<> map = RecMap<>();
      for (const auto &pair: xrec) {
        map.insert(make_pair(share(pair.first), share(pair.second)));
      }
      return share(Rec(map, furi));
    }

    static Obj_p to_inst(const InstValue &value, const fURI_p &furi = INST_FURI) {
      assert(furi->path(0, 1) == OTYPE_STR.at(OType::INST));
      return share(Obj(value, furi));
    }

    static Obj_p to_inst(const string opcode, const List<Obj_p> &args, const InstFunction &function,
                         const fURI_p &furi = nullptr) {
      const fURI_p fix = !furi ? share(fURI(string("/inst/") + opcode)) : furi;
      assert(fix->path(0, 1) == OTYPE_STR.at(OType::INST));
      return share(Obj(std::make_pair(args, function), fix));
    }

    static Obj_p to_bcode(const List<Obj_p> &insts, const fURI_p &furi = BCODE_FURI) {
      assert(furi->path(0, 1) == OTYPE_STR.at(OType::BCODE));
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

    static List<Obj_p> cast(const List<Obj> &list) {
      List<Obj_p> newList = List<Obj_p>();
      for (const auto &obj: list) {
        newList.push_back(share(Obj(obj)));
      }
      return newList;
    }
  };
  static Uri u(const char *uri) { return Uri(fURI(uri)); }

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
