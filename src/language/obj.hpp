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

#endif
#ifndef FL_INT_TYPE
#define FL_INT_TYPE int
#endif

#define FOS_TYPE_PREFIX "/type/"
#define FOS_BASE_TYPE_INDEX 1

#include <fhatos.hpp>
#include <util/ptr_helper.hpp>
#include <utility>
#include <variant>
#include "furi.hpp"

namespace fhatos {
  /// @brief The base types of mm-ADT
  enum class OType : uint8_t {
    /// The base type of all types is the obj
    OBJ,
    /// A "null" object type used to kill a processing monad
    NOOBJ,
    OBJS,
    /// A boolean monotype
    BOOL,
    /// An integral number monotype in Z
    INT,
    /// A real number mono-type in R
    REAL,
    /// A string monotype denoting a sequence of characters where a "char" is equivalent to str[0]
    STR,
    /// A Uniform Resource Identifier monotype
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

  static const Enums<OType> OTypes = Enums<OType>({
    {OType::OBJ, "obj"},
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
    {OType::TYPE, "type"}
  });

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
  using Poly = Obj;
  using Poly_p = Obj_p;
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
  using BObj_p = ptr<Pair<uint32_t, fbyte *>>;

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
  static Consumer<BObj *> bobj_deleter = [](const BObj *bobj) {
    free(bobj->second);
    delete bobj;
  };
  static const Enums<IType> ITypeDomains = Enums<IType>({
    {IType::ZERO_TO_ZERO, "."},
    {IType::ZERO_TO_ONE, "."},
    {IType::ZERO_TO_MANY, "."},
    {IType::ONE_TO_ZERO, "o"},
    {IType::MANY_TO_ZERO, "O"},
    {IType::ONE_TO_ONE, "o"},
    {IType::ONE_TO_MANY, "o"},
    {IType::MANY_TO_ONE, "O"},
    {IType::MANY_TO_MANY, "O"}
  });
  static const Enums<IType> ITypeRanges = Enums<IType>({
    {IType::ZERO_TO_ZERO, "."},
    {IType::ZERO_TO_ONE, "o"},
    {IType::ZERO_TO_MANY, "O"},
    {IType::ONE_TO_ZERO, "."},
    {IType::MANY_TO_ZERO, "."},
    {IType::ONE_TO_ONE, "o"},
    {IType::ONE_TO_MANY, "O"},
    {IType::MANY_TO_ONE, "o"},
    {IType::MANY_TO_MANY, "O"}
  });
  static const Enums<IType> ITypeSignatures = Enums<IType>({
    {IType::ZERO_TO_ZERO, ".->."},
    {IType::ZERO_TO_ONE, ".->o"},
    {IType::ZERO_TO_MANY, ".->O"},
    {IType::ONE_TO_ZERO, "o->."},
    {IType::MANY_TO_ZERO, "O->."},
    {IType::ONE_TO_ONE, "o->o"},
    {IType::ONE_TO_MANY, "o->O"},
    {IType::MANY_TO_ONE, "O->o"},
    {IType::MANY_TO_MANY, "O->O"}
  });
  static const Enums<IType> ITypeDescriptions = Enums<IType>({
    {IType::ZERO_TO_ZERO, "Ø->Ø (transient)"},
    {IType::ZERO_TO_ONE, "Ø->o (supplier)"},
    {IType::ZERO_TO_MANY, "Ø->Œ (source)"},
    {IType::ONE_TO_ZERO, "o->Ø (consumer)"},
    {IType::MANY_TO_ZERO, "Œ->Ø (terminal)"},
    {IType::ONE_TO_ONE, "o->o (map)"},
    {IType::ONE_TO_MANY, "o->Œ (flatmap)"},
    {IType::MANY_TO_ONE, "Œ->o (reduce)"},
    {IType::MANY_TO_MANY, "Œ->Œ (barrier)"},
  });
  //

  using InstOpcode = string;
  using InstArgs = List<Obj_p>;
  using InstFunction = Function<Obj_p, Obj_p>;
  using InstFunctionSupplier = Function<InstArgs, InstFunction>;
  using InstSeedSupplier = Function<Obj_p, Obj_p>;
  using InstSeed = Obj_p;
  using InstValue = Quad<InstArgs, InstFunctionSupplier, IType, InstSeedSupplier>;
  using InstList = List<Inst_p>;
  using InstList_p = ptr<InstList>;
  static const ID_p OBJ_FURI = share<ID>(ID(FOS_TYPE_PREFIX "obj/"));
  static const ID_p NOOBJ_FURI = share<ID>(ID(FOS_TYPE_PREFIX "noobj/"));
  static const ID_p TYPE_FURI = share<ID>(ID(FOS_TYPE_PREFIX "type/"));
  static const ID_p BOOL_FURI = share<ID>(ID(FOS_TYPE_PREFIX "bool/"));
  static const ID_p INT_FURI = share<ID>(ID(FOS_TYPE_PREFIX "int/"));
  static const ID_p REAL_FURI = share<ID>(ID(FOS_TYPE_PREFIX "real/"));
  static const ID_p URI_FURI = share<ID>(ID(FOS_TYPE_PREFIX "uri/"));
  static const ID_p STR_FURI = share<ID>(ID(FOS_TYPE_PREFIX "str/"));
  static const ID_p LST_FURI = share<ID>(ID(FOS_TYPE_PREFIX "lst/"));
  static const ID_p REC_FURI = share<ID>(ID(FOS_TYPE_PREFIX "rec/"));
  static const ID_p INST_FURI = share<ID>(ID(FOS_TYPE_PREFIX "inst/"));
  static const ID_p BCODE_FURI = share<ID>(ID(FOS_TYPE_PREFIX "bcode/"));
  static const ID_p OBJS_FURI = share<ID>(ID(FOS_TYPE_PREFIX "objs/"));
  static const Map<OType, ID_p> OTYPE_FURI = {
    {
      {OType::NOOBJ, NOOBJ_FURI},
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
      {OType::TYPE, TYPE_FURI}
    }
  };
  static TriFunction<const Obj &, const OType, const ID_p &, ID_p> TYPE_CHECKER = [](const Obj &, const OType,
    const ID_p &) {
    LOG(DEBUG, "TYPE_CHECKER undefined at this point in bootstrap.\n");
    return nullptr;
  };
  static Function<const string &, Obj_p> OBJ_PARSER = [](const string &) {
    LOG(DEBUG, "OBJ_PARSER undefined at this point in bootstrap.\n");
    return nullptr;
  };
  static QuadConsumer<const ID &, const ID &, const Obj_p &, const bool> MESSAGE_INTERCEPT =
      [](const ID &, const ID &, const Obj_p &, const bool) {
    LOG(DEBUG, "MESSAGE_INTERCEPT undefined at this point in bootstrap.\n");
    return nullptr;
  };

