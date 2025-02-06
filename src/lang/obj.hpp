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

#include <any>

#define MMADT_INST_SCHEME COMPONENT_SEPARATOR MMADT_SCHEME
#define MMADT_SCHEME "/mmadt"

#include "../fhatos.hpp"
#include "../furi.hpp"
#include "../util/tsl/ordered_map.h"
#include "mmadt/compiler.hpp"
#include <utility>
#include <variant>


namespace fhatos {
  /// @brief The base types of mm-ADT
  enum class OType {
    /// The base type of all types is the obj
    OBJ,
    /// A "null" object type used to kill a processing monad
    NOOBJ,
    /// A obj reference to a type id (internal bcode is the compilation)
    TYPE,
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

  static const auto OTypes = Enums<OType>({
    {OType::OBJ, "obj"},
    {OType::NOOBJ, "noobj"},
    {OType::TYPE, "type"},
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

  using Obj_p = shared_ptr<const Obj>;
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
  using Ctx = Obj;
  using Ctx_p = Obj_p;
  using Inst = Obj;
  using Inst_p = Obj_p;
  using BCode = Obj;
  using BCode_p = Obj_p;
  using Objs = Obj;
  using Objs_p = Obj_p;
  using Type = Obj;
  using Type_p = Obj_p;
  using Error = Obj;
  using Error_p = Obj_p;
  using BObj = Pair<uint32_t, fbyte *>;
  using BObj_p = ptr<Pair<uint32_t, fbyte *>>;

  static Consumer<BObj *> bobj_deleter = [](const BObj *bobj) {
    free(bobj->second);
    delete bobj;
  };

  template<typename KEY, typename VALUE>
  const VALUE &get_with_default(const Map<KEY, VALUE> &m, const KEY &key, const VALUE &default_value) {
    auto it = m.find(key);
    if(it == m.end()) return default_value;
    return it->second;
  }

  const Map<const Pair<IntCoefficient, IntCoefficient>, string> SIGNATURE_DESCRIPTIONS = {
    {{{0, 0}, {0, 0}}, ".->. (transient)"},
    {{{0, 0}, {1, 1}}, ".->o (supplier)"},
    {{{0, 0}, {0,INT_MAX}}, ".->* (initial)"},
    {{{1, 1}, {0, 0}}, "o->. (consumer)"},
    {{{0,INT_MAX}, {0, 0}}, "*->. (terminal)"},
    {{{1, 1}, {1, 1}}, "o->o (map)"},
    {{{1, 1}, {0,INT_MAX}}, "o->* (flatmap)"},
    {{{0, 1}, {1,INT_MAX}}, "?->* (potential)"},
    {{{0, 1}, {0, 0}}, "?->o (flip)"},
    {{{0, 1}, {0, 0}}, "?->. (spark)"},
    {{{0, 1}, {0, 1}}, "?->? (flux)"},
    {{{1, 1}, {0, 1}}, "o->? (filter)"},
    {{{0, 0}, {0, 1}}, ".->? (check)"},
    {{{0, 1}, {0, 1}}, "?-? (strain)"},
    {{{0,INT_MAX}, {1, 1}}, "*->o (reduce)"},
    {{{0,INT_MAX}, {0,INT_MAX}}, "*->* (barrier)"},
  };
  using InstArgs = Rec_p;
  using Cpp = BiFunction<const Obj_p, const InstArgs, Obj_p>;
  using Cpp_p = ptr<Cpp>;
  using InstF = std::variant<Obj_p, Cpp_p>;
  using InstF_p = ptr<InstF>;
  using InstValue = Trip<InstArgs, InstF_p, Obj_p>;
  using InstValue_p = ptr<InstValue>;
  using InstList = List<Inst_p>;
  using InstList_p = ptr<InstList>;
  static const auto MMADT_ID = make_shared<ID>(MMADT_SCHEME);
  static const auto OBJ_FURI = make_shared<ID>(MMADT_SCHEME "/obj");
  static const auto NOOBJ_FURI = make_shared<ID>(MMADT_SCHEME "/noobj");
  static const auto TYPE_FURI = make_shared<ID>(MMADT_SCHEME "/type");
  static const auto BOOL_FURI = make_shared<ID>(MMADT_SCHEME "/bool");
  static const auto INT_FURI = make_shared<ID>(MMADT_SCHEME "/int");
  static const auto REAL_FURI = make_shared<ID>(MMADT_SCHEME "/real");
  static const auto URI_FURI = make_shared<ID>(MMADT_SCHEME "/uri");
  static const auto STR_FURI = make_shared<ID>(MMADT_SCHEME "/str");
  static const auto LST_FURI = make_shared<ID>(MMADT_SCHEME "/lst");
  static const auto REC_FURI = make_shared<ID>(MMADT_SCHEME "/rec");
  static const auto INST_FURI = make_shared<ID>(MMADT_SCHEME "/inst");
  static const auto BCODE_FURI = make_shared<ID>(MMADT_SCHEME "/bcode");
  static const auto ERROR_FURI = make_shared<ID>(MMADT_SCHEME "/error");
  static const auto OBJS_FURI = make_shared<ID>(MMADT_SCHEME "/objs");


  template<typename T>
  using Coefficient = Pair<T, T>;
  using IntCoefficient = Coefficient<FOS_INT_TYPE>;

  struct DomainRange {
    fURI domain;
    IntCoefficient dom_coeff;
    fURI range;
    IntCoefficient rng_coeff;

    explicit DomainRange(const fURI &domain, const IntCoefficient &dom_coeff, const fURI &range,
                         const IntCoefficient &rng_coeff) : domain{domain},
                                                            dom_coeff{dom_coeff},
                                                            range{range},
                                                            rng_coeff{rng_coeff} {
    }

    [[nodiscard]] bool equals(const DomainRange &other) const {
      return this->domain == other.domain &&
             this->dom_coeff == other.dom_coeff &&
             this->range == other.range &&
             this->rng_coeff == other.rng_coeff;
    }

    [[nodiscard]] bool within_domain(const IntCoefficient &other_coeff) const {
      return this->dom_coeff.first >= other_coeff.first && this->dom_coeff.second <= other_coeff.second;
    }

    [[nodiscard]] bool within_range(const IntCoefficient &other_coeff) const {
      return this->rng_coeff.first >= other_coeff.first && this->rng_coeff.second <= other_coeff.second;
    }

    [[nodiscard]] bool is_single() const {
      return this->dom_coeff == IntCoefficient(1, 1) && this->rng_coeff == IntCoefficient(1, 1);
    }

    static DomainRange from(const fURI &furi) {
      const std::vector<string> dom_coeff_str = furi.query_values(FOS_DOM_COEF);
      const IntCoefficient dom_coeff = dom_coeff_str.empty()
                                         ? IntCoefficient(1, 1)
                                         : IntCoefficient(stoi(dom_coeff_str.at(0)), stoi(dom_coeff_str.at(1)));
      const std::vector<string> rng_coeff_str = furi.query_values(FOS_RNG_COEF);
      const IntCoefficient rng_coeff = rng_coeff_str.empty()
                                         ? IntCoefficient(1, 1)
                                         : IntCoefficient(stoi(rng_coeff_str.at(0)), stoi(rng_coeff_str.at(1)));
      const auto dr = DomainRange(fURI(furi.query_value(FOS_DOMAIN).value_or(OBJ_FURI->toString())),
                                  dom_coeff,
                                  fURI(furi.query_value(FOS_RANGE).value_or(furi.no_query().toString())),
                                  rng_coeff);
      return dr;
    }
  };

  static const Map<const OType, shared_ptr<ID>, std::less<const OType>> OTYPE_FURI = {{{OType::NOOBJ, NOOBJ_FURI},
    {OType::OBJ, OBJ_FURI},
    {OType::TYPE, TYPE_FURI},
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
    {*TYPE_FURI, OType::TYPE},
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

  static ID_p SCHEDULER_ID = nullptr;
  static ID_p ROUTER_ID = nullptr;


  struct ObjPrinter {
    bool show_id;
    bool show_type;
    bool show_domain_range;
    bool strict;
    bool ansi;
    bool propagate;

    [[nodiscard]] const ObjPrinter *next() const {
      return this->propagate ? this : nullptr;
    }

    [[nodiscard]] unique_ptr<ObjPrinter> clone() const {
      return make_unique<ObjPrinter>(ObjPrinter{
        this->show_id,
        this->show_type,
        this->show_domain_range,
        this->strict,
        this->ansi,
        this->propagate});
    }
  };

  static auto NO_ANSI_PRINTER = new ObjPrinter{
    .show_id = true,
    .show_type = true,
    .show_domain_range = false,
    .strict = false,
    .ansi = false,
    .propagate = false
  };
  static auto SERIALIZER_PRINTER = new ObjPrinter{
    .show_id = true,
    .show_type = true,
    .show_domain_range = true,
    .strict = true,
    .ansi = false,
    .propagate = true
  };
  static auto DEFAULT_OBJ_PRINTER = new ObjPrinter{
    .show_id = true,
    .show_type = true,
    .show_domain_range = false,
    .strict = false,
    .ansi = true,
    .propagate = false
  };
  static auto DEFAULT_INST_PRINTER = new ObjPrinter{
    .show_id = true,
    .show_type = true,
    .show_domain_range = true,
    .strict = false,
    .ansi = true,
    .propagate = false
  };
  static auto DEFAULT_BCODE_PRINTER = new ObjPrinter{
    .show_id = true,
    .show_type = true,
    .show_domain_range = true,
    .strict = false,
    .ansi = true,
    .propagate = false
  };
  static auto DEFAULT_NOOBJ_PRINTER = new ObjPrinter{
    .show_id = false,
    .show_type = false,
    .show_domain_range = false,
    .strict = false,
    .ansi = true,
    .propagate = false
  };
  static Map<OType, ObjPrinter *> GLOBAL_PRINTERS = {
    {OType::NOOBJ, DEFAULT_NOOBJ_PRINTER},
    {OType::TYPE, DEFAULT_OBJ_PRINTER},
    {OType::OBJ, DEFAULT_OBJ_PRINTER},
    {OType::BOOL, DEFAULT_OBJ_PRINTER},
    {OType::INT, DEFAULT_OBJ_PRINTER},
    {OType::REAL, DEFAULT_OBJ_PRINTER},
    {OType::STR, DEFAULT_OBJ_PRINTER},
    {OType::URI, DEFAULT_OBJ_PRINTER},
    {OType::LST, DEFAULT_OBJ_PRINTER},
    {OType::REC, DEFAULT_OBJ_PRINTER},
    {OType::INST, DEFAULT_INST_PRINTER},
    {OType::BCODE, DEFAULT_BCODE_PRINTER},
    {OType::OBJS, DEFAULT_OBJ_PRINTER},
    {OType::ERROR, DEFAULT_OBJ_PRINTER},
  };

  inline Runnable ROUTER_POP_FRAME = []() {
    LOG(TRACE, "!ROUTER_POP_FRAME!! undefined at this point in bootstrap\n");
  };
  inline BiConsumer<Pattern, Rec_p> ROUTER_PUSH_FRAME = [](const Pattern &pattern, const Rec_p &) {
    LOG(TRACE, "!ROUTER_PUSH_FRAME!! undefined at this point in bootstrap: %s\n", pattern.toString().c_str());
  };
  inline TriFunction<const ID_p &, const ID_p &, List<ID_p> *, const bool> IS_TYPE_OF =
      [](const ID_p &is_type_id, const ID_p &, List<ID_p> *) {
    LOG(TRACE, "!IS_TYPE_OF!! undefined at this point in bootstrap: %s\n", is_type_id->toString().c_str());
    return false;
  };
  inline TriFunction<const Obj *, const ID_p &, const bool, const bool> TYPE_CHECKER =
      [](const Obj *, const ID_p &type_id, const bool = true) -> bool {
    LOG(TRACE, "!yTYPE_CHECKER!! undefined at this point in bootstrap: %s\n", type_id->toString().c_str());
    return true;
  };
  inline Function<const string &, const Obj_p> OBJ_PARSER = [](const string &) {
    LOG(TRACE, "!yOBJ_PARSER!! undefined at this point in bootstrap.\n");
    return nullptr;
  };
  inline Function<BCode_p, Objs_p> BCODE_PROCESSOR = [](const BCode_p &) {
    LOG(TRACE, "!yBCODE_PROCESSOR!! undefined at this point in bootstrap.\n");
    return nullptr;
  };
  inline Function<const fURI, const fURI_p> ROUTER_RESOLVE = [](const fURI &furi) {
    LOG(TRACE, "!yROUTER_RESOLVE!! undefined at this point in bootstrap.\n");
    return furi_p(furi);
  };
  inline TriConsumer<const fURI_p &, const Obj_p &, const bool> ROUTER_WRITE =
      [](const fURI_p &, const Obj_p &, const bool) -> void {
    LOG(TRACE, "!yROUTER_WRITE!! undefined at this point in bootstrap.\n");
  };
  inline Function<const fURI_p &, const Obj_p> ROUTER_READ = [](const fURI_p &) -> Obj_p {
    LOG(TRACE, "!yROUTER_READ!! undefined at this point in bootstrap.\n");
    return nullptr;
  };
  inline BiConsumer<const ID_p, const Obj_p> TYPE_SAVER = [](const ID_p &type_id, const Obj_p &obj) {
    LOG(TRACE, "!yTYPE_SAVER!! undefined at this point in bootstrap: %s\n", type_id->toString().c_str());
    ROUTER_WRITE(type_id, obj, true);
  };
  inline BiFunction<const Obj_p &, const Inst_p &, Inst_p> TYPE_INST_RESOLVER = [
      ](const Obj_p &, const Inst_p &) {
    LOG(TRACE, "!RESOLVE_INST!! undefined at this point in bootstrap.\n");
    return nullptr;
  };
  inline Runnable FEED_WATCDOG = []() {
  };

  class ObjsSet;

  //////////////////////////////////////////////////
  ////////////////////// OBJ //////////////////////
  /////////////////////////////////////////////////
  /// An mm-ADT abstract object from which all other types derive
  class Obj : public Typed,
              public Valued,
              public Function<Obj_p, Obj_p>,
              public BiFunction<Obj_p, InstArgs, Obj_p>,
              public enable_shared_from_this<const Obj> {
  public:
    const OType otype_;
    Any value_;

    struct objp_hash {
      size_t operator()(const Obj_p &obj) const {  return std::hash<std::string>{}(obj->toString()); }
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
        return k1->toString() > k2->toString();
      }
    };

    ///////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    using LstList = List<Obj_p>;
    using LstList_p = ptr<LstList>;
    template<typename HASH = objp_hash, typename EQ = objp_equal_to>
    using RecMap = OrderedMap<Obj_p, Obj_p, HASH, EQ>;
    template<typename HASH = objp_hash, typename EQ = objp_equal_to>
    using RecMap_p = ptr<RecMap<HASH, EQ>>;
    ///////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    explicit Obj(const Any &value, const OType otype, const ID_p &type_id,
                 const ID_p &value_id = nullptr) : Typed(OTYPE_FURI.at(otype)),
                                                   Valued(value_id),
                                                   otype_(otype),
                                                   value_(value) {
      if(otype == OType::INST && nullptr == std::get<2>(*std::any_cast<InstValue_p>(value))) {
        this->value_ = make_shared<InstValue>(make_tuple(std::get<0>(*std::any_cast<InstValue_p>(value)),
                                                         std::get<1>(*std::any_cast<InstValue_p>(value)),
                                                         this->is_gather() ? Obj::to_objs() : Obj::to_noobj()));
      }
      if(otype == OType::INST && type_id->has_query(FOS_RANGE) && type_id->no_query().equals(
           ID(type_id->query_value(FOS_RANGE).value()))) {
        throw fError("the range of an inst can not be its type: %s", type_id->toString().c_str());
      }
      if(value.has_value()) { // value token
        TYPE_CHECKER(this, type_id, true);
        this->tid_ = type_id;
        if(value_id) {
          const Obj_p strip = this->clone();
          //   if(!vid_->has_query())
          //const_cast<Obj *>(strip.get())->vid_ = nullptr;
          ROUTER_WRITE(value_id, strip, true);
        }
      } else {
        this->tid_ = type_id; // type token
      }
    }

    static ptr<Obj> create(const Any &value, const OType otype, const ID_p &type_id, const ID_p &value_id = nullptr) {
      return make_shared<Obj>(value, otype, type_id, value_id);
    }

    static fError TYPE_ERROR(const Obj *obj, const char *function, [[maybe_unused]] const int line_number = __LINE__) {
      const size_t index = string(function).find("_value");
      return fError(FURI_WRAP " %s !yaccessed!! as !b%s!! L%i", obj->vid_or_tid()->toString().c_str(),
                    obj->toString().c_str(),
                    index == string::npos ? function : string(function).replace(index, 6, "").c_str(),
                    line_number);
    }

    //////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////
    [[nodiscard]] virtual ID_p vid_or_tid() const {
      return this->vid_ ? this->vid_ : this->tid_;
    }

    virtual void save() const {
      this->at(this->vid_);
    }

    virtual void load() {
      if(this->vid_) {
        const Obj_p other = ROUTER_READ(this->vid_);
        if(this->otype_ != other->otype_ || !this->tid_->equals(*other->tid_))
          throw fError("type of obj structural encoding changed (try locking): %s %s", this->tid_->toString().c_str(),
                       other->tid_->toString().c_str());
        this->value_ = other->value_;
      }
    }

    template<typename VALUE>
    [[nodiscard]] VALUE value() const {
      try {
        return std::any_cast<VALUE>(this->value_);
      } catch(const std::bad_any_cast &) {
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      }
    }

    [[nodiscard]] List_p<Obj_p> objs_value() const {
      if(!this->is_objs())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->value<List_p<Obj_p>>();
    }

    [[nodiscard]] Obj_p objs_value(const size_t index) const {
      return (index >= this->objs_value()->size()) ? to_noobj() : this->objs_value()->at(index);
    }

    [[nodiscard]] bool bool_value() const {
      if(!this->is_bool())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      if(!this->value_.has_value())
        throw fError("obj is a bool type, not a bool value");
      return this->value<bool>();
    }

    [[nodiscard]] FOS_INT_TYPE int_value() const {
      if(!this->is_int())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->value<FOS_INT_TYPE>();
    }

    [[nodiscard]] FOS_REAL_TYPE real_value() const {
      if(!this->is_real())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->value<FOS_REAL_TYPE>();
    }

    [[nodiscard]] fURI uri_value() const {
      if(!this->is_uri())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->value<fURI>();
    }

    [[nodiscard]] ID_p id_p_value() const {
      if(!this->is_uri())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return id_p(this->value<fURI>());
    }

    template<typename FURI>
    [[nodiscard]] ptr<FURI> uri_p_value() const {
      if(!this->is_uri())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return make_shared<FURI>(this->value<FURI>());
    }

    [[nodiscard]] string str_value() const {
      if(!this->is_str())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->value<string>();
    }

    [[nodiscard]] Obj_p str_get(const Int_p &index) const {
      if(!this->is_str())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return (static_cast<size_t>(index->int_value()) >= this->str_value().length())
               ? Obj::to_noobj()
               : Obj::to_str(string() + (this->str_value()[index->int_value()]));
    }

    [[nodiscard]] LstList_p lst_value() const {
      if(!this->is_lst())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->value<LstList_p>();
    }

    [[nodiscard]] Obj_p type_value() const {
      if(!this->is_type())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->value<Obj_p>();
    }

    void lst_add(const Obj_p &obj) const {
      if(!this->is_lst())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      this->lst_value()->push_back(obj);
    }

    [[nodiscard]] Obj_p deref(const Obj_p &uri, const bool uri_on_fail = true) const {
      if(this->is_rec())
        return this->rec_get(uri);
      if(this->is_lst())
        return this->lst_get(uri);
      if(this->is_objs()) {
        const Objs_p transform = Obj::to_objs(make_shared<List<Obj_p>>());
        for(const auto &o: *this->objs_value()) {
          transform->add_obj(o->deref(uri, uri_on_fail));
        }
        return transform;
      }
      return uri_on_fail ? uri : Obj::to_noobj();
    }

    [[nodiscard]] Int_p rec_size() const { return Obj::to_int(this->rec_value()->size()); }

    [[nodiscard]] Int_p lst_size() const { return Obj::to_int(this->lst_value()->size()); }

    [[nodiscard]] Obj_p lst_get(const string &index) const {
      return this->lst_get(Obj::to_uri(index));
    }

    [[nodiscard]] Obj_p lst_get(const FOS_INT_TYPE &index) const { return this->lst_get(Obj::to_int(index)); }

    [[nodiscard]] Obj_p lst_get(const Obj_p &index) const {
      if(!this->is_lst())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      if(index->is_uri()) {
        const auto segment = string(index->uri_value().segment(0));
        const int i = StringHelper::is_integer(segment)
                        ? std::stoi(segment)
                        : (StringHelper::has_wildcards(segment) ? -1 : -100);
        if(-100 == i)
          return Obj::to_noobj();
        //   throw fError("segment !b%s!! of !b%s!! !ris not!! an !yint!! or wildcard", segment.c_str(),
        //                index->uri_value().toString().c_str());
        if(i >= static_cast<int>(this->lst_value()->size()))
          return to_noobj();
        const Obj_p segment_value = Obj::to_objs();
        if(i == -1) {
          for(const auto &e: *this->lst_value()) {
            segment_value->add_obj(e);
          }
        } else {
          segment_value->add_obj(this->lst_value()->at(i));
        }
        const Obj_p final_segment_value = segment_value->none_one_all();
        if(final_segment_value->is_noobj())
          return to_noobj();
        return index->uri_value().path_length() <= 1
                 ? final_segment_value
                 : final_segment_value->deref(Obj::to_uri(index->uri_value().subpath(1)), false);
      }
      return (static_cast<size_t>(index->int_value()) >= this->lst_value()->size())
               ? Obj::to_noobj()
               : this->lst_value()->at(index->int_value());
    }

    void lst_set(const string &index, const Obj_p &val, const bool nest = true) const {
      return lst_set(Obj::to_uri(index), val, nest);
    }

    void lst_set(const int index, const Obj_p &val, const bool nest = true) const {
      return lst_set(Obj::to_int(index), val, nest);
    }

    void lst_set(const Obj_p &index, const Obj_p &val, const bool nest = true) const {
      const Obj_p undo = this->lst_get(index);
      ////////////////////////////////////////
      if(nest &&
         index->is_uri() &&
         index->uri_value().path_length() > 1 &&
         StringHelper::is_integer(index->uri_value().segment(0))) {
        const size_t current_index = std::strtol(index->uri_value().segment(0), nullptr, 10);
        Obj_p current_obj = this->lst_get(current_index);
        if(current_obj->is_noobj()) {
          if(const size_t offset = ((current_index + 1) - this->lst_value()->size());
            offset > 0) {
            for(size_t j = 0; j < offset; j++) {
              this->lst_value()->push_back(Obj::to_noobj());
            }
          }
          const bool is_a_lst = StringHelper::is_integer(index->uri_value().segment(1));
          current_obj = is_a_lst ? Obj::to_lst() : Obj::to_rec();
          this->lst_value()->erase(this->lst_value()->begin() + current_index);
          this->lst_value()->insert(this->lst_value()->begin() + current_index, current_obj);
        }
        current_obj->poly_set(Obj::to_uri(index->uri_value().pretract()), val);
      } else {
        const int current_index = index->is_int()
                                    ? index->int_value()
                                    : index->is_uri()
                                        ? std::stoi(index->uri_value().toString())
                                        : -1;
        if(-1 == current_index)
          throw fError("invalid lst index: %s\n", index->toString().c_str());
        if(const size_t offset = (current_index + 1) - this->lst_value()->size();
          offset > 0) {
          for(size_t j = 0; j < offset; j++) {
            this->lst_value()->push_back(Obj::to_noobj());
          }
        }
        this->lst_value()->erase(this->lst_value()->begin() + current_index);
        if(!val->is_noobj())
          this->lst_value()->insert(this->lst_value()->begin() + current_index, val);
      }
      ////////////////////////////////////////
      if(!this->is_base_type()) {
        try {
          Compiler(true, false).type_check(this, this->tid_);
        } catch(const fError &) {
          this->lst_set(index, undo);
          LOG_OBJ(WARN, this, "!blst!! entry write reverted: !g[!!%s !m=>!! %s!g]!!\n",
                  index->toString().c_str(),
                  undo->toString().c_str());
          throw;
        }
      }
      ////////////////////////////////////////
    }

    [[nodiscard]] RecMap_p<> rec_value() const {
      if(!this->is_rec())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->value<RecMap_p<>>();
    }

    [[nodiscard]] List_p<Obj_p> rec_values() const {
      const auto list = make_shared<List<Obj_p>>();
      for(const auto &[k,v]: *this->rec_value()) {
        list->push_back(v);
      }
      return list;
    }

    [[nodiscard]] Obj_p arg(const size_t index) const {
      if(this->is_inst())
        return this->inst_args()->arg(index);
      size_t counter = 0;
      for(const auto &[k,v]: *this->rec_value()) {
        if(index == counter)
          return v;
        counter++;
      }
      return Obj::to_noobj();
    }

    [[nodiscard]] Obj_p arg(const Obj_p &key) const {
      if(this->is_inst()) return this->inst_args()->arg(key);
      return this->rec_value()->count(key) ? this->rec_value()->at(key) : Obj::to_noobj();
    }

    [[nodiscard]] Obj_p arg(const string &key) const {
      if(this->is_inst()) return this->inst_args()->arg(key);
      const Uri_p uri_key = Obj::to_uri(key);
      return this->rec_get(uri_key);
    }

    [[nodiscard]] Obj_p rec_get(const Obj_p &key, const Obj_p &or_else = nullptr) const {
      if(!this->is_rec())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      Obj_p result;
      if(key->is_uri()) {
        const auto segment = 0 == key->uri_value().path_length()
                               ? key->uri_value().toString()
                               : string(key->uri_value().segment(0));
        Objs_p segment_value = Obj::to_objs();
        const Uri_p segment_uri = Obj::to_uri(segment);
        const bool match_all = StringHelper::has_wildcards(segment);
        for(const auto &[k,v]: *this->rec_value()) {
          if(match_all || k->match(segment_uri))
            segment_value->add_obj(v);
        }
        segment_value = segment_value->none_one_all();
        result = key->uri_value().path_length() <= 1
                   ? segment_value
                   : segment_value->deref(to_uri(key->uri_value().subpath(1)), false);
      } else {
        const Objs_p segment_value = Obj::to_objs();
        for(const auto &[k, v]: *this->rec_value()) {
          if(k->match(key))
            segment_value->add_obj(v);
        }
        result = segment_value->none_one_all();
      }
      return result->is_noobj() && or_else ? or_else : result;
    }

    [[nodiscard]] Obj_p rec_get(const fURI_p &key, const Obj_p &or_else = nullptr) const {
      return rec_get(to_uri(*key), or_else);
    }

    [[nodiscard]] Obj_p rec_get(const char *uri_key, const Obj_p &or_else = nullptr) const {
      return rec_get(to_uri(uri_key), or_else);
    }

    [[nodiscard]] Rec_p rec_merge(const RecMap_p<> &rmap) const {
      for(const auto&[key, value] : *rmap) {
        this->rec_value()->insert_or_assign(key,value);
      }
      return shared_from_this();
    }

    virtual void rec_set(const char *uri_key, const Obj_p &val, const bool nest = true) const {
      this->rec_set(Obj::to_uri(uri_key), val, nest);
    }

    virtual void rec_set(const Obj_p &key, const Obj_p &val, const bool nest = true) const {
      const Obj_p undo = this->rec_get(key);
      ////////////////////////////////////////
      if(nest && key->is_uri() && key->uri_value().path_length() > 1) {
        const Uri_p current_key = Obj::to_uri(key->uri_value().segment(0));
        Obj_p current_obj = this->rec_get(current_key);
        const bool is_lst = StringHelper::is_integer(key->uri_value().segment(1));
        if(current_obj->is_noobj()) {
          current_obj = is_lst ? Obj::to_lst() : Obj::to_rec();
          this->rec_value()->insert_or_assign(current_key, current_obj);
        }
        current_obj->poly_set(Obj::to_uri(key->uri_value().pretract()), val);
      } else {
        if(val->is_noobj()) this->rec_value()->erase(key);
        else this->rec_value()->insert_or_assign(key, val);
      }
      ////////////////////////////////////////
      if(!this->is_base_type()) {
        try {
          Compiler(true, false).type_check(this, this->tid_);
        } catch(const fError &) {
          this->rec_set(key, undo);
          LOG_OBJ(WARN, this, "!brec!! entry write reverted: !g[!!%s !m=>!! %s!g]!!\n",
                  key->toString().c_str(),
                  undo->toString().c_str());
          throw;
        }
      }
      ////////////////////////////////////////
    }

    virtual void rec_set(const Obj &key, const Obj &value) const {
      Obj::rec_set(make_shared<Obj>(key), make_shared<Obj>(value));
    }

    virtual void rec_set(const fURI_p &key, const Obj &value) const {
      Obj::rec_set(Obj::to_uri(*key), make_shared<Obj>(value));
    }

    virtual void rec_set(const fURI_p &key, const Obj_p &value) const { Obj::rec_set(Obj::to_uri(*key), value); }

    virtual void rec_add(const Rec_p &other) const {
      for(const auto &[k, v]: *other->rec_value()) {
        if(this->rec_value()->count(k))
          this->rec_value()->erase(k);
        this->rec_set(k, v);
      }
      if(this->vid_)
        ROUTER_WRITE(this->vid_, Obj::to_rec(make_shared<RecMap<>>(*this->rec_value()), id_p(*this->tid_)), true);
    }

    void rec_delete(const Obj &key) const { Obj::rec_set(make_shared<Obj>(key), Obj::to_noobj()); }

    void poly_set(const Obj_p &key, const Obj_p &value) const {
      if(!this->is_poly())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      if(this->is_rec())
        this->rec_set(key, value);
      else if(this->is_lst())
        this->lst_set(key, value);
      else
        throw fError("unknown poly base type (logic error): %s", this->tid_->toString().c_str());
    }

    Obj_p poly_get(const Obj_p &key) const {
      if(!this->is_poly())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      if(this->is_rec())
        return this->rec_get(key);
      if(this->is_lst())
        return this->lst_get(key);
      throw fError("unknown poly base type (logic error): %s", this->tid_->toString().c_str());
    }

    [[nodiscard]] InstValue_p inst_value() const {
      if(!this->is_inst())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->value<InstValue_p>();
    }

    [[nodiscard]] string inst_op() const {
      if(!this->is_inst())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->tid_->name();
    }

    [[nodiscard]] InstArgs inst_args() const {
      if(this->is_inst())
        return std::get<0>(*this->inst_value());
      if(!this->is_rec())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->shared_from_this();
    }

    [[nodiscard]] bool has_arg(const Obj_p &key) const {
      return (this->is_inst() ? this->inst_args().get() : this)->rec_value()->count(key) != 0;
    }

    [[nodiscard]] InstF_p inst_f() const {
      if(!this->is_inst())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return std::get<1>(*this->inst_value());
    }

    [[nodiscard]] Obj_p inst_seed_supplier() const {
      if(!this->is_inst())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return std::get<2>(*this->inst_value());
    }

    [[nodiscard]] Obj_p inst_seed(const Obj_p &arg) const { return this->inst_seed_supplier()->apply(arg); }

    [[nodiscard]] Pair<Obj_p, Inst_p> error_value() const {
      if(!this->is_error())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->value<Pair<Obj_p, Inst_p>>();
    }

    [[nodiscard]] DomainRange domain_range() const {
      return DomainRange::from(*this->tid_);
    }

    [[nodiscard]] ID_p domain() const {
      if(this->tid_->has_query(FOS_DOMAIN))
        return id_p(*ROUTER_RESOLVE(fURI(this->tid_->query_value(FOS_DOMAIN).value())));
      if(this->is_bcode() && !this->bcode_value()->empty())
        return this->bcode_value()->front()->domain();
      if(this->is_inst() && this->inst_f() && std::holds_alternative<Obj_p>(*this->inst_f()))
        return std::get<Obj_p>(*this->inst_f())->domain();
      return OBJ_FURI;
    }


    [[nodiscard]] IntCoefficient domain_coefficient() const {
      const std::vector<string> coefs = this->tid_->query_values(FOS_DOM_COEF);
      return coefs.empty() ? IntCoefficient{1, 1} : IntCoefficient{stoi(coefs.at(0)), stoi(coefs.at(1))};
    }

    [[nodiscard]] IntCoefficient range_coefficient() const {
      const std::vector<string> coefs = this->tid_->query_values(FOS_RNG_COEF);
      return coefs.empty() ? IntCoefficient{1, 1} : IntCoefficient{stoi(coefs.at(0)), stoi(coefs.at(1))};
    }

    [[nodiscard]] ID_p range() const {
      if(this->tid_->has_query(FOS_RANGE))
        return id_p(*ROUTER_RESOLVE(fURI(this->tid_->query_value(FOS_RANGE).value())));
      if(this->is_bcode() && !this->bcode_value()->empty())
        return this->bcode_value()->back()->range();
      if(this->is_inst())
        return this->inst_f() && std::holds_alternative<Obj_p>(*this->inst_f())
                 ? std::get<Obj_p>(*this->inst_f())->range()
                 : OBJ_FURI;
      return id_p(this->tid_->no_query());
    }

    bool CHECK_OBJ_TO_INST_SIGNATURE(const Inst_p &resolved, const bool domain_or_range,
                                     const bool throw_exception = true) const {
      if(domain_or_range) {
        if(resolved->is_generative()) {
          // do nothing
        } else if(this->is_noobj()) {
          if(!resolved->is_initial()) {
            if(!throw_exception) return false;
            throw fError("%s [%s] not in domain of %s [!y%s!!]",
                         this->toString().c_str(),
                         Obj::to_type(NOOBJ_FURI)->toString().c_str(),
                         resolved->toString().c_str(),
                         "SIGNATURE HERE");
          }
        } else if(this->is_objs()) {
          if(!resolved->is_gather()) {
            if(!throw_exception) return false;
            throw fError("%s [%s] not in domain of %s [!y%s!!]",
                         this->toString().c_str(),
                         Obj::to_type(OBJS_FURI)->toString().c_str(),
                         resolved->toString().c_str(), "SIGNATURE HERE");
          }
        } else {
          if(const auto &[a,b] = resolved->domain_coefficient(); !(a == 1 && b == 1)) {
            if(!throw_exception) return false;
            throw fError("%s [%s] not in domain of %s [!y%s!!]",
                         this->toString().c_str(),
                         Obj::to_type(OBJ_FURI)->toString().c_str(),
                         resolved->toString().c_str(),
                         "SIGNATURE HERE");
          }
        }
      } else {
        if(const auto &[a,b] = resolved->range_coefficient(); (a == 0 && b > 0)) {
          // do nothing
        } else if(this->is_noobj()) {
          if(!resolved->is_terminal()) {
            if(!throw_exception) return false;
            throw fError("%s [%s] not in range of %s [!y%s!!]",
                         this->toString().c_str(),
                         Obj::to_type(NOOBJ_FURI)->toString().c_str(),
                         resolved->toString().c_str(),
                         "SIGNATURE HERE");
          }
        } else if(this->is_objs()) {
          if(!resolved->is_scatter()) {
            if(!throw_exception) return false;
            throw fError("%s [%s] not in range of %s [!y%s!!]",
                         this->toString().c_str(),
                         Obj::to_type(OBJS_FURI)->toString().c_str(),
                         resolved->toString().c_str(),
                         "SIGNATURE HERE");
          }
        } else {
          if(const auto &[a,b] = resolved->range_coefficient(); a != 1 && b != 1) {
            if(!throw_exception) return false;
            throw fError("%s [%s] not in range of %s [!y%s!!]",
                         this->toString().c_str(),
                         Obj::to_type(OBJ_FURI)->toString().c_str(),
                         resolved->toString().c_str(),
                         "SIGNATURE HERE");
          }
        }
      }
      return true;
    }


    [[nodiscard]] Obj_p type() const {
      return ROUTER_READ(this->tid_);
    }

    [[nodiscard]] InstList_p bcode_value() const {
      if(this->is_noobj())
        return {};
      if(!this->is_bcode())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->value<InstList_p>();
    }

    [[nodiscard]] BCode_p bcode_starts(const Objs_p &starts) const {
      if(!this->is_bcode())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      const auto new_code = make_shared<List<Inst_p>>();
      new_code->push_back(Obj::to_inst(Obj::to_inst_args({starts}), id_p("start")));
      for(const auto &inst: *this->bcode_value()) {
        new_code->push_back(inst);
      }
      return Obj::to_bcode(new_code, this->tid_);
      //return Obj::create(new_code, OType::BCODE, this->tid_, this->vid_);
    }

    ////////////////////////////////////////////////////////////
    Obj_p this_add(const ID &relative_id, const Obj_p &inst, const bool at_type = true) const {
      if(!at_type && !this->vid_)
        throw fError("only objs with a value id can have properties and insts");
      //if(inst->is_code())
      ROUTER_WRITE(id_p(this->vid_->append(relative_id)), inst, true);
      return this->shared_from_this();
    }

    Obj_p this_add_inst(const ID &relative_id, const Obj_p &inst, const bool at_type = true) const {
      if(!at_type && !this->vid_)
        throw fError("only objs with a value id can have properties and insts");
      //if(inst->is_code())
      ROUTER_WRITE(id_p(this->vid_->add_component(relative_id)), inst, true);
      return this->shared_from_this();
    }

    Obj_p this_get(const char *key) const {
      // TODO: if not, vid, then tid, then tid -> tid, then tid -> tid -> tid;
      Obj_p result = ROUTER_READ(furi_p(this->vid_->extend(key)));
      return result;
    }

    Obj_p this_get(const fURI &furi) const {
      // TODO: if not, vid, then tid, then tid -> tid, then tid -> tid -> tid;
      Obj_p result = ROUTER_READ(furi_p(this->vid_->extend(furi)));
      return result;
    }

    Obj_p this_set(const char *key, const Obj_p &obj) {
      ROUTER_WRITE(furi_p(this->vid_->extend(key)), obj, true);
      return this->shared_from_this();
    }

    Obj_p static_get(const char *key) const {
      return ROUTER_READ(furi_p(this->tid_->extend(key)));
    }

    ////////////////////////////////////////////////////////////

    BCode_p add_inst(const Inst_p &inst, const bool mutate = true) const {
      if(!this->is_bcode())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      if(mutate) {
        const InstList_p insts = bcode_value();
        insts->push_back(inst);
        return to_bcode(insts);
      } else {
        const auto insts = make_shared<InstList>();
        for(const auto &i: *this->bcode_value()) {
          insts->push_back(i);
        }
        insts->push_back(inst);
        return to_bcode(insts);
      }
    }

    Obj_p doc_write(const string &documentation) {
      ROUTER_WRITE(furi_p(this->vid_or_tid()->query("doc")), Obj::to_str(documentation), true);
      return this->shared_from_this();
    }

    [[nodiscard]] BCode_p add_bcode(const BCode_p &bcode, [[maybe_unused]] const bool mutate = true) const {
      if(!this->is_bcode() || !bcode->is_bcode())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      InstList_p insts = make_shared<InstList>();
      for(const auto &inst: *bcode->bcode_value()) {
        insts->push_back(inst);
      }
      return Obj::to_bcode(insts);
    }

    [[nodiscard]] Obj_p none_one_all() const {
      if(this->is_objs()) {
        if(this->objs_value()->empty())
          return Obj::to_noobj();
        if(this->objs_value()->size() == 1)
          return this->objs_value()->front();
      }
      return this->shared_from_this();
    }

    [[nodiscard]] Obj_p none_one() const {
      return this->is_objs()
               ? (this->objs_value()->empty() ? Obj::to_noobj() : this->objs_value()->front())
               : this->shared_from_this();
    }

    void add_obj(const Obj_p &obj, [[maybe_unused]] const bool mutate = true) const {
      if(!this->is_objs())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      if(obj->is_noobj())
        return;
      if(obj->is_objs()) {
        for(const Obj_p &o: *obj->objs_value()) {
          this->add_obj(o);
        }
      } else {
        this->objs_value()->push_back(obj);
      }
    }

    [[nodiscard]] size_t hash() const { return std::hash<std::string>{}(this->toString()); }

// TODO: make obj.cpp/hpp and then reference PrinterHelper for printing
    [[nodiscard]] string toString(const ObjPrinter *obj_printer = nullptr) const {
      if(!obj_printer)
        obj_printer = GLOBAL_PRINTERS.at(this->otype_);
      string obj_string;
      if(this->is_noobj())
        obj_string = "!r" STR(FOS_NOOBJ_TOKEN) "!!";
      else {
        switch(this->otype_) {
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
            obj_string = "!_" + (obj_printer->strict
                                   ? "<" + this->uri_value().toString() + ">"
                                   : this->uri_value().toString()) + "!!";
            break;
          case OType::STR:
            obj_string = "!m'!!!~" + this->str_value() + "!m'!!";
            break;
          case OType::LST: {
            if(this->lst_value()->empty())
              obj_string = "!m[]!!";
            else {
              obj_string = "!m[!!";
              bool first = true;
              for(const auto &obj: *this->lst_value()) {
                if(first) {
                  first = false;
                } else {
                  obj_string += "!m,!!";
                }
                obj_string += obj->toString(obj_printer->next());
              }
              obj_string += "!m]!!";
            }
            break;
          }
          case OType::REC: {
            if(this->rec_value()->empty())
              obj_string = "!m[!g=>!m]!!";
            else {
              obj_string = "!m[!!";
              bool first = true;
              for(const auto &[k, v]: *this->rec_value()) {
                if(first) {
                  first = false;
                } else {
                  obj_string += "!m,";
                }
                obj_string += "!c";
                obj_string += k->toString(obj_printer->next()); // {ansi=false});
                obj_string += "!g=>!!";
                obj_string += v->toString(obj_printer->next());
              }
              obj_string += "!m]!!";
            }
            break;
          }
          case OType::INST: {
            bool first = true;
            for(const auto &[k,v]: *this->inst_args()->rec_value()) {
              if(first) {
                first = false;
              } else {
                obj_string += "!m,!!";
              }
              if(!k->is_indexed_arg()) {
                obj_string += "!c";
                obj_string += k->toString(obj_printer->next());
                obj_string += "!g=>!!";
              }
              obj_string += v->toString();
            }
            break;
          }
          case OType::BCODE: {
            if(this->bcode_value()->empty())
              obj_string = "_";
            else {
              // objString += "!b" + this->bcode_range()->name() + "!g<=!b" + this->bcode_domain()->name() + "!g[!!";
              bool first = true;
              for(const auto &inst: *this->bcode_value()) {
                if(first) {
                  first = false;
                } else {
                  obj_string += "!g.!!";
                }
                obj_string += inst->toString(GLOBAL_PRINTERS.at(OType::OBJ));
              }
              // objString += "!g]!!";
            }
            break;
          }
          case OType::OBJS: {
            obj_string += "!m{!!";
            bool first = true;
            for(const auto &obj: *this->objs_value()) {
              if(first) {
                first = false;
              } else {
                obj_string += "!m,!!";
              }
              obj_string += obj->toString(obj_printer->next());
            };
            obj_string += "!m}!!";
            break;
          }
          case OType::ERROR: {
            obj_string = "!r<<!!";
            obj_string += this->error_value().first->toString(obj_printer->next());
            obj_string += "!r@!!";
            obj_string += this->error_value().second->toString(obj_printer->next());
            obj_string += "!r>>!!";
            break;
          }
          case OType::NOOBJ: {
            obj_string = "!r" STR(FOS_NOOBJ_TOKEN) "!!"; // TODO: repeated above (is_noobj()) to skip the typing checks
            break;
          }
          case OType::TYPE: {
            obj_string = this->type_value()->toString();
            break;
          }
          default:
            throw fError("unknown obj type in toString(): %s", OTypes.to_chars(this->otype_).c_str());
        }
      }
      if(!this->is_noobj() &&
         (this->is_type() ||
          this->is_inst() ||
          (obj_printer->show_type && !this->is_base_type()))) {
        string typing;
        if(this->is_type())
          typing += "!m[!!";
        typing += this->is_base_type() && !this->is_inst() && !this->is_type() && !this->is_uri()
                    ? ""
                    : string("!b")
                    .append(obj_printer->strict ? this->tid_->toString() : this->tid_->name()).append("!!");
        // TODO: remove base_type check
        if(obj_printer->show_domain_range) {
          const string dom_str = this->has_domain(1, 1) && !obj_printer->strict
                                   ? ""
                                   : this->has_domain(0, 1)
                                       ? "?"
                                       : this->has_domain(1,INT_MAX)
                                           ? "+"
                                           : this->is_initial()
                                               ? "."
                                               : this->is_gather()
                                                   ? "*"
                                                   : to_string(this->domain_coefficient().first)
                                                   .append(",")
                                                   .append(to_string(this->domain_coefficient().second));
          const string rng_str = this->has_range(1, 1) && !obj_printer->strict
                                   ? ""
                                   : this->has_range(0, 1)
                                       ? "?"
                                       : this->has_range(1,INT_MAX)
                                           ? "+"
                                           : this->is_terminal()
                                               ? "."
                                               : this->is_scatter()
                                                   ? "*"
                                                   : to_string(this->range_coefficient().first)
                                                   .append(",")
                                                   .append(to_string(this->range_coefficient().second));

          if(!dom_str.empty() || !rng_str.empty()) {
            typing = typing.append("!m?!!")
                .append("!c").append(obj_printer->strict ? this->range()->toString() : this->range()->name())
                .append(rng_str.empty() ? "" : string("!m{!c").append(rng_str).append("!m}!!"))
                .append("!m<=!!")
                .append("!c").append(obj_printer->strict ? this->domain()->toString() : this->domain()->name())
                .append(dom_str.empty() ? "" : string("!m{!c").append(dom_str).append("!m}!!"));
          }
        }
        if(this->is_type())
          typing += "!m]!!";
        obj_string = this->is_base_type() && !this->is_inst() && !this->is_type()
                       ? typing.append(obj_string)
                       : typing.append(this->is_inst() ? "!g(!!" : "!g[!!")
                       .append(obj_string)
                       .append(this->is_inst() ? "!g)!!" : "!g]!!");

        if(this->is_inst() && this->inst_f()) {
          obj_string = obj_string
              .append("!g[!!")
              .append(std::holds_alternative<Obj_p>(*this->inst_f())
                        ? std::get<Obj_p>(*this->inst_f())->toString()
                        : "!ycpp!!")
              .append("!g]!!");
        }
      }
      if(obj_printer->show_id && this->vid_)
        obj_string.append("!m@!b").append(this->vid_->toString()).append("!!");

      obj_string = obj_printer->ansi ? obj_string : Ansi<>::strip(obj_string);
      return obj_string;
    }

    [[nodiscard]] int compare(const Obj &rhs) const { return this->toString().compare(rhs.toString()); }

    bool operator>(const Obj &rhs) const {
      switch(this->otype_) {
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
          throw fError("%s is not relational (>)", OTypes.to_chars(this->otype_).c_str());
      }
    }

    bool operator<(const Obj &rhs) const {
      switch(this->otype_) {
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
          throw fError("%s is not relational (<)", OTypes.to_chars(this->otype_).c_str());
      }
    }

    bool operator<=(const Obj &rhs) const { return *this == rhs || *this < rhs; }

    bool operator>=(const Obj &rhs) const { return *this == rhs || *this > rhs; }

    Obj operator/(const Obj &rhs) const {
      switch(this->otype_) {
        case OType::NOOBJ:
          return *to_noobj();
        case OType::INT:
          return Obj(this->int_value() / rhs.int_value(), OType::INT, this->tid_, this->vid_);
        case OType::REAL:
          return Obj(this->real_value() / rhs.real_value(), OType::REAL, this->tid_, this->vid_);
        default:
          throw fError("%s can not be divided (/)", OTypes.to_chars(this->otype_).c_str());
      }
    }

    Obj operator-(const Obj &rhs) const {
      switch(this->otype_) {
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
        //  return Obj(string(this->str_value()).replace(string(rhs.str_value()), this->vid_);
        case OType::LST: {
          auto list = std::make_shared<LstList>();
          for(const auto &obj: *this->lst_value()) {
            if(std::find(rhs.lst_value()->begin(), rhs.lst_value()->end(), obj) != std::end(*rhs.lst_value()))
              list->push_back(obj);
          }
          return Lst(list, OType::LST, this->tid_, this->vid_);
        }
        case OType::REC: {
          auto map = std::make_shared<RecMap<>>();
          for(const auto &pair: *this->rec_value()) {
            map->insert(pair);
          }
          for(const auto &pair: *rhs.rec_value()) {
            map->insert(pair);
          }
          return Rec(map, OType::REC, this->tid_, this->vid_);
        }
        default:
          throw fError("%s can not be subtracted (-)", OTypes.to_chars(this->otype_).c_str());
      }
    }

    [[nodiscard]] bool equals(const Obj &other) const {
      if(this->otype_ != other.otype_ || !this->tid_->no_query().equals(other.tid_->no_query()))
        return false;
      switch(this->otype_) {
        case OType::NOOBJ: return true;
        case OType::OBJ: return !other.value_.has_value();
        case OType::TYPE: return this->type_value()->equals(*other.type_value());
        case OType::BOOL: return this->bool_value() == other.bool_value();
        case OType::INT: return this->int_value() == other.int_value();
        case OType::REAL: return this->real_value() == other.real_value();
        case OType::STR: return this->str_value() == other.str_value();
        case OType::URI: return this->uri_value() == other.uri_value();
        case OType::LST: {
          const auto objs_a = this->lst_value();
          const auto objs_b = other.lst_value();
          if(objs_a->size() != objs_b->size())
            return false;
          auto it_b = objs_b->begin();
          for(const auto &it_a: *objs_a) {
            if(*it_a != **it_b)
              return false;
            ++it_b;
          }
          return true;
        }
        case OType::REC: {
          const RecMap_p<> a = this->rec_value();
          const RecMap_p<> b = other.rec_value();
          if(a->size() != b->size())
            return false;
          if(!a->empty()) {
            auto itty = a->cbegin();
            for(const auto &[b1,b2]: *b) {
              if(*itty->first != *b1 || *itty->second != *b2)
                return false;
              ++itty;
            }
          }
          return true;
        }
        case OType::OBJS: {
          const auto objs_a = this->objs_value();
          const auto objs_b = other.objs_value();
          if(objs_a->size() != objs_b->size())
            return false;
          auto it_b = objs_b->begin();
          for(const auto &it_a: *objs_a) {
            if(*it_a != **it_b)
              return false;
            ++it_b;
          }
          return true;
        }
        case OType::BCODE: {
          const auto insts_a = this->bcode_value();
          const auto insts_b = other.bcode_value();
          if(insts_a->size() != insts_b->size())
            return false;
          auto it_b = insts_b->begin();
          for(const auto &it_a: *insts_a) {
            if(*it_a != **it_b)
              return false;
            ++it_b;
          }
          return true;
        }
        case OType::INST: return
              *this->inst_args() == *other.inst_args() &&
              this->inst_f()->index() == other.inst_f()->index() &&
              (this->inst_f()->index() == 1 ||
               (*std::get<Obj_p>(*this->inst_f()) == *std::get<Obj_p>(*other.inst_f()))) &&
              this->domain_coefficient() == other.domain_coefficient() &&
              this->range_coefficient() == other.range_coefficient() &&
              *this->inst_seed_supplier() == *other.inst_seed_supplier();
        // TODO: Tuple equality
        default: throw fError("unknown obj type in ==: %s", OTypes.to_chars(this->otype_).c_str());
      }
    }


    bool operator!=(const Obj &other) const { return !(*this == other); }

    bool operator==(const Obj &other) const {
      return this->equals(other);
    }

    [[nodiscard]] Obj_p operator[](const char *id) const { return this->rec_get(id_p(id)); }

    [[nodiscard]] bool is_type() const { return this->otype_ == OType::TYPE; }

    [[nodiscard]] bool is_noobj() const { return this->otype_ == OType::NOOBJ; }

    [[nodiscard]] bool is_bool() const { return this->otype_ == OType::BOOL; }

    [[nodiscard]] bool is_int() const { return this->otype_ == OType::INT; }

    [[nodiscard]] bool is_real() const { return this->otype_ == OType::REAL; }

    [[nodiscard]] bool is_uri() const { return this->otype_ == OType::URI; }

    [[nodiscard]] bool is_str() const { return this->otype_ == OType::STR; }

    [[nodiscard]] bool is_lst() const { return this->otype_ == OType::LST; }

    [[nodiscard]] bool is_poly() const {
      return this->is_lst() || this->is_rec(); // || this->is_objs() /*|| this->is_bcode() || this->is_inst()
    }

    [[nodiscard]] bool is_rec() const { return this->otype_ == OType::REC; }

    [[nodiscard]] bool is_inst() const { return this->otype_ == OType::INST; }

    [[nodiscard]] bool is_objs() const { return this->otype_ == OType::OBJS; }

    [[nodiscard]] bool is_bcode() const { return this->otype_ == OType::BCODE; }

    [[nodiscard]] bool is_code() const { return this->is_bcode() || this->is_inst(); }

    [[nodiscard]] bool is_error() const { return this->otype_ == OType::ERROR; }

    [[nodiscard]] bool is_empty_bcode() const { return this->is_bcode() && this->bcode_value()->empty(); }

    bool is_initial() const {
      const auto &[dmin,dmax] = this->domain_coefficient();
      return dmin == 0 && dmax == 0;
    }

    bool is_generative() const {
      const auto &[dmin,dmax] = this->domain_coefficient();
      return dmin == 0 && dmax > 0;
    }

    bool is_scatter() const {
      const auto &[rmin,rmax] = this->range_coefficient();
      return rmin == 0 && rmax > 1;
    };


    bool is_gather() const {
      const auto &[dmin,dmax] = this->domain_coefficient();
      return dmin == 0 && dmax > 1;
    }

    bool is_terminal() const {
      const auto &[rmin,rmax] = this->range_coefficient();
      return rmin == 0 && rmax == 0;
    }

    bool has_domain(const FOS_INT_TYPE min, const FOS_INT_TYPE max) const {
      return this->domain_coefficient().first == min && this->domain_coefficient().second == max;
    }

    bool has_range(const FOS_INT_TYPE min, const FOS_INT_TYPE max) const {
      return this->range_coefficient().first == min && this->range_coefficient().second == max;
    }

    bool is_map() const {
      const auto &[dmin,dmax] = this->domain_coefficient();
      const auto &[rmin,rmax] = this->range_coefficient();
      return 1 == dmin && 1 == dmax && 1 == rmin && 1 == rmax;
    }

    bool is_filter() const {
      const auto &[rmin,rmax] = this->range_coefficient();
      return !this->is_initial() && rmin == 0 && rmax > 0;
    }

    bool is_indexed_arg() const {
      if(!this->is_uri()) return false;
      return StringHelper::is_integer(this->uri_value().toString());
    }

    bool is_indexed_args() const {
      if(this->is_inst())
        return this->inst_args()->is_indexed_args();
      if(!this->is_rec())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      int counter = 0;
      for(const auto &[k,v]: *this->rec_value()) {
        if(!(k->is_uri() &&
             k->uri_value().toString() == to_string(counter++)))
          return false;
      }
      return true;
    }

    /////////////////////////////////////////////////////////////////
    //////////////////////////// APPLY //////////////////////////////
    /////////////////////////////////////////////////////////////////

    auto operator()(const Obj_p &obj, const InstArgs &args) const {
      return this->apply(obj, args);
    }

    Obj_p apply(const Obj_p &lhs, const InstArgs &args) const {
      const Rec_p remake = Obj::to_rec();
      //// apply lhs to args
      for(const auto &[k,v]: *args->rec_value()) {
        remake->rec_value()->emplace(k, v->apply(lhs));
      }
      ROUTER_PUSH_FRAME("#", args);
      try {
        const Obj_p result = this->apply(lhs);
        ROUTER_POP_FRAME();
        return result;
      } catch(const std::exception& e) {
        ROUTER_POP_FRAME();
        throw;
      }
    }

    Obj_p apply(const Obj_p &lhs) const {
      if(lhs->is_error())
        return lhs;
      //  if(!lhs->is_bcode() || !this->is_bcode()) {
      //   return shared_from_this()->apply(lhs->apply(shared_from_this()));
      //}
      ///////////////////////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////// TYPE (type -> obj) ////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////////////////////////////////////////////////////////
      if(lhs->is_type()) {
        auto next = lhs->type_value();
        if(this->is_inst()) {
          LOG(INFO, "apply type to inst: %s => %s\n", lhs->toString().c_str(), this->toString().c_str());
          if(Compiler(true, false).type_check(lhs, this->domain())) {
          }
          if(lhs->range_coefficient().first < this->domain_coefficient().first) {
            throw fError("%s range coefficient outside the boundaries of %s domain coefficient: {%i,%1} / {%i,%i}",
                         lhs->toString().c_str(), lhs->range_coefficient().first, lhs->range_coefficient().second,
                         this->toString().c_str(), this->domain_coefficient().first, this->domain_coefficient().second);
          }
          if(lhs->range_coefficient().second > this->domain_coefficient().second) {
            throw fError("%s range coefficient outside the boundaries of %s domain coefficient: {%i,%1} / {%i,%i}",
                         lhs->toString().c_str(), lhs->range_coefficient().first, lhs->range_coefficient().second,
                         this->toString().c_str(), this->domain_coefficient().first, this->domain_coefficient().second);
          }
          //   const Inst_p resolved = TYPE_INST_RESOLVER(lhs, this->shared_from_this());
          const Inst_p resolved = Compiler(true, true).resolve_inst(lhs, this->shared_from_this());
          next = next->add_inst(resolved);
        } else {
          throw fError("only inst currently supported: %s", this->toString().c_str());
        } /*else if(this->is_bcode()) {
          next = next->add_bcode(this->shared_from_this(), false);
        } else {
          next = next->add_inst(this->shared_from_this(), false);
        }*/
        // TYPE_CHECKER(this, lhs->tid_, true);
        return Obj::to_type(this->range(), next, lhs->vid_);
      }
      ///////////////////////////////////////////////////////////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////////////////////////////////////////////////////////
      switch(this->otype_) {
        case OType::TYPE: return lhs->is_noobj() ? shared_from_this() : lhs->as(this->tid_);
        case OType::BOOL:
        case OType::INT:
        case OType::REAL:
        case OType::STR:
        case OType::NOOBJ:
        case OType::ERROR:
          return shared_from_this();
        case OType::URI:
          return lhs->deref(shared_from_this());
        case OType::LST: {
          const auto new_values = make_shared<LstList>();
          for(const auto &obj: *this->lst_value()) {
            new_values->emplace_back(obj->apply(lhs));
          }
          return Obj::to_lst(new_values, this->tid_);
        }
        case OType::REC: {
          const auto new_pairs = make_shared<RecMap<>>();
          for(const auto &[key, value]: *this->rec_value()) {
            if(const Obj_p key_apply = key->apply(lhs); !key_apply->is_noobj())
              new_pairs->insert({key_apply, value/*->apply(key_apply)*/});
          }
          return Obj::to_rec(new_pairs, this->tid_);
        }
        case OType::INST: {
          //// dynamically fetch inst implementation if no function body exists (stub inst)
          auto compiler = Compiler(true, false);
          const Inst_p inst = compiler.resolve_inst(lhs, this->shared_from_this());
          if(!lhs->is_code()) {
            //TYPE_CHECKER(lhs.get(), inst->domain(), true);
            if(compiler.reset(true, true)->type_check(lhs, inst->domain())) {
            }
          }
          // compute args
          InstArgs remake;
          //// don't evaluate args for block()-inst -- TODO: don't have these be a 'special inst'
          if(this->inst_op() == "block" ||
             this->inst_op() == "each" ||
             this->inst_op() == "within") {
            remake = inst->inst_args()->clone();
          } else {
            remake = Obj::to_rec();
            //// apply lhs to args
            for(const auto &[k,v]: *inst->inst_args()->rec_value()) {
              remake->rec_value()->insert({k, v->apply(lhs)});
            }
          }
          ROUTER_PUSH_FRAME("#", remake);
          //// TODO: type check lhs-based on inst type_id domain
          //// TODO: don't evaluate inst for type objs for purpose of compilation
          try {
            if(nullptr == inst->inst_f()) {
              throw fError("!runable to resolve!! %s relative to !b%s!g[!!%s!g]!!", inst->toString().c_str(),
                           lhs->tid_->name().c_str(),
                           lhs->toString().c_str());
            }
            const Obj_p result = std::holds_alternative<Obj_p>(*inst->inst_f())
                                   ? (*const_cast<Obj *>(std::get<Obj_p>(*inst->inst_f()).get()))(lhs, remake)
                                   : (*std::get<Cpp_p>(*inst->inst_f()))(lhs, remake);
            if(!result->is_code())
              compiler.reset(true, true)->type_check(result, inst->range());
            ROUTER_POP_FRAME();
            return result;
          } catch(std::exception &e) {
            ROUTER_POP_FRAME();
            // TODO: does this clear all frames automatically through exception recurssion?
            throw fError("%s\n\t\t!rthrown at !yinst!!  %s !g=>!! %s", e.what(),
                         lhs->toString().c_str(),
                         this->toString().c_str());
          }
        }
        case OType::BCODE: {
          if(this->is_empty_bcode()) return lhs;
          if(!lhs->is_objs() && 1 == this->bcode_value()->size()) // TODO: this is because barriers
            return this->bcode_value()->front()->apply(lhs);
          return BCODE_PROCESSOR(this->bcode_starts(to_objs({lhs})))->none_one_all();
        }
        case OType::OBJS: {
          Objs_p objs = Obj::to_objs();
          for(const Obj_p &obj: *this->objs_value()) {
            objs->add_obj(obj->apply(lhs));
          }
          return objs;
        }
        default:
          throw fError("unknown obj type in apply(): %s", OTypes.to_chars(this->otype_).c_str());
      }
    }

    [[nodiscard]]
    bool is_base_type() const { return this->tid_->equals(*OTYPE_FURI.at(this->otype_)); }

    [[nodiscard]] bool match(const Obj_p &type_obj, const bool require_same_type_id = true) const {
      // LOG(TRACE, "!ymatching!!: %s ~ %s\n", this->toString().c_str(), type->toString().c_str());
      if(type_obj->is_empty_bcode())
        return true;
      if(type_obj->is_type())
        return IS_TYPE_OF(this->tid_, type_obj->tid_, {}) && !this->clone()->apply(type_obj->type_value())->is_noobj();
      if(type_obj->is_code() && !this->is_code()) {
        if(type_obj->is_code() && !this->is_code()) {
          const Obj_p result = type_obj->apply(this->clone());
          return result->is_noobj() && type_obj->range_coefficient().first == 0 ? true : !result->is_noobj();
        }
      }
      /* if(!type_obj->value_.has_value() &&
          (type_obj->tid_->equals(*OBJ_FURI) || (FURI_OTYPE.count(type_obj->tid_->no_query()) && FURI_OTYPE.at(
                                                     type_obj->tid_->no_query()) == this->otype_)))
         return true;*/
      if(this->otype_ != type_obj->otype_)
        return false;
      if(require_same_type_id && (*this->tid_ != *type_obj->tid_))
        return false;
      switch(this->otype_) {
        case OType::TYPE:
          return this->type_value()->match(type_obj->is_type() ? type_obj->type_value() : type_obj);
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
          if(objs_a->size() != objs_b->size())
            return false;
          auto b = objs_b->begin();
          for(const auto &a: *objs_a) {
            if(!a->match(*b))
              return false;
            ++b;
          }
          return true;
        }
        case OType::REC: {
          const auto pairs_a = this->rec_value();
          const auto pairs_b = type_obj->rec_value();
          for(const auto &[b_id, b_obj]: *pairs_b) {
            bool found = false;
            for(const auto &[a_id, a_obj]: *pairs_a) {
              if((b_id->is_uri() && b_id->uri_value().toString().find(':') != string::npos) || (
                   a_id->match(b_id) && a_obj->match(b_obj))) {
                found = true;
                break;
              }
            }
            if(!found)
              return false;
          }
          return true;
        }
        case OType::INST: {
          const auto args_a = this->inst_args();
          if(const auto args_b = type_obj->inst_args(); !args_a->match(args_b))
            return false;
          if(this->domain_coefficient() != type_obj->domain_coefficient() ||
             this->range_coefficient() != type_obj->range_coefficient())
            return false;
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
          throw fError("unknown obj type in match(): %s", OTypes.to_chars(this->otype_).c_str());
      }
    }

    /*[[nodiscard]] Obj_p as(const char *furi) const {
      return this->as(id_p(furi));
    }*/

    [[nodiscard]] Obj_p as(const ID_p &type_id) const {
      return Obj::create(this->value_, this->otype_, type_id, this->vid_);
    }

    Obj_p at(const ID_p &value_id) const {
      if(value_id == nullptr && this->vid_ == nullptr)
        return this->shared_from_this();
      return Obj::create(this->value_, this->otype_, this->tid_, value_id);
    }

    [[nodiscard]] Option<fURI> lock() const {
      return this->vid_ && this->vid_->query_value("lock").has_value()
               ? Option<fURI>(fURI(this->vid_->query_value("lock").value().c_str()))
               : Option<fURI>();
    }


    [[nodiscard]] Obj_p lock(const fURI &user) const {
      if(this->vid_ == nullptr)
        throw fError("only objs with a value id can be locked: %s\n", this->toString().c_str());
      if(this->lock().has_value())
        throw fError("obj currently locked by %s: %s\n", this->vid_->query_value("lock").value().c_str());
      const string new_query = strlen(this->vid_->query()) == 0
                                 ? string("lock=").append(user.toString())
                                 : string(this->vid_->query()).append("&lock=").append(user.toString());
      const ID new_vid = this->vid_->query(new_query.c_str());
      const Obj_p new_obj = this->at(id_p(new_vid));
      LOG_OBJ(INFO, this, "!g[!r.!y.!c.!g]!m@!b%s !yobj!! locked by !b%s!!\n",
              this->vid_->no_query().toString().c_str(), user.toString().c_str());
      return new_obj;
    }

    [[nodiscard]] Obj_p unlock(const fURI &user) const {
      if(this->vid_ == nullptr)
        throw fError("only objs with a value id can be locked and unlocked: %s\n", this->toString().c_str());
      if(!this->lock().has_value())
        throw fError("obj is not locked: %s\n", this->toString().c_str());
      if(this->lock().value() == user) {
        const ID new_vid = this->vid_->query(""); // TODO: selectively remove lock
        const Obj_p new_obj = this->at(id_p(new_vid));
        LOG_OBJ(INFO, this, "!g[!r.!y.!c.!g]!m@!b%s !yobj!! unlocked by !b%s!!\n",
              this->vid_->no_query().toString().c_str(), user.toString().c_str());
        return new_obj;
      } else {
        throw fError("only the owner %s can unlock %s\n", this->vid_->query_value("lock").value().c_str(),
                     this->toString().c_str());
      }
      return this->shared_from_this();
    }

    [[nodiscard]] Inst_p next_inst(const Inst_p &current_inst) const {
      if(current_inst == nullptr)
        return this->bcode_value()->front();
      if(current_inst->is_noobj())
        return current_inst;
      bool found = false;
      for(const auto &inst: *this->bcode_value()) {
        if(found)
          return inst;
        if(inst == current_inst)
          found = true;
      }
      return Obj::to_noobj();
    }

    Obj_p or_else(const Obj_p &other) const {
      return this->is_noobj() ? other : this->shared_from_this();
    }

    /// STATIC TYPE CONSTRAINED CONSTRUCTORS
    static Obj_p to_type(const ID_p &type_id,
                         const Obj_p &obj = Obj::create(make_shared<List<Inst_p>>(), OType::BCODE,
                                                        OTYPE_FURI.at(OType::BCODE)),
                         const ID_p &value_id = nullptr) {
      return Obj::create(obj, OType::TYPE, type_id, value_id);
    }

    static Obj_p to_noobj() {
      static auto noobj = Obj::create(Any(nullptr), OType::NOOBJ, NOOBJ_FURI);
      // id_p(NOOBJ_FURI->query({{"dc", "0,0"}, {"rc", "0,0"}})));
      return noobj;
    }

    static Bool_p to_bool(const bool value, const ID_p &typed_id = BOOL_FURI, const ID_p &value_id = nullptr) {
      return Obj::create(value, OType::BOOL, typed_id, value_id);
    }

    static Int_p to_int(const FOS_INT_TYPE value, const ID_p &type_id = INT_FURI, const ID_p &value_id = nullptr) {
      return Obj::create(value, OType::INT, type_id, value_id);
    }

    static Real_p to_real(const FOS_REAL_TYPE value, const ID_p &type_id = REAL_FURI, const ID_p &value_id = nullptr) {
      return Obj::create(value, OType::REAL, type_id, value_id);
    }

    static Str_p to_str(const string &value, const ID_p &type_id = STR_FURI, const ID_p &value_id = nullptr) {
      return Obj::create(value, OType::STR, type_id, value_id);
    }

    static Str_p to_str(const char *value, const ID_p &type_id = STR_FURI, const ID_p &value_id = nullptr) {
      return Obj::create(string(value), OType::STR, type_id, value_id);
    }

    static Uri_p to_uri(const fURI &value, const ID_p &type_id = URI_FURI, const ID_p &value_id = nullptr) {
      return Obj::create(fURI(value), OType::URI, type_id, value_id);
    }

    static Uri_p to_uri(const char *value, const ID_p &type_id = URI_FURI, const ID_p &value_id = nullptr) {
      return Obj::create(fURI(value), OType::URI, type_id, value_id);
    }


    static Lst_p to_lst(const LstList_p &xlst, const ID_p &type_id = LST_FURI, const ID_p &value_id = nullptr) {
      return Obj::create(xlst, OType::LST, type_id, value_id);
    }

    static Lst_p to_lst(const ID_p &type_id = LST_FURI, const ID_p &value_id = nullptr) {
      return to_lst(make_shared<LstList>(), type_id, value_id);
    }

    static Lst_p to_lst(const std::initializer_list<Obj> &xlst, const ID_p &type_id = LST_FURI,
                        const ID_p &value_id = nullptr) {
      const auto list = make_shared<LstList>();
      for(const auto &obj: xlst) {
        list->push_back(obj.clone());
      }
      return to_lst(list, type_id, value_id);
    }

    static Lst_p to_lst(const std::initializer_list<Obj_p> &xlst, const ID_p &type_id = LST_FURI,
                        const ID_p &value_id = nullptr) {
      return to_lst(make_shared<LstList>(xlst), type_id, value_id);
    }


    static Rec_p to_rec(const RecMap_p<> &map, const ID_p &type_id = REC_FURI, const ID_p &value_id = nullptr) {
      return Obj::create(map, OType::REC, type_id, value_id);
    }

    static Rec_p to_rec(const ID_p &type_id = REC_FURI, const ID_p &value_id = nullptr) {
      return to_rec(make_shared<RecMap<>>(), type_id, value_id);
    }

    static Rec_p to_rec(const std::initializer_list<Pair<const Obj, Obj>> &xrec, const ID_p &type_id = REC_FURI,
                        const ID_p &value_id = nullptr) {
      const auto map = make_shared<Obj::RecMap<>>();
      for(const auto &[key, value]: xrec) {
        map->insert(make_pair(make_shared<Obj>(key), make_shared<Obj>(value)));
      }
      return to_rec(map, type_id, value_id);
    }

    static Rec_p to_rec(const std::initializer_list<Pair<const Obj_p, Obj_p>> &xrec, const ID_p &type_id = REC_FURI,
                        const ID_p &value_id = nullptr) {
      const auto map = make_shared<Obj::RecMap<>>();
      for(const auto &[key, value]: xrec) {
        map->insert(make_pair(key, value));
      }
      return to_rec(map, type_id, value_id);
    }

    static Rec_p to_rec(const std::initializer_list<Pair<const string, Obj_p>> &xrec, const ID_p &type_id = REC_FURI,
                        const ID_p &value_id = nullptr) {
      const auto map = make_shared<Obj::RecMap<>>();
      for(const auto &[key, value]: xrec) {
        map->insert(make_pair(Obj::to_uri(key), value));
      }
      return to_rec(map, type_id, value_id);
    }

    static Inst_p to_inst(const InstValue_p &value, const ID_p &type_id = INST_FURI, const ID_p &value_id = nullptr) {
      if(!std::get<0>(*value)->is_rec())
        TYPE_ERROR(std::get<0>(*value).get(),__FILE__,__LINE__);
      return Obj::create(value, OType::INST, type_id, value_id);
    }

    static InstArgs to_inst_args() {
      return Obj::to_rec();
    }

    static InstArgs to_inst_args(const List<Obj_p> &args) {
      const Rec_p a = Obj::to_rec();
      for(size_t i = 0; i < args.size(); i++) {
        a->rec_value()->insert({Obj::to_uri(to_string(i)), args.at(i)});
      }
      return a;
    }

    static Inst_p to_inst(const std::initializer_list<Obj_p> &args, const ID_p &type_id,
                          const ID_p &value_id = nullptr) {
      return to_inst(type_id->name(), Obj::to_inst_args(args), nullptr, to_noobj(), type_id,
                     value_id);
    }

    static Inst_p to_inst(const List<Obj_p> &args, const ID_p &type_id, const ID_p &value_id = nullptr) {
      return to_inst(type_id->name(), Obj::to_inst_args(args), nullptr, to_noobj(), type_id,
                     value_id);
    }

    static Inst_p to_inst(const InstArgs &args, const ID_p &type_id, const ID_p &value_id = nullptr) {
      return to_inst(type_id->name(), args, nullptr, to_noobj(), type_id, value_id);
    }

    static Inst_p to_inst(const string &opcode, const InstArgs &args, const InstF_p &function,
                          const Obj_p &seed = Obj::to_noobj(), const ID_p &type_id = nullptr,
                          const ID_p &value_id = nullptr) {
      const ID_p fix = type_id != nullptr ? type_id : id_p(*ROUTER_RESOLVE(fURI(opcode)));
      return to_inst(make_shared<InstValue>(make_tuple(args, function, seed)), fix, value_id);
    }

    static BCode_p to_bcode(const InstList_p &insts, const ID_p &type_id = BCODE_FURI, const ID_p &value_id = nullptr) {
      return Obj::create(insts, OType::BCODE, type_id, value_id);
    }

    static BCode_p to_bcode(const InstList &insts, const ID_p &type_id = BCODE_FURI, const ID_p &value_id = nullptr) {
      return Obj::to_bcode(make_shared<InstList>(insts), type_id, value_id);
    }

    static BCode_p to_bcode(const ID_p &type_id = BCODE_FURI, const ID_p &value_id = nullptr) {
      return Obj::to_bcode(make_shared<InstList>(), type_id, value_id);
    }

    static BCode_p ___() {
      return to_bcode();
    }

    static Objs_p to_objs(const List_p<Obj_p> &objs, const ID_p &type_id = OBJS_FURI, const ID_p &value_id = nullptr) {
      const auto list = make_shared<List<Obj_p>>();
      for(const auto &obj: *objs) {
        if(obj->is_objs()) {
          for(const auto &o: *obj->objs_value()) {
            if(o->is_objs())
              throw fError("nested objs not allowed: %s\n", o->toString().c_str());
            list->push_back(o);
          }
        } else
          list->push_back(obj);
      }
      return Obj::create(list, OType::OBJS, type_id, value_id);
    }

    static Objs_p to_objs(const std::initializer_list<Obj_p> &list, const ID_p &type_id = OBJS_FURI,
                          const ID_p &value_id = nullptr) {
      return Obj::to_objs(make_shared<List<Obj_p>>(list.begin(), list.end()), type_id, value_id);
    }

    static Objs_p to_objs(const ID_p &type_id = OBJS_FURI, const ID_p &value_id = nullptr) {
      return to_objs(make_shared<List<Obj_p>>(), type_id, value_id);
    }

    static Error_p to_error(const Obj_p &obj, const Inst_p &inst, const ID_p &type_id = ERROR_FURI) {
      return Obj::create(Pair<Obj_p, Inst_p>(obj, inst), OType::ERROR, type_id);
    }

    /*std::__allocator_base<Obj> allocator = std::allocator<Obj>()*/
    Obj_p clone() const {
      switch(this->otype_) {
        case OType::NOOBJ: return Obj::to_noobj();
        case OType::OBJ:
        case OType::ERROR:
        case OType::BOOL:
        case OType::INT:
        case OType::REAL:
        case OType::STR:
        case OType::URI: {
          auto r = Obj::create(this->value_, this->otype_, this->tid_);
          r->vid_ = this->vid_;
          return r;
        }
        case OType::LST: {
          const auto new_list = make_shared<LstList>();
          for(const auto &e: *this->lst_value()) {
            new_list->push_back(e->clone());
          }
          auto r = Lst::create(new_list, OType::LST, this->tid_);
          r->vid_ = this->vid_;
          return r;
        }
        case OType::REC: {
          const auto new_map = make_shared<RecMap<>>();
          for(const auto &[k, v]: *this->rec_value()) {
            new_map->insert({k->clone(), v->clone()});
          }
          auto r = Rec::create(new_map, OType::REC, this->tid_);
          r->vid_ = this->vid_;
          return r;
        }
        case OType::INST: {
          auto r = Inst::create(make_shared<InstValue>(make_tuple(this->inst_args(),
                                                                  this->inst_f(),
                                                                  this->inst_seed_supplier()
                                                                    ? this->inst_seed_supplier()->clone()
                                                                    : Obj::to_noobj())), OType::INST, this->tid_);
          r->vid_ = this->vid_;
          return r;
        }
        case OType::BCODE: {
          InstList_p new_insts = make_shared<InstList>();
          for(const auto &inst: *this->bcode_value()) {
            new_insts->push_back(inst->clone());
          }
          auto r = BCode::create(new_insts, OType::BCODE, this->tid_);
          r->vid_ = this->vid_;
          return r;
        }
        case OType::OBJS: {
          const auto new_list = make_shared<List<Obj_p>>();
          for(const auto &e: *this->objs_value()) {
            new_list->push_back(e->clone());
          }
          auto r = Objs::create(new_list, OType::OBJS, this->tid_);
          r->vid_ = this->vid_;
          return r;
        }
        case OType::TYPE: {
          auto r = Type::create(this->type_value()->clone(), OType::TYPE, this->tid_);
          r->vid_ = this->vid_;
          return r;
        }
        default: throw fError("unknown base type: %i", this->otype_);
      }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    BObj_p serialize() const {
      LOG(DEBUG, "serializing obj %s\n", this->toString().c_str());
      const string serial = this->toString(SERIALIZER_PRINTER);
      return ptr<BObj>(new BObj(serial.length(), reinterpret_cast<fbyte *>(strdup(serial.c_str()))), bobj_deleter);
    }

    static Obj_p deserialize(const BObj_p &bobj) {
      LOG(DEBUG, "deserializing bytes %s (length %i)\n", bobj->second, bobj->first);
      return OBJ_PARSER(string(reinterpret_cast<char *>(bobj->second), bobj->first));
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  protected
  :
    class ObjsSet {
    public:
      const unique_ptr<OrderedMap<Obj_p, long, objp_hash, objp_equal_to>> internal =
          make_unique<OrderedMap<Obj_p, long, objp_hash, objp_equal_to>>();

      void push_back(const Obj_p &obj, const long bulk = 1) const {
        this->add(obj, bulk);
      }

      void add(const Obj_p &obj, const long bulk = 1) const {
        if(this->internal->count(obj)) {
          const long prev_bulk = this->internal->at(obj);
          this->internal->insert_or_assign(obj, prev_bulk + bulk);
        } else {
          this->internal->insert_or_assign(obj, bulk);
        }
      }

      [[nodiscard]] Obj_p next() const {
        if(this->internal->empty())
          return Obj::to_noobj();
        auto [obj, bulk] = this->internal->front();
        if(bulk < 2)
          this->internal->erase(obj);
        else
          this->internal->insert_or_assign(obj, bulk - 1);
        return obj;
      }

      [[nodiscard]] long bulk_size() const {
        long bulk_total = 0;
        for(const auto &[obj,bulk]: *this->internal) {
          bulk_total += bulk;
        }
        return bulk_total;
      }

      [[nodiscard]] unsigned long size() const {
        return this->internal->size();
      }

      [[nodiscard]] bool empty() const {
        return this->internal->empty();
      }
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  };

  [[maybe_unused]] static Uri_p vri(const fURI &xuri, const ID_p &type = URI_FURI) { return Obj::to_uri(xuri, type); }

  [[maybe_unused]] static Uri_p vri(const ID_p &xuri, const ID_p &type_id = URI_FURI) {
    return Obj::to_uri(*xuri, type_id);
  }

  [[maybe_unused]] static Uri_p vri(const fURI_p &xuri, const ID_p &type_id = URI_FURI) {
    return Obj::to_uri(*xuri, type_id);
  }

  [[maybe_unused]] static Uri_p vri(const char *xuri, const ID_p &type_id = URI_FURI) {
    return Obj::to_uri(fURI(xuri), type_id);
  }

  [[maybe_unused]] static Uri_p vri(const string &xuri, const ID_p &type_id = URI_FURI) {
    return Obj::to_uri(fURI(xuri), type_id);
  }

  [[maybe_unused]] static Bool_p dool(const bool xbool, const ID_p &type_id = BOOL_FURI,
                                      const ID_p &value_id = nullptr) {
    return Obj::to_bool(xbool, type_id, value_id);
  }

  [[maybe_unused]] static Int_p jnt(const FOS_INT_TYPE xint, const ID_p &type_id = INT_FURI,
                                    const ID_p &value_id = nullptr) {
    return Obj::to_int(xint, type_id, value_id);
  }

  [[maybe_unused]] static Str_p str(const char *xstr, const ID_p &type_id = STR_FURI, const ID_p &value_id = nullptr) {
    return Obj::to_str(xstr, type_id, value_id);
  }

  [[maybe_unused]] static Str_p str(const string &xstr, const ID_p &type_id = STR_FURI,
                                    const ID_p &value_id = nullptr) {
    return Obj::to_str(xstr, type_id, value_id);
  }

  [[maybe_unused]] static Real_p real(const FOS_REAL_TYPE &xreal, const ID_p &type_id = REAL_FURI,
                                      const ID_p &value_id = nullptr) {
    return Obj::to_real(xreal, type_id, value_id);
  }

  [[maybe_unused]] static NoObj_p noobj() { return Obj::to_noobj(); }

  [[maybe_unused]] static Lst_p lst() { return Obj::to_lst(make_shared<Obj::LstList>()); }

  [[maybe_unused]] static Lst_p lst(const List<Obj_p> &list) { return Obj::to_lst(make_shared<Obj::LstList>(list)); }

  [[maybe_unused]] static Lst_p lst(const List_p<Obj_p> &list) { return Obj::to_lst(list); }

  [[maybe_unused]] static Rec_p rec() { return Obj::to_rec(make_shared<Obj::RecMap<>>()); }

  [[maybe_unused]] static Rec_p rec(const Obj::RecMap<> &map, const ID_p &tid = REC_FURI, const ID_p &vid = nullptr) {
    return Obj::to_rec(make_shared<Obj::RecMap<>>(map), tid, vid);
  }

  [[maybe_unused]] static Rec_p rec(const std::initializer_list<Pair<const Obj_p, Obj_p>> &map,
                                    const ID_p &type = REC_FURI, const ID_p &id = nullptr) {
    return Obj::to_rec(map, type, id);
  }

  [[maybe_unused]] static Rec_p rec(const std::initializer_list<Pair<const string, Obj_p>> &map,
                                    const ID_p &type = REC_FURI, const ID_p &id = nullptr) {
    Obj::RecMap<> rec_map = Obj::RecMap<>();
    std::transform(map.begin(), map.end(),
                   std::inserter(rec_map, rec_map.end()),
                   [](const auto &pair) { return std::make_pair(vri(pair.first), pair.second); });
    return rec(rec_map, type, id);
  }

  [[maybe_unused]] static Objs_p objs() { return Obj::to_objs(make_shared<List<Obj_p>>()); }

  [[maybe_unused]] static Objs_p objs(const List<Obj_p> &list) {
    return Obj::to_objs(make_shared<List<Obj_p>>(list));
  }

  [[maybe_unused]] static Objs_p objs(const List_p<Obj_p> &list) { return Obj::to_objs(list); }

  [[maybe_unused]] static BCode_p bcode(const InstList &list) { return Obj::to_bcode(list); }

  [[maybe_unused]] static BCode_p bcode() { return Obj::to_bcode(); }

  static Obj::RecMap_p<> rmap(const initializer_list<Pair<fURI, Obj_p>> &pairs) {
    auto m = make_shared<Obj::RecMap<>>();
    for(const auto &[id, obj]: pairs) {
      m->insert({vri(id), obj});
    }
    return m;
  }

  static Obj_p from(const Uri_p &uri, const Obj_p &default_arg = noobj()) {
    return Obj::to_inst(
      "from", Obj::to_inst_args({uri, default_arg}),
      make_shared<InstF>(make_shared<BiFunction<const Obj_p, const InstArgs, Obj_p>>(
        [](const Uri_p &, const InstArgs &args) {
          const Obj_p result = ROUTER_READ(furi_p(args->arg(0)->uri_value()));
          return result->is_noobj() ? args->arg(1) : result;
        })));
  }

    static Obj_p from(const char* uri, const Obj_p &default_arg = noobj()) {
    return from(Obj::to_uri(uri),default_arg);
  }

  [[maybe_unused]] static Inst_p block(const Obj_p &arg) {
    return Obj::to_inst({arg}, id_p("block"));
  }

  [[maybe_unused]] static Inst_p x(const uint8_t arg_num, const Obj_p &default_arg = noobj()) {
    return from(Obj::to_uri(to_string(arg_num)), default_arg);
  }

  [[maybe_unused]] static Inst_p x(const uint8_t arg_num, const char *arg_name, const Obj_p &default_arg = noobj()) {
    return from(Obj::to_uri(ID(to_string(arg_num)).query(arg_name)), default_arg);
  }

  /* [[maybe_unused]] static Inst_p x(const string &arg_name, const Obj_p &default_arg = noobj()) {
     from(Obj::to_uri(arg_name), default_arg);
   }*/

  inline std::ostream &operator <<(std::ostream &os, const Obj &value) {
    os << value.toString();
    return os;
  }
} // namespace fhatos
#endif
