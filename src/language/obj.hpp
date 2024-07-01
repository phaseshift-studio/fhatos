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
#include <unordered_map>
#include <util/mutex_deque.hpp>
#endif
#ifndef FL_INT_TYPE
#define FL_INT_TYPE int
#endif

#include <fhatos.hpp>
#include <structure/furi.hpp>
#include <util/ptr_helper.hpp>
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
    BCODE,
    /// A valueless obj
    TYPE
  };


  class Obj;
  using Obj_p = ptr<Obj>;
  using NoObj = Obj;
  using NoObj_p = Obj_p;
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
  using Type = Obj;
  using Type_p = Obj_p;
  //
  using BObj = Pair<uint32_t, fbyte *>;
  class PtrSerializer;
  // Inst structures
  using InstFunction = Function<Obj_p, Obj_p>;
  using InstArgs = List<ptr<Obj>>;
  using InstOpcode = string;
  using InstValue = Pair<InstArgs, InstFunction>;
  class LocalRouter;

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
                                                      {OType::BCODE, "bcode"},
                                                      {OType::TYPE, "type"}}};
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
                                                {"bcode", OType::BCODE},
                                                {"type", OType::TYPE}}};
  static const Map<OType, fURI_p> OTYPE_FURI = {{{OType::NOOBJ, share(fURI("/noobj/"))},
                                                 {OType::NOINST, share(fURI("/noinst/"))},
                                                 {OType::OBJ, share(fURI("/obj/"))},
                                                 {OType::OBJS, share(fURI("/objs/"))},
                                                 {OType::URI, share(fURI("/uri/"))},
                                                 {OType::BOOL, share(fURI("/bool/"))},
                                                 {OType::INT, share(fURI("/int/"))},
                                                 {OType::REAL, share(fURI("/real/"))},
                                                 {OType::STR, share(fURI("/str/"))},
                                                 {OType::LST, share(fURI("/lst/"))},
                                                 {OType::REC, share(fURI("/rec/"))},
                                                 {OType::INST, share(fURI("/inst/"))},
                                                 {OType::BCODE, share(fURI("/bcode/"))},
                                                 {OType::TYPE, share(fURI("/type/"))}}};

  static const fURI_p OBJ_FURI = fURI_p(new fURI("/obj/"));
  static const fURI_p NOOBJ_FURI = fURI_p(new fURI("/noobj/"));
  static const fURI_p TYPE_FURI = fURI_p(new fURI("/type/"));
  static const fURI_p BOOL_FURI = fURI_p(new fURI("/bool/"));
  static const fURI_p INT_FURI = fURI_p(new fURI("/int/"));
  static const fURI_p REAL_FURI = fURI_p(new fURI("/real/"));
  static const fURI_p URI_FURI = fURI_p(new fURI("/uri/"));
  static const fURI_p STR_FURI = fURI_p(new fURI("/str/"));
  static const fURI_p REC_FURI = fURI_p(new fURI("/rec/"));
  static const fURI_p INST_FURI = fURI_p(new fURI("/inst/"));
  static const fURI_p BCODE_FURI = fURI_p(new fURI("/bcode/"));
  static const fURI_p OBJS_FURI = fURI_p(new fURI("/objs/"));

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
  class Obj;
  static TriFunction<const Obj &, const OType, const fURI &, bool> TYPE_CHECKER = [](const Obj &, const OType,
                                                                                     const fURI &) { return true; };
  //////////////////////////////////////////////////
  ////////////////////// OBJ //////////////////////
  /////////////////////////////////////////////////
  /// An mm-ADT abstract object from which all other types derive
  class Obj : public IDed /*, public std::enable_shared_from_this<Obj>*/ {
  protected:
    Any _value;
    //////////////////////////////////////////
  public:
    struct obj_hash {
      size_t operator()(const Obj_p &obj) const { return obj->hash(); }
    };

    struct obj_comp : public std::less<> {
      template<class K1 = Obj, class K2 = Obj>
      auto operator()(K1 &k1, K2 &k2) const {
        return k1.hash() < k2.hash();
      }
    };

    struct obj_equal_to : std::binary_function<Obj_p &, Obj_p &, bool> {
      bool operator()(const Obj_p &a, const Obj_p &b) const { return *a == *b; }
    };

    template<typename K = ptr<Obj>, typename V = ptr<Obj>, typename H = obj_hash, typename Q = obj_equal_to>
    using RecMap = OrderedMap<K, V, H, Q>;
    template<typename K = ptr<Obj>, typename V = ptr<Obj>, typename H = obj_hash, typename Q = obj_equal_to>
    using RecMap_p = ptr<RecMap<K, V, H, Q>>;
    using InstList = List<Inst_p>;
    using InstList_p = ptr<InstList>;
    virtual ~Obj() override = default;
    explicit Obj(Any value, const OType otype, const fURI &typeId) :
        IDed(OTYPE_FURI.at(otype)), _value(std::move(value)) {
      if (TYPE_CHECKER(*this, otype, typeId))
        this->_id = share(ID(typeId));
      else {
        throw fError("Obj %s is not a %s\n", this->toString().c_str(), typeId.toString().c_str());
      }
    }
    explicit Obj(const Any &value, const fURI_p &typeId) : Obj(value, STR_OTYPE.at(typeId->path(0, 1)), *typeId) {}
    /////
    static fError TYPE_ERROR(const Obj *obj, const int lineNumber = __LINE__) {
      // if(true) exit(1);
      return fError("Unexpected internal value for %s [%s]\n", OTYPE_STR.at(obj->o_type()),
                    (string(__FILE__) + ":" + std::to_string(lineNumber)).c_str());
    }
    //////////////////////////////////////////////////////////////
    //// IMPLICIT CONVERSIONS (FOR NATIVE C++ CONSTRUCTIONS) ////
    //////////////////////////////////////////////////////////////
    template<class T, class = typename std::enable_if_t<std::is_same_v<bool, T>>>
    Obj(const T xbool, const char *typeId = "") :
        Obj(Any(xbool), OType::BOOL, OTYPE_FURI.at(OType::BOOL)->resolve(typeId)) {}
    Obj(const FL_INT_TYPE xint, const char *typeId = "") :
        Obj(Any(xint), OType::INT, OTYPE_FURI.at(OType::INT)->resolve(typeId)) {}
    Obj(const FL_REAL_TYPE xreal, const char *typeId = "") :
        Obj(Any(xreal), OType::REAL, OTYPE_FURI.at(OType::REAL)->resolve(typeId)) {}
    Obj(const fURI &xuri, const char *typeId = "") :
        Obj(Any(xuri), OType::URI, OTYPE_FURI.at(OType::URI)->resolve(typeId)) {}
    Obj(const char *xstr, const char *typeId = "") :
        Obj(Any(string(xstr)), OType::STR, OTYPE_FURI.at(OType::STR)->resolve(typeId)) {}
    Obj(const string &xstr, const char *typeId = "") :
        Obj(Any(xstr), OType::STR, OTYPE_FURI.at(OType::STR)->resolve(typeId)) {}
    Obj(const std::initializer_list<Pair<const Obj, Obj>> &xrec, const char *typeId = "") :
        Obj(Any(share(RecMap<>())), OType::REC, OTYPE_FURI.at(OType::REC)->resolve(typeId)) {
      auto map = std::any_cast<ptr<RecMap<>>>(this->_value);
      for (const auto &[key, val]: xrec) {
        map->insert(make_pair(share(Obj(key)), share(Obj(val))));
      }
      // this->_value = map;
    }
    Obj(const List<Inst> &bcode, const char *typeId = "") :
        Obj(Obj::cast(bcode), OType::BCODE, OTYPE_FURI.at(OType::BCODE)->resolve(typeId)) {}
    Obj(const InstList &bcode, const char *typeId = "") :
        Obj(Any(bcode), OType::BCODE, OTYPE_FURI.at(OType::BCODE)->resolve(typeId)) {}

    /*Obj(const List<Obj_p> &objList) : IDed(OBJS_FURI), _value(objList) {
      if (objList.empty())
        throw fError("Obj type can not be deduced from list contents. Construct with fURI specified.");
      else {
        if (objList.front()->o_type() == OType::INST)
          this->_id = share(ID(*BCODE_FURI));
        else
          this->_id = share(ID(*OBJS_FURI));
      }
    }*/
    //////////////////////////////////////////////////////////////
    OType o_type() const { return STR_OTYPE.at(this->_id->path(0, 1)); }
    template<typename VALUE>
    const VALUE value() const {
      return std::any_cast<VALUE>(this->_value);
    }
    List<Obj_p> objs_value() const {
      if (this->o_type() != OType::OBJS)
        throw TYPE_ERROR(this, __LINE__);
      return this->value<List<Obj_p>>();
    }
    const bool bool_value() const {
      if (this->o_type() != OType::BOOL)
        throw TYPE_ERROR(this, __LINE__);
      return this->value<bool>();
    }
    const FL_INT_TYPE int_value() const {
      if (this->o_type() != OType::INT)
        throw TYPE_ERROR(this, __LINE__);
      return this->value<FL_INT_TYPE>();
    }
    const FL_REAL_TYPE real_value() const {
      if (this->o_type() != OType::REAL)
        throw TYPE_ERROR(this, __LINE__);
      return this->value<FL_REAL_TYPE>();
    }
    const fURI uri_value() const {
      if (this->o_type() != OType::URI)
        throw TYPE_ERROR(this, __LINE__);
      return this->value<fURI>();
    }
    const string str_value() const {
      if (this->o_type() != OType::STR)
        throw TYPE_ERROR(this, __LINE__);
      return this->value<string>();
    }
    RecMap_p<> rec_value() const {
      if (this->o_type() != OType::REC)
        throw TYPE_ERROR(this, __LINE__);
      return this->value<RecMap_p<>>();
    }
    Obj_p rec_get(const Obj_p &key) const {
      return this->rec_value()->count(key) ? this->rec_value()->at(key) : Obj::to_noobj();
    }
    Obj_p rec_get(const Obj &key) const { return Obj::rec_get(share(key)); }
    void rec_set(const Obj_p &key, const Obj_p &val) const {
      this->rec_value()->erase(key);
      if (!val->isNoObj())
        this->rec_value()->insert({key, val});
    }
    void rec_set(const Obj &key, const Obj &value) const { Obj::rec_set(share(key), share(value)); }
    void rec_delete(const Obj &key) const { Obj::rec_set(share(key), Obj::to_noobj()); }
    const InstValue inst_value() const {
      if (this->o_type() != OType::INST)
        throw TYPE_ERROR(this, __LINE__);
      return this->value<InstValue>();
    }
    const string inst_op() const {
      if (this->o_type() != OType::INST)
        throw TYPE_ERROR(this, __LINE__);
      return this->_id->lastSegment();
    }
    const List<Obj_p> inst_args() const { return this->inst_value().first; }
    Obj_p inst_arg(const uint8_t index) const { return this->inst_value().first.at(index); }

    const InstFunction inst_f() const { return this->inst_value().second; }
    List<Obj_p> bcode_value() const {
      if (this->o_type() == OType::NOOBJ)
        return {};
      if (this->o_type() != OType::BCODE)
        throw TYPE_ERROR(this, __LINE__);
      return this->value<List<Obj_p>>();
    }

    fURI_p bcode_domain() const { return this->bcode_value().empty() ? OBJ_FURI : this->bcode_value().front()->id(); }

    fURI_p bcode_range() const { return this->bcode_value().empty() ? OBJ_FURI : this->bcode_value().back()->id(); }

    const size_t hash() const { return std::hash<std::string>{}(this->toString()); }

    const string toString(const bool includeType = true, const bool ansi = true) const {
      string objString;
      switch (this->o_type()) {
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
          for (const auto &[k, v]: *this->rec_value()) {
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
        case OType::OBJS: {
          objString += "!m<!!";
          bool first = true;
          for (const auto &obj: this->objs_value()) {
            if (first) {
              first = false;
            } else {
              objString += "!m,!!";
            }
            objString += obj->toString();
          };
          objString += "!m>!!";
          break;
        }
        default:
          throw fError("Unknown obj type in toString(): %s\n", OTYPE_STR.at(this->o_type()));
      }
      objString =
          includeType
              ? (this->_id->pathLength() > 1 && !this->_id->lastSegment().empty()
                     ? (this->_id->user()->empty() ? "" : ("!b" + this->_id->user().value() + "!g@!b/!!")) +
                           ("!b" + this->_id->lastSegment() + "!g[!!" + objString + "!g]!!")
                     : (this->_id->user()->empty() ? "" : ("!b" + this->_id->user().value() + "!g@!!")) + objString)
              : objString;
      return ansi ? objString : Ansi<FOS_DEFAULT_PRINTER>::singleton()->strip(objString.c_str());
    }
    int compare(const Obj &rhs) const { return this->toString().compare(rhs.toString()); }
    // operator const Obj_p &() { return shared_from_this(); }
    bool operator&&(const Obj &rhs) const { return this->bool_value() && rhs.bool_value(); }
    bool operator||(const Obj &rhs) const { return this->bool_value() || rhs.bool_value(); }
    bool operator>(const Obj &rhs) const {
      switch (this->o_type()) {
        case OType::NOOBJ:
          return false;
        case OType::INT:
          return this->int_value() > rhs.int_value();
        case OType::REAL:
          return this->real_value() > rhs.real_value();
        case OType::URI:
          return this->uri_value().toString() > rhs.uri_value().toString();
        case OType::STR:
          return this->str_value() > rhs.str_value();
        default:
          throw fError("Unknown obj type in >: %s\n", OTYPE_STR.at(this->o_type()));
      }
    }
    bool operator<(const Obj &rhs) const {
      switch (this->o_type()) {
        case OType::NOOBJ:
          return false;
        case OType::INT:
          return this->int_value() < rhs.int_value();
        case OType::REAL:
          return this->real_value() < rhs.real_value();
        case OType::URI:
          return this->uri_value().toString() < rhs.uri_value().toString();
        case OType::STR:
          return this->str_value() < rhs.str_value();
        default:
          throw fError("Unknown obj type in >: %s\n", OTYPE_STR.at(this->o_type()));
      }
    }
    bool operator<=(const Obj &rhs) const { return *this == rhs || *this < rhs; }
    bool operator>=(const Obj &rhs) const { return *this == rhs || *this > rhs; }
    Obj operator*(const Obj &rhs) const {
      switch (this->o_type()) {
        case OType::NOOBJ:
          return *Obj::to_noobj();
        case OType::BOOL:
          return Obj(this->bool_value() && rhs.bool_value(), this->id());
        case OType::INT:
          return Obj(this->int_value() * rhs.int_value(), this->id());
        case OType::REAL:
          return Obj(this->real_value() * rhs.real_value(), this->id());
        // case OType::URI:
        //   return Obj(this->uri_value().extend(rhs.uri_value().path().c_str()), this->id());
        //   case OType::STR:
        //   return Obj(this->str_value() + rhs.str_value(), this->id());
        case OType::REC: {
          RecMap_p<> map = ptr<RecMap<>>(new RecMap<>());
          auto itB = rhs.rec_value()->begin();
          for (auto itA = this->rec_value()->begin(); itA != this->rec_value()->end(); ++itA) {
            map->insert(std::make_pair(share(Obj(*itA->first * *itB->first, itA->first->id())),
                                       share(Obj(*itA->second * *itB->second, itA->second->id()))));
            ++itB;
          }
          return Rec(map, this->id());
        }
        default:
          throw fError("Unknown obj type in +: %s\n", OTYPE_STR.at(this->o_type()));
      }
    }
    Obj operator+(const Obj &rhs) const {
      switch (this->o_type()) {
        case OType::NOOBJ:
          return *Obj::to_noobj();
        case OType::BOOL:
          return Obj(this->bool_value() || rhs.bool_value(), this->id());
        case OType::INT:
          return Obj(this->int_value() + rhs.int_value(), this->id());
        case OType::REAL:
          return Obj(this->real_value() + rhs.real_value(), this->id());
        case OType::URI:
          return Obj(fURI(this->uri_value()).extend(fURI(rhs.uri_value()).toString().c_str()), this->id());
        case OType::STR:
          return Obj(string(this->str_value()) + string(rhs.str_value()), this->id());
        case OType::REC: {
          RecMap_p<> map = ptr<RecMap<>>(new RecMap<>());
          for (const auto &pair: *this->rec_value()) {
            map->insert(pair);
          }
          for (const auto &pair: *rhs.rec_value()) {
            map->insert(pair);
          }
          return Obj(map, this->id());
        }
        default:
          throw fError("Unknown obj type in +: %s\n", OTYPE_STR.at(this->o_type()));
      }
    }
    Obj operator%(const Obj &other) const { return Obj(this->int_value() % other.int_value(), this->id()); }
    bool operator!=(const Obj &other) const { return !(*this == other); }
    bool operator==(const Obj &other) const {
      if (!this->_id->equals(*other._id)) // type check
        return false;
      switch (this->o_type()) {
        case OType::NOOBJ:
          return other.isNoObj();
        case OType::BOOL:
          return this->bool_value() == other.bool_value();
        case OType::INT:
          return this->int_value() == other.int_value();
        case OType::REAL:
          return this->real_value() == other.real_value();
        case OType::URI:
          return this->uri_value().equals(other.uri_value());
        case OType::STR:
          return this->str_value() == other.str_value();
        case OType::REC: {
          auto pairsA = this->rec_value();
          auto pairsB = other.rec_value();
          if (pairsA->size() != pairsB->size())
            return false;
          auto itB = pairsB->begin();
          for (const auto &itA: *pairsA) {
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
          throw fError("Unknown obj type in ==: %s\n", OTYPE_STR.at(this->o_type()));
      }
    }
    bool isNoObj() const { return this->o_type() == OType::NOOBJ; }
    bool isBool() const { return this->o_type() == OType::BOOL; }
    bool isInt() const { return this->o_type() == OType::INT; }
    bool isReal() const { return this->o_type() == OType::REAL; }
    bool isUri() const { return this->o_type() == OType::URI; }
    bool isStr() const { return this->o_type() == OType::STR; }
    bool isRec() const { return this->o_type() == OType::REC; }
    bool isInst() const { return this->o_type() == OType::INST; }
    bool isObjs() const { return this->o_type() == OType::OBJS; }
    bool isBytecode() const { return this->o_type() == OType::BCODE; }
    bool isNoOpBytecode() const { return this->o_type() == OType::BCODE && this->bcode_value().empty(); }
    Obj_p apply(const Obj_p &lhs) {
      /*if(lhs.isType() && (!this->isBytecode() && !this->isInst()) ) {
        Types<>::verifyType(PtrHelper::no_delete<Obj>(this),lhs->_furi);

      }*/
      switch (this->o_type()) {
        case OType::BOOL:
          return PtrHelper::no_delete<Bool>(this);
        case OType::INT:
          return PtrHelper::no_delete<Int>(this);
        case OType::REAL:
          return PtrHelper::no_delete<Real>(this);
        case OType::URI:
          return PtrHelper::no_delete<Uri>(this);
        case OType::STR:
          return PtrHelper::no_delete<Str>(this);
        case OType::REC:
          return PtrHelper::no_delete<Rec>(this);
        case OType::INST: {
          /*if (lhs->isBytecode()) {
            List<Inst_p> list = List<Inst_p>(lhs->bcode_value());
            list.push_back(shared_from_this());
            return lhs->split(list);
          } else {*/
          return this->inst_f()(lhs);
          //  }
        }
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
          throw fError("Unknown obj type in apply(): %s\n", OTYPE_STR.at(this->o_type()));
      }
    }
    const Obj_p
    split(const Any &newValue,
          const std::variant<ptr<fURI>, const char *> &newType = std::variant<ptr<fURI>, const char *>(nullptr)) const {
      const Obj temp = Obj(newValue, this->id());
      if (nullptr != std::get<const char *>(newType)) {
        return temp.as(std::holds_alternative<const char *>(newType)
                           ? ptr<fURI>(new fURI(std::get<const char *>(newType)))
                           : std::get<ptr<fURI>>(newType));
      }
      return share(temp);
    }

    const fURI type() const { return this->_id->authority(""); }

    const bool match(const Obj_p &pattern, const bool sameType = true) const {
      if (pattern->isNoOpBytecode())
        return true;
      if (pattern->isBytecode() && !this->isBytecode())
        return !pattern->apply(PtrHelper::no_delete<Obj>((Obj *) this))->isNoObj();
      if (sameType && (this->type() != pattern->type()))
        return false;
      switch (this->o_type()) {
        case OType::NOOBJ:
          return true;
        case OType::BOOL:
          return *this == *pattern;
        case OType::INT:
          return *this == *pattern;
        case OType::REAL:
          return *this == *pattern;
        case OType::URI:
          return *this == *pattern;
        case OType::STR:
          return *this == *pattern;
        case OType::REC: {
          auto pairsA = this->rec_value();
          auto pairsB = pattern->rec_value();
          if (pairsA->size() != pairsB->size())
            return false;
          auto itB = pairsB->begin();
          for (const auto &itA: *pairsA) {
            LOG(DEBUG_MORE, "MATCHING: %s vs. %s\n", itA.second->toString().c_str(), itB->second->toString().c_str());
            if (!itA.first->match(itB->first) || !itA.second->match(itB->second))
              return false;
            ++itB;
          }
          return true;
        }
        case OType::INST: {
          auto argsA = this->inst_args();
          auto argsB = pattern->inst_args();
          if (argsA.size() != argsB.size())
            return false;
          auto itB = argsB.begin();
          for (const auto &itA: argsA) {
            if (!itA->match(*itB))
              return false;
          }
          return true;
        }
        case OType::BCODE: {
          auto instsA = this->bcode_value();
          auto instsB = pattern->bcode_value();
          if (instsA.size() != instsB.size())
            return false;
          auto itB = instsB.begin();
          for (const auto &itA: instsA) {
            if (!itA->match(*itB))
              return false;
          }
          return true;
        }
        default:
          throw fError("Unknown obj type in match(): %s\n", OTYPE_STR.at(this->o_type()));
      }
      return false;
    }

    Obj_p as(const fURI_p &furi) const { return this->as(furi->toString().c_str()); }
    Obj_p as(const char *furi) const { return share(Obj(this->_value, this->o_type(), this->_id->resolve(furi))); }

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

    static Bool_p to_bool(const bool value, const fURI_p &furi = BOOL_FURI) {
      assert(furi->path(0, 1) == OTYPE_STR.at(OType::BOOL));
      return share(Obj(value, furi));
    }

    static Int_p to_int(const FL_INT_TYPE value, const fURI_p &furi = INT_FURI) {
      assert(furi->path(0, 1) == OTYPE_STR.at(OType::INT));
      return share(Obj(value, furi));
    }

    static Real_p to_real(const FL_REAL_TYPE value, const fURI_p &furi = REAL_FURI) {
      assert(furi->path(0, 1) == OTYPE_STR.at(OType::REAL));
      return share(Obj(value, furi));
    }

    static Str_p to_str(const string &value, const fURI_p &furi = STR_FURI) {
      assert(furi->path(0, 1) == OTYPE_STR.at(OType::STR));
      return share(Obj(value, furi));
    }

    static Str_p to_str(const char *value, const fURI_p &furi = STR_FURI) {
      assert(furi->path(0, 1) == OTYPE_STR.at(OType::STR));
      return share(Obj(string(value), furi));
    }

    static Uri_p to_uri(const fURI &value, const fURI_p &furi = URI_FURI) {
      assert(furi->path(0, 1) == OTYPE_STR.at(OType::URI));
      return share(Obj(value, furi));
    }

    static Uri_p to_uri(const char *value, const fURI_p &furi = URI_FURI) {
      assert(furi->path(0, 1) == OTYPE_STR.at(OType::URI));
      return share(Obj(value, furi));
    }

    static Rec_p to_rec(const RecMap_p<> &map, const fURI_p &furi = REC_FURI) {
      assert(furi->path(0, 1) == OTYPE_STR.at(OType::REC));
      return share(Obj(map, furi));
    }

    static Rec_p to_rec(const std::initializer_list<Pair<const Obj, Obj>> &xrec, const fURI_p &furi = REC_FURI) {
      RecMap<> map = RecMap<>();
      for (const auto &pair: xrec) {
        map.insert(make_pair(share(pair.first), share(pair.second)));
      }
      return to_rec(share(map), furi);
    }

    static Inst_p to_inst(const InstValue &value, const fURI_p &furi = INST_FURI) {
      assert(furi->path(0, 1) == OTYPE_STR.at(OType::INST));
      return share(Obj(value, furi));
    }

    static Inst_p to_inst(const string opcode, const List<Obj_p> &args, const InstFunction &function,
                          const fURI_p &furi = nullptr) {
      const fURI_p fix = !furi ? share(fURI(string("/inst/") + opcode)) : furi;
      return to_inst(std::make_pair(args, function), fix);
    }

    static BCode_p to_bcode(const List<Obj_p> &insts, const fURI_p &furi = BCODE_FURI) {
      assert(furi->path(0, 1) == OTYPE_STR.at(OType::BCODE));
      return share(Obj(insts, furi));
    }

    const ptr<BObj> serialize() const {
      auto *bytes = static_cast<fbyte *>(malloc(sizeof(*this)));
      memcpy(bytes, reinterpret_cast<const fbyte *>(this), sizeof(*this));
      return share(BObj{sizeof(*this), bytes});
    }

    template<typename OBJ>
    static const ptr<OBJ> deserialize(const ptr<BObj> bobj) {
      return ptr<OBJ>(new Obj(*((OBJ *) bobj->second)));
    }

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


#endif