  //////////////////////////////////////////////////
  ////////////////////// OBJ //////////////////////
  /////////////////////////////////////////////////
  /// An mm-ADT abstract object from which all other types derive
  class Obj final : public IDed, public std::enable_shared_from_this<Obj>, Function<Obj_p, Obj_p> {
  public:
    Any _value;

    struct objp_hash {
      size_t operator()(const Obj_p &obj) const { return obj->hash(); }
    };

    struct objp_equal_to : std::binary_function<Obj_p &, Obj_p &, bool> {
      bool operator()(const Obj_p &a, const Obj_p &b) const { return *a == *b; }
    };

    struct obj_comp : std::less<> {
      template<class K1 = Obj, class K2 = Obj>
      auto operator()(K1 &k1, K2 &k2) const {
        return k1.hash() < k2.hash();
      }
    };

    template<typename V = Obj_p>
    using LstList = List<V>;
    template<typename V = Obj_p>
    using LstList_p = List_p<V>;
    template<typename K = Obj_p, typename V = Obj_p, typename H = objp_hash, typename Q = objp_equal_to>
    using RecMap = OrderedMap<K, V, H, Q>;
    template<typename K = Obj_p, typename V = Obj_p, typename H = objp_hash, typename Q = objp_equal_to>
    using RecMap_p = ptr<RecMap<K, V, H, Q>>;

    explicit Obj(const Any &value, const OType otype, const ID_p &typeId) : IDed(OTYPE_FURI.at(otype)),
                                                                            _value(value) {
      TYPE_CHECKER(*this, otype, typeId);
      this->id_ = typeId;
    }

    explicit Obj(const Any &value, const ID_p &typeId) : Obj(
      value, OTypes.toEnum(typeId->path(FOS_BASE_TYPE_INDEX)), typeId) {
    }

    /////
    static fError TYPE_ERROR(const Obj *obj, const char *function,
                             [[maybe_unused]] const int lineNumber = __LINE__) {
      return fError("!b%s!g[!!%s!g]!! !yaccessed!! as !b%s!!\n", OTypes.toChars(obj->o_type()).c_str(),
                    obj->toString().c_str(), string(function).replace(string(function).find("_value"), 6, "").c_str());
    }

    //////////////////////////////////////////////////////////////
    //// IMPLICIT CONVERSIONS (FOR NATIVE C++ CONSTRUCTIONS) ////
    //////////////////////////////////////////////////////////////
    template<class T, class = std::enable_if_t<std::is_same_v<bool, T>>>
    Obj(const T xbool, const char *typeId = EMPTY_CHARS) : Obj(Any(xbool), OType::BOOL,
                                                               id_p(BOOL_FURI->resolve(typeId))) {
    }

    Obj(const FL_INT_TYPE xint, const char *typeId = EMPTY_CHARS) : Obj(
      Any(xint), OType::INT, id_p(INT_FURI->resolve(typeId))) {
    }

    Obj(const FL_REAL_TYPE xreal, const char *typeId = EMPTY_CHARS) : Obj(
      Any(xreal), OType::REAL, id_p(REAL_FURI->resolve(typeId))) {
    }

    Obj(const fURI &xuri, const char *typeId = EMPTY_CHARS) : Obj(Any(xuri), OType::URI,
                                                                  id_p(URI_FURI->resolve(typeId))) {
    }

    Obj(const char *xstr, const char *typeId = EMPTY_CHARS) : Obj(Any(string(xstr)), OType::STR,
                                                                  id_p(STR_FURI->resolve(typeId))) {
    }

    Obj(const string &xstr, const char *typeId = EMPTY_CHARS) : Obj(
      Any(xstr), OType::STR, id_p(STR_FURI->resolve(typeId))) {
    }

    Obj(const std::initializer_list<Pair<const Obj, Obj>> &xrec, const char *typeId = EMPTY_CHARS) : Obj(
      Any(share(RecMap<>())), OType::REC, id_p(REC_FURI->resolve(typeId))) {
      auto map = this->value<RecMap<>>();
      for (const auto &[key, val]: xrec) {
        map.insert(make_pair(share(Obj(key)), share(Obj(val))));
      }
    }

