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

#ifndef FOS_REAL_TYPE
#define FOS_REAL_TYPE float_t

#include <any>

#endif
#ifndef FOS_INT_TYPE
#define FOS_INT_TYPE int32_t
#endif

#ifndef FOS_STR_ENCODING
#define FOS_STR_ENCODING sizeof(std::string::value_type)
#endif

#define FOS_TYPE_PREFIX "/type/"
#define FOS_DRIVER_PREFIX "/driver/"
// #define FOS_BASE_TYPE_INDEX 1

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
    /// A stream of objs
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
    /// An error
    ERROR
  };

  static const auto OTypes = Enums<OType>({{OType::OBJ, "obj"},
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
                                           {OType::ERROR, "error"}});

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
  using Error = Obj;
  using Error_p = Obj_p;
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
  static const auto ITypeDomains = Enums<IType>({{IType::ZERO_TO_ZERO, "."},
                                                 {IType::ZERO_TO_ONE, "."},
                                                 {IType::ZERO_TO_MANY, "."},
                                                 {IType::ONE_TO_ZERO, "o"},
                                                 {IType::MANY_TO_ZERO, "O"},
                                                 {IType::ONE_TO_ONE, "o"},
                                                 {IType::ONE_TO_MANY, "o"},
                                                 {IType::MANY_TO_ONE, "O"},
                                                 {IType::MANY_TO_MANY, "O"}});
  static const auto ITypeRanges = Enums<IType>({{IType::ZERO_TO_ZERO, "."},
                                                {IType::ZERO_TO_ONE, "o"},
                                                {IType::ZERO_TO_MANY, "O"},
                                                {IType::ONE_TO_ZERO, "."},
                                                {IType::MANY_TO_ZERO, "."},
                                                {IType::ONE_TO_ONE, "o"},
                                                {IType::ONE_TO_MANY, "O"},
                                                {IType::MANY_TO_ONE, "o"},
                                                {IType::MANY_TO_MANY, "O"}});
  static const auto ITypeSignatures = Enums<IType>({{IType::ZERO_TO_ZERO, ".->."},
                                                    {IType::ZERO_TO_ONE, ".->o"},
                                                    {IType::ZERO_TO_MANY, ".->O"},
                                                    {IType::ONE_TO_ZERO, "o->."},
                                                    {IType::MANY_TO_ZERO, "O->."},
                                                    {IType::ONE_TO_ONE, "o->o"},
                                                    {IType::ONE_TO_MANY, "o->O"},
                                                    {IType::MANY_TO_ONE, "O->o"},
                                                    {IType::MANY_TO_MANY, "O->O"}});
  static const auto ITypeDescriptions = Enums<IType>({
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
  using InstF = BiFunction<Obj_p, InstArgs, Obj_p>;
  using InstFunctionSupplier = Function<InstArgs, Function<Obj_p, Obj_p>>;
  using InstSeedSupplier = Function<Obj_p, Obj_p>;
  using InstSeed = Obj_p;
  using InstValue = Quad<InstArgs, InstFunctionSupplier, IType, InstSeedSupplier>;
  using InstList = List<Inst_p>;
  using InstList_p = ptr<InstList>;
  static const auto OBJ_FURI = make_shared<ID>(FOS_TYPE_PREFIX "obj/");
  static const auto NOOBJ_FURI = make_shared<ID>(FOS_TYPE_PREFIX "noobj/");
  static const auto TYPE_FURI = make_shared<ID>(FOS_TYPE_PREFIX "type/");
  static const auto BOOL_FURI = make_shared<ID>(FOS_TYPE_PREFIX "bool/");
  static const auto INT_FURI = make_shared<ID>(FOS_TYPE_PREFIX "int/");
  static const auto REAL_FURI = make_shared<ID>(FOS_TYPE_PREFIX "real/");
  static const auto URI_FURI = make_shared<ID>(FOS_TYPE_PREFIX "uri/");
  static const auto STR_FURI = make_shared<ID>(FOS_TYPE_PREFIX "str/");
  static const auto LST_FURI = make_shared<ID>(FOS_TYPE_PREFIX "lst/");
  static const auto REC_FURI = make_shared<ID>(FOS_TYPE_PREFIX "rec/");
  static const auto INST_FURI = make_shared<ID>(FOS_TYPE_PREFIX "inst/");
  static const auto BCODE_FURI = make_shared<ID>(FOS_TYPE_PREFIX "bcode/");
  static const auto ERROR_FURI = make_shared<ID>(FOS_TYPE_PREFIX "error/");
  static const auto OBJS_FURI = make_shared<ID>(FOS_TYPE_PREFIX "objs/");
  /*static const auto OBJ_FURI = make_shared<ID>("mmadt:obj");
  static const auto NOOBJ_FURI = make_shared<ID>("mmadt:noobj");
  static const auto BOOL_FURI = make_shared<ID>("mmadt:bool");
  static const auto INT_FURI = make_shared<ID>("mmadt:int");
  static const auto REAL_FURI = make_shared<ID>("mmadt:real");
  static const auto URI_FURI = make_shared<ID>("mmadt:uri");
  static const auto STR_FURI = make_shared<ID>("mmadt:str");
  static const auto LST_FURI = make_shared<ID>("mmadt:lst");
  static const auto REC_FURI = make_shared<ID>("mmadt:rec");
  static const auto INST_FURI = make_shared<ID>("mmadt:inst");
  static const auto BCODE_FURI = make_shared<ID>("mmadt:bcode");
  static const auto ERROR_FURI = make_shared<ID>("mmadt:error");
  static const auto OBJS_FURI = make_shared<ID>("mmadt:objs");*/
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
                                               {OType::ERROR, ERROR_FURI}}};
  static const Map<ID, OType> FURI_OTYPE = {{{*NOOBJ_FURI, OType::NOOBJ},
                                             {*OBJ_FURI, OType::OBJ},
                                             {*OBJS_FURI, OType::OBJS},
                                             {*URI_FURI, OType::URI},
                                             {*BOOL_FURI, OType::BOOL},
                                             {*INT_FURI, OType::INT},
                                             {*REAL_FURI, OType::REAL},
                                             {*STR_FURI, OType::STR},
                                             {*LST_FURI, OType::LST},
                                             {*REC_FURI, OType::REC},
                                             {*INST_FURI, OType::INST},
                                             {*BCODE_FURI, OType::BCODE},
                                             {*ERROR_FURI, OType::ERROR}}};

  static BiConsumer<const ID_p, const Obj_p> TYPE_SAVER = [](const ID_p &type_id, const Obj_p &) {
    LOG(DEBUG, "!yTYPE_SAVER!! undefined at this point in bootstrap: %s\n", type_id->toString().c_str());
  };
  static TriFunction<const Obj *, const ID_p &, const bool, const bool> TYPE_CHECKER =
      [](const Obj *, const ID_p &type_id, const bool = true) -> bool {
    LOG(DEBUG, "!yTYPE_CHECKER!! undefined at this point in bootstrap: %s\n", type_id->toString().c_str());
    return false;
  };
  static BiFunction<Obj_p, ID_p, Obj_p> TYPE_MAKER = [](Obj_p, ID_p) {
    LOG(TRACE, "!yTYPE_MAKER!! undefined at this point in bootstrap.\n");
    return nullptr;
  };
  static Function<const string &, Obj_p> OBJ_PARSER = [](const string &) {
    LOG(TRACE, "!yOBJ_PARSER!! undefined at this point in bootstrap.\n");
    return nullptr;
  };
  static BiFunction<const Objs_p, BCode_p, Objs_p> BCODE_PROCESSOR = [](const Objs_p &, const BCode_p &) {
    LOG(TRACE, "!yBCODE_PROCESSOR!! undefined at this point in bootstrap.\n");
    return nullptr;
  };
  static TriConsumer<const fURI_p &, const Obj_p &, const bool> ROUTER_WRITE = [](const fURI_p &, const Obj_p &,
    const bool) -> void {
    LOG(TRACE, "!yROUTER_WRITE!! undefined at this point in bootstrap.\n");
  };
  static Function<const fURI_p &, const Obj_p> ROUTER_READ = [](const fURI_p &) -> Obj_p {
    LOG(TRACE, "!yROUTER_READ!! undefined at this point in bootstrap.\n");
    return nullptr;
  };
  static TriFunction<const uint8_t, const char *, const Obj_p &, const Inst_p> INST_ARG = [
      ](const uint8_t arg_num, const char *arg_name, const Obj_p &default_arg) -> Obj_p {
    LOG(TRACE, "!yINST_ARG!! undefined at this point in bootstrap.\n");
    return nullptr;
  };
  struct Subscription;
  static Consumer<const ptr<Subscription> &> ROUTER_SUBSCRIBE = [](const ptr<Subscription> &) {
    LOG(TRACE, "!yROUTER_SUBSCRIBE!! undefined at this point in bootstrap.\n");
    return nullptr;
  };

  //////////////////////////////////////////////////
  ////////////////////// OBJ //////////////////////
  /////////////////////////////////////////////////
  /// An mm-ADT abstract object from which all other types derive
  class Obj : public Typed, public Valued, public Function<Obj_p, Obj_p>, public enable_shared_from_this<Obj> {
  public:
    const OType otype_;
    Any value_;

    struct objp_hash {
      size_t operator()(const Obj_p &obj) const { return obj->hash(); }
    };

    struct objp_equal_to : std::binary_function<Obj_p &, Obj_p &, bool> {
      bool operator()(const Obj_p &a, const Obj_p &b) const { return a->equals(*b); }
    };

    struct obj_comp : std::less<> {
      template<class K1 = Obj, class K2 = Obj>
      auto operator()(K1 &k1, K2 &k2) const {
        return k1.hash() < k2.hash();
      }
    };

    struct objp_comp : std::less<> {
      template<class K1 = Obj_p, class K2 = Obj_p>
      auto operator()(K1 &k1, K2 &k2) const {
        return k1->hash() < k2->hash();
      }
    };

    //////////////////////////////////////////////////////
    using LstList = List<Obj_p>;
    //////////////////////////////////////////////////////
    using LstList_p = ptr<LstList>;
    //////////////////////////////////////////////////////
    template<typename COMPARATOR = objp_comp,
             typename ALLOCATOR=std::allocator<std::pair<const Obj_p, Obj_p>>>
    using RecMap = Map<Obj_p, Obj_p, COMPARATOR, ALLOCATOR>;
    //////////////////////////////////////////////////////
    template<typename COMPARATOR = objp_comp,
             typename ALLOCATOR=std::allocator<std::pair<const Obj_p, Obj_p>>>
    using RecMap_p = ptr<RecMap<COMPARATOR, ALLOCATOR>>;
    //////////////////////////////////////////////////////

    explicit Obj(const Any &value, const OType otype, const ID_p &type_id, const ID_p &value_id = nullptr) :
      Typed(OTYPE_FURI.at(otype)), Valued(value_id), otype_(otype), value_(value) {
      TYPE_CHECKER(this, type_id, true);
      this->tid_ = type_id;
      if (value_id) {
        const Obj_p strip = this->clone();
        strip->vid_ = nullptr;
        ROUTER_WRITE(value_id, strip, true);
      }
    }

    /////
    static fError TYPE_ERROR(const Obj *obj, const char *function, [[maybe_unused]] const int lineNumber = __LINE__) {
      return fError("!b%s!g[!!%s!g]!! !yaccessed!! as !b%s!!", OTypes.to_chars(obj->o_type()).c_str(),
                    obj->toString().c_str(), string(function).replace(string(function).find("_value"), 6, "").c_str());
    }

    //////////////////////////////////////////////////////////////
    //// IMPLICIT CONVERSIONS (FOR NATIVE C++ CONSTRUCTIONS) ////
    //////////////////////////////////////////////////////////////
    // Obj(const Obj &other) : Obj(other._value, other.id()) {}
    template<class T, class = std::enable_if_t<std::is_same_v<bool, T>>>
    Obj(const T xbool, const char *type_id = EMPTY_CHARS) :
      Obj(Any(xbool), OType::BOOL, id_p(BOOL_FURI->resolve(type_id))) {
    }

    Obj(const FOS_INT_TYPE xint, const char *type_id = EMPTY_CHARS) :
      Obj(Any(xint), OType::INT, id_p(INT_FURI->resolve(type_id))) {
    }

    Obj(const FOS_REAL_TYPE xreal, const char *type_id = EMPTY_CHARS) :
      Obj(Any(xreal), OType::REAL, id_p(REAL_FURI->resolve(type_id))) {
    }

    Obj(const fURI &xuri, const char *type_id = EMPTY_CHARS) :
      Obj(Any(xuri), OType::URI, id_p(URI_FURI->resolve(type_id))) {
    }

    Obj(const char *xstr, const char *type_id = EMPTY_CHARS) :
      Obj(Any(string(xstr)), OType::STR, id_p(STR_FURI->resolve(type_id))) {
    }

    Obj(const string &xstr, const char *type_id = EMPTY_CHARS) :
      Obj(Any(xstr), OType::STR, id_p(STR_FURI->resolve(type_id))) {
    }

    Obj(const std::initializer_list<Pair<const Obj, Obj>> &xrec, const char *type_id = EMPTY_CHARS) :
      Obj(Any(make_shared<RecMap<>>()), OType::REC, id_p(REC_FURI->resolve(type_id))) {
      auto map = this->value<RecMap<>>();
      for (const auto &[key, val]: xrec) {
        map.insert(make_pair(make_shared<Obj>(key), make_shared<Obj>(val)));
      }
    }

    ///////////////////
    Obj(const std::initializer_list<Obj> &xlst, const char *type_id = EMPTY_CHARS) :
      Obj(Any(make_shared<LstList>()), OType::LST, id_p(LST_FURI->resolve(type_id))) {
      const auto list = this->value<LstList_p>();
      for (const auto &obj: xlst) {
        list->push_back(make_shared<Obj>(obj));
      }
    }

    Obj(const List<Inst> &bcode, const char *type_id = EMPTY_CHARS) :
      Obj(Any(make_shared<InstList>(PtrHelper::clone(bcode))), OType::BCODE, id_p(BCODE_FURI->resolve(type_id))) {
    }

    Obj(const InstList_p &bcode, const char *type_id = EMPTY_CHARS) :
      Obj(Any(bcode), OType::BCODE, id_p(BCODE_FURI->resolve(type_id))) {
    }

    //////////////////////////////////////////////////////////////
    [[nodiscard]] virtual Obj_p save(const ID_p &id = nullptr) {
      if (id)
        return make_shared<Obj>(this->value_, this->otype_, this->tid_, id);
      if (this->vid_)
        return make_shared<Obj>(this->value_, this->otype_, this->tid_, this->vid_);
      return shared_from_this();
    }


    [[nodiscard]] OType o_type() const { return this->otype_; }

    template<typename VALUE>
    VALUE value() const {
      try {
        return std::any_cast<VALUE>(this->value_);
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

    [[nodiscard]] FOS_INT_TYPE int_value() const {
      if (!this->is_int())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->value<FOS_INT_TYPE>();
    }

    [[nodiscard]] FOS_REAL_TYPE real_value() const {
      if (!this->is_real())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->value<FOS_REAL_TYPE>();
    }

    [[nodiscard]] fURI uri_value() const {
      if (!this->is_uri())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->value<fURI>();
    }

    [[nodiscard]] ID_p id_p_value() const {
      if (!this->is_uri())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return id_p(this->value<fURI>());
    }

    template<typename FURI>
    [[nodiscard]] ptr<FURI> uri_p_value() const {
      if (!this->is_uri())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return make_shared<FURI>(this->value<FURI>());
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

    [[nodiscard]] LstList_p lst_value() const {
      if (!this->is_lst())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->value<LstList_p>();
    }

    void lst_add(const Obj_p &obj) const {
      if (!this->is_lst())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      this->lst_value()->push_back(obj);
    }

    [[nodiscard]] Int_p lst_size() const { return Obj::to_int(this->lst_value()->size()); }

    [[nodiscard]] Obj_p lst_get(const uint16_t &index) const { return this->lst_get(Obj::to_int(index)); }

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

    [[nodiscard]] Obj_p rec_get(const Obj_p &key, const Runnable &on_error = nullptr) const {
      if (key->is_uri() && key->uri_value().path_length() > 1) {
        const Uri_p segment = to_uri(key->uri_value().path(0));
        Obj_p segment_value = nullptr;
        for (const auto &[k, v]: *this->rec_value()) {
          if (k->match(segment)) {
            segment_value = v;
            break;
          }
        }
        if (!segment_value) {
          if (on_error)
            on_error();
          return Obj::to_noobj();
        }
        if (!segment_value->is_rec())
          throw fError("path %s of %s is not a rec", segment->toString().c_str(), key->toString().c_str(),
                       segment_value->toString().c_str());
        return segment_value->rec_get(to_uri(key->uri_value().path(1, 255)));
      } else {
        for (const auto &[k, v]: *this->rec_value()) {
          if (k->match(key))
            return v;
        }
        return Obj::to_noobj();
      }
      // return this->rec_value()->count(key) ? this->rec_value()->at(key) : Obj::to_noobj();
    }

    [[nodiscard]] Obj_p rec_get(const Obj &key, const Runnable &on_error = nullptr) const {
      return rec_get(make_shared<Obj>(key), on_error);
    }

    [[nodiscard]] Obj_p rec_get(const fURI_p &key, const Runnable &on_error = nullptr) const {
      return rec_get(to_uri(*key), on_error);
    }

    [[nodiscard]] Obj_p rec_get(const char *uri_key, const Runnable &on_error = nullptr) const {
      return rec_get(to_uri(fURI(uri_key)), on_error);
    }

    [[nodiscard]] Rec_p rec_merge(const RecMap_p<> &rmap) {
      this->rec_value()->insert(rmap->begin(), rmap->end());
      return shared_from_this();
    }

    virtual void rec_set(const char *uri_key, const Obj_p &val, const bool nest = true) const {
      this->rec_set(Obj::to_uri(uri_key), val, nest);
    }

    virtual void rec_set(const Obj_p &key, const Obj_p &val, const bool nest = true) const {
      if (nest && key->is_uri() && key->uri_value().path_length() > 1) {
        const Rec *current_rec = const_cast<Rec *>(this);
        for (int i = 0; i < key->uri_value().path_length(); i++) {
          const auto p = string(key->uri_value().path(i));
          if (!current_rec->rec_value()->count(to_uri(p)))
            break;
          if (i == key->uri_value().path_length() - 1)
            current_rec->rec_value()->erase(to_uri(p));
          else {
            const Rec *next_rec = current_rec->rec_value()->at(to_uri(p)).get();
            if (!next_rec->is_rec())
              throw fError("path %s of %s is not a rec", p.c_str(), key->toString().c_str(),
                           next_rec->toString().c_str());
            current_rec = next_rec;
          }
        }
        if (!val->is_noobj()) {
          for (int i = 0; i < key->uri_value().path_length(); i++) {
            const auto p = string(key->uri_value().path(i));
            if (i == key->uri_value().path_length() - 1)
              current_rec->rec_value()->insert({to_uri(p), val});
            else {
              if (!current_rec->rec_value()->count(to_uri(p)))
                current_rec->rec_value()->insert({to_uri(p), Obj::to_rec()});
              current_rec = current_rec->rec_value()->at(to_uri(p)).get();
            }
          }
        }
      } else {
        this->rec_value()->erase(key);
        if (!val->is_noobj())
          this->rec_value()->insert({key, val});
        if (this->vid_)
          ROUTER_WRITE(this->vid_, Obj::to_rec(make_shared<RecMap<>>(*this->rec_value()), id_p(*this->tid_)), true);
      }
    }

    virtual void rec_set(const Obj &key, const Obj &value) const {
      Obj::rec_set(make_shared<Obj>(key), make_shared<Obj>(value));
    }

    virtual void rec_set(const fURI_p &key, const Obj &value) const {
      Obj::rec_set(Obj::to_uri(*key), make_shared<Obj>(value));
    }

    virtual void rec_set(const fURI_p &key, const Obj_p &value) const { Obj::rec_set(Obj::to_uri(*key), value); }

    virtual void rec_add(const Rec_p &other) const {
      for (const auto &[k, v]: *other->rec_value()) {
        if (this->rec_value()->count(k))
          this->rec_value()->erase(k);
        this->rec_set(k, v);
      }
      if (this->vid_)
        ROUTER_WRITE(this->vid_, Obj::to_rec(make_shared<RecMap<>>(*this->rec_value()), id_p(*this->tid_)), true);
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
      return this->tid_->name();
    }

    [[nodiscard]] InstArgs inst_args() const { return std::get<0>(this->inst_value()); }

    /*    [[nodiscard]] static Obj_p inst_arg(const uint8_t index, const InstArgs &args, const Obj_p &lhs) {
          return (0 == index && args.size() == 0) || args.at(0)->equals(*Obj::to_bcode()) ? lhs :
       args.at(index)->apply(lhs);
        }*/

    [[nodiscard]] Obj_p inst_arg(const uint8_t index) const { return std::get<0>(this->inst_value()).at(index); }

    [[nodiscard]] InstFunctionSupplier inst_f() const { return std::get<1>(this->inst_value()); }

    [[nodiscard]] InstSeedSupplier inst_seed_supplier() const { return std::get<3>(this->inst_value()); }

    [[nodiscard]] Obj_p inst_seed(const Obj_p &arg) const { return this->inst_seed_supplier()(arg); }

    [[nodiscard]] Pair<Obj_p, Inst_p> error_value() const {
      if (!this->is_error())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->value<Pair<Obj_p, Inst_p>>();
    }


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
        const auto insts = make_shared<InstList>();
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

    Obj_p none_one_all() {
      if (this->is_objs()) {
        if (this->objs_value()->empty())
          return Obj::to_noobj();
        if (this->objs_value()->size() == 1)
          return this->objs_value()->front();
      }
      return shared_from_this();
    }

    void add_obj(const Obj_p &obj, [[maybe_unused]] const bool mutate = true) {
      if (!this->is_objs())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      if (obj->is_noobj())
        return;
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

    bool equals(const BaseTyped &other) const override { return *this == (Obj &) other; }

    string toString(const bool include_type = true, const bool include_id = true, const bool ansi = true,
                    const bool strict = false) const {
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
            obj_string += obj->toString(include_type, include_id, ansi, strict);
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
            obj_string += k->toString(include_type, include_id, false, strict);
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
            obj_string += arg->toString(include_type, include_id, ansi, strict);
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
              obj_string += inst->toString(include_type, include_id, ansi, strict);
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
            obj_string += obj->toString(include_type, include_id, ansi, strict);
          };
          obj_string += "!m}!!";
          break;
        }
        case OType::ERROR: {
          obj_string = "!r<<!!";
          obj_string += this->error_value().first->toString(include_type, include_id, ansi, strict);
          obj_string += "!r@!!";
          obj_string += this->error_value().second->toString(include_type, include_id, ansi, strict);
          obj_string += "!r>>!!";
          break;
        }
        default:
          throw fError("unknown obj type in toString(): %s", OTypes.to_chars(this->o_type()).c_str());
      }
      if (this->is_error() || (strict && this->is_uri()) || (include_type && (this->tid_->path_length() > 2))) {
        obj_string = string("!b")
            .append(this->tid_->name())
            .append(this->is_inst() ? "!g(!!" : "!g[!!")
            .append(obj_string)
            .append(this->is_inst() ? "!g)!!" : "!g]!!");
      }
      if (include_id && this->vid_) {
        obj_string += "!m@!b";
        obj_string += this->vid_->toString();
        obj_string += "!!";
      }
      return ansi ? obj_string : Ansi<>::strip(obj_string);
    }

    [[nodiscard]] int compare(const Obj &rhs) const { return this->toString().compare(rhs.toString()); }

    bool operator&&(const Obj &rhs) const {
      if (this->is_bool() && rhs.is_bool())
        return this->bool_value() && rhs.bool_value();
      throw fError("%s is not conjunctive (&&)", OTypes.to_chars(this->o_type()).c_str());
    }

    bool operator||(const Obj &rhs) const {
      if (this->is_bool() && rhs.is_bool())
        return this->bool_value() || rhs.bool_value();
      throw fError("%s is not disjunctive (||)", OTypes.to_chars(this->o_type()).c_str());
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
          throw fError("%s is not relational (>)", OTypes.to_chars(this->o_type()).c_str());
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
          throw fError("%s is not relational (<)", OTypes.to_chars(this->o_type()).c_str());
      }
    }

    bool operator<=(const Obj &rhs) const { return *this == rhs || *this < rhs; }

    bool operator>=(const Obj &rhs) const { return *this == rhs || *this > rhs; }

    Obj operator*(const Obj &rhs) const {
      switch (this->o_type()) {
        case OType::NOOBJ:
          return *Obj::to_noobj();
        case OType::BOOL:
          return Obj(this->bool_value() && rhs.bool_value(), OType::BOOL, this->tid_, this->vid_);
        case OType::INT:
          return Obj(this->int_value() * rhs.int_value(), OType::INT, this->tid_, this->vid_);
        case OType::REAL:
          return Obj(this->real_value() * rhs.real_value(), OType::REAL, this->tid_, this->vid_);
        case OType::URI:
          return Obj(this->uri_value().resolve(rhs.uri_value()), OType::URI, this->tid_, this->vid_);
        //   case OType::STR:
        //   return Obj(this->str_value() + rhs.str_value(), this->vid());
        case OType::LST: {
          auto list = std::make_shared<LstList>();
          auto itB = rhs.lst_value()->begin();
          for (auto itA = this->lst_value()->begin(); itA != this->lst_value()->end(); ++itA) {
            list->push_back(make_shared<Obj>(**itA * **itB, itA->get()->o_type(), itA->get()->tid_, itA->get()->vid_));
            ++itB;
          }
          return Lst(list, OType::LST, this->tid_, this->vid_);
        }
        case OType::REC: {
          auto map = std::make_shared<RecMap<>>();
          auto itB = rhs.rec_value()->begin();
          for (auto itA = this->rec_value()->begin(); itA != this->rec_value()->end(); ++itA) {
            map->insert(std::make_pair(
                make_shared<Obj>(*itA->first * *itB->first, itA->first->otype_, itA->first->tid_, itA->first->vid_),
                make_shared<Obj>(*itA->second * *itB->second, itA->second->otype_, itA->second->tid_,
                                 itA->second->vid_)));
            ++itB;
          }
          return Rec(map, OType::REC, this->tid_, this->vid_);
        }
        /*case OType::BCODE: {
          if (rhs.isInst()) {
            return *PtrHelper::no_delete<Obj>((Obj *) this)->add_inst(share(rhs), true);
          } else if (rhs.is_bcode()) {
            return *PtrHelper::no_delete<Obj>((Obj*)this)->add_bcode(share(rhs),true);
           }
        }*/
        default:
          throw fError("%s can not be multiplied (*)", OTypes.to_chars(this->o_type()).c_str());
      }
    }

    Obj operator/(const Obj &rhs) const {
      switch (this->o_type()) {
        case OType::NOOBJ:
          return *to_noobj();
        case OType::INT:
          return Obj(this->int_value() / rhs.int_value(), OType::INT, this->tid_, this->vid_);
        case OType::REAL:
          return Obj(this->real_value() / rhs.real_value(), OType::REAL, this->tid_, this->vid_);
        default:
          throw fError("%s can not be divided (/)", OTypes.to_chars(this->o_type()).c_str());
      }
    }

    Obj operator+(const Obj &rhs) const {
      switch (this->o_type()) {
        case OType::NOOBJ:
          return *to_noobj();
        case OType::BOOL:
          return Obj(this->bool_value() || rhs.bool_value(), OType::BOOL, this->tid_, this->vid_);
        case OType::INT:
          return Obj(this->int_value() + rhs.int_value(), OType::INT, this->tid_, this->vid_);
        case OType::REAL:
          return Obj(this->real_value() + rhs.real_value(), OType::REAL, this->tid_, this->vid_);
        case OType::URI:
          return Obj(this->uri_value().extend(rhs.uri_value()), OType::URI, this->tid_, this->vid_);
        case OType::STR:
          return Obj(string(this->str_value()) + string(rhs.str_value()), OType::STR, this->tid_, this->vid_);
        case OType::LST: {
          auto list = std::make_shared<LstList>();
          for (const auto &obj: *this->lst_value()) {
            list->push_back(obj);
          }
          for (const auto &obj: *rhs.lst_value()) {
            list->push_back(obj);
          }
          return Lst(list, OType::LST, this->tid_, this->vid_);
        }
        case OType::REC: {
          auto map = std::make_shared<RecMap<>>();
          for (const auto &pair: *this->rec_value()) {
            map->insert(pair);
          }
          for (const auto &pair: *rhs.rec_value()) {
            map->insert(pair);
          }
          return Rec(map, OType::REC, this->tid_, this->vid_);
        }
        default:
          throw fError("%s can not be added (+)", OTypes.to_chars(this->o_type()).c_str());
      }
    }

    Obj operator-(const Obj &rhs) const {
      switch (this->o_type()) {
        case OType::NOOBJ:
          return *Obj::to_noobj();
        case OType::BOOL:
          return Bool(!this->bool_value(), OType::BOOL, this->tid_, this->vid_);
        case OType::INT:
          return Int(this->int_value() - rhs.int_value(), OType::INT, this->tid_, this->vid_);
        case OType::REAL:
          return Real(this->real_value() - rhs.real_value(), OType::REAL, this->tid_, this->vid_);
        case OType::URI:
          return Uri(this->uri_value().retract(), OType::URI, this->tid_, this->vid_);
        // case OType::STR:
        //  return Obj(string(this->str_value()).replace(string(rhs.str_value()), this->vid());
        case OType::LST: {
          auto list = std::make_shared<LstList>();
          for (const auto &obj: *this->lst_value()) {
            if (std::find(rhs.lst_value()->begin(), rhs.lst_value()->end(), obj) != std::end(*rhs.lst_value()))
              list->push_back(obj);
          }
          return Lst(list, OType::LST, this->tid_, this->vid_);
        }
        case OType::REC: {
          auto map = std::make_shared<RecMap<>>();
          for (const auto &pair: *this->rec_value()) {
            map->insert(pair);
          }
          for (const auto &pair: *rhs.rec_value()) {
            map->insert(pair);
          }
          return Rec(map, OType::REC, this->tid_, this->vid_);
        }
        default:
          throw fError("%s can not be subtracted (-)", OTypes.to_chars(this->o_type()).c_str());
      }
    }

    Obj operator%(const Obj &other) const {
      switch (this->o_type()) {
        case OType::INT:
          return Obj(this->int_value() % other.int_value(), OType::INT, this->tid_, this->vid_);
        default:
          throw fError("%s can not be moduloed (%)", OTypes.to_chars(this->o_type()).c_str());
      }
    }

    bool operator!=(const Obj &other) const { return !(*this == other); }

    bool operator==(const Obj &other) const {
      if (!this->tid_->equals(*other.tid_)) // type check
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
          return this->uri_value() == other.uri_value();
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
          const auto map_a = this->rec_value();
          const auto map_b = other.rec_value();
          if (map_a->size() != map_b->size())
            return false;
          for (const auto &[first, second]: *map_a) {
            if (!map_b->count(first) || !map_b->at(first)->equals(*second))
              return false;
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
          throw fError("Unknown obj type in ==: %s", OTypes.to_chars(this->o_type()).c_str());
      }
    }

    [[nodiscard]] Obj operator[](const Obj &key) const {
      switch (this->o_type()) {
        case OType::STR:
          return *this->str_get(share(key));
        case OType::LST:
          return *this->lst_get(share(key));
        case OType::REC:
          return *this->rec_get(share(key));
        default:
          throw fError("Unknown obj type in []: %s", OTypes.to_chars(this->o_type()).c_str());
      }
    }

    [[nodiscard]] Obj_p operator[](const char *id) const { return this->rec_get(id_p(id)); }

    [[nodiscard]] bool is_noobj() const { return this->o_type() == OType::NOOBJ; }

    [[nodiscard]] bool is_bool() const { return this->o_type() == OType::BOOL; }

    [[nodiscard]] bool is_int() const { return this->o_type() == OType::INT; }

    [[nodiscard]] bool is_real() const { return this->o_type() == OType::REAL; }

    [[nodiscard]] bool is_uri() const { return this->o_type() == OType::URI; }

    [[nodiscard]] bool is_str() const { return this->o_type() == OType::STR; }

    [[nodiscard]] bool is_lst() const { return this->o_type() == OType::LST; }

    [[nodiscard]] bool is_poly() const {
      return this->is_lst() || this->is_rec(); // || this->is_objs() /*|| this->is_bcode() || this->is_inst()
    }

    [[nodiscard]] bool is_rec() const { return this->o_type() == OType::REC; }

    [[nodiscard]] bool is_inst() const { return this->o_type() == OType::INST; }

    [[nodiscard]] bool is_objs() const { return this->o_type() == OType::OBJS; }

    [[nodiscard]] bool is_bcode() const { return this->o_type() == OType::BCODE; }

    [[nodiscard]] bool is_code() const { return this->is_bcode() || this->is_inst(); }

    [[nodiscard]] bool is_error() const { return this->o_type() == OType::ERROR; }

    [[nodiscard]] bool is_noop_bcode() const { return this->is_bcode() && this->bcode_value()->empty(); }


    Obj_p apply(const Obj_p &lhs, const InstArgs &args) {
      if (this->is_inst() && !args.empty()) {
        return this->inst_f()(args)(lhs);
      } else if (this->is_bcode() && !args.empty()) {
        return replace_from_bcode(shared_from_this(), args)->apply(lhs);
      } else {
        return this->apply(lhs);
      }
    }

    /////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    static Inst_p replace_from_inst(const Obj_p &old_inst, const InstArgs &args, const Obj_p &lhs = Obj::to_noobj()) {
      if (old_inst->inst_op() == "lambda") {
        return Obj::to_inst(old_inst->inst_op(), args, old_inst->inst_f(), old_inst->itype(),
                            old_inst->inst_seed_supplier());
      }
      const bool is_from = old_inst->inst_op() == "from";
      if (is_from && old_inst->inst_arg(0)->is_uri() && old_inst->inst_arg(0)->uri_value().toString()[0] == '_') {
        const uint8_t index = stoi(old_inst->inst_arg(0)->uri_value().name().substr(1));
        if (index < args.size())
          return args.at(index);
        if (old_inst->inst_args().size() == 2)
          return old_inst->inst_args().at(1); // default argument
        throw fError("%s requires !y%i!! arguments and !y%i!! were provided", old_inst->toString().c_str(),
                     old_inst->inst_args().size(), args.size());

      } else if (is_from && old_inst->inst_arg(0)->toString() == "_") {
        return lhs;
      } else {
        InstArgs new_args;
        for (const Obj_p &old_arg: old_inst->inst_args()) {
          new_args.push_back(replace_from_obj(old_arg, args, lhs));
        }
        return Obj::to_inst(old_inst->inst_op(), new_args, old_inst->inst_f(), old_inst->itype(),
                            old_inst->inst_seed_supplier());
      }
    }

    static Obj_p replace_from_obj(const Obj_p &old_obj, const InstArgs &args, const Obj_p &lhs = Obj::to_noobj()) {
      if (old_obj->is_inst())
        return replace_from_inst(old_obj, args, lhs);
      else if (old_obj->is_bcode())
        return replace_from_bcode(old_obj, args, lhs);
      else if (old_obj->is_rec())
        return replace_from_rec(old_obj, args, lhs);
      else if (old_obj->is_lst())
        return replace_from_lst(old_obj, args, lhs);
      else
        return old_obj;
    }

    static BCode_p replace_from_bcode(const Obj_p &old_bcode, const InstArgs &args,
                                      const Obj_p &lhs = Obj::to_noobj()) {
      BCode_p new_bcode = Obj::to_bcode();
      LOG(TRACE, "old bcode: %s\n", old_bcode->toString().c_str());
      for (const Inst_p &old_inst: *old_bcode->bcode_value()) {
        LOG(TRACE, "replacing old bcode inst: %s\n", old_inst->toString().c_str());
        const Inst_p new_inst = replace_from_inst(old_inst, args, lhs);
        new_bcode = new_bcode->add_inst(new_inst);
      }
      LOG(TRACE, "new bcode: %s\n", new_bcode->toString().c_str());
      return new_bcode;
    }

    static Rec_p replace_from_rec(const Obj_p &old_rec, const InstArgs &args, const Obj_p &lhs = Obj::to_noobj()) {
      Rec_p new_rec = Obj::to_rec();
      for (const auto &[key, value]: *old_rec->rec_value()) {
        new_rec->rec_set(replace_from_obj(key, args, lhs), replace_from_obj(value, args, lhs));
      }
      return new_rec;
    }

    static Lst_p replace_from_lst(const Obj_p &old_lst, const InstArgs &args, const Obj_p &lhs = Obj::to_noobj()) {
      Lst_p new_lst = Obj::to_lst();
      for (const auto &element: *old_lst->lst_value()) {
        new_lst->lst_add(replace_from_obj(element, args, lhs));
      }
      return new_lst;
    }

    /////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////

    Obj_p apply(const Obj_p &lhs) {
      if (lhs->is_error())
        return lhs;
      switch (this->o_type()) {
        case OType::BOOL:
          return shared_from_this(); //PtrHelper::no_delete(this);
        case OType::INT:
          return shared_from_this(); //PtrHelper::no_delete(this);
        case OType::REAL:
          return shared_from_this(); //PtrHelper::no_delete(this);
        case OType::URI: {
          return (lhs->is_uri() && this->uri_value().is_relative())
                   ? Obj::to_uri(lhs->uri_value().resolve(this->uri_value()))
                   : shared_from_this(); //PtrHelper::no_delete(this);
        }
        case OType::STR:
          return shared_from_this(); //PtrHelper::no_delete<Obj>(this);
        case OType::LST: {
          const auto new_values = make_shared<LstList>();
          for (const auto &obj: *this->lst_value()) {
            new_values->push_back(obj->apply(lhs));
          }
          return Obj::to_lst(new_values);
        }
        case OType::REC: {
          const auto new_pairs = make_shared<RecMap<>>();
          for (const auto &[key, value]: *this->rec_value()) {
            const Obj_p key_apply = key->apply(lhs);
            if (!key_apply->is_noobj())
              new_pairs->insert({key_apply, value->apply(key_apply)});
          }
          return Obj::to_rec(new_pairs, id_p(*this->tid_));
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
          return current_obj->is_objs() ? current_obj->objs_value()->front() : current_obj->clone();
          // objs unrolled (front popped)
        }
        case OType::OBJS: {
          Objs_p objs = Obj::to_objs();
          for (const Obj_p &obj: *this->objs_value()) {
            objs->add_obj(obj->apply(lhs));
          }
          return objs;
        }
        case OType::NOOBJ:
          return Obj::to_noobj();
        case OType::ERROR:
          return shared_from_this();
        default:
          throw fError("Unknown obj type in apply(): %s", OTypes.to_chars(this->o_type()).c_str());
      }
    }

    [[nodiscard]] bool is_base_type() const { return this->tid_->equals(*OTYPE_FURI.at(this->otype_)); }

    // const fURI type() const { return this->_id->authority(""); }

    [[nodiscard]] bool match(const Obj_p &type_obj, const bool require_same_type_id = true) const {
      // LOG(TRACE, "!ymatching!!: %s ~ %s\n", this->toString().c_str(), type->toString().c_str());
      if (type_obj->is_noop_bcode())
        return true;
      if (type_obj->is_bcode() && !this->is_bcode())
        return !type_obj->apply(this->clone())->is_noobj();
      if (this->o_type() != type_obj->o_type())
        return false;
      if (require_same_type_id && (*this->tid_ != *type_obj->tid_))
        return false;
      switch (this->o_type()) {
        case OType::NOOBJ:
          return true;
        case OType::BOOL:
          return this->bool_value() == type_obj->bool_value();
        case OType::INT:
          return this->int_value() == type_obj->int_value();
        case OType::REAL:
          return this->real_value() == type_obj->real_value();
        case OType::URI:
          return this->uri_value().matches(type_obj->uri_value());
        case OType::STR:
          return this->str_value() == type_obj->str_value();
        case OType::LST: {
          const auto objs_a = this->lst_value();
          const auto objs_b = type_obj->lst_value();
          if (objs_a->size() != objs_b->size())
            return false;
          auto b = objs_b->begin();
          for (const auto &a: *objs_a) {
            if (!a->match(*b))
              return false;
            ++b;
          }
          return true;
        }
        case OType::REC: {
          const auto pairs_a = this->rec_value();
          const auto pairs_b = type_obj->rec_value();
          for (const auto &[b_id, b_obj]: *pairs_b) {
            bool found = false;
            if (!found) {
              for (const auto &[a_id, a_obj]: *pairs_a) {
                if (a_id->match(b_id) && a_obj->match(b_obj)) {
                  found = true;
                  break;
                }
              }
            }
            if (!found)
              return false;
          }
          return true;
        }
        case OType::INST: {
          const auto args_a = this->inst_args();
          auto args_b = type_obj->inst_args();
          if (args_a.size() != args_b.size())
            return false;
          if (this->itype() != type_obj->itype())
            return false;
          const auto b = args_b.begin();
          for (const auto &a: args_a) {
            if (!a->match(*b))
              return false;
          }
          return true;
        }
        case OType::BCODE: {
          /*const auto insts_a = this->bcode_value();
          const auto insts_b = type_obj->bcode_value();
          if (insts_a->size() != insts_b->size())
            return false;
          const auto b = insts_b->begin();
          for (const auto &a: *insts_a) {
            if (!a->match(*b))
              return false;
          }*/
          return true;
        }
        default:
          throw fError("Unknown obj type in match(): %s", OTypes.to_chars(this->o_type()).c_str());
      }
      return false;
    }

    [[nodiscard]] Obj_p as(const ID_p &type_id) const {
      Obj_p obj = TYPE_MAKER(make_shared<Obj>(*this), type_id); // TODO: expensive.
      return obj;
    }

    Obj_p as(const char *furi) const { return this->as(id_p(furi)); }

    Obj_p at(const ID_p &value_id) { return make_shared<Obj>(this->value_, this->otype_, this->tid_, value_id); }

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
      static auto noobj = make_shared<Obj>(Any(nullptr), OType::NOOBJ, NOOBJ_FURI);
      return noobj;
    }

    static Bool_p to_bool(const bool value, const ID_p &furi = BOOL_FURI) {
      return make_shared<Bool>(value, OType::BOOL, furi);
    }

    static Int_p to_int(const FOS_INT_TYPE value, const ID_p &furi = INT_FURI) {
      return make_shared<Int>(value, OType::INT, furi);
    }

    static Real_p to_real(const FOS_REAL_TYPE value, const ID_p &furi = REAL_FURI) {
      return make_shared<Real>(value, OType::REAL, furi);
    }

    static Str_p to_str(const string &value, const ID_p &furi = STR_FURI) {
      return make_shared<Str>(value, OType::STR, furi);
    }

    static Str_p to_str(const char *value, const ID_p &furi = STR_FURI) {
      return make_shared<Str>(string(value), OType::STR, furi);
    }

    static Uri_p to_uri(const fURI &value, const ID_p &furi = URI_FURI) {
      return make_shared<Uri>(value, OType::URI, furi);
    }

    static Uri_p to_uri(const char *value, const ID_p &furi = URI_FURI) {
      return make_shared<Uri>(fURI(value), OType::URI, furi);
    }

    static Lst_p to_lst(const ID_p &furi = LST_FURI) {
      return make_shared<Lst>(make_shared<LstList>(), OType::LST, furi);
    }

    static Lst_p to_lst(const LstList_p &xlst, const ID_p &furi = LST_FURI) {
      return make_shared<Lst>(xlst, OType::LST, furi);
    }

    static Lst_p to_lst(const std::initializer_list<Obj> &xlst, const ID_p &furi = LST_FURI) {
      const auto list = make_shared<LstList>();
      for (const auto &obj: xlst) {
        list->push_back(share(obj));
      }
      return to_lst(list, furi);
    }

    static Lst_p to_lst(const std::initializer_list<Obj_p> &xlst, const ID_p &furi = LST_FURI) {
      return to_lst(make_shared<LstList>(xlst), furi);
    }

    static Rec_p to_rec(const ID_p &type = REC_FURI, const ID_p &id = nullptr) {
      return make_shared<Rec>(make_shared<RecMap<>>(), OType::REC, type, id);
    }

    static Rec_p to_rec(const RecMap_p<> &map, const ID_p &type = REC_FURI, const ID_p &id = nullptr) {
      return make_shared<Rec>(map, OType::REC, type, id);
    }

    static Rec_p to_rec(const std::initializer_list<Pair<const Obj, Obj>> &xrec, const ID_p &type = REC_FURI,
                        const ID_p &id = nullptr) {
      const auto map = make_shared<Obj::RecMap<>>();
      for (const auto &[key, value]: xrec) {
        map->insert(make_pair(make_shared<Obj>(key), make_shared<Obj>(value)));
      }
      return to_rec(map, type, id);
    }

    static Rec_p to_rec(const std::initializer_list<Pair<const Obj_p, Obj_p>> &xrec, const ID_p &type = REC_FURI,
                        const ID_p &id = nullptr) {
      const auto map = make_shared<Obj::RecMap<>>();
      for (const auto &[key, value]: xrec) {
        map->insert(make_pair(key, value));
      }
      return to_rec(map, type, id);
    }

    static Rec_p to_rec(const std::initializer_list<Pair<const string, Obj_p>> &xrec, const ID_p &type = REC_FURI,
                        const ID_p &id = nullptr) {
      const auto map = make_shared<Obj::RecMap<>>();
      for (const auto &[key, value]: xrec) {
        map->insert(make_pair(Obj::to_uri(key), value));
      }
      return to_rec(map, type, id);
    }

    static InstSeedSupplier objs_seed() {
      return [](const Obj_p &) { return to_objs(); };
    }

    static InstSeedSupplier noobj_seed() {
      return [](const Obj_p &) { return to_noobj(); };
    }

    static Inst_p to_inst(const InstValue &value, const ID_p &type = INST_FURI) {
      return make_shared<Inst>(value, OType::INST, type);
    }

    static Inst_p to_inst(const string &opcode, const List<Obj_p> &args, const InstFunctionSupplier &function,
                          const IType itype, const InstSeedSupplier &seed = noobj_seed(), const ID_p &type = nullptr) {
      const ID_p fix = type ? type : id_p(INST_FURI->resolve(opcode));
      return to_inst({args, function, itype, seed}, fix);
    }

    static BCode_p to_bcode(const InstList_p &insts, const ID_p &furi = BCODE_FURI) {
      return make_shared<BCode>(insts, OType::BCODE, furi);
    }

    static BCode_p to_bcode(const InstList &insts, const ID_p &furi = BCODE_FURI) {
      return Obj::to_bcode(make_shared<InstList>(insts), furi);
    }


    static BCode_p to_bcode(const ID_p &furi = BCODE_FURI) { return Obj::to_bcode(share<InstList>({}), furi); }

    static BCode_p to_bcode(const BiFunction<Obj_p, InstArgs, Obj_p> &function, const InstArgs &args,
                            const ID &opcode = ID("cxx:bifunc")) {
      return Obj::to_bcode({Obj::lambda(
          [function](const Obj_p &obj, const InstArgs &args2) { return function(obj, args2); }, args, opcode)});
    }

    static BCode_p to_bcode(const Function<Obj_p, Obj_p> &function, const ID &opcode = ID("cxx:func")) {
      return Obj::to_bcode(
          {Obj::lambda([function](const Obj_p &obj, const InstArgs &) { return function(obj); }, {}, opcode)});
    }

    static Obj_p lambda(const BiFunction<Obj_p, InstArgs, Obj_p> &function, const InstArgs &args, const ID &opcode) {
      const ID_p inst_id = id_p(INST_FURI->extend(opcode));
      const Inst_p inst = Obj::to_inst(
          inst_id->name(), args,
          [function](const InstArgs &args2) {
            return [function, args2](const Obj_p &input) { return function(input, args2); };
          },
          IType::ONE_TO_ONE, noobj_seed(), inst_id);
      // TYPE_SAVER(inst_id, inst);
      return inst;
    }

    static Objs_p to_objs(const ID_p &type_id = OBJS_FURI) {
      // fError::OTYPE_CHECK(furi->path(FOS_BASE_TYPE_INDEX), OTypes.to_chars(OType::OBJS));
      return to_objs(make_shared<List<Obj_p>>(), type_id);
    }

    static Objs_p to_objs(const List_p<Obj_p> &objs, const ID_p &type_id = OBJS_FURI) {
      // fError::OTYPE_CHECK(furi->path(FOS_BASE_TYPE_INDEX), OTypes.to_chars(OType::OBJS));
      return make_shared<Objs>(objs, OType::OBJS, type_id);
    }

    static Objs_p to_objs(const List<Obj> &objs, const ID_p &type_id = OBJS_FURI) {
      return Obj::to_objs(make_shared<List<Obj_p>>(PtrHelper::clone<Obj>(objs)), type_id);
    }

    static Error_p to_error(const Obj_p &obj, const Inst_p &inst, const ID_p &type_id = ERROR_FURI) {
      return make_shared<Error>(Pair<Obj_p, Inst_p>(obj, inst), OType::ERROR, type_id);
    }

    /*std::__allocator_base<Obj> allocator = std::allocator<Obj>()*/
    Obj_p clone() const {
      const ID_p type_id_clone = id_p(*this->tid_);
      if (this->is_rec()) {
        // REC
        const auto new_map = make_shared<RecMap<>>();
        for (const auto &[k, v]: *this->rec_value()) {
          new_map->insert({k->clone(), v->clone()});
        }
        auto r = make_shared<Rec>(new_map, OType::REC, type_id_clone);
        r->vid_ = this->vid_;
        return r;
      }
      if (this->is_lst()) {
        // LST
        const auto new_list = make_shared<LstList>();
        for (const auto &e: *this->lst_value()) {
          new_list->push_back(e->clone());
        }
        auto r = make_shared<Lst>(new_list, OType::LST, type_id_clone);
        r->vid_ = this->vid_;
        return r;
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
        auto r = make_shared<Objs>(new_list, OType::OBJS, type_id_clone);
        r->vid_ = this->vid_;
        return r;
      }
      auto r = make_shared<Obj>(any(this->value_), this->otype_, type_id_clone);
      r->vid_ = this->vid_;
      return r;
    }

    BObj_p serialize() const {
      LOG(DEBUG, "Serializing obj %s\n", this->toString().c_str());
      const string serial = this->toString(true, true, false, true);
      return ptr<BObj>(new BObj{serial.length(), reinterpret_cast<fbyte *>(strdup(serial.c_str()))}, bobj_deleter);
    }

    static Obj_p deserialize(const BObj_p &bobj) {
      LOG(DEBUG, "Deserializing bytes %s (length %i)\n", bobj->second, bobj->first);
      return OBJ_PARSER(string(reinterpret_cast<char *>(bobj->second), bobj->first));
    }
  };

  [[maybe_unused]] static Uri_p vri(const fURI &xuri, const ID_p &type = URI_FURI) { return Obj::to_uri(xuri, type); }

  [[maybe_unused]] static Uri_p vri(const fURI_p &xuri, const ID_p &type = URI_FURI) {
    return Obj::to_uri(*xuri, type);
  }

  [[maybe_unused]] static Uri_p vri(const char *xuri, const ID_p &type = URI_FURI) {
    return Obj::to_uri(fURI(xuri), type);
  }

  [[maybe_unused]] static Uri_p vri(const string &xuri, const ID_p &type = URI_FURI) {
    return Obj::to_uri(fURI(xuri), type);
  }

  [[maybe_unused]] static Bool_p dool(const bool xbool, const ID_p &type = BOOL_FURI) {
    return Obj::to_bool(xbool, type);
  }

  [[maybe_unused]] static Int_p jnt(const FOS_INT_TYPE xint, const ID_p &type = INT_FURI) {
    return Obj::to_int(xint, type);
  }

  [[maybe_unused]] static Str_p str(const char *xstr) { return Obj::to_str(xstr); }

  [[maybe_unused]] static Str_p str(const string &xstr) { return Obj::to_str(xstr); }

  [[maybe_unused]] static Real_p real(const FOS_REAL_TYPE &xreal, const ID_p &type = REAL_FURI) {
    return Obj::to_real(xreal, type);
  }

  [[maybe_unused]] static NoObj_p noobj() { return Obj::to_noobj(); }

  [[maybe_unused]] static Lst_p lst() { return Obj::to_lst(make_shared<Obj::LstList>()); }

  [[maybe_unused]] static Lst_p lst(const List<Obj_p> &list) { return Obj::to_lst(share(list)); }

  [[maybe_unused]] static Lst_p lst(const List_p<Obj_p> &list) { return Obj::to_lst(list); }

  [[maybe_unused]] static Rec_p rec() { return Obj::to_rec(make_shared<Obj::RecMap<>>()); }

  [[maybe_unused]] static Rec_p rec(const Obj::RecMap<> &map, const ID_p &tid = REC_FURI, const ID_p &vid = nullptr) {
    return Obj::to_rec(make_shared<Obj::RecMap<>>(map), tid, vid);
  }

  [[maybe_unused]] static Rec_p rec(const std::initializer_list<Pair<const Obj_p, Obj_p>> &map,
                                    const ID_p &type = REC_FURI, const ID_p &id = nullptr) {
    return Obj::to_rec(map, type, id);
  }

  [[maybe_unused]] static Objs_p objs() { return Obj::to_objs(make_shared<List<Obj_p>>()); }

  [[maybe_unused]] static Objs_p objs(const List<Obj_p> &list) { return Obj::to_objs(make_shared<List<Obj_p>>(list)); }

  [[maybe_unused]] static Objs_p objs(const List_p<Obj_p> &list) { return Obj::to_objs(list); }

  [[maybe_unused]] static BCode_p bcode(const InstList &list) { return Obj::to_bcode(list); }

  [[maybe_unused]] static BCode_p bcode() { return Obj::to_bcode(); }

  [[maybe_unused]] static InstFunctionSupplier noobj_func() {
    return [](const InstArgs &) { return [](const Obj_p &) { return noobj(); }; };
  }

  static Obj::RecMap_p<> rmap(const initializer_list<Pair<fURI, Obj_p>> &pairs) {
    auto m = make_shared<Obj::RecMap<>>();
    for (const auto &[id, obj]: pairs) {
      m->insert({vri(id), obj});
    }
    return m;
  }

  static Obj::RecMap_p<> rmap(const initializer_list<Pair<ID_p, Obj_p>> &pairs) {
    auto m = make_shared<Obj::RecMap<>>();
    for (const auto &[id, obj]: pairs) {
      m->insert({vri(id), obj});
    }
    return m;
  }

  /*  static Obj::RecMap_p<> rmap_merge(const Obj::RecMap_p<> &a, const Obj::RecMap_p<> &b) {
      const Obj::RecMap_p<> c = make_shared<Obj::RecMap<>>();
      c->insert(a->begin(), a->end());
      c->insert(b->begin(), b->end());
      return c;

    }*/

} // namespace fhatos
#endif