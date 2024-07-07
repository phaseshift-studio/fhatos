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
#pragma once
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
    /// A sequence of instructions denoting a program
    BCODE,
    /// A valueless obj
    TYPE
  };
  static const Enums<OType> OTypes = Enums<OType>({{OType::OBJ, "obj"},
                                                   {OType::NOOBJ, "noobj"},
                                                   {OType::OBJS, "objs"},
                                                   {OType::BOOL, "bool"},
                                                   {OType::INT, "int"},
                                                   {OType::REAL, "real"},
                                                   {OType::URI, "uri"},
                                                   {OType::STR, "str"},
                                                   {OType::LST, "lst"},
                                                   {OType::REC, "rec"},
                                                   {OType::INST, "inst"},
                                                   {OType::BCODE, "bcode"},
                                                   {OType::TYPE, "type"}});

  class Obj;
  using Obj_p = ptr<Obj>;
  using NoObj = Obj;
  using NoObj_p = Obj_p;
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
  using Lst = Obj;
  using Lst_p = Obj_p;
  using Inst = Obj;
  using Inst_p = Obj_p;
  using BCode = Obj;
  using BCode_p = Obj_p;
  using Objs = Obj;
  using Objs_p = Obj_p;
  using Type = Obj;
  using Type_p = Obj_p;
  using BObj = Pair<uint32_t, fbyte *>;
  enum class IType : uint8_t {
    ZERO_TO_ZERO,
    ZERO_TO_ONE,
    ZERO_TO_MANY,
    ONE_TO_ZERO,
    MANY_TO_ZERO,
    ONE_TO_ONE,
    ONE_TO_MANY,
    MANY_TO_ONE,
    MANY_TO_MANY,
  }; // TYPE
  static const Map<IType, const char *> ITYPE_STR = {{
      {IType::ZERO_TO_ZERO, "0->0 (Ø)"},
      {IType::ZERO_TO_ONE, "f(Ø)->y (supplier)"},
      {IType::ZERO_TO_MANY, "f(Ø)->y* (source)"},
      {IType::ONE_TO_ZERO, "f(x)->Ø (consumer)"},
      {IType::MANY_TO_ZERO, "f(x*)->Ø (terminal)"},
      {IType::ONE_TO_ONE, "f(x)->y (map)"},
      {IType::ONE_TO_MANY, "f(x)->y* (flatmap)"},
      {IType::MANY_TO_ONE, "f(x*)->y (reduce)"},
      {IType::MANY_TO_MANY, "f(x*)->y* (barrier)"},
  }};
  //
  using InstFunction = Function<Obj_p, Obj_p>;
  using InstArgs = List<ptr<Obj>>;
  using InstOpcode = string;
  using InstArgs = List<Obj_p>;
  using InstFunction = Function<Obj_p, Obj_p>;
  using InstSeed = Obj_p;
  using InstValue = Quad<InstArgs, InstFunction, IType, InstSeed>;
  using InstList = List<Inst_p>;
  using InstList_p = ptr<InstList>;
  static const fURI_p OBJ_FURI = fURI_p(new fURI("/obj/"));
  static const fURI_p NOOBJ_FURI = fURI_p(new fURI("/noobj/"));
  static const fURI_p TYPE_FURI = fURI_p(new fURI("/type/"));
  static const fURI_p BOOL_FURI = fURI_p(new fURI("/bool/"));
  static const fURI_p INT_FURI = fURI_p(new fURI("/int/"));
  static const fURI_p REAL_FURI = fURI_p(new fURI("/real/"));
  static const fURI_p URI_FURI = fURI_p(new fURI("/uri/"));
  static const fURI_p STR_FURI = fURI_p(new fURI("/str/"));
  static const fURI_p LST_FURI = fURI_p(new fURI("/lst/"));
  static const fURI_p REC_FURI = fURI_p(new fURI("/rec/"));
  static const fURI_p INST_FURI = fURI_p(new fURI("/inst/"));
  static const fURI_p BCODE_FURI = fURI_p(new fURI("/bcode/"));
  static const fURI_p OBJS_FURI = fURI_p(new fURI("/objs/"));
  static const Map<OType, fURI_p> OTYPE_FURI = {{{OType::NOOBJ, NOOBJ_FURI},
                                                 {OType::OBJ, OBJ_FURI},
                                                 {OType::OBJS, OBJS_FURI},
                                                 {OType::URI, URI_FURI},
                                                 {OType::BOOL, BOOL_FURI},
                                                 {OType::INT, INT_FURI},
                                                 {OType::REAL, REAL_FURI},
                                                 {OType::STR, STR_FURI},
                                                 {OType::LST, LST_FURI},
                                                 {OType::REC, REC_FURI},
                                                 {OType::INST, INST_FURI},
                                                 {OType::BCODE, BCODE_FURI},
                                                 {OType::TYPE, TYPE_FURI}}};
  static TriFunction<const Obj &, const OType, const fURI &, ID_p> TYPE_CHECKER = [](const Obj &, const OType,
                                                                                     const fURI &) { return nullptr; };
  static Function<const string, Type_p> TYPE_PARSER = [](const string &) { return nullptr; };
  static BiFunction<const fURI, Type_p, Type_p> TYPE_SAVER = [](const fURI &, const Type_p &) { return nullptr; };
  //////////////////////////////////////////////////
  ////////////////////// OBJ //////////////////////
  /////////////////////////////////////////////////
  /// An mm-ADT abstract object from which all other types derive
  class Obj : public IDed, public std::enable_shared_from_this<Obj> {
  protected:
    Any _value;
    //////////////////////////////////////////
  public:
    struct obj_hash {
      size_t operator()(const Obj_p &obj) const { return obj->hash(); }
    };

    struct obj_comp : std::less<> {
      template<class K1 = Obj, class K2 = Obj>
      auto operator()(K1 &k1, K2 &k2) const {
        return k1.hash() < k2.hash();
      }
    };

    struct obj_equal_to : std::binary_function<Obj_p &, Obj_p &, bool> {
      bool operator()(const Obj_p &a, const Obj_p &b) const { return *a == *b; }
    };
    template<typename V = Obj_p>
    using LstList = List<V>;
    template<typename V = Obj_p>
    using LstList_p = List_p<V>;
    template<typename K = Obj_p, typename V = Obj_p, typename H = obj_hash, typename Q = obj_equal_to>
    using RecMap = OrderedMap<K, V, H, Q>;
    template<typename K = Obj_p, typename V = Obj_p, typename H = obj_hash, typename Q = obj_equal_to>
    using RecMap_p = ptr<RecMap<K, V, H, Q>>;

    ~Obj() override = default;
    explicit Obj(const Any value, const OType otype, const fURI &typeId) :
        IDed(OTYPE_FURI.at(otype)), _value(std::move(value)) {
      TYPE_CHECKER(*this, otype, typeId);
      this->_id = share(ID(typeId));
    }
    explicit Obj(const Any &value, const fURI_p &typeId) :
        Obj(value, OTypes.toEnum(typeId->path(0, 1).c_str()), *typeId) {}
    /////
    static fError TYPE_ERROR(const Obj *obj, const char *function, const int lineNumber = __LINE__) {
      // if(true) exit(1);
      return fError("%s[%s] unexpectedly acccessed for %s [line:%i]\n", OTypes.toChars(obj->o_type()),
                    obj->toString().c_str(), function, lineNumber);
    }
    //////////////////////////////////////////////////////////////
    //// IMPLICIT CONVERSIONS (FOR NATIVE C++ CONSTRUCTIONS) ////
    //////////////////////////////////////////////////////////////
    template<class T, class = typename std::enable_if_t<std::is_same_v<bool, T>>>
    Obj(const T xbool, const char *typeId = "") : Obj(Any(xbool), OType::BOOL, BOOL_FURI->resolve(typeId)) {}
    Obj(const FL_INT_TYPE xint, const char *typeId = "") : Obj(Any(xint), OType::INT, INT_FURI->resolve(typeId)) {}
    Obj(const FL_REAL_TYPE xreal, const char *typeId = "") : Obj(Any(xreal), OType::REAL, REAL_FURI->resolve(typeId)) {}
    Obj(const fURI &xuri, const char *typeId = "") : Obj(Any(xuri), OType::URI, URI_FURI->resolve(typeId)) {}
    Obj(const char *xstr, const char *typeId = "") : Obj(Any(string(xstr)), OType::STR, STR_FURI->resolve(typeId)) {}
    Obj(const string &xstr, const char *typeId = "") : Obj(Any(xstr), OType::STR, STR_FURI->resolve(typeId)) {}
    Obj(const std::initializer_list<Pair<const Obj, Obj>> &xrec, const char *typeId = "") :
        Obj(Any(share(RecMap<>())), OType::REC, REC_FURI->resolve(typeId)) {
      auto map = this->value<RecMap<>>();
      for (const auto &[key, val]: xrec) {
        map.insert(make_pair(share(Obj(key)), share(Obj(val))));
      }
    }
    Obj(const std::initializer_list<Obj> &xlst, const char *typeId = "") :
        Obj(Any(share(LstList<>())), OType::LST, LST_FURI->resolve(typeId)) {
      auto list = this->value<LstList_p<>>();
      for (const auto &obj: xlst) {
        list->push_back(share(Obj(obj)));
      }
    }
    Obj(const List<Inst> &bcode, const char *typeId = "") :
        Obj(Any(PtrHelper::clone(bcode)), OType::BCODE, BCODE_FURI->resolve(typeId)) {}
    Obj(const InstList &bcode, const char *typeId = "") : Obj(Any(bcode), OType::BCODE, BCODE_FURI->resolve(typeId)) {}

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
    OType o_type() const { return OTypes.toEnum(this->_id->path(0, 1).c_str()); }
    template<typename VALUE>
    const VALUE value() const {
      try {
        return std::any_cast<VALUE>(this->_value);
      } catch (const std::bad_any_cast &) {
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      }
    }
    List_p<Obj_p> objs_value() const {
      if (!this->isObjs())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->value<List_p<Obj_p>>();
    }
    const bool bool_value() const {
      if (!this->isBool())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->value<bool>();
    }
    const FL_INT_TYPE int_value() const {
      if (!this->isInt())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->value<FL_INT_TYPE>();
    }
    const FL_REAL_TYPE real_value() const {
      if (!this->isReal())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->value<FL_REAL_TYPE>();
    }
    const fURI uri_value() const {
      if (!this->isUri())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->value<fURI>();
    }
    const string str_value() const {
      if (!this->isStr())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->value<string>();
    }
    LstList_p<> lst_value() const {
      if (!this->isLst())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->value<LstList_p<>>();
    }

    Obj_p lst_get(const Int_p &index) const {
      if (!this->isLst())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->lst_value()->at(index->int_value());
    }

    void lst_set(const Int_p &index, const Obj_p &obj) const {
      if (!this->isLst())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      this->lst_value()->insert(this->lst_value()->begin() + index->int_value(), obj);
    }

    RecMap_p<> rec_value() const {
      if (!this->isRec())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
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
      if (!this->isInst())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->value<InstValue>();
    }
    const string inst_op() const {
      if (!this->isInst())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->_id->lastSegment();
    }
    const InstArgs inst_args() const { return std::get<0>(this->inst_value()); }
    Obj_p inst_arg(const uint8_t index) const { return std::get<0>(this->inst_value()).at(index); }

    const InstFunction inst_f() const { return std::get<1>(this->inst_value()); }
    const IType inst_itype() const { return std::get<2>(this->inst_value()); }
    const Obj_p inst_seed() const { return std::get<3>(this->inst_value()); }
    List<Obj_p> bcode_value() const {
      if (this->isNoObj())
        return {};
      if (!this->isBytecode())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->value<List<Obj_p>>();
    }
    const BCode_p add_inst(const Inst_p &inst, const bool mutate = true) {
      if (!this->isBytecode())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      if (mutate) {
        List<Inst_p> insts = bcode_value();
        insts.push_back(inst);
        return Obj::to_bcode(insts);
      } else {
        List<Inst_p> insts = {};
        for (const auto &i: this->bcode_value()) {
          insts.push_back(i);
        }
        insts.push_back(inst);
        return Obj::to_bcode(insts);
      }
    }

    const BCode_p add_bcode(const BCode_p &bcode, const bool mutate = true) {
      if (!this->isBytecode() || !bcode->isBytecode())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      List<Inst_p> insts = {};
      for (const auto &inst: bcode->bcode_value()) {
        insts.push_back(inst);
      }
      return Obj::to_bcode(insts);
    }

    fURI_p bcode_domain() const { return OBJ_FURI; }

    fURI_p bcode_range() const { return OBJ_FURI; }

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
          objString = "!_" + this->uri_value().toString() + "!!";
          break;
        case OType::STR:
          objString = "!m'!!" + this->str_value() + "!m'!!";
          break;
        case OType::LST: {
          objString = "!m[!!";
          bool first = true;
          for (const auto &obj: *this->lst_value()) {
            if (first) {
              first = false;
            } else {
              objString += "!m,!!";
            }
            objString += obj->toString();
          }
          objString += "!m]!!";
          break;
        }
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
          for (const auto &arg: this->inst_args()) {
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
            //objString += "!b" + this->bcode_range()->name() + "!g<=!b" + this->bcode_domain()->name() + "!g[!!";
            bool first = true;
            for (const auto &inst: this->bcode_value()) {
              if (first) {
                first = false;
              } else {
                objString += "!g.!!";
              }
              objString += inst->toString();
            }
            //objString += "!g]!!";
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
          for (const auto &obj: *this->objs_value()) {
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
          throw fError("Unknown obj type in toString(): %s\n", OTypes.toChars(this->o_type()));
      }
      objString =
          includeType
              ? (this->_id->pathLength() > 1 && !this->_id->lastSegment().empty()
                     ? (this->_id->user()->empty() ? "" : ("!b" + this->_id->user().value() + "!g@!b/!!")) +
                           ("!b" + this->_id->lastSegment() + "!g[!!" + objString + "!g]!!")
                     : (this->_id->user()->empty() ? "" : ("!b" + this->_id->user().value() + "!g@!!")) + objString)
              : objString;
      return ansi ? objString : GLOBAL_OPTIONS->printer<>()->strip(objString.c_str());
    }
    int compare(const Obj &rhs) const { return this->toString().compare(rhs.toString()); }
    // operator const Obj_p &() { return shared_from_this(); }
    bool operator&&(const Obj &rhs) const {
      if (this->isBool() && rhs.isBool())
        return this->bool_value() && rhs.bool_value();
      throw fError("Unknown obj type in &&: %s\n", OTypes.toChars(this->o_type()));
    }
    bool operator||(const Obj &rhs) const {
      if (this->isBool() && rhs.isBool())
        return this->bool_value() || rhs.bool_value();
      throw fError("Unknown obj type in ||: %s\n", OTypes.toChars(this->o_type()));
    }
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
          throw fError("Unknown obj type in >: %s\n", OTypes.toChars(this->o_type()));
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
          throw fError("Unknown obj type in >: %s\n", OTypes.toChars(this->o_type()));
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
        case OType::URI:
          return Obj(fURI(this->uri_value()).resolve(fURI(rhs.uri_value()).toString().c_str()), this->id());
        //   case OType::STR:
        //   return Obj(this->str_value() + rhs.str_value(), this->id());
        case OType::LST: {
          LstList_p<> list = LstList_p<>(new LstList<>());
          auto itB = rhs.lst_value()->begin();
          for (auto itA = this->lst_value()->begin(); itA != this->lst_value()->end(); ++itA) {
            list->push_back(share(Obj(**itA * **itB, itA->get()->id())));
            ++itB;
          }
          return Lst(list, this->id());
        }
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
        /*case OType::BCODE: {
          if (rhs.isInst()) {
            return *PtrHelper::no_delete<Obj>((Obj *) this)->add_inst(share(rhs), true);
          } else if (rhs.isBytecode()) {
            return *PtrHelper::no_delete<Obj>((Obj*)this)->add_bcode(share(rhs),true);
           }
        }*/
        default:
          throw fError("Unknown obj type in +: %s\n", OTypes.toChars(this->o_type()));
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
        case OType::LST: {
          LstList_p<> list = LstList_p<>(new LstList<>());
          for (const auto &obj: *this->lst_value()) {
            list->push_back(obj);
          }
          for (const auto &obj: *rhs.lst_value()) {
            list->push_back(obj);
          }
          return Lst(list, this->id());
        }
        case OType::REC: {
          RecMap_p<> map = ptr<RecMap<>>(new RecMap<>());
          for (const auto &pair: *this->rec_value()) {
            map->insert(pair);
          }
          for (const auto &pair: *rhs.rec_value()) {
            map->insert(pair);
          }
          return Rec(map, this->id());
        }
        default:
          throw fError("Unknown obj type in +: %s\n", OTypes.toChars(this->o_type()));
      }
    }
    Obj operator-(const Obj &rhs) const {
      switch (this->o_type()) {
        case OType::NOOBJ:
          return *Obj::to_noobj();
        case OType::BOOL:
          return Bool(!this->bool_value(), this->id());
        case OType::INT:
          return Int(this->int_value() - rhs.int_value(), this->id());
        case OType::REAL:
          return Real(this->real_value() - rhs.real_value(), this->id());
        case OType::URI:
          return Uri(fURI(this->uri_value()).retract(), this->id());
          // case OType::STR:
          //  return Obj(string(this->str_value()).replace(string(rhs.str_value()), this->id());
        case OType::LST: {
          LstList_p<> list = LstList_p<>(new LstList<>());
          for (const auto &obj: *this->lst_value()) {
            if (std::find(rhs.lst_value()->begin(), rhs.lst_value()->end(), obj) != std::end(*rhs.lst_value()))
              list->push_back(obj);
          }
          return Lst(list, this->id());
        }
        case OType::REC: {
          RecMap_p<> map = ptr<RecMap<>>(new RecMap<>());
          for (const auto &pair: *this->rec_value()) {
            map->insert(pair);
          }
          for (const auto &pair: *rhs.rec_value()) {
            map->insert(pair);
          }
          return Rec(map, this->id());
        }
        default:
          throw fError("Unknown obj type in +: %s\n", OTypes.toChars(this->o_type()));
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
        case OType::LST: {
          auto objsA = this->lst_value();
          auto objsB = other.lst_value();
          if (objsA->size() != objsB->size())
            return false;
          auto itB = objsB->begin();
          for (const auto &itA: *objsA) {
            if (*itA != **itB || *itA != **itB)
              return false;
            ++itB;
          }
          return true;
        }
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
          if (this->inst_itype() != other.inst_itype())
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
        case OType::OBJS: {
          auto objsA = this->objs_value();
          auto objsB = other.objs_value();
          if (objsA->size() != objsB->size())
            return false;
          auto itB = objsB->begin();
          for (const auto &itA: *objsA) {
            if (*itA != **itB)
              return false;
            ++itB;
          }
          return true;
        }
        default:
          throw fError("Unknown obj type in ==: %s\n", OTypes.toChars(this->o_type()));
      }
    }
    Obj operator[](Obj &key) const {
      switch (this->o_type()) {
        case OType::STR:
          return Str(this->str_value()[key.int_value()]);
        case OType::LST:
          return *this->lst_get(share(key));
        case OType::REC:
          return *this->rec_get(share(key));
        default:
          throw fError("Unknown obj type in []: %s\n", OTypes.toChars(this->o_type()));
      }
    }
    bool isNoObj() const { return this->o_type() == OType::NOOBJ; }
    bool isBool() const { return this->o_type() == OType::BOOL; }
    bool isInt() const { return this->o_type() == OType::INT; }
    bool isReal() const { return this->o_type() == OType::REAL; }
    bool isUri() const { return this->o_type() == OType::URI; }
    bool isStr() const { return this->o_type() == OType::STR; }
    bool isLst() const { return this->o_type() == OType::LST; }
    bool isRec() const { return this->o_type() == OType::REC; }
    bool isInst() const { return this->o_type() == OType::INST; }
    bool isObjs() const { return this->o_type() == OType::OBJS; }
    bool isBytecode() const { return this->o_type() == OType::BCODE; }
    bool isNoOpBytecode() const { return this->o_type() == OType::BCODE && this->bcode_value().empty(); }
    bool isType() const { return !this->_value.has_value(); }
    Obj_p apply(const Obj_p &lhs) {
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
        case OType::LST:
          return PtrHelper::no_delete<Lst>(this);
        case OType::REC:
          return PtrHelper::no_delete<Rec>(this);
        case OType::INST: {
          if (lhs->isBytecode()) {
            List<Obj_p> newArgs = List<Obj_p>();
            for (const auto &arg: this->inst_args()) {
              newArgs.push_back(arg->apply(lhs));
            }
            // Quad<InstArgs, InstFunction, IType, InstSeed>;
            return lhs->add_inst(
                Obj::to_inst(InstValue(newArgs, this->inst_f(), this->inst_itype(), this->inst_seed()), this->_id),
                false);
          } else
            return this->inst_f()(lhs);
        }
        case OType::BCODE: {
          if (lhs->isBytecode())
            return lhs->add_bcode(this->shared_from_this(), true);
          ptr<Obj> currentObj = lhs;
          for (const Inst_p &currentInst: this->bcode_value()) {
            LOG(TRACE, "Applying %s => %s\n", currentObj->toString().c_str(), currentInst->toString().c_str());
            currentObj = currentInst->apply(currentObj);
            if (currentObj->isNoObj())
              break;
          }
          return currentObj; //(currentObj->type() == OType::URI) ? relativeUri((ptr<Uri>currentObj) : currentObj;
        }
        case OType::NOOBJ:
          return Obj::to_noobj();
        default:
          throw fError("Unknown obj type in apply(): %s\n", OTypes.toChars(this->o_type()));
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
        case OType::LST: {

          auto objsA = this->lst_value();
          auto objsB = pattern->lst_value();
          if (objsA->size() != objsB->size())
            return false;
          auto itB = objsB->begin();
          for (const auto &itA: *objsA) {
            LOG(TRACE, "MATCHING: %s vs. %s\n", itA->toString().c_str(), (*itB)->toString().c_str());
            if (!itA->match(*itB) || !itA->match(*itB))
              return false;
            ++itB;
          }
          return true;
        }
        case OType::REC: {
          auto pairsA = this->rec_value();
          auto pairsB = pattern->rec_value();
          if (pairsA->size() != pairsB->size())
            return false;
          auto itB = pairsB->begin();
          for (const auto &itA: *pairsA) {
            LOG(TRACE, "MATCHING: %s vs. %s\n", itA.second->toString().c_str(), itB->second->toString().c_str());
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
          if (this->inst_itype() != pattern->inst_itype())
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
          throw fError("Unknown obj type in match(): %s\n", OTypes.toChars(this->o_type()));
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
      assert(furi->path(0, 1) == OTypes.toChars(OType::BOOL));
      return share(Obj(value, furi));
    }

    static Int_p to_int(const FL_INT_TYPE value, const fURI_p &furi = INT_FURI) {
      assert(furi->path(0, 1) == OTypes.toChars(OType::INT));
      return share(Obj(value, furi));
    }

    static Real_p to_real(const FL_REAL_TYPE value, const fURI_p &furi = REAL_FURI) {
      assert(furi->path(0, 1) == OTypes.toChars(OType::REAL));
      return share(Obj(value, furi));
    }

    static Str_p to_str(const string &value, const fURI_p &furi = STR_FURI) {
      assert(furi->path(0, 1) == OTypes.toChars(OType::STR));
      return share(Obj(Any(value), furi));
    }

    static Str_p to_str(const char *value, const fURI_p &furi = STR_FURI) {
      assert(furi->path(0, 1) == OTypes.toChars(OType::STR));
      return share(Obj(Any(string(value)), furi));
    }

    static Uri_p to_uri(const fURI &value, const fURI_p &furi = URI_FURI) {
      assert(furi->path(0, 1) == OTypes.toChars(OType::URI));
      return share(Obj(value, furi));
    }

    static Uri_p to_uri(const char *value, const fURI_p &furi = URI_FURI) {
      assert(furi->path(0, 1) == OTypes.toChars(OType::URI));
      return share(Obj(value, furi));
    }

    static Lst_p to_lst(const LstList_p<> &xlst, const fURI_p &furi = LST_FURI) {
      assert(furi->path(0, 1) == OTypes.toChars(OType::LST));
      return share(Obj(xlst, furi));
    }

    static Lst_p to_lst(const std::initializer_list<Obj> &xlst, const fURI_p &furi = LST_FURI) {
      LstList<> list = LstList<>();
      for (const auto &obj: xlst) {
        list.push_back(share(obj));
      }
      return to_lst(share(list), furi);
    }

    static Lst_p to_lst(const std::initializer_list<Obj_p> &xlst, const fURI_p &furi = LST_FURI) {
      return to_lst(share(LstList<>(xlst)), furi);
    }

    static Rec_p to_rec(const RecMap_p<> &map, const fURI_p &furi = REC_FURI) {
      assert(furi->path(0, 1) == OTypes.toChars(OType::REC));
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
      assert(furi->path(0, 1) == OTypes.toChars(OType::INST));
      return share(Inst(value, furi));
    }

    static Inst_p to_inst(const string &opcode, const List<Obj_p> &args, const InstFunction &function,
                          const IType itype, const Obj_p &seed = Obj::to_noobj(), const fURI_p &furi = nullptr) {
      const fURI_p fix = !furi ? share(fURI(string("/inst/") + opcode)) : furi;
      return to_inst({args, function, itype, seed}, fix);
    }

    static BCode_p to_bcode(const List<Inst_p> &insts, const fURI_p &furi = BCODE_FURI) {
      assert(furi->path(0, 1) == OTypes.toChars(OType::BCODE));
      return share(BCode(insts, furi));
    }

    static Objs_p to_objs(const List_p<Obj_p> &objs, const fURI_p &furi = OBJS_FURI) {
      assert(furi->path(0, 1) == OTypes.toChars(OType::OBJS));
      return share(Objs(objs, furi));
    }

    static Objs_p to_objs(const List<Obj_p> &objs, const fURI_p &furi = OBJS_FURI) {
      return Obj::to_objs(share(objs), furi);
    }

    static Objs_p to_objs(const List<Obj> &objs, const fURI_p &furi = OBJS_FURI) {
      return Obj::to_objs(share(PtrHelper::clone(objs)), furi);
    }

    ptr<BObj> serialize() const {
      auto *bytes = static_cast<fbyte *>(malloc(sizeof(*this)));
      memcpy(bytes, reinterpret_cast<const fbyte *>(this), sizeof(*this));
      return share(BObj{sizeof(*this), bytes});
    }

    template<typename OBJ>
    static ptr<OBJ> deserialize(const ptr<BObj> bobj) {
      ptr<OBJ> obj = ptr<OBJ>(new Obj(*((OBJ *) bobj->second)));
      return obj;
    }
  };
  static Uri u(const char *uri) { return Uri(fURI(uri)); }
  static ptr<Uri> u_p(const char *uri) { return share(Uri(fURI(uri))); }
  static ptr<Obj> o_p(const Obj &obj) { return share(obj); }

} // namespace fhatos


#endif