    Obj(const std::initializer_list<Obj> &xlst, const char *typeId = EMPTY_CHARS) : Obj(
      Any(share(LstList<>())), OType::LST, id_p(LST_FURI->resolve(typeId))) {
      auto list = this->value<LstList_p<>>();
      for (const auto &obj: xlst) {
        list->push_back(share(Obj(obj)));
      }
    }

    Obj(const List<Inst> &bcode, const char *typeId = EMPTY_CHARS) : Obj(
      Any(share<InstList>(PtrHelper::clone(bcode))), OType::BCODE, id_p(BCODE_FURI->resolve(typeId))) {
    }

    Obj(const InstList_p &bcode, const char *typeId = EMPTY_CHARS) : Obj(
      Any(bcode), OType::BCODE, id_p(BCODE_FURI->resolve(typeId))) {
    }

    //////////////////////////////////////////////////////////////
    [[nodiscard]] OType o_type() const { return OTypes.toEnum(this->id_->path(FOS_BASE_TYPE_INDEX)); }

    template<typename VALUE>
    VALUE value() const {
      try {
        return std::any_cast<VALUE>(this->_value);
      } catch (const std::bad_any_cast &) {
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      }
    }

    [[nodiscard]] List_p<Obj_p> objs_value() const {
      if (!this->is_objs())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->value<List_p<Obj_p>>();
    }

    [[nodiscard]] Obj_p objs_value(const uint16_t index) const {
      return this->objs_value()->at(index);
    }

    [[nodiscard]] bool bool_value() const {
      if (!this->is_bool())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->value<bool>();
    }

    [[nodiscard]] FL_INT_TYPE int_value() const {
      if (!this->is_int())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->value<FL_INT_TYPE>();
    }

    [[nodiscard]] FL_REAL_TYPE real_value() const {
      if (!this->is_real())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->value<FL_REAL_TYPE>();
    }

    [[nodiscard]] fURI uri_value() const {
      if (!this->is_uri())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->value<fURI>();
    }

    [[nodiscard]] string str_value() const {
      if (!this->is_str())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->value<string>();
    }

    [[nodiscard]] LstList_p<> lst_value() const {
      if (!this->is_lst())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->value<LstList_p<>>();
    }

    void lst_add(const Obj_p &obj) const {
      if (!this->is_lst())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      this->lst_value()->push_back(obj);
    }

    [[nodiscard]] Obj_p lst_get(const Int_p &index) const {
      if (!this->is_lst())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->lst_value()->at(index->int_value());
    }

    void lst_set(const Int_p &index, const Obj_p &obj) const {
      if (!this->is_lst())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      this->lst_value()->insert(this->lst_value()->begin() + index->int_value(), obj);
    }

    [[nodiscard]] RecMap_p<> rec_value() const {
      if (!this->is_rec())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->value<RecMap_p<>>();
    }

    [[nodiscard]] Obj_p rec_get(const Obj_p &key) const {
      return this->rec_value()->count(key) ? this->rec_value()->at(key) : Obj::to_noobj();
    }

    [[nodiscard]] Obj_p rec_get(const Obj &key) const { return Obj::rec_get(share(key)); }

    void rec_set(const Obj_p &key, const Obj_p &val) const {
      this->rec_value()->erase(key);
      if (!val->is_noobj())
        this->rec_value()->insert({key, val});
    }

    void rec_set(const Obj &key, const Obj &value) const { Obj::rec_set(share(key), share(value)); }

    void rec_delete(const Obj &key) const { Obj::rec_set(share(key), Obj::to_noobj()); }

    [[nodiscard]] InstValue inst_value() const {
      if (!this->is_inst())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->value<InstValue>();
    }

    [[nodiscard]] string inst_op() const {
      if (!this->is_inst())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->id_->name();
    }

    [[nodiscard]] InstArgs inst_args() const { return std::get<0>(this->inst_value()); }

    [[nodiscard]] Obj_p inst_arg(const uint8_t index) const { return std::get<0>(this->inst_value()).at(index); }

    [[nodiscard]] InstFunctionSupplier inst_f() const { return std::get<1>(this->inst_value()); }

    [[nodiscard]] InstSeedSupplier inst_seed_supplier() const { return std::get<3>(this->inst_value()); }

    [[nodiscard]] Obj_p inst_seed(const Obj_p &arg) const { return this->inst_seed_supplier()(arg); }

    Obj_p operator()(const Obj_p &input) {
      return this->apply(input);
    }

    IType itype() const {
      if (this->is_inst())
        return std::get<2>(this->inst_value());
      if (this->is_bcode()) {
        const IType domain = this->bcode_value()->front()->itype();
        const IType range = this->bcode_value()->back()->itype();
        return ITypeSignatures.toEnum(string(ITypeDomains.toChars(domain) + "->" + ITypeRanges.toChars(range)));
      }
      if (this->is_objs())
        return IType::ONE_TO_MANY;
      return IType::ONE_TO_ONE;
    }

    [[nodiscard]] InstList_p bcode_value() const {
      if (this->is_noobj())
        return {};
      if (!this->is_bcode())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->value<InstList_p>();
    }

    BCode_p add_inst(const Inst_p &inst, const bool mutate = true) const {
      if (!this->is_bcode())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      if (mutate) {
        const InstList_p insts = bcode_value();
        insts->push_back(inst);
        return Obj::to_bcode(insts);
      } else {
        InstList_p insts = share<InstList>({});
        for (const auto &i: *this->bcode_value()) {
          insts->push_back(i);
        }
        insts->push_back(inst);
        return Obj::to_bcode(insts);
      }
    }

