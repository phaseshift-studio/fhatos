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

#ifndef FL_STR_ENCODING
#define FL_STR_ENCODING sizeof(std::string::value_type)
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
  [[maybe_unused]] static bool is_initial(const IType itype) {
    return itype == IType::ZERO_TO_ONE || itype == IType::ZERO_TO_MANY || itype == IType::ZERO_TO_ZERO;
  }

  [[maybe_unused]] static bool is_barrier_in(const IType itype) {
    return itype == IType::ONE_TO_MANY || itype == IType::MANY_TO_MANY || itype == IType::ZERO_TO_MANY;
  }

  [[maybe_unused]] static bool is_barrier_out(const IType itype) {
    return itype == IType::MANY_TO_ONE || itype == IType::MANY_TO_MANY || itype == IType::MANY_TO_ZERO;
  }

  [[maybe_unused]] static bool is_terminal(const IType itype) {
    return itype == IType::ONE_TO_ZERO || itype == IType::MANY_TO_ZERO || itype == IType::ZERO_TO_ZERO;
  }

  static Consumer<BObj *> bobj_deleter = [](const BObj *bobj) {
    free(bobj->second);
    delete bobj;
  };
  static const Enums<IType> ITypeDomains = Enums<IType>({{IType::ZERO_TO_ZERO, "."},
    {IType::ZERO_TO_ONE, "."},
    {IType::ZERO_TO_MANY, "."},
    {IType::ONE_TO_ZERO, "o"},
    {IType::MANY_TO_ZERO, "O"},
    {IType::ONE_TO_ONE, "o"},
    {IType::ONE_TO_MANY, "o"},
    {IType::MANY_TO_ONE, "O"},
    {IType::MANY_TO_MANY, "O"}});
  static const Enums<IType> ITypeRanges = Enums<IType>({{IType::ZERO_TO_ZERO, "."},
    {IType::ZERO_TO_ONE, "o"},
    {IType::ZERO_TO_MANY, "O"},
    {IType::ONE_TO_ZERO, "."},
    {IType::MANY_TO_ZERO, "."},
    {IType::ONE_TO_ONE, "o"},
    {IType::ONE_TO_MANY, "O"},
    {IType::MANY_TO_ONE, "o"},
    {IType::MANY_TO_MANY, "O"}});
  static const Enums<IType> ITypeSignatures = Enums<IType>({{IType::ZERO_TO_ZERO, ".->."},
    {IType::ZERO_TO_ONE, ".->o"},
    {IType::ZERO_TO_MANY, ".->O"},
    {IType::ONE_TO_ZERO, "o->."},
    {IType::MANY_TO_ZERO, "O->."},
    {IType::ONE_TO_ONE, "o->o"},
    {IType::ONE_TO_MANY, "o->O"},
    {IType::MANY_TO_ONE, "O->o"},
    {IType::MANY_TO_MANY, "O->O"}});
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
  static const ID_p OBJ_FURI = make_shared<ID>(FOS_TYPE_PREFIX "obj/");
  static const ID_p NOOBJ_FURI = make_shared<ID>(FOS_TYPE_PREFIX "noobj/");
  static const ID_p TYPE_FURI = make_shared<ID>(FOS_TYPE_PREFIX "type/");
  static const ID_p BOOL_FURI = make_shared<ID>(FOS_TYPE_PREFIX "bool/");
  static const ID_p INT_FURI = make_shared<ID>(FOS_TYPE_PREFIX "int/");
  static const ID_p REAL_FURI = make_shared<ID>(FOS_TYPE_PREFIX "real/");
  static const ID_p URI_FURI = make_shared<ID>(FOS_TYPE_PREFIX "uri/");
  static const ID_p STR_FURI = make_shared<ID>(FOS_TYPE_PREFIX "str/");
  static const ID_p LST_FURI = make_shared<ID>(FOS_TYPE_PREFIX "lst/");
  static const ID_p REC_FURI = make_shared<ID>(FOS_TYPE_PREFIX "rec/");
  static const ID_p INST_FURI = make_shared<ID>(FOS_TYPE_PREFIX "inst/");
  static const ID_p BCODE_FURI = make_shared<ID>(FOS_TYPE_PREFIX "bcode/");
  static const ID_p OBJS_FURI = make_shared<ID>(FOS_TYPE_PREFIX "objs/");
  static const Map<OType, ID_p> OTYPE_FURI = {{{OType::NOOBJ, NOOBJ_FURI},
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
  static TriFunction<const Obj &, const OType, const ID_p &, ID_p> TYPE_CHECKER = [](const Obj &, const OType,
    const ID_p &) {
    LOG(DEBUG, "!yTYPE_CHECKER!! undefined at this point in bootstrap.\n");
    return nullptr;
  };
  static BiFunction<const Obj_p, const ID_p, Obj_p> TYPE_MAKER = [](const Obj_p &, const ID_p &) {
    LOG(DEBUG, "!yTYPE_MAKER!! undefined at this point in bootstrap.\n");
    return nullptr;
  };
  static Function<const string &, Obj_p> OBJ_PARSER = [](const string &) {
    LOG(DEBUG, "!yOBJ_PARSER!! undefined at this point in bootstrap.\n");
    return nullptr;
  };
  static BiFunction<const Objs_p, BCode_p, Objs_p> BCODE_PROCESSOR = [](const Objs_p &, const BCode_p &) {
    LOG(DEBUG, "!yBCODE_PROCESSOR!! undefined at this point in bootstrap.\n");
    return nullptr;
  };
  static BiConsumer<const ID, const ID> PROCESS_SPAWNER = [](const ID &, const ID &) {
    LOG(DEBUG, "!PROCESS_SPAWNER!! undefined at this point in bootstrap.\n");
  };
  static BiConsumer<const Pattern, const Rec_p> STRUCTURE_ATTACHER = [](const ID &, const Rec_p &) {
    LOG(DEBUG, "!ySTRUCTURE_ATTACHER!! undefined at this point in bootstrap.\n");
  };
  static TriFunction<const fURI &, const Obj_p &, const bool, const bool> SCHEDULER_INTERCEPT =
      [](const fURI &, const Obj_p &, const bool) -> bool {
    LOG(DEBUG, "!ySCHEDULER_INTERCEPT!! undefined at this point in bootstrap.\n");
    return false;
  };
  static TriFunction<const fURI &, const Obj_p &, const bool, const bool> ROUTER_INTERCEPT =
      [](const fURI &, const Obj_p &, const bool) -> bool {
    LOG(DEBUG, "!yROUTER_INTERCEPT!! undefined at this point in bootstrap.\n");
    return false;
  };

  //////////////////////////////////////////////////
  ////////////////////// OBJ //////////////////////
  /////////////////////////////////////////////////
  /// An mm-ADT abstract object from which all other types derive
  class Obj final : public IDed, public Function<Obj_p, Obj_p> {
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

    explicit Obj(const Any &value, const OType otype, const ID_p &type_id) : IDed(OTYPE_FURI.at(otype)), _value(value) {
      TYPE_CHECKER(*this, otype, type_id);
      this->id_ = type_id;
    }

    explicit Obj(const Any &value, const ID_p &type_id) : Obj(
      value, OTypes.to_enum(string(type_id->path(FOS_BASE_TYPE_INDEX))), type_id) {
    }

    /////
    static fError TYPE_ERROR(const Obj *obj, const char *function, [[maybe_unused]] const int lineNumber = __LINE__) {
      return fError("!b%s!g[!!%s!g]!! !yaccessed!! as !b%s!!\n", OTypes.to_chars(obj->o_type()).c_str(),
                    obj->toString().c_str(), string(function).replace(string(function).find("_value"), 6, "").c_str());
      /*
            const size_t index = string(function).find("_value");
                  const string fstring = index != string::npos
                                           ? string(function).replace(index, 6, "")
                                           : string(function);
                  return fError("!b%s!g[!!%s!g]!! !yaccessed!! as !b%s!!\n", OTypes.to_chars(obj->o_type()).c_str(),
                                obj->toString().c_str(), fstring.c_str());
       */
    }

    //////////////////////////////////////////////////////////////
    //// IMPLICIT CONVERSIONS (FOR NATIVE C++ CONSTRUCTIONS) ////
    //////////////////////////////////////////////////////////////
    // Obj(const Obj &other) : Obj(other._value, other.id()) {}
    template<class T, class = std::enable_if_t<std::is_same_v<bool, T>>>
    Obj(const T xbool, const char *type_id = EMPTY_CHARS) : Obj(Any(xbool), OType::BOOL,
                                                                id_p(BOOL_FURI->resolve(type_id))) {
    }

    Obj(const FL_INT_TYPE xint, const char *type_id = EMPTY_CHARS) : Obj(
      Any(xint), OType::INT, id_p(INT_FURI->resolve(type_id))) {
    }

    Obj(const FL_REAL_TYPE xreal, const char *type_id = EMPTY_CHARS) : Obj(
      Any(xreal), OType::REAL, id_p(REAL_FURI->resolve(type_id))) {
    }

    Obj(const fURI &xuri, const char *type_id = EMPTY_CHARS) : Obj(Any(xuri), OType::URI,
                                                                   id_p(URI_FURI->resolve(type_id))) {
    }

    Obj(const char *xstr, const char *type_id = EMPTY_CHARS) : Obj(Any(string(xstr)), OType::STR,
                                                                   id_p(STR_FURI->resolve(type_id))) {
    }

    Obj(const string &xstr, const char *type_id = EMPTY_CHARS) : Obj(Any(xstr), OType::STR,
                                                                     id_p(STR_FURI->resolve(type_id))) {
    }

    Obj(const std::initializer_list<Pair<const Obj, Obj>> &xrec, const char *type_id = EMPTY_CHARS) : Obj(
      Any(make_shared<RecMap<>>()), OType::REC, id_p(REC_FURI->resolve(type_id))) {
      auto map = this->value<RecMap<>>();
      for (const auto &[key, val]: xrec) {
        map.insert(make_pair(make_shared<Obj>(key), make_shared<Obj>(val)));
      }
    }

    Obj(const std::initializer_list<Obj> &xlst, const char *type_id = EMPTY_CHARS) : Obj(
      Any(make_shared<LstList<>>()), OType::LST, id_p(LST_FURI->resolve(type_id))) {
      const auto list = this->value<LstList_p<>>();
      for (const auto &obj: xlst) {
        list->push_back(make_shared<Obj>(obj));
      }
    }

    Obj(const List<Inst> &bcode, const char *type_id = EMPTY_CHARS) : Obj(
      Any(make_shared<InstList>(PtrHelper::clone(bcode))), OType::BCODE, id_p(BCODE_FURI->resolve(type_id))) {
    }

    Obj(const InstList_p &bcode, const char *type_id = EMPTY_CHARS) : Obj(
      Any(bcode), OType::BCODE, id_p(BCODE_FURI->resolve(type_id))) {
    }

    //////////////////////////////////////////////////////////////
    [[nodiscard]] OType o_type() const { return OTypes.to_enum(string(this->id_->path(FOS_BASE_TYPE_INDEX))); }

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

    [[nodiscard]] Obj_p objs_value(const uint16_t index) const { return this->objs_value()->at(index); }

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

    [[nodiscard]] Obj_p str_get(const Int_p &index) const {
      if (!this->is_str())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return (static_cast<size_t>(index->int_value()) >= this->str_value().length())
               ? Obj::to_noobj()
               : Obj::to_str(string() + (this->str_value()[index->int_value()]));
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
      return (static_cast<size_t>(index->int_value()) >= this->lst_value()->size())
               ? Obj::to_noobj()
               : this->lst_value()->at(index->int_value());
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
      for (const auto &[k, v]: *this->rec_value()) {
        if (k->match(key))
          return v;
      }
      return Obj::to_noobj();
      // return this->rec_value()->count(key) ? this->rec_value()->at(key) : Obj::to_noobj();
    }

    [[nodiscard]] Obj_p rec_get(const Obj &key) const { return Obj::rec_get(make_shared<Obj>(key)); }

    void rec_set(const Obj_p &key, const Obj_p &val) const {
      this->rec_value()->erase(key);
      if (!val->is_noobj())
        this->rec_value()->insert({key, val});
    }

    void rec_set(const Obj &key, const Obj &value) const {
      Obj::rec_set(make_shared<Obj>(key), make_shared<Obj>(value));
    }

    void rec_add(const Rec_p &other) const {
      for (const auto &[k, v]: *other->rec_value()) {
        if (this->rec_value()->count(k))
          this->rec_value()->erase(k);
        this->rec_value()->insert({k, v});
      }
    }

    void rec_delete(const Obj &key) const { Obj::rec_set(make_shared<Obj>(key), Obj::to_noobj()); }

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

    Obj_p operator()(const Obj_p &input) { return this->apply(input); }

    IType itype() const {
      if (this->is_inst())
        return std::get<2>(this->inst_value());
      if (this->is_bcode()) {
        const IType domain = this->bcode_value()->front()->itype();
        const IType range = this->bcode_value()->back()->itype();
        return ITypeSignatures.to_enum(string(ITypeDomains.to_chars(domain) + "->" + ITypeRanges.to_chars(range)));
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
        return to_bcode(insts);
      } else {
        InstList_p insts = make_shared<InstList>();
        for (const auto &i: *this->bcode_value()) {
          insts->push_back(i);
        }
        insts->push_back(inst);
        return to_bcode(insts);
      }
    }

    [[nodiscard]] BCode_p add_bcode(const BCode_p &bcode, [[maybe_unused]] const bool mutate = true) const {
      if (!this->is_bcode() || !bcode->is_bcode())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      InstList_p insts = make_shared<InstList>();
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

    string toString(const bool include_type = true, const bool ansi = true, const bool strict = false) const {
      string obj_string;
      switch (this->o_type()) {
        case OType::BOOL:
          obj_string = this->bool_value() ? "!ytrue!!" : "!yfalse!!";
          break;
        case OType::INT:
          obj_string = std::to_string(this->int_value());
          break;
        case OType::REAL:
          obj_string = std::to_string(this->real_value());
          break;
        case OType::URI:
          obj_string = "!_" + (strict ? "<" + this->uri_value().toString() + ">" : this->uri_value().toString()) + "!!";
          break;
        case OType::STR:
          obj_string = "!m'!!!~" + this->str_value() + "!m'!!";
          break;
        case OType::LST: {
          obj_string = "!m[!!";
          bool first = true;
          for (const auto &obj: *this->lst_value()) {
            if (first) {
              first = false;
            } else {
              obj_string += "!m,!!";
            }
            obj_string += obj->toString(include_type, ansi, strict);
          }
          obj_string += "!m]!!";
          break;
        }
        case OType::REC: {
          obj_string = "!m[!!";
          bool first = true;
          for (const auto &[k, v]: *this->rec_value()) {
            if (first) {
              first = false;
            } else {
              obj_string += "!m,";
            }
            obj_string += "!c";
            obj_string += k->toString(include_type, false, strict);
            obj_string += "!m=>!!";
            obj_string += v->toString();
          }
          obj_string += "!m]!!";
          break;
        }
        case OType::INST: {
          bool first = true;
          for (const auto &arg: this->inst_args()) {
            if (first) {
              first = false;
            } else {
              obj_string += "!m,!!";
            }
            obj_string += arg->toString(include_type, ansi, strict);
          }
          break;
        }
        case OType::BCODE: {
          if (this->bcode_value()->empty())
            obj_string = "_";
          else {
            // objString += "!b" + this->bcode_range()->name() + "!g<=!b" + this->bcode_domain()->name() + "!g[!!";
            bool first = true;
            for (const auto &inst: *this->bcode_value()) {
              if (first) {
                first = false;
              } else {
                obj_string += "!g.!!";
              }
              obj_string += inst->toString(include_type, ansi, strict);
            }
            // objString += "!g]!!";
          }
          break;
        }
        case OType::NOOBJ: {
          obj_string = "!r" STR(FOS_NOOBJ_TOKEN) "!!";
          break;
        }
        case OType::OBJS: {
          obj_string += "!m{!!";
          bool first = true;
          for (const auto &obj: *this->objs_value()) {
            if (first) {
              first = false;
            } else {
              obj_string += "!m,!!";
            }
            obj_string += obj->toString(include_type, ansi, strict);
          };
          obj_string += "!m}!!";
          break;
        }
        default:
          throw fError("Unknown obj type in toString(): %s\n", OTypes.to_chars(this->o_type()).c_str());
      }
      obj_string = (include_type && (this->id_->path_length() > 2))
                     ? string("!b")
                     .append(this->id_->name())
                     .append(this->is_inst() ? "!g(!!" : "!g[!!")
                     .append(obj_string)
                     .append(this->is_inst() ? "!g)!!" : "!g]!!")
                     : obj_string;
      return ansi ? obj_string : Ansi<>::strip(obj_string);
    }

    [[nodiscard]] int compare(const Obj &rhs) const { return this->toString().compare(rhs.toString()); }

    bool operator&&(const Obj &rhs) const {
      if (this->is_bool() && rhs.is_bool())
        return this->bool_value() && rhs.bool_value();
      throw fError("%s is not conjunctive (&&)\n", OTypes.to_chars(this->o_type()).c_str());
    }

    bool operator||(const Obj &rhs) const {
      if (this->is_bool() && rhs.is_bool())
        return this->bool_value() || rhs.bool_value();
      throw fError("%s is not disjunctive (||)\n", OTypes.to_chars(this->o_type()).c_str());
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
          throw fError("%s is not relational (>)\n", OTypes.to_chars(this->o_type()).c_str());
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
          throw fError("%s is not relational (<)\n", OTypes.to_chars(this->o_type()).c_str());
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
            list->push_back(make_shared<Obj>(**itA * **itB, itA->get()->id()));
            ++itB;
          }
          return Lst(list, this->id());
        }
        case OType::REC: {
          auto map = std::make_shared<RecMap<>>();
          auto itB = rhs.rec_value()->begin();
          for (auto itA = this->rec_value()->begin(); itA != this->rec_value()->end(); ++itA) {
            map->insert(std::make_pair(make_shared<Obj>(*itA->first * *itB->first, itA->first->id()),
                                       make_shared<Obj>(*itA->second * *itB->second, itA->second->id())));
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
          throw fError("%s can not be multiplied (*)\n", OTypes.to_chars(this->o_type()).c_str());
      }
    }

    Obj operator/(const Obj &rhs) const {
      switch (this->o_type()) {
        case OType::NOOBJ:
          return *to_noobj();
        case OType::INT:
          return Obj(this->int_value() / rhs.int_value(), this->id());
        case OType::REAL:
          return Obj(this->real_value() / rhs.real_value(), this->id());
        default:
          throw fError("%s can not be divided (/)\n", OTypes.to_chars(this->o_type()).c_str());
      }
    }

    Obj operator+(const Obj &rhs) const {
      switch (this->o_type()) {
        case OType::NOOBJ:
          return *to_noobj();
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
          throw fError("%s can not be added (+)\n", OTypes.to_chars(this->o_type()).c_str());
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
            if (std::find(rhs.lst_value()->begin(), rhs.lst_value()->end(), obj) != std::end(*rhs.lst_value()))
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
          throw fError("%s can not be subtracted (-)\n", OTypes.to_chars(this->o_type()).c_str());
      }
    }

    Obj operator%(const Obj &other) const {
      switch (this->o_type()) {
        case OType::INT:
          return Obj(this->int_value() % other.int_value(), this->id());
        default:
          throw fError("%s can not be moduloed (%)\n", OTypes.to_chars(this->o_type()).c_str());
      }
    }

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
          throw fError("Unknown obj type in ==: %s\n", OTypes.to_chars(this->o_type()).c_str());
      }
    }

    Obj operator[](const Obj &key) const {
      switch (this->o_type()) {
        case OType::STR:
          return *this->str_get(share(key));
        case OType::LST:
          return *this->lst_get(share(key));
        case OType::REC:
          return *this->rec_get(share(key));
        default:
          throw fError("Unknown obj type in []: %s\n", OTypes.to_chars(this->o_type()).c_str());
      }
    }

    [[nodiscard]] bool is_noobj() const { return this->o_type() == OType::NOOBJ; }

    [[nodiscard]] bool is_bool() const { return this->o_type() == OType::BOOL; }

    [[nodiscard]] bool is_int() const { return this->o_type() == OType::INT; }

    [[nodiscard]] bool is_real() const { return this->o_type() == OType::REAL; }

    [[nodiscard]] bool is_uri() const { return this->o_type() == OType::URI; }

    [[nodiscard]] bool is_str() const { return this->o_type() == OType::STR; }

    [[nodiscard]] bool is_lst() const { return this->o_type() == OType::LST; }

    [[nodiscard]] bool is_poly() const {
      return this->is_lst() || this->is_rec() || this->is_objs() /*|| this->is_bcode() || this->is_inst()*/;
    }

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
          const auto new_values = make_shared<LstList<Obj_p>>();
          for (const auto &obj: *this->lst_value()) {
            new_values->push_back(obj->apply(lhs));
          }
          return Obj::to_lst(new_values);
        }
        case OType::REC: {
          const auto new_pairs = make_shared<RecMap<>>();
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
          // if (lhs->is_bcode())
          //   return lhs->add_bcode(PtrHelper::no_delete(this), true);
          ptr<Obj> current_obj = lhs; //->clone();
          for (const Inst_p &current_inst: *this->bcode_value()) {
            LOG(TRACE, "Applying %s => %s\n", current_obj->toString().c_str(), current_inst->toString().c_str());
            if (current_inst->is_noobj())
              break;
            current_obj = current_inst->apply(current_obj);
          }
          // const Objs_p objs = Options::singleton()->processor<Obj, BCode, Obj>(lhs, PtrHelper::no_delete(this));
          //  return objs->objs_value()->empty() ? Obj::to_noobj() : objs->objs_value()->front();
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
          throw fError("Unknown obj type in apply(): %s\n", OTypes.to_chars(this->o_type()).c_str());
      }
    }

    // const fURI type() const { return this->_id->authority(""); }

    [[nodiscard]] bool match(const Obj_p &type, const bool same_type = true) const {
      //LOG(TRACE, "!ymatching!!: %s ~ %s\n", this->toString().c_str(), type->toString().c_str());
      if (type->is_noop_bcode())
        return true;
      if (type->is_bcode() && !this->is_bcode())
        return !type->apply(PtrHelper::no_delete<Obj>(const_cast<Obj *>(this)))->is_noobj();
      if (this->o_type() != type->o_type())
        return false;
      if (same_type && (*this->id() != *type->id()))
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
          const auto objs_a = this->lst_value();
          const auto objs_b = type->lst_value();
          if (objs_a->size() != objs_b->size())
            return false;
          auto it_b = objs_b->begin();
          for (const auto &it_a: *objs_a) {
            if (!it_a->match(*it_b))
              return false;
            ++it_b;
          }
          return true;
        }
        case OType::REC: {
          const auto pairs_a = this->rec_value();
          const auto pairs_b = type->rec_value();
          if (pairs_a->size() != pairs_b->size())
            return false;
          auto it_b = pairs_b->begin();
          for (const auto &it_a: *pairs_a) {
            if (!it_a.first->match(it_b->first) || !it_a.second->match(it_b->second))
              return false;
            ++it_b;
          }
          return true;
        }
        case OType::INST: {
          const auto args_a = this->inst_args();
          auto args_b = type->inst_args();
          if (args_a.size() != args_b.size())
            return false;
          if (this->itype() != type->itype())
            return false;
          const auto it_b = args_b.begin();
          for (const auto &it_a: args_a) {
            if (!it_a->match(*it_b))
              return false;
          }
          return true;
        }
        case OType::BCODE: {
          const auto insts_a = this->bcode_value();
          const auto insts_b = type->bcode_value();
          if (insts_a->size() != insts_b->size())
            return false;
          const auto it_b = insts_b->begin();
          for (const auto &it_a: *insts_a) {
            if (!it_a->match(*it_b))
              return false;
          }
          return true;
        }
        default:
          throw fError("Unknown obj type in match(): %s\n", OTypes.to_chars(this->o_type()).c_str());
      }
      return false;
    }

    [[nodiscard]] Obj_p as(const ID_p &furi) const {
      const ID_p resolution = id_p(this->id_->resolve(*furi));
      const Obj_p obj = TYPE_MAKER(PtrHelper::no_delete<Obj>(const_cast<Obj *>(this)), resolution);
      return obj;
      // return share<Obj>(Obj(this->_value, OTypes.to_enum(resolution->path(FOS_BASE_TYPE_INDEX)), resolution));
    }

    Obj_p as(const char *furi) const { return this->as(id_p(furi)); }

    [[nodiscard]] Inst_p next_inst(const Inst_p &current_inst) const {
      if (current_inst == nullptr)
        return this->bcode_value()->front();
      if (current_inst->is_noobj())
        return current_inst;
      bool found = false;
      for (const auto &inst: *this->bcode_value()) {
        if (found)
          return inst;
        if (inst == current_inst)
          found = true;
      }
      return Obj::to_noobj();
    }

    /// STATIC TYPE CONSTRAINED CONSTRUCTORS
    static Obj_p to_noobj() {
      static auto noobj = make_shared<Obj>(Any(nullptr), NOOBJ_FURI);
      return noobj;
    }

    static Bool_p to_bool(const bool value, const ID_p &furi = BOOL_FURI) {
      fError::OTYPE_CHECK(furi->path(FOS_BASE_TYPE_INDEX), OTypes.to_chars(OType::BOOL));
      return make_shared<Bool>(value, furi);
    }

    static Int_p to_int(const FL_INT_TYPE value, const ID_p &furi = INT_FURI) {
      fError::OTYPE_CHECK(furi->path(FOS_BASE_TYPE_INDEX), OTypes.to_chars(OType::INT));
      return make_shared<Int>(value, furi);
    }

    static Real_p to_real(const FL_REAL_TYPE value, const ID_p &furi = REAL_FURI) {
      fError::OTYPE_CHECK(furi->path(FOS_BASE_TYPE_INDEX), OTypes.to_chars(OType::REAL));
      return make_shared<Real>(value, furi);
    }

    static Str_p to_str(const string &value, const ID_p &furi = STR_FURI) {
      fError::OTYPE_CHECK(furi->path(FOS_BASE_TYPE_INDEX), OTypes.to_chars(OType::STR));
      return make_shared<Str>(value, furi);
    }

    static Str_p to_str(const char *value, const ID_p &furi = STR_FURI) {
      fError::OTYPE_CHECK(furi->path(FOS_BASE_TYPE_INDEX), OTypes.to_chars(OType::STR));
      return make_shared<Str>(string(value), furi);
    }

    static Uri_p to_uri(const fURI &value, const ID_p &furi = URI_FURI) {
      fError::OTYPE_CHECK(furi->path(FOS_BASE_TYPE_INDEX), OTypes.to_chars(OType::URI));
      return make_shared<Uri>(value, furi);
    }

    static Uri_p to_uri(const char *value, const ID_p &furi = URI_FURI) {
      fError::OTYPE_CHECK(furi->path(FOS_BASE_TYPE_INDEX), OTypes.to_chars(OType::URI));
      return make_shared<Uri>(fURI(value), furi);
    }

    static Lst_p to_lst(const ID_p &furi = LST_FURI) {
      fError::OTYPE_CHECK(furi->path(FOS_BASE_TYPE_INDEX), OTypes.to_chars(OType::LST));
      return make_shared<Lst>(make_shared<LstList<>>(), furi);
    }

    static Lst_p to_lst(const LstList_p<> &xlst, const ID_p &furi = LST_FURI) {
      fError::OTYPE_CHECK(furi->path(FOS_BASE_TYPE_INDEX), OTypes.to_chars(OType::LST));
      return make_shared<Lst>(xlst, furi);
    }

    static Lst_p to_lst(const std::initializer_list<Obj> &xlst, const ID_p &furi = LST_FURI) {
      const auto list = make_shared<LstList<>>();
      for (const auto &obj: xlst) {
        list->push_back(share(obj));
      }
      return to_lst(list, furi);
    }

    static Lst_p to_lst(const std::initializer_list<Obj_p> &xlst, const ID_p &furi = LST_FURI) {
      return to_lst(make_shared<LstList<>>(xlst), furi);
    }

    static Rec_p to_rec(const ID_p &furi = REC_FURI) {
      fError::OTYPE_CHECK(furi->path(FOS_BASE_TYPE_INDEX), OTypes.to_chars(OType::REC));
      return make_shared<Rec>(make_shared<RecMap<>>(), furi);
    }

    static Rec_p to_rec(const RecMap_p<> &map, const ID_p &furi = REC_FURI) {
      fError::OTYPE_CHECK(furi->path(FOS_BASE_TYPE_INDEX), OTypes.to_chars(OType::REC));
      return make_shared<Rec>(map, furi);
    }

    static Rec_p to_rec(const std::initializer_list<Pair<const Obj, Obj>> &xrec, const ID_p &furi = REC_FURI) {
      const auto map = make_shared<Obj::RecMap<>>();
      for (const auto &[key, value]: xrec) {
        map->insert(make_pair(make_shared<Obj>(key), make_shared<Obj>(value)));
      }
      return to_rec(map, furi);
    }

    static Rec_p to_rec(const std::initializer_list<Pair<const Obj_p, Obj_p>> &xrec, const ID_p &furi = REC_FURI) {
      const auto map = make_shared<Obj::RecMap<>>();
      for (const auto &[key, value]: xrec) {
        map->insert(make_pair(key, value));
      }
      return to_rec(map, furi);
    }

    static Inst_p to_inst(const InstValue &value, const ID_p &furi = INST_FURI) {
      fError::OTYPE_CHECK(furi->path(FOS_BASE_TYPE_INDEX), OTypes.to_chars(OType::INST));
      return make_shared<Inst>(value, furi);
    }

    static Inst_p to_inst(
      const string &opcode, const List<Obj_p> &args, const InstFunctionSupplier &function, const IType itype,
      const InstSeedSupplier &seed = [](const Obj_p &) { return Obj::to_noobj(); }, const ID_p &furi = nullptr) {
      const ID_p fix = furi ? furi : make_shared<ID>(string(FOS_TYPE_PREFIX "inst/") + opcode);
      return to_inst({args, function, itype, seed}, fix);
    }

    static BCode_p to_bcode(const InstList &insts, const ID_p &furi = BCODE_FURI) {
      return Obj::to_bcode(make_shared<InstList>(insts), furi);
    }

    static BCode_p to_bcode(const ID_p &furi = BCODE_FURI) { return Obj::to_bcode(share<InstList>({}), furi); }

    static BCode_p to_bcode(const InstList_p &insts, const ID_p &furi = BCODE_FURI) {
      fError::OTYPE_CHECK(furi->path(FOS_BASE_TYPE_INDEX), OTypes.to_chars(OType::BCODE));
      return make_shared<BCode>(insts, furi);
    }

    static Objs_p to_objs(const ID_p &furi = OBJS_FURI) {
      fError::OTYPE_CHECK(furi->path(FOS_BASE_TYPE_INDEX), OTypes.to_chars(OType::OBJS));
      return to_objs(make_shared<List<Obj_p>>(), furi);
    }

    static InstSeedSupplier objs_seed() {
      return [](const Obj_p &) { return to_objs(); };
    }

    static InstSeedSupplier noobj_seed() {
      return [](const Obj_p &) { return to_noobj(); };
    }

    static Objs_p to_objs(const List_p<Obj_p> &objs, const ID_p &furi = OBJS_FURI) {
      fError::OTYPE_CHECK(furi->path(FOS_BASE_TYPE_INDEX), OTypes.to_chars(OType::OBJS));
      auto os = make_shared<Objs>(objs, furi);
      return os;
    }

    static Objs_p to_objs(const List<Obj> &objs, const ID_p &furi = OBJS_FURI) {
      return Obj::to_objs(make_shared<List<Obj_p>>(PtrHelper::clone(objs)), furi);
    }

    /*std::__allocator_base<Obj> allocator = std::allocator<Obj>()*/
    Obj_p clone() const {
      const ID_p id_clone = id_p(*this->id_);
      if (this->is_rec()) {
        // REC
        const auto new_map = make_shared<RecMap<>>();
        for (const auto &[k, v]: *this->rec_value()) {
          new_map->insert({k->clone(), v->clone()});
        }
        return to_rec(new_map, id_clone);
      }
      if (this->is_lst()) {
        // LST
        const auto new_list = make_shared<LstList<>>();
        for (const auto &e: *this->lst_value()) {
          new_list->push_back(e->clone());
        }
        return to_lst(new_list, id_clone);
      }
      if (this->is_inst()) {
        // INST
        //   if(true)
        //   return PtrHelper::clone<Obj>(*this);
        InstArgs new_args;
        for (const auto &arg: this->inst_args()) {
          new_args.push_back(arg->clone());
        }
        return to_inst(string(this->inst_op()), new_args, this->inst_f(), this->itype(), this->inst_seed_supplier());
      }
      if (this->is_bcode()) {
        // BCODE
        //   if(true)
        //    return PtrHelper::clone<Obj>(*this);
        InstList new_insts;
        for (const auto &inst: *this->bcode_value()) {
          new_insts.push_back(inst->clone());
        }
        return to_bcode(new_insts);
      }
      if (this->is_objs()) {
        // OBJS
        const auto new_list = make_shared<List<Obj_p>>();
        for (const auto &e: *this->objs_value()) {
          new_list->push_back(e->clone());
        }
        return to_objs(new_list, id_clone);
      }
      return make_shared<Obj>(any(this->_value), id_clone);
    }

    BObj_p serialize() const {
      LOG(DEBUG, "Serializing obj %s\n", this->toString().c_str());
      const string serial = this->toString(true, false, true);
      return ptr<BObj>(new BObj{serial.length(), reinterpret_cast<fbyte *>(strdup(serial.c_str()))}, bobj_deleter);
    }

    static Obj_p deserialize(const BObj_p &bobj) {
      LOG(DEBUG, "Deserializing bytes %s (length %i)\n", bobj->second, bobj->first);
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

  [[maybe_unused]] static Bool_p dool(const bool xbool, const ID_p &type = BOOL_FURI) {
    return Obj::to_bool(xbool, type);
  }

  [[maybe_unused]] static Int_p jnt(const FL_INT_TYPE xint, const ID_p &type = INT_FURI) {
    return Obj::to_int(xint, type);
  }

  [[maybe_unused]] static Str_p str(const char *xstr) { return Obj::to_str(xstr); }

  [[maybe_unused]] static Str_p str(const string &xstr) { return Obj::to_str(xstr); }

  [[maybe_unused]] static Real_p real(const FL_REAL_TYPE &xreal) { return Obj::to_real(xreal); }

  [[maybe_unused]] static NoObj_p noobj() { return Obj::to_noobj(); }

  [[maybe_unused]] static Obj_p obj(const Any &value, const ID_p &type) { return make_shared<Obj>(value, type); }

  [[maybe_unused]] static Obj_p obj(const Obj &obj) { return make_shared<Obj>(obj); }

  [[maybe_unused]] static Lst_p lst() { return Obj::to_lst(make_shared<Obj::LstList<>>()); }

  [[maybe_unused]] static Lst_p lst(const List<Obj_p> &list) { return Obj::to_lst(share(list)); }

  [[maybe_unused]] static Lst_p lst(const List_p<Obj_p> &list) { return Obj::to_lst(list); }

  [[maybe_unused]] static Rec_p rec() { return Obj::to_rec(make_shared<Obj::RecMap<>>()); }

  [[maybe_unused]] static Rec_p rec(const Obj::RecMap<> &map) { return Obj::to_rec(make_shared<Obj::RecMap<>>(map)); }

  [[maybe_unused]] static Rec_p rec(const std::initializer_list<Pair<const Obj, Obj>> &map,
                                    const ID_p &furi = REC_FURI) {
    return Obj::to_rec(map, furi);
  }

  [[maybe_unused]] static Rec_p rec(const std::initializer_list<Pair<const Obj_p, Obj_p>> &map,
                                    const ID_p &furi = REC_FURI) {
    return Obj::to_rec(map, furi);
  }

  [[maybe_unused]] static Objs_p objs() { return Obj::to_objs(make_shared<List<Obj_p>>()); }

  [[maybe_unused]] static Objs_p objs(const List<Obj_p> &list) { return Obj::to_objs(make_shared<List<Obj_p>>(list)); }

  [[maybe_unused]] static Objs_p objs(const List_p<Obj_p> &list) { return Obj::to_objs(list); }

  [[maybe_unused]] static BCode_p bcode(const InstList &list) { return Obj::to_bcode(list); }

  [[maybe_unused]] static BCode_p bcode() { return Obj::to_bcode(); }
} // namespace fhatos
#endif