    [[nodiscard]] BCode_p add_bcode(const BCode_p &bcode, [[maybe_unused]] const bool mutate = true) const {
      if (!this->is_bcode() || !bcode->is_bcode())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      InstList_p insts = share<InstList>({});
      for (const auto &inst: *bcode->bcode_value()) {
        insts->push_back(inst);
      }
      return Obj::to_bcode(insts);
    }

    void add_obj(const Obj_p &obj, [[maybe_unused]] const bool mutate = true) {
      if (!this->is_objs())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      if (obj->is_objs()) {
        for (const Obj_p &o: *obj->objs_value()) {
          this->add_obj(o);
        }
      } else {
        this->objs_value()->push_back(obj);
      }
    }

    [[nodiscard]] fURI_p bcode_domain() { return OBJ_FURI; }

    [[nodiscard]] fURI_p bcode_range() const { return OBJ_FURI; }

    [[nodiscard]] size_t hash() const { return std::hash<std::string>{}(this->toString()); }

    string toString(const bool includeType = true, const bool ansi = true) const {
      string objString;
      switch (this->o_type()) {
        case OType::BOOL:
          objString = this->bool_value() ? "!ytrue!!" : "!yfalse!!";
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
          objString = "!m'!!!~" + this->str_value() + "!m'!!";
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
              objString += "!m,";
            }
            objString += "!c";
            objString += k->toString(includeType, false);
            objString += "!m=>!!";
            objString += v->toString();
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
          if (this->bcode_value()->empty())
            objString = "_";
          else {
            // objString += "!b" + this->bcode_range()->name() + "!g<=!b" + this->bcode_domain()->name() + "!g[!!";
            bool first = true;
            for (const auto &inst: *this->bcode_value()) {
              if (first) {
                first = false;
              } else {
                objString += "!g.!!";
              }
              objString += inst->toString();
            }
            // objString += "!g]!!";
          }
          break;
        }
        case OType::NOOBJ: {
          objString = "!r" STR(FOS_NOOBJ_TOKEN) "!!";
          break;
        }
        case OType::OBJS: {
          objString += "!m{!!";
          bool first = true;
          for (const auto &obj: *this->objs_value()) {
            if (first) {
              first = false;
            } else {
              objString += "!m,!!";
            }
            objString += obj->toString();
          };
          objString += "!m}!!";
          break;
        }
        default:
          throw fError("Unknown obj type in toString(): %s\n", OTypes.toChars(this->o_type()).c_str());
      }
      objString = (includeType && (this->id_->path_length() > 2))
                    ? string("!b")
                    .append(this->id_->name())
                    .append(this->is_inst() ? "!g(!!" : "!g[!!")
                    .append(objString)
                    .append(this->is_inst() ? "!g)!!" : "!g]!!")
                    : objString;
      return ansi ? objString : Ansi<>::strip(objString);
    }

    [[nodiscard]] int compare(const Obj &rhs) const { return this->toString().compare(rhs.toString()); }

    // operator const Obj_p &() { return shared_from_this(); }
    bool operator&&(const Obj &rhs) const {
      if (this->is_bool() && rhs.is_bool())
        return this->bool_value() && rhs.bool_value();
      throw fError("Unknown obj type in &&: %s\n", OTypes.toChars(this->o_type()).c_str());
    }

    /*Obj_p operator*() {
      return TYPE_READER(*this->_id);
    }*/
    bool operator||(const Obj &rhs) const {
      if (this->is_bool() && rhs.is_bool())
        return this->bool_value() || rhs.bool_value();
      throw fError("Unknown obj type in ||: %s\n", OTypes.toChars(this->o_type()).c_str());
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
          throw fError("Unknown obj type in >: %s\n", OTypes.toChars(this->o_type()).c_str());
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
          throw fError("Unknown obj type in >: %s\n", OTypes.toChars(this->o_type()).c_str());
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
          auto list = std::make_shared<LstList<>>();
          auto itB = rhs.lst_value()->begin();
          for (auto itA = this->lst_value()->begin(); itA != this->lst_value()->end(); ++itA) {
            list->push_back(share(Obj(**itA * **itB, itA->get()->id())));
            ++itB;
          }
          return Lst(list, this->id());
        }
        case OType::REC: {
          auto map = std::make_shared<RecMap<>>();
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
          } else if (rhs.is_bcode()) {
            return *PtrHelper::no_delete<Obj>((Obj*)this)->add_bcode(share(rhs),true);
           }
        }*/
        default:
          throw fError("Unknown obj type in +: %s\n", OTypes.toChars(this->o_type()).c_str());
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
          return Obj(this->uri_value().extend(rhs.uri_value().toString().c_str()), this->id());
        case OType::STR:
          return Obj(string(this->str_value()) + string(rhs.str_value()), this->id());
        case OType::LST: {
          auto list = std::make_shared<LstList<>>();
          for (const auto &obj: *this->lst_value()) {
            list->push_back(obj);
          }
          for (const auto &obj: *rhs.lst_value()) {
            list->push_back(obj);
          }
          return Lst(list, this->id());
        }
        case OType::REC: {
          auto map = std::make_shared<RecMap<>>();
          for (const auto &pair: *this->rec_value()) {
            map->insert(pair);
          }
          for (const auto &pair: *rhs.rec_value()) {
            map->insert(pair);
          }
          return Rec(map, this->id());
        }
        default:
          throw fError("Unknown obj type in +: %s\n", OTypes.toChars(this->o_type()).c_str());
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
          auto list = std::make_shared<LstList<>>();
          for (const auto &obj: *this->lst_value()) {
            if (std::find(rhs.lst_value()->begin(), rhs.lst_value()->end(), obj) != std::end(
                  *rhs.lst_value()))
              list->push_back(obj);
          }
          return Lst(list, this->id());
        }
        case OType::REC: {
          auto map = std::make_shared<RecMap<>>();
          for (const auto &pair: *this->rec_value()) {
            map->insert(pair);
          }
          for (const auto &pair: *rhs.rec_value()) {
            map->insert(pair);
          }
          return Rec(map, this->id());
        }
        default:
          throw fError("Unknown obj type in +: %s\n", OTypes.toChars(this->o_type()).c_str());
      }
    }

    Obj operator%(const Obj &other) const { return Obj(this->int_value() % other.int_value(), this->id()); }

    bool operator!=(const Obj &other) const { return !(*this == other); }

    bool operator==(const Obj &other) const {
      if (!this->id_->equals(*other.id_)) // type check
        return false;
      switch (this->o_type()) {
        case OType::NOOBJ:
          return other.is_noobj();
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
          const auto objs_a = this->lst_value();
          const auto objs_b = other.lst_value();
          if (objs_a->size() != objs_b->size())
            return false;
          auto it_b = objs_b->begin();
          for (const auto &it_a: *objs_a) {
            if (*it_a != **it_b)
              return false;
            ++it_b;
          }
          return true;
        }
        case OType::REC: {
          const auto pairs_a = this->rec_value();
          const auto pairs_b = other.rec_value();
          if (pairs_a->size() != pairs_b->size())
            return false;
          auto it_b = pairs_b->begin();
          for (const auto &[first, second]: *pairs_a) {
            if (*first != *it_b->first || *second != *it_b->second)
              return false;
            ++it_b;
          }
          return true;
        }
        case OType::INST: {
          if (other.inst_op() != this->inst_op())
            return false;
          const auto args_a = this->inst_args();
          auto args_b = other.inst_args();
          if (args_a.size() != args_b.size())
            return false;
          if (this->itype() != other.itype())
            return false;
          auto it_b = args_b.begin();
          for (const auto &it_a: args_a) {
            if (*it_a != **it_b)
              return false;
            ++it_b;
          }
          return true;
        }
        case OType::BCODE: {
          const auto insts_a = this->bcode_value();
          const auto insts_b = other.bcode_value();
          if (insts_a->size() != insts_b->size())
            return false;
          auto it_b = insts_b->begin();
          for (const auto &it_a: *insts_a) {
            if (*it_a != **it_b)
              return false;
            ++it_b;
          }
          return true;
        }
        case OType::OBJS: {
          const auto objs_a = this->objs_value();
          const auto objs_b = other.objs_value();
          if (objs_a->size() != objs_b->size())
            return false;
          auto it_b = objs_b->begin();
          for (const auto &it_a: *objs_a) {
            if (*it_a != **it_b)
              return false;
            ++it_b;
          }
          return true;
        }
        default:
          throw fError("Unknown obj type in ==: %s\n", OTypes.toChars(this->o_type()).c_str());
      }
    }

    Obj operator[](const Obj &key) const {
      switch (this->o_type()) {
        case OType::STR:
          return Obj(this->str_value()[key.int_value()]);
        case OType::LST:
          return *this->lst_get(share(key));
        case OType::REC:
          return *this->rec_get(share(key));
        default:
          throw fError("Unknown obj type in []: %s\n", OTypes.toChars(this->o_type()).c_str());
      }
    }

    [[nodiscard]] bool is_noobj() const { return this->o_type() == OType::NOOBJ; }

    [[nodiscard]] bool is_bool() const { return this->o_type() == OType::BOOL; }

    [[nodiscard]] bool is_int() const { return this->o_type() == OType::INT; }

    [[nodiscard]] bool is_real() const { return this->o_type() == OType::REAL; }

    [[nodiscard]] bool is_uri() const { return this->o_type() == OType::URI; }

    [[nodiscard]] bool is_str() const { return this->o_type() == OType::STR; }

    [[nodiscard]] bool is_lst() const { return this->o_type() == OType::LST; }

    [[nodiscard]] bool is_rec() const { return this->o_type() == OType::REC; }

    [[nodiscard]] bool is_inst() const { return this->o_type() == OType::INST; }

    [[nodiscard]] bool is_objs() const { return this->o_type() == OType::OBJS; }

    [[nodiscard]] bool is_bcode() const { return this->o_type() == OType::BCODE; }

    [[nodiscard]] bool is_noop_bcode() const { return this->is_bcode() && this->bcode_value()->empty(); }

    Obj_p apply(const Obj_p &lhs) {
      switch (this->o_type()) {
        case OType::BOOL:
          return PtrHelper::no_delete<Bool>(this);
        case OType::INT:
          return PtrHelper::no_delete<Int>(this);
        case OType::REAL:
          return PtrHelper::no_delete<Real>(this);
        case OType::URI: {
          return (lhs->is_uri() && this->uri_value().is_relative())
                   ? Obj::to_uri(lhs->uri_value().resolve(this->uri_value()))
                   : PtrHelper::no_delete<Uri>(this);
        }
        case OType::STR:
          return PtrHelper::no_delete<Str>(this);
        case OType::LST: {
          const LstList_p<Obj_p> new_values = share(LstList<Obj_p>());
          for (const auto &obj: *this->lst_value()) {
            new_values->push_back(obj->apply(lhs));
          }
          return Obj::to_lst(new_values);
        }
        case OType::REC: {
          RecMap_p<> new_pairs = share(RecMap<>());
          for (const auto &[key, value]: *this->rec_value()) {
            const Obj_p key_apply = key->apply(lhs);
            new_pairs->insert({key_apply, value->apply(key_apply)});
          }
          return Obj::to_rec(new_pairs, this->id_);
        }
        case OType::INST: {
          return this->inst_f()(this->inst_args())(lhs);
        }
        case OType::BCODE: {
          //if (lhs->is_bcode())
          //  return lhs->add_bcode(PtrHelper::no_delete(this), true);
          ptr<Obj> current_obj = lhs;
          for (const Inst_p &current_inst: *this->bcode_value()) {
            LOG(TRACE, "Applying %s => %s\n", current_obj->toString().c_str(),
                current_inst->toString().c_str());
            if (current_inst->is_noobj())
              break;
            current_obj = current_inst->apply(current_obj);
          }
          //const Objs_p objs = Options::singleton()->processor<Obj, BCode, Obj>(lhs, PtrHelper::no_delete(this));
          // return objs->objs_value()->empty() ? Obj::to_noobj() : objs->objs_value()->front();
          return current_obj;
        }
        case OType::OBJS: {
          Objs_p objs = Obj::to_objs();
          for (const Obj_p &obj: *this->objs_value()) {
            objs->objs_value()->push_back(obj->apply(lhs));
          }
          return objs;
        }
        case OType::NOOBJ:
          return Obj::to_noobj();
        default:
          throw fError("Unknown obj type in apply(): %s\n", OTypes.toChars(this->o_type()).c_str());
      }
    }

    // const fURI type() const { return this->_id->authority(""); }

    [[nodiscard]] bool match(const Obj_p &type, const bool sameType = true) const {
      LOG(TRACE, "!ymatching!!: %s ~ %s\n", this->toString().c_str(), type->toString().c_str());
      if (type->is_noop_bcode())
        return true;
      if (type->is_bcode() && !this->is_bcode())
        return !type->apply(PtrHelper::no_delete<Obj>(const_cast<Obj *>(this)))->is_noobj();
      if (this->o_type() != type->o_type())
        return false;
      if (sameType && (*this->id() != *type->id()))
        return false;
      switch (this->o_type()) {
        case OType::NOOBJ:
          return true;
        case OType::BOOL:
          return this->bool_value() == type->bool_value();
        case OType::INT:
          return this->int_value() == type->int_value();
        case OType::REAL:
          return this->real_value() == type->real_value();
        case OType::URI:
          return this->uri_value().matches(type->uri_value());
        case OType::STR:
          return this->str_value() == type->str_value();
        case OType::LST: {
          auto objsA = this->lst_value();
          auto objsB = type->lst_value();
          if (objsA->size() != objsB->size())
            return false;
          auto itB = objsB->begin();
          for (const auto &itA: *objsA) {
            if (!itA->match(*itB))
              return false;
            ++itB;
          }
          return true;
        }
        case OType::REC: {
          auto pairsA = this->rec_value();
          auto pairsB = type->rec_value();
          if (pairsA->size() != pairsB->size())
            return false;
          auto itB = pairsB->begin();
          for (const auto &itA: *pairsA) {
            if (!itA.first->match(itB->first) || !itA.second->match(itB->second))
              return false;
            ++itB;
          }
          return true;
        }
        case OType::INST: {
          auto argsA = this->inst_args();
          auto argsB = type->inst_args();
          if (argsA.size() != argsB.size())
            return false;
          if (this->itype() != type->itype())
            return false;
          auto itB = argsB.begin();
          for (const auto &itA: argsA) {
            if (!itA->match(*itB))
              return false;
          }
          return true;
        }
        case OType::BCODE: {
          const auto instsA = this->bcode_value();
          const auto instsB = type->bcode_value();
          if (instsA->size() != instsB->size())
            return false;
          const auto itB = instsB->begin();
          for (const auto &itA: *instsA) {
            if (!itA->match(*itB))
              return false;
          }
          return true;
        }
        default:
          throw fError("Unknown obj type in match(): %s\n", OTypes.toChars(this->o_type()).c_str());
      }
      return false;
    }

    [[nodiscard]] Obj_p as(const ID_p &furi) const {
      const ID_p resolution = share(ID(this->id_->resolve(*furi)));
      return share<Obj>(Obj(this->_value, OTypes.toEnum(resolution->path(FOS_BASE_TYPE_INDEX)), resolution));
    }

    Obj_p as(const char *furi) const { return this->as(id_p(furi)); }

    [[nodiscard]] Inst_p nextInst(const Inst_p &currentInst) const {
      if (currentInst == nullptr)
        return this->bcode_value()->front();
      if (currentInst->is_noobj())
        return currentInst;
      bool found = false;
      for (const auto &inst: *this->bcode_value()) {
        if (found)
          return inst;
        if (inst == currentInst)
          found = true;
      }
      return Obj::to_noobj();
    }

    /// STATIC TYPE CONSTRAINED CONSTRUCTORS
    static Obj_p to_noobj() {
      static Obj_p noobj = share(Obj(Any(nullptr), NOOBJ_FURI));
      return noobj;
    }

    static Bool_p to_bool(const bool value, const ID_p &furi = BOOL_FURI) {
      fError::OTYPE_CHECK(furi->path(FOS_BASE_TYPE_INDEX), OTypes.toChars(OType::BOOL));
      return share(Obj(value, furi));
    }

    static Int_p to_int(const FL_INT_TYPE value, const ID_p &furi = INT_FURI) {
      fError::OTYPE_CHECK(furi->path(FOS_BASE_TYPE_INDEX), OTypes.toChars(OType::INT));
      return share(Obj(value, furi));
    }

    static Real_p to_real(const FL_REAL_TYPE value, const ID_p &furi = REAL_FURI) {
      fError::OTYPE_CHECK(furi->path(FOS_BASE_TYPE_INDEX), OTypes.toChars(OType::REAL));
      return share(Obj(value, furi));
    }

    static Str_p to_str(const string &value, const ID_p &furi = STR_FURI) {
      fError::OTYPE_CHECK(furi->path(FOS_BASE_TYPE_INDEX), OTypes.toChars(OType::STR));
      return share(Obj(value, furi));
    }

    static Str_p to_str(const char *value, const ID_p &furi = STR_FURI) {
      fError::OTYPE_CHECK(furi->path(FOS_BASE_TYPE_INDEX), OTypes.toChars(OType::STR));
      return share(Obj(string(value), furi));
    }

    static Uri_p to_uri(const fURI &value, const ID_p &furi = URI_FURI) {
      fError::OTYPE_CHECK(furi->path(FOS_BASE_TYPE_INDEX), OTypes.toChars(OType::URI));
      return share(Obj(value, furi));
    }

    static Uri_p to_uri(const char *value, const ID_p &furi = URI_FURI) {
      fError::OTYPE_CHECK(furi->path(FOS_BASE_TYPE_INDEX), OTypes.toChars(OType::URI));
      return share(Obj(fURI(value), furi));
    }

    static Lst_p to_lst(const ID_p &furi = LST_FURI) {
      fError::OTYPE_CHECK(furi->path(FOS_BASE_TYPE_INDEX), OTypes.toChars(OType::LST));
      return share(Obj(share(LstList<>()), furi));
    }

    static Lst_p to_lst(const LstList_p<> &xlst, const ID_p &furi = LST_FURI) {
      fError::OTYPE_CHECK(furi->path(FOS_BASE_TYPE_INDEX), OTypes.toChars(OType::LST));
      return share(Obj(xlst, furi));
    }

    static Lst_p to_lst(const std::initializer_list<Obj> &xlst, const ID_p &furi = LST_FURI) {
      LstList_p<> list = share(LstList<>());
      for (const auto &obj: xlst) {
        list->push_back(share(obj));
      }
      return to_lst(list, furi);
    }

    static Lst_p to_lst(const std::initializer_list<Obj_p> &xlst, const ID_p &furi = LST_FURI) {
      return to_lst(share(LstList<>(xlst)), furi);
    }

    static Rec_p to_rec(const RecMap_p<> &map, const ID_p &furi = REC_FURI) {
      fError::OTYPE_CHECK(furi->path(FOS_BASE_TYPE_INDEX), OTypes.toChars(OType::REC));
      return share(Obj(map, furi));
    }

    static Rec_p to_rec(const std::initializer_list<Pair<const Obj, Obj>> &xrec, const ID_p &furi = REC_FURI) {
      RecMap<> map = RecMap<>();
      for (const auto &pair: xrec) {
        map.insert(make_pair(share(pair.first), share(pair.second)));
      }
      return to_rec(share(map), furi);
    }

    static Rec_p to_rec(const std::initializer_list<Pair<const Obj_p, Obj_p>> &xrec, const ID_p &furi = REC_FURI) {
      RecMap<> map = RecMap<>();
      for (const auto &pair: xrec) {
        map.insert(make_pair(pair.first, pair.second));
      }
      return to_rec(share(map), furi);
    }

    static Inst_p to_inst(const InstValue &value, const ID_p &furi = INST_FURI) {
      fError::OTYPE_CHECK(furi->path(FOS_BASE_TYPE_INDEX), OTypes.toChars(OType::INST));
      return share(Inst(value, furi));
    }

    static Inst_p to_inst(const string &opcode, const List<Obj_p> &args, const InstFunctionSupplier &function,
                          const IType itype,
                          const InstSeedSupplier &seed = [](const Obj_p &) { return Obj::to_noobj(); },
                          const ID_p &furi = nullptr) {
      const ID_p fix = furi ? furi : share(ID(string(FOS_TYPE_PREFIX "inst/") + opcode));
      return to_inst({args, function, itype, seed}, fix);
    }

    static BCode_p to_bcode(const InstList &insts, const ID_p &furi = BCODE_FURI) {
      return Obj::to_bcode(share(insts), furi);
    }

    static BCode_p to_bcode(const ID_p &furi = BCODE_FURI) { return Obj::to_bcode(share<InstList>({}), furi); }

    static BCode_p to_bcode(const InstList_p &insts, const ID_p &furi = BCODE_FURI) {
      fError::OTYPE_CHECK(furi->path(FOS_BASE_TYPE_INDEX), OTypes.toChars(OType::BCODE));
      return share(BCode(insts, furi));
    }

    static Objs_p to_objs(const ID_p &furi = OBJS_FURI) {
      fError::OTYPE_CHECK(furi->path(FOS_BASE_TYPE_INDEX), OTypes.toChars(OType::OBJS));
      return to_objs(share(List<Obj_p>()), furi);
    }

    static InstSeedSupplier objs_seed() {
      return [](const Obj_p &) { return to_objs(); };
    }

    static InstSeedSupplier noobj_seed() {
      return [](const Obj_p &) { return to_noobj(); };
    }

    static Objs_p to_objs(const List_p<Obj_p> &objs, const ID_p &furi = OBJS_FURI) {
      fError::OTYPE_CHECK(furi->path(FOS_BASE_TYPE_INDEX), OTypes.toChars(OType::OBJS));
      Objs_p os = share(Objs(objs, furi));
      return os;
    }

    static Objs_p to_objs(const List<Obj> &objs, const ID_p &furi = OBJS_FURI) {
      return Obj::to_objs(share(PtrHelper::clone(objs)), furi);
    }

    Obj_p clone() {
      return share(Obj(any(this->_value), id_p(*this->id_)));
    }

    ptr<BObj> serialize() const {
      LOG(TRACE, "Serializing %s\n", this->toString().c_str());
      if (this->is_noobj()) {
        auto *bytes = static_cast<fbyte *>(malloc(1));
        bytes[0] = 'x';
        return {new BObj{1, bytes}, bobj_deleter};
      }
      // auto *bytes = static_cast<fbyte *>(malloc(sizeof(*this)));
      // memcpy(bytes, reinterpret_cast<const fbyte *>(this->toString().c_str()), this->toString().length());
      return ptr<BObj>(
        new BObj{
          this->toString().length(),
          reinterpret_cast<fbyte *>(strdup(Ansi<>::strip(this->toString()).c_str()))
        },
        bobj_deleter);
    }

    template<typename OBJ>
    static ptr<OBJ> deserialize(const ptr<BObj> &bobj) {
      LOG(TRACE, "Deserializing obj with bytes %s (length %i)\n", bobj->second, bobj->first);
      if (bobj->first == 1 && bobj->second[0] == 'x')
        return Obj::to_noobj();
      return OBJ_PARSER(string(reinterpret_cast<char *>(bobj->second), bobj->first));
    }
  };

  [[maybe_unused]] static Uri u(const char *uri) { return Uri(fURI(uri)); }

  [[maybe_unused]] static Uri u(const fURI &uri) { return Uri(uri); }

  [[maybe_unused]] static Uri_p uri(const fURI &xuri, const ID_p &type = URI_FURI) { return Obj::to_uri(xuri, type); }

  [[maybe_unused]] static Uri_p uri(const fURI_p &xuri, const ID_p &type = URI_FURI) {
    return Obj::to_uri(*xuri, type);
  }

  [[maybe_unused]] static Uri_p uri(const char *xuri, const ID_p &type = URI_FURI) {
    return Obj::to_uri(fURI(xuri), type);
  }

  [[maybe_unused]] static Uri_p uri(const string &xuri, const ID_p &type = URI_FURI) {
    return Obj::to_uri(fURI(xuri), type);
  }

  [[maybe_unused]] static Int_p dool(const bool xbool, const ID_p &type = BOOL_FURI) {
    return Obj::to_bool(xbool, type);
  }

  [[maybe_unused]] static Int_p jnt(const FL_INT_TYPE xint, const ID_p &type = INT_FURI) {
    return Obj::to_int(xint, type);
  }

  [[maybe_unused]] static Str_p str(const char *xstr) { return Obj::to_str(xstr); }

  [[maybe_unused]] static Str_p str(const string &xstr) { return Obj::to_str(xstr); }

  [[maybe_unused]] static Real_p real(const FL_REAL_TYPE &xreal) { return Obj::to_real(xreal); }

  [[maybe_unused]] static NoObj_p noobj() { return Obj::to_noobj(); }

  [[maybe_unused]] static Obj_p obj(const Any &value, const ID_p &type) { return share(Obj(value, type)); }

  [[maybe_unused]] static Obj_p obj(const Obj &obj) { return share(Obj(obj)); }

  [[maybe_unused]] static Lst_p lst(const List<Obj_p> &list) { return Obj::to_lst(share(list)); }

  [[maybe_unused]] static Lst_p lst(const List_p<Obj_p> &list) { return Obj::to_lst(list); }

  [[maybe_unused]] static Rec_p rec(const Obj::RecMap<> &map) { return Obj::to_rec(share(map)); }

  [[maybe_unused]] static Rec_p rec(const std::initializer_list<Pair<const Obj, Obj>> &map,
                                    const ID_p &furi = REC_FURI) {
    return Obj::to_rec(map, furi);
  }

  [[maybe_unused]] static Rec_p rec(const std::initializer_list<Pair<const Obj_p, Obj_p>> &map,
                                    const ID_p &furi = REC_FURI) {
    return Obj::to_rec(map, furi);
  }


  [[maybe_unused]] static Objs_p objs() { return Obj::to_objs(share(List<Obj_p>())); }

  [[maybe_unused]] static Objs_p objs(const List<Obj_p> &list) { return Obj::to_objs(share(list)); }

  [[maybe_unused]] static Objs_p objs(const List_p<Obj_p> &list) { return Obj::to_objs(list); }

  [[maybe_unused]] static BCode_p bcode(const InstList &list) { return Obj::to_bcode(list); }

  [[maybe_unused]] static BCode_p bcode() { return Obj::to_bcode(); }

  struct CInstFunction : InstFunction {
    const string &filename;
    const int line_number;

    [[nodiscard]] string toString() const {
      return StringHelper::format("c-impl-%s[line:%i]", this->filename.c_str(), this->line_number);
    }
  };
} // namespace fhatos
#endif
