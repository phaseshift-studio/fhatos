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

#include <fmt/core.h>
#include <fmt/format.h>
#include <utility>
#include <variant>
#include "../fhatos.hpp"
#include "../furi.hpp"
#include "../util/string_helper.hpp"
#include "../util/tsl/ordered_map.h"
#include "mmadt/compiler.hpp"
#ifdef ESP_PLATFORM
#include "esp_task_wdt.h"
#endif


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

  static const auto OTypes = Enums<OType>({{OType::OBJ, "obj"},
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
    if(it == m.end())
      return default_value;
    return it->second;
  }

  const Map<const Pair<IntCoefficient, IntCoefficient>, string> SIGNATURE_DESCRIPTIONS = {
      {{{0, 0}, {0, 0}}, ".->. (transient)"},      {{{0, 0}, {1, 1}}, ".->o (supplier)"},
      {{{0, 0}, {0, INT_MAX}}, ".->* (initial)"},  {{{1, 1}, {0, 0}}, "o->. (consumer)"},
      {{{0, INT_MAX}, {0, 0}}, "*->. (terminal)"}, {{{1, 1}, {1, 1}}, "o->o (map)"},
      {{{1, 1}, {0, INT_MAX}}, "o->* (flatmap)"},  {{{0, 1}, {1, INT_MAX}}, "?->* (potential)"},
      {{{0, 1}, {0, 0}}, "?->o (flip)"},           {{{0, 1}, {0, 0}}, "?->. (spark)"},
      {{{0, 1}, {0, 1}}, "?->? (flux)"},           {{{1, 1}, {0, 1}}, "o->? (filter)"},
      {{{0, 0}, {0, 1}}, ".->? (check)"},          {{{0, 1}, {0, 1}}, "?-? (strain)"},
      {{{0, INT_MAX}, {1, 1}}, "*->o (reduce)"},   {{{0, INT_MAX}, {0, INT_MAX}}, "*->* (barrier)"},
  };
  using InstArgs = Rec_p;
  using Cpp = BiFunction<const Obj_p, const InstArgs, Obj_p>;
  using Cpp_p = ptr<Cpp>;
  using InstF = std::variant<Obj_p, Cpp_p>;
  using InstValue = Trip<InstArgs, InstF, Obj_p>;
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

    explicit DomainRange(const fURI &domain, IntCoefficient dom_coeff, const fURI &range, IntCoefficient rng_coeff) :
        domain{domain}, dom_coeff{std::move(dom_coeff)}, range{range}, rng_coeff{std::move(rng_coeff)} {}

    [[nodiscard]] bool equals(const DomainRange &other) const {
      return this->domain == other.domain && this->dom_coeff == other.dom_coeff && this->range == other.range &&
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
      const auto dr = DomainRange(fURI(furi.query_value(FOS_DOMAIN).value_or(OBJ_FURI->toString())), dom_coeff,
                                  fURI(furi.query_value(FOS_RANGE).value_or(furi.no_query().toString())), rng_coeff);
      return dr;
    }
  };

  static const std::map<OType, ID_p> OTYPE_FURI = {{{OType::NOOBJ, NOOBJ_FURI},
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

  static ID_p SCHEDULER_ID = id_p("/sys/scheduler");
  static ID_p ROUTER_ID = id_p("/sys/router");
  static ID_p TYPER_ID = id_p("/sys/typer");


  struct ObjPrinter {
    bool show_id;
    bool show_type;
    bool show_domain_range;
    bool strict;
    bool ansi;
    bool propagate;

    [[nodiscard]] const ObjPrinter *next() const { return this->propagate ? this : nullptr; }

    [[nodiscard]] unique_ptr<ObjPrinter> clone() const {
      return make_unique<ObjPrinter>(ObjPrinter{this->show_id, this->show_type, this->show_domain_range, this->strict,
                                                this->ansi, this->propagate});
    }
  };

  static auto NO_ANSI_PRINTER = new ObjPrinter{.show_id = true,
                                               .show_type = true,
                                               .show_domain_range = false,
                                               .strict = false,
                                               .ansi = false,
                                               .propagate = false};
  static auto SERIALIZER_PRINTER = new ObjPrinter{
      .show_id = true, .show_type = true, .show_domain_range = true, .strict = true, .ansi = false, .propagate = true};
  static auto DEFAULT_OBJ_PRINTER = new ObjPrinter{.show_id = true,
                                                   .show_type = true,
                                                   .show_domain_range = false,
                                                   .strict = false,
                                                   .ansi = true,
                                                   .propagate = false};
  static auto DEFAULT_INST_PRINTER = new ObjPrinter{
      .show_id = true, .show_type = true, .show_domain_range = true, .strict = false, .ansi = true, .propagate = false};
  static auto FULL_OBJ_PRINTER = new ObjPrinter{
      .show_id = true, .show_type = true, .show_domain_range = true, .strict = true, .ansi = true, .propagate = true};
  static auto DEFAULT_BCODE_PRINTER = new ObjPrinter{
      .show_id = true, .show_type = true, .show_domain_range = true, .strict = false, .ansi = true, .propagate = false};
  static auto DEFAULT_NOOBJ_PRINTER = new ObjPrinter{.show_id = false,
                                                     .show_type = false,
                                                     .show_domain_range = false,
                                                     .strict = false,
                                                     .ansi = true,
                                                     .propagate = false};
  static Map<OType, ObjPrinter *> GLOBAL_PRINTERS = {
      {OType::NOOBJ, DEFAULT_NOOBJ_PRINTER}, {OType::TYPE, DEFAULT_OBJ_PRINTER},  {OType::OBJ, DEFAULT_OBJ_PRINTER},
      {OType::BOOL, DEFAULT_OBJ_PRINTER},    {OType::INT, DEFAULT_OBJ_PRINTER},   {OType::REAL, DEFAULT_OBJ_PRINTER},
      {OType::STR, DEFAULT_OBJ_PRINTER},     {OType::URI, DEFAULT_OBJ_PRINTER},   {OType::LST, DEFAULT_OBJ_PRINTER},
      {OType::REC, DEFAULT_OBJ_PRINTER},     {OType::INST, DEFAULT_INST_PRINTER}, {OType::BCODE, DEFAULT_BCODE_PRINTER},
      {OType::OBJS, DEFAULT_OBJ_PRINTER},    {OType::ERROR, DEFAULT_OBJ_PRINTER},
  };

  inline TriConsumer<LOG_TYPE, const Obj *, std::function<std::string()>> LOG_WRITE =
      [](const LOG_TYPE log_type, const Obj *source, const std::function<std::string()> &message) {
        LOG(log_type, "%s", message().c_str());
      };
  inline Supplier<Obj_p> ROUTER_GET_FRAME_DATA = []() {
    LOG(ERROR, "!yROUTER_GET_FRAME!! undefined at this point in bootstrap\n");
    return nullptr;
  };
  inline TriFunction<const Pattern, const Rec_p, const Supplier<Obj_p>, Obj_p> ROUTER_EXEC_WITHIN_FRAME =
      [](const Pattern &, const Rec_p &, const Supplier<Obj_p> &) {
        LOG(ERROR, "!yROUTER_EXEC_WITHIN_FRAME!! undefined at this point in bootstrap\n");
        return nullptr;
      };
  inline Runnable ROUTER_POP_FRAME = [] { LOG(TRACE, "!ROUTER_POP_FRAME!! undefined at this point in bootstrap\n"); };
  inline BiConsumer<Pattern, Rec_p> ROUTER_PUSH_FRAME = [](const Pattern &pattern, const Rec_p &) {
    LOG(ERROR, "!yROUTER_PUSH_FRAME!! undefined at this point in bootstrap: %s\n", pattern.toString().c_str());
  };
  inline TriFunction<const ID_p &, const ID_p &, List<ID_p> *, const bool> IS_TYPE_OF = [](const ID_p &is_type_id,
                                                                                           const ID_p &, List<ID_p> *) {
    LOG(ERROR, "!yIS_TYPE_OF!! undefined at this point in bootstrap: %s\n", is_type_id->toString().c_str());
    return false;
  };
  inline Function<const string &, const Obj_p> OBJ_PARSER = [](const string &code) {
    LOG(ERROR, "!yOBJ_PARSER!! undefined at this point in bootstrap: %s\n", code.c_str());
    return nullptr;
  };
  inline Function<BCode_p, Objs_p> BCODE_PROCESSOR = [](const BCode_p &) {
    LOG(ERROR, "!yBCODE_PROCESSOR!! undefined at this point in bootstrap\n");
    return nullptr;
  };
  inline Function<const fURI, const fURI> ROUTER_RESOLVE = [](const fURI &furi) {
    if(!BOOTING)
      LOG(ERROR, "!yROUTER_RESOLVE!! undefined at this point in bootstrap\n");
    return furi;
  };
  inline TriConsumer<const fURI &, const Obj_p &, const bool> ROUTER_WRITE = [](const fURI &, const Obj_p &,
                                                                                const bool) -> void {
    LOG(ERROR, "!yROUTER_WRITE!! undefined at this point in bootstrap\n");
  };
  inline BiConsumer<const fURI &, const Obj_p &> ROUTER_APPEND = [](const fURI &, const Obj_p &) -> void {
    LOG(ERROR, "!yROUTER_APPEND!! undefined at this point in bootstrap\n");
  };
  inline Function<const fURI &, const Obj_p> ROUTER_READ = [](const fURI &furi) -> Obj_p {
    LOG(ERROR, "!yROUTER_READ!! undefined at this point in bootstrap: !b%s!!\n", furi.toString().c_str());
    return nullptr;
  };
  inline BiConsumer<const ID, const Obj_p> TYPER_SAVE_TYPE = [](const ID &type_id, const Obj_p &obj) {
    LOG(ERROR, "!yTYPE_SAVER!! undefined at this point in bootstrap: %s\n", type_id.toString().c_str());
    ROUTER_WRITE(type_id, obj, true);
  };
  inline BiFunction<const Obj_p &, const Inst_p &, Inst_p> TYPE_INST_RESOLVER = [](const Obj_p &, const Inst_p &) {
    LOG(ERROR, "!RESOLVE_INST!! undefined at this point in bootstrap\n");
    return nullptr;
  };
  inline BiFunction<const Obj_p &, const ObjPrinter *, string> PRINT_OBJ = [](const Obj_p &, const ObjPrinter *) {
    LOG(ERROR, "!PRINT_OBJ!! undefined at this point in bootstrap\n");
    return "";
  };
  inline Runnable FEED_WATCHDOG = []() {
#ifdef ESP_PLATFORM
    // esp_task_wdt_reset();
    vTaskDelay(1); // feeds the watchdog for the task
#endif
  };

  class ObjsSet;

  //////////////////////////////////////////////////
  ////////////////////// OBJ //////////////////////
  /////////////////////////////////////////////////
  /// An mm-ADT abstract object from which all other types derive
  inline auto MODEL_MAP = make_unique<Map<ID, ptr<void>>>();
  inline auto MODEL_CREATOR2 = make_unique<Map<ID, Function<Obj_p, ptr<void>>>>();
  class Obj : public Typed,
              public Valued,
              public Function<Obj_p, Obj_p>,
              public BiFunction<Obj_p, InstArgs, Obj_p>,
              public enable_shared_from_this<const Obj> {
  public:
    OType otype;
    Any value_;

    template<typename T>
    [[nodiscard]] ptr<const T> get_shared_from_this() const {
      return std::dynamic_pointer_cast<const T>(this->shared_from_this());
    }

    struct objp_hash {
      size_t operator()(const Obj_p &obj) const { return std::hash<std::string>{}(obj->toString()); }
    };

    struct objp_equal_to {
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

    explicit Obj(Any value, const OType otype, const ID_p &type_id, const ID_p &value_id = nullptr) :
        Typed(type_id), Valued(value_id), otype(otype), value_(value) {}

    Obj(Obj &&other) noexcept : Obj(std::move(other.value_), other.otype, std::move(other.tid), std::move(other.vid)) {
      other.value_ = nullptr;
    }

    Obj &operator=(Obj &&other) noexcept {
      if(this == &other) {
        return *this;
      }
      this->value_ = std::move(other.value_);
      this->otype = other.otype;
      this->vid = std::move(other.vid);
      this->tid = std::move(other.tid);
      other.value_ = nullptr;
      return *this;
    }

    virtual ~Obj() override = default;

    Obj(const Obj &other) : Obj(any(nullptr), other.otype, other.tid, other.vid) {
      switch(other.otype) {
        case OType::NOOBJ:
          this->value_ = any(nullptr);
          break;
        case OType::BOOL:
        case OType::INT:
        case OType::REAL:
          this->value_ = other.value_;
          break;
        case OType::STR:
          this->value_ = any(string(other.str_value()));
          break;
        case OType::URI:
          this->value_ = any(fURI(other.uri_value()));
          break;
        case OType::LST: {
          const auto list = make_shared<std::vector<Obj_p>>();
          for(const Obj_p &element: *other.lst_value()) {
            list->push_back(make_shared<Obj>(*element));
          }
          this->value_ = any(list);
          break;
        }
        case OType::REC: {
          const auto record = make_shared<RecMap<>>();
          for(const auto &[k, v]: *other.rec_value()) {
            record->insert_or_assign(make_shared<Obj>(*k), make_shared<Obj>(*v));
          }
          this->value_ = any(record);
          break;
        }
        case OType::BCODE: {
          const auto insts = make_shared<InstList>();
          for(const auto &inst: *other.bcode_value()) {
            insts->push_back(make_shared<Obj>(*inst));
          }
          this->value_ = any(insts);
          break;
        }
        case OType::INST:
        case OType::OBJS:
        case OType::ERROR:
        case OType::TYPE:
          this->value_ = other.value_;
          break;
        default:
          throw fError("unknown obj type being created: %s", other.otype);
      }
    }

    static Obj_p create(const Any &value, const OType otype, const ID_p &type_id, const ID_p &value_id = nullptr) {
      const auto creation = make_shared<Obj>(value, otype, type_id, value_id);
      creation->type_check();
      if(creation->vid)
        ROUTER_WRITE(*value_id, creation, true);
      return creation;
    }

    void type_check() {
      if(this->is_base_type())
        return;
      if(this->otype == OType::INST && nullptr == std::get<2>(std::any_cast<InstValue>(this->value_))) {
        this->value_ = make_tuple(std::get<0>(std::any_cast<InstValue>(this->value_)),
                                  std::get<1>(std::any_cast<InstValue>(this->value_)),
                                  this->is_gather() ? Obj::to_objs() : Obj::to_noobj());
      }
      if(this->otype == OType::INST && this->tid->has_query(FOS_RANGE) &&
         this->tid->no_query().equals(ID(this->tid->query_value(FOS_RANGE).value()))) {
        throw fError("the range of an inst can not be its type: %s", this->tid->toString().c_str());
      }
      if(this->value_.has_value()) { // value token
        try {
          if((otype == OType::REC || otype == OType::LST) && !this->tid->equals(*OTYPE_FURI.at(otype))) {
            if(/*(otype == OType::REC || otype == OType::LST) && !this->tid->equals(*OTYPE_FURI.at(otype)) &&*/
               !this->tid->matches("/sys/#") && // TODO: REMOVE WHEN ALL TYPES HAVE BEEN WRITTEN USING QUERY TYPES
               !this->tid->matches("/io/#")) {
              if(const Obj_p type_obj = ROUTER_READ(*this->tid);
                 !type_obj->is_noobj() && type_obj->otype == this->otype) {
                Obj_p plain_type_obj;
                if(type_obj->otype == OType::REC) {
                  const auto r = std::make_shared<RecMap<>>(*type_obj->value<RecMap_p<>>());
                  if(r->count(Obj::to_uri(COMPONENT_SEPARATOR)))
                    r->erase(Obj::to_uri(COMPONENT_SEPARATOR));
                  plain_type_obj = make_shared<Obj>(r, OType::REC, REC_FURI);
                } else {
                  plain_type_obj = make_shared<Obj>(type_obj->value_, type_obj->otype, OTYPE_FURI.at(type_obj->otype));
                }
                const Obj_p plain_obj = make_shared<Obj>(this->value_, this->otype, OTYPE_FURI.at(this->otype));
                // TODO: localize code for uniquness with no_query on inst arg keys
                if(plain_obj->is_rec() && plain_type_obj->is_rec()) {
                  for(const auto &[k1, v1]: *plain_type_obj->rec_value()) {
                    for(const auto &[k2, v2]: *plain_obj->rec_value()) {
                      if(k1->is_uri() && k2->is_uri() &&
                         k1->uri_value().no_query().equals(k2->uri_value().no_query())) {
                        if(k1->uri_value().has_query() && !k2->uri_value().has_query()) {
                          const_cast<Obj *>(k2.get())->value_ = k1->uri_value();
                        } else if(!k1->uri_value().has_query() && k2->uri_value().has_query()) {
                          const_cast<Obj *>(k1.get())->value_ = k2->uri_value();
                        }
                      }
                    }
                  }
                }
                const Obj_p applied_obj = plain_type_obj->apply(plain_obj);
                if(plain_obj->is_rec() && applied_obj->is_rec()) {
                  for(const auto &[k, v]: *plain_obj->rec_value()) {
                    if(!applied_obj->rec_value()->count(k))
                      applied_obj->rec_set(k, v);
                  }
                }
                this->value_ = applied_obj->value_;
                this->otype = applied_obj->otype;
              }
            }
          }
        } catch(const fError &e) {
          LOG_WRITE(
              WARN, this,
              L("unable to build {} from poly type !b{}!!: {}\n", this->toString(), this->tid->toString(), e.what()));
        }
        Compiler().with_derivation_tree().type_check(this, *this->tid);
      }
    }

    static fError TYPE_ERROR(const Obj *obj, const char *function, [[maybe_unused]] const int line_number = __LINE__) {
      const size_t index = string(function).find("_value");
      const auto error = fError(FURI_WRAP " %s !yaccessed!! using !b%s!!", obj->vid_or_tid()->toString().c_str(),
                                obj->toString().c_str(),
                                index == string::npos ? function : string(function).replace(index, 6, "").c_str());
      return error;
    }

    //////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////
    [[nodiscard]] virtual ID_p vid_or_tid() const { return this->vid ? this->vid : this->tid; }

    static Obj_p load(const ID &vid) { return ROUTER_READ(vid); }

    static Obj_p load(const ID_p &vid) { return ROUTER_READ(*vid); }

    static Obj_p load(const Uri_p &vid_uri) { return ROUTER_READ(vid_uri->uri_value()); }

    [[nodiscard]] Obj_p load() const { return ROUTER_READ(*this->vid); }

    virtual void sync(const fURI &subset) const {
      if(this->vid) {
        if(this->is_rec() && !subset.equals("#") && !subset.empty()) {
          const fURI subset_furi = this->vid->extend(subset);
          const Obj_p fresh = ROUTER_READ(subset_furi);
          const Uri_p subset_uri = Obj::to_uri(subset_furi);
          if(fresh->is_noobj())
            this->rec_drop(subset_uri);
          else
            this->rec_set(subset_uri, fresh);
        } else {
          const Obj_p fresh = ROUTER_READ(*this->vid);
          if(this->otype != fresh->otype) {
            throw fError("%s synchronization yielded different base types: %s != %s", this->vid->toString().c_str(),
                         OTypes.to_chars(this->otype).c_str(), OTypes.to_chars(fresh->otype).c_str());
          }
          const_cast<Obj *>(this)->value_ = fresh->value_;
          const_cast<Obj *>(this)->tid = fresh->tid;
        }
      }
    }

    virtual void save() const { this->at(this->vid); }

    /* virtual void save(const fURI &subset) const {
       ROUTER_WRITE(this->vid->extend(subset), this->rec_get(subset), true);
     }*/

    /*virtual void load() {
      if(this->vid) {
        const Obj_p other = ROUTER_READ(*this->vid);
        if(this->otype != other->otype || !this->tid->equals(*other->tid))
          throw fError("type of obj structural encoding changed (try locking): %s %s", this->tid->toString().c_str(),
                       other->tid->toString().c_str());
        this->value_ = other->value_;
      }
    }*/

    /*virtual Obj_p load() const {
      if(this->vid) {
        const Obj_p other = ROUTER_READ(*this->vid);
        if(this->otype != other->otype || !this->tid->equals(*other->tid))
          throw fError("type of obj structural encoding changed (try locking): %s %s", this->tid->toString().c_str(),
                       other->tid->toString().c_str());
        return other;
      }
      return this->shared_from_this();
    }*/

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
      const fURI furi = this->value<fURI>();
      const auto value_furi = fURI(furi);
      return value_furi;
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

    template<typename T>
    [[nodiscard]] std::vector<T> lst_value(const Function<const Obj_p, T> transformer) const {
      if(!this->is_lst())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      std::vector<T> r;
      for(const Obj_p &o: *this->value<LstList_p>()) {
        r.push_back(transformer(o));
      }
      return r;
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

    void lst_remove(const Obj_p &obj) const {
      if(!this->is_lst())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      const LstList_p l = this->lst_value();
      l->erase(std::remove_if(l->begin(), l->end(), [obj](const Obj_p &o) { return obj->equals(*o); }), l->end());
    }

    [[nodiscard]] Obj_p deref(const Obj_p &key, const bool obj_on_fail = true) const {
      if(this->is_rec())
        return this->rec_get(key);
      if(this->is_lst())
        return this->lst_get(key);
      return obj_on_fail ? key : Obj::to_noobj();
    }

    [[nodiscard]] Int_p rec_size() const { return Obj::to_int(this->rec_value()->size()); }

    [[nodiscard]] Int_p lst_size() const { return Obj::to_int(this->lst_value()->size()); }

    [[nodiscard]] Obj_p lst_get(const string &index) const { return this->lst_get(Obj::to_uri(index)); }

    [[nodiscard]] Obj_p lst_get(const FOS_INT_TYPE &index) const { return this->lst_get(Obj::to_int(index)); }

    [[nodiscard]] Obj_p lst_get(const Obj_p &index) const {
      if(!this->is_lst())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      if(index->is_uri()) {
        const auto segment = string(index->uri_value().segment(0));
        const int i =
            StringHelper::is_integer(segment) ? std::stoi(segment) : (StringHelper::has_wildcards(segment) ? -1 : -100);
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
      if(nest && index->is_uri() && index->uri_value().path_length() > 1 &&
         StringHelper::is_integer(index->uri_value().segment(0))) {
        const size_t current_index = std::strtol(index->uri_value().segment(0), nullptr, 10);
        Obj_p current_obj = this->lst_get(current_index);
        if(current_obj->is_noobj()) {
          if(const size_t offset = ((current_index + 1) - this->lst_value()->size()); offset > 0) {
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
        const int current_index = index->is_int()   ? index->int_value()
                                  : index->is_uri() ? std::stoi(index->uri_value().toString())
                                                    : -1;
        if(-1 == current_index)
          throw fError("invalid lst index: %s\n", index->toString().c_str());
        if(const size_t offset = (current_index + 1) - this->lst_value()->size(); offset > 0) {
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
          Compiler().type_check(this, *this->tid);
        } catch(const fError &) {
          this->lst_set(index, undo);
          LOG_WRITE(WARN, this,
                    L("!blst!! entry write reverted: !g[!!{} !m=>!! {}!g]!!\n", index->toString(), undo->toString()));
          throw;
        }
      }
      ////////////////////////////////////////
    }

    /*[[nodiscard]] Uri_p uri_set(const Int_p &index, const Obj_p &val) {
      fURI furi = this->uri_value();

    }*/

    [[nodiscard]] RecMap_p<> rec_value() const {
      if(!this->is_rec())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->value<RecMap_p<>>();
    }

    [[nodiscard]] Rec_p embedding() const {
      if(!this->vid)
        return this->is_rec() ? this->shared_from_this() : Obj::to_rec();
      const Rec_p &sct = ROUTER_READ(this->vid->as_branch());
      return this->is_rec() ? this->rec_merge(sct->rec_value()) : sct;
    }

    [[nodiscard]] Obj_p arg(const size_t index) const {
      if(this->is_inst())
        return this->inst_args()->arg(index);
      size_t counter = 0;
      for(const auto &[k, v]: *this->rec_value()) {
        if(index == counter)
          return v;
        counter++;
      }
      return Obj::to_noobj();
    }

    [[nodiscard]] Obj_p arg(const Obj_p &key) const {
      if(this->is_inst())
        return this->inst_args()->arg(key);
      return this->rec_value()->count(key) ? this->rec_value()->at(key) : Obj::to_noobj();
    }

    [[nodiscard]] Obj_p arg(const string &key) const {
      if(this->is_inst())
        return this->inst_args()->arg(key);
      const Uri_p uri_key = Obj::to_uri(key);
      return this->rec_get(uri_key);
    }

    [[nodiscard]] bool is_inst_stub() const { return this->is_noobj() || (this->is_inst() && !this->has_inst_f()); }

    [[nodiscard]] Obj_p rec_get(const Obj_p &key, const Obj_p &or_else = nullptr) const {
      if(!this->is_rec())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      Obj_p result;
      if(key->is_uri()) {
        const fURI key_no_query = key->uri_value().no_query();
        const string segment =
            0 == key_no_query.path_length() ? key_no_query.toString() : string(key_no_query.segment(0));
        Objs_p segment_value = Obj::to_objs();
        const Uri_p segment_uri = Obj::to_uri(segment);
        const bool match_all = StringHelper::has_wildcards(segment);
        const Objs_p full_match = Obj::to_objs();
        for(const auto &[k, v]: *this->rec_value()) {
          const fURI_p k_furi = k->is_uri() ? furi_p(k->uri_value()) : nullptr;
          if(match_all || k->match(segment_uri)) {
            segment_value->add_obj(v);
          } else if(k_furi && k_furi->matches(key_no_query)) {
            full_match->add_obj(v);
          }
        }
        segment_value = segment_value->none_one_all();
        Obj_p temp;
        if(key_no_query.path_length() <= 1)
          temp = segment_value;
        else {
          const auto sub = string(key_no_query.subpath(1));
          const Uri_p sub_uri = Obj::to_uri(sub);
          temp = segment_value->deref(sub_uri, false);
        }
        full_match->add_obj(temp);
        result = full_match->none_one_all();
      } else {
        const Objs_p segment_value = Obj::to_objs();
        for(const auto &[k, v]: *this->rec_value()) {
          if(k->match(key)) {
            const Obj_p to_add = v->apply(key);
            segment_value->add_obj(to_add);
          }
        }
        result = segment_value->none_one_all();
      }
      return result->is_noobj() && or_else ? or_else : result;
    }

    [[nodiscard]] Rec_p rec_no_query() const {
      if(!this->is_rec())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      const Rec_p new_rec = Obj::to_rec();
      for(const auto &[k, v]: *this->rec_value()) {
        if(k->is_uri() && k->uri_value().has_query())
          new_rec->insert_into_position(Obj::to_uri(k->uri_value().no_query()), v);
        else
          new_rec->insert_into_position(k, v);
      }
      return new_rec;
    }

    [[nodiscard]] Obj_p rec_get(const fURI &key, const Obj_p &or_else = nullptr) const {
      const Uri_p uri_key = to_uri(key);
      const Obj_p ret = rec_get(uri_key, or_else);
      return ret;
    }

    [[nodiscard]] Obj_p rec_get(const char *key, const Obj_p &or_else = nullptr) const {
      const auto string_key = string(key);
      const Uri_p uri_key = to_uri(string_key);
      const Obj_p ret = rec_get(uri_key, or_else);
      return ret;
    }

    [[nodiscard]] Obj_p obj_get(const fURI &key) const {
      if(this->is_poly()) {
        if(this->vid) {
          const Obj_p value = ROUTER_READ(this->vid->extend(key));
          if(value->is_noobj())
            this->poly_drop(Obj::to_uri(key));
          else
            this->poly_set(Obj::to_uri(key), value);
          return value;
        }
        return this->poly_get(Obj::to_uri(key));
      }
      return this->vid ? ROUTER_READ(this->vid->extend(key)) : Obj::to_noobj();
    }

    void obj_set(const fURI &key, const Obj_p &value) const {
      if(this->is_poly())
        this->poly_set(Obj::to_uri(key), value);
      if(this->vid)
        ROUTER_WRITE(this->vid->extend(key), value, true);
    }

    void obj_set_component(const fURI &key, const Obj_p &value) const {
      this->obj_set(ID(COMPONENT_SEPARATOR).extend(key), value);
    }

    template<typename T>
    [[nodiscard]] T get(const fURI &key) const {
      try {
        const Obj_p v = this->poly_get(Obj::to_uri(key));
        if(v->is_noobj())
          throw fError("!b%s!! has no value for !b%s!!", key.toString().c_str());
        return std::any_cast<T>(v->value_);
      } catch(const std::bad_any_cast &e) {
        throw fError::create("wrong underlying type of %s in %s: %s", key.toString().c_str(), this->toString().c_str(),
                             e.what());
      }
    }

    [[nodiscard]] bool has(const fURI &key) const {
      return this->vid         ? !ROUTER_READ(this->vid->extend(key))->is_noobj()
             : this->is_poly() ? !this->poly_get(Obj::to_uri(key))->is_noobj()
                               : false;
    }

    [[nodiscard]] Rec_p rec_merge(const RecMap_p<> &rmap) const {
      if(!this->is_rec())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      for(const auto &[key, value]: *rmap) {
        this->insert_into_position(key, value);
      }
      return shared_from_this();
    }

    virtual void rec_set(const char *uri_key, const Obj_p &val, const bool nest = true) const {
      this->rec_set(Obj::to_uri(uri_key), val, nest);
    }

    template<typename A>
    static void drop_if(const RecMap_p<> &m, const Predicate<A> &pred) {
      for(auto it = m->begin(); it != m->end();) {
        if(pred(*it)) {
          it = m->erase(it);
        } else {
          ++it;
        }
      }
    }

    virtual void lst_drop(const Int_p &index) const {
      if(index->is_int()) {
        this->lst_value()->erase(this->lst_value()->begin() + index->int_value());
      } else if(index->is_uri() && StringHelper::is_integer(index->uri_value().toString())) {
        this->lst_value()->erase(this->lst_value()->begin() + atoi(index->uri_value().toString().c_str()));
      } else
        throw fError("unable to drop non-indexed key from lst: %s", index->toString().c_str());
    }

    virtual void rec_drop(const Obj_p &key) const {
      if(key->is_uri()) {
        drop_if<std::pair<const Obj_p, Obj_p>>(this->rec_value(), [&key](const std::pair<const Obj_p, Obj_p> &pair) {
          if(pair.first->is_uri() && pair.first->uri_value().no_query().matches(key->uri_value().no_query())) {
            return true;
          }
          return pair.first->match(key);
        });
      } else {
        this->rec_value()->erase(key);
      }
    }

    void insert_into_position(const Obj_p &key, const Obj_p &value) const {
      if(!this->is_rec())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      if(this->rec_value()->count(key))
        this->rec_value()->at(key) = value;
      else
        this->rec_value()->insert_or_assign(key, value);
    }

    virtual void rec_set(const Obj_p &key, const Obj_p &value, const bool nest = true) const {
      const Obj_p undo = this->rec_get(key);
      ////////////////////////////////////////
      if(key->is_uri() && key->uri_value().has_query()) {
        Compiler().with_derivation_tree().type_check(value, key->uri_value().query());
      }
      ///////////////////////////////////////////
      if(nest && key->is_uri() && key->uri_value().is_node() && key->uri_value().path_length() > 1) {
        // const fURI key_no_query = key->uri_value().no_query();
        const fURI current_key_furi = key->uri_value().segment(0);
        const Uri_p current_key = Obj::to_uri(current_key_furi);
        Obj_p current_obj = this->rec_get(current_key);
        const bool is_lst = StringHelper::is_integer(key->uri_value().segment(1));
        if(current_obj->is_noobj()) {
          current_obj = is_lst ? Obj::to_lst() : Obj::to_rec();
          this->insert_into_position(current_key, current_obj);
        }
        const fURI pretracted = key->uri_value().pretract();
        current_obj->poly_set(Obj::to_uri(pretracted), value);
      } else {
        // this->rec_drop(key);
        // if(!val->is_noobj())
        this->insert_into_position(key, value);
      }
      ////////////////////////////////////////
      if(!this->is_base_type()) {
        try {
          Compiler().type_check(this, *this->tid);
        } catch(const fError &) {
          this->rec_set(key, undo);
          LOG_WRITE(WARN, this,
                    L("!brec!! entry write reverted: !g[!!{} !m=>!! {}!g]!!\n", key->toString(), undo->toString()));
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
      if(this->vid)
        ROUTER_WRITE(*this->vid, Obj::to_rec(make_shared<RecMap<>>(*this->rec_value()), id_p(*this->tid)), true);
    }

    void rec_delete(const Obj &key) const { Obj::rec_set(make_shared<Obj>(key), Obj::to_noobj()); }

    void poly_drop(const Obj_p &key) const {
      if(!this->is_poly() && !this->is_objs())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      if(this->is_rec())
        this->rec_drop(key);
      else if(this->is_lst())
        this->lst_drop(key);
      else if(this->is_objs()) {
        for(const auto &o: *this->objs_value()) {
          if(o->is_poly())
            o->poly_drop(key);
        }
      } else
        throw fError("unknown poly base type (logic error): %s", this->tid->toString().c_str());
    }

    void poly_set(const Obj_p &key, const Obj_p &value) const {
      if(!this->is_poly() && !this->is_objs())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      if(this->is_rec())
        this->rec_set(key, value);
      else if(this->is_lst())
        this->lst_set(key, value);
      else if(this->is_objs()) {
        for(const auto &o: *this->objs_value()) {
          if(o->is_poly())
            o->poly_set(key, value);
        }
      } else
        throw fError("unknown poly base type (logic error): %s", this->tid->toString().c_str());
    }

    Obj_p poly_get(const Obj_p &key) const {
      if(!this->is_poly() && !this->is_objs())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      if(this->is_rec())
        return this->rec_get(key);
      if(this->is_lst())
        return this->lst_get(key);
      if(this->is_objs()) {
        const Obj_p os = Objs::to_objs();
        for(const auto &o: *this->objs_value()) {
          if(o->is_poly())
            os->add_obj(o->poly_get(key));
        }
        return os;
      }
      throw fError("unknown poly base type (logic error): %s", this->tid->toString().c_str());
    }

    [[nodiscard]] InstValue inst_value() const {
      if(!this->is_inst())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->value<InstValue>();
    }

    [[nodiscard]] string inst_op() const {
      if(!this->is_inst())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->tid->name();
    }

    [[nodiscard]] InstArgs inst_args() const {
      if(this->is_inst())
        return std::get<0>(this->inst_value());
      if(!this->is_rec())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->shared_from_this();
    }

    [[nodiscard]] bool has_arg(const Obj_p &key) const {
      return !(this->is_inst() ? this->inst_args().get() : this)->rec_get(key)->is_noobj();
    }

    [[nodiscard]] bool has_arg(const fURI &furi_key) const { return this->has_arg(Obj::to_uri(furi_key)); }

    [[nodiscard]] InstF inst_f() const {
      if(!this->is_inst())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return std::get<1>(this->inst_value());
    }

    [[nodiscard]] Obj_p inst_seed_supplier() const {
      if(!this->is_inst())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return std::get<2>(this->inst_value());
    }

    [[nodiscard]] Obj_p inst_seed(const Obj_p &arg) const { return this->inst_seed_supplier()->apply(arg); }

    [[nodiscard]] Pair<Obj_p, Inst_p> error_value() const {
      if(!this->is_error())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->value<Pair<Obj_p, Inst_p>>();
    }

    [[nodiscard]] DomainRange domain_range() const { return DomainRange::from(*this->tid); }

    [[nodiscard]] ID_p domain() const {
      if(this->tid->has_query(FOS_DOMAIN)) {
        const string rs = this->tid->query_value(FOS_DOMAIN).value();
        const fURI rf = ROUTER_RESOLVE(rs);
        const ID_p ri = id_p(rf);
        return ri;
      }
      if(this->is_bcode() && !this->bcode_value()->empty())
        return this->bcode_value()->front()->domain();
      if(this->is_inst() && std::holds_alternative<Obj_p>(this->inst_f()))
        return std::get<Obj_p>(this->inst_f())->domain();
      return OBJ_FURI;
    }


    [[nodiscard]] IntCoefficient domain_coefficient() const {
      const std::vector<string> coefs = this->tid->query_values(FOS_DOM_COEF);
      return coefs.empty() ? IntCoefficient{1, 1} : IntCoefficient{stoi(coefs.at(0)), stoi(coefs.at(1))};
    }

    [[nodiscard]] IntCoefficient range_coefficient() const {
      const std::vector<string> coefs = this->tid->query_values(FOS_RNG_COEF);
      return coefs.empty() ? IntCoefficient{1, 1} : IntCoefficient{stoi(coefs.at(0)), stoi(coefs.at(1))};
    }

    [[nodiscard]] ID_p range() const {
      if(this->tid->has_query(FOS_RANGE)) {
        const string rs = this->tid->query_value(FOS_RANGE).value();
        const fURI rf = ROUTER_RESOLVE(rs);
        const ID_p ri = id_p(rf);
        return ri;
      }
      if(this->is_not_empty_bcode())
        return this->bcode_value()->back()->range();
      if(this->is_inst())
        return std::holds_alternative<Obj_p>(this->inst_f()) ? std::get<Obj_p>(this->inst_f())->range() : OBJ_FURI;
      return id_p(this->tid->no_query());
    }

    /*[[nodiscard]] bool CHECK_OBJ_TO_INST_SIGNATURE(const Inst_p &resolved, const bool domain_or_range,
                                                   const bool throw_exception = true) const {
      this->resolve();
      resolved->resolve();
      if(domain_or_range) {
        if(resolved->is_generative()) {
          // do nothing
        } else if(this->is_noobj()) {
          if(!resolved->is_initial()) {
            if(!throw_exception)
              return false;
            throw fError("%s [%s] not in domain of %s [!y%s!!]", this->toString().c_str(),
                         Obj::to_type(NOOBJ_FURI)->toString().c_str(), resolved->toString().c_str(), "SIGNATURE HERE");
          }
        } else if(this->is_objs()) {
          if(!resolved->is_gather()) {
            if(!throw_exception)
              return false;
            throw fError("%s [%s] not in domain of %s [!y%s!!]", this->toString().c_str(),
                         Obj::to_type(OBJS_FURI)->toString().c_str(), resolved->toString().c_str(), "SIGNATURE HERE");
          }
        } else {
          if(const auto &[a, b] = resolved->domain_coefficient(); !(a == 1 && b == 1)) {
            if(!throw_exception)
              return false;
            throw fError("%s [%s] not in domain of %s [!y%s!!]", this->toString().c_str(),
                         Obj::to_type(OBJ_FURI)->toString().c_str(), resolved->toString().c_str(), "SIGNATURE HERE");
          }
        }
      } else {
        if(const auto &[a, b] = resolved->range_coefficient(); (a == 0 && b > 0)) {
          // do nothing
        } else if(this->is_noobj()) {
          if(!resolved->is_terminal()) {
            if(!throw_exception)
              return false;
            throw fError("%s [%s] not in range of %s [!y%s!!]", this->toString().c_str(),
                         Obj::to_type(NOOBJ_FURI)->toString().c_str(), resolved->toString().c_str(), "SIGNATURE HERE");
          }
        } else if(this->is_objs()) {
          if(!resolved->is_scatter()) {
            if(!throw_exception)
              return false;
            throw fError("%s [%s] not in range of %s [!y%s!!]", this->toString().c_str(),
                         Obj::to_type(OBJS_FURI)->toString().c_str(), resolved->toString().c_str(), "SIGNATURE HERE");
          }
        } else {
          if(const auto &[a, b] = resolved->range_coefficient(); a != 1 && b != 1) {
            if(!throw_exception)
              return false;
            throw fError("%s [%s] not in range of %s [!y%s!!]", this->toString().c_str(),
                         Obj::to_type(OBJ_FURI)->toString().c_str(), resolved->toString().c_str(), "SIGNATURE HERE");
          }
        }
      }
      return true;
    }*/


    [[nodiscard]] Obj_p type() const { return ROUTER_READ(*this->tid); }

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
      return Obj::to_bcode(new_code, this->tid);
      // return Obj::create(new_code, OType::BCODE, this->tid, this->vid);
    }

    ////////////////////////////////////////////////////////////
    Obj_p this_add(const ID &relative_id, const Obj_p &inst, const bool at_type = true) const {
      if(!at_type && !this->vid)
        throw fError("only objs with a value id can have properties and insts");
      // if(inst->is_code())
      ROUTER_WRITE(this->vid->append(relative_id), inst, true);
      return this->shared_from_this();
    }

    Obj_p this_add_inst(const ID &relative_id, const Obj_p &inst, const bool at_type = true) const {
      if(!at_type && !this->vid)
        throw fError("only objs with a value id can have properties and insts");
      // if(inst->is_code())
      ROUTER_WRITE(this->vid->add_component(relative_id), inst, true);
      return this->shared_from_this();
    }

    Obj_p this_get(const char *key) const {
      // TODO: if not, vid, then tid, then tid -> tid, then tid -> tid -> tid;
      Obj_p result = ROUTER_READ(this->vid->extend(key));
      return result;
    }

    Obj_p this_get(const fURI &furi) const {
      // TODO: if not, vid, then tid, then tid -> tid, then tid -> tid -> tid;
      Obj_p result = ROUTER_READ(this->vid->extend(furi));
      return result;
    }

    Obj_p this_set(const char *key, const Obj_p &obj) {
      ROUTER_WRITE(this->vid->extend(key), obj, true);
      return this->shared_from_this();
    }

    Obj_p static_get(const char *key) const { return ROUTER_READ(this->tid->extend(key)); }

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
      ROUTER_WRITE(this->vid_or_tid()->query("doc"), Obj::to_str(documentation), true);
      return this->shared_from_this();
    }

    [[nodiscard]] BCode_p add_bcode(const BCode_p &bcode, [[maybe_unused]] const bool mutate = true) const {
      if(!this->is_bcode() || !bcode->is_code())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      const InstList_p insts = make_shared<InstList>();
      if(this->is_bcode()) {
        for(const auto &inst: *this->bcode_value()) {
          insts->push_back(inst);
        }
      } else if(this->is_inst()) {
        insts->push_back(this->clone());
      }
      if(bcode->is_bcode()) {
        for(const auto &inst: *bcode->bcode_value()) {
          insts->push_back(inst);
        }
      } else if(bcode->is_inst()) {
        insts->push_back(bcode);
      }
      return Obj::to_bcode(insts);
    }

    [[nodiscard]] Obj_p inst_bcode_obj() const {
      return this->is_bcode() && 1 == this->bcode_value()->size() ? this->bcode_value()->front()
                                                                  : this->shared_from_this();
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
      return this->is_objs() ? (this->objs_value()->empty() ? Obj::to_noobj() : this->objs_value()->front())
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

    [[nodiscard]] Obj_p inst_apply(const ID &inst_id, const List<Obj_p> &args = {}) const {
      return Obj::to_inst(Obj::to_inst_args(args), id_p(inst_id))->apply(this->shared_from_this());
    }

    // TODO: make obj.cpp/hpp and then reference PrinterHelper for printing
    [[nodiscard]] string toString(const ObjPrinter *obj_printer = nullptr) const {
      return std::move(PRINT_OBJ(this->shared_from_this(), obj_printer));
    }

    [[nodiscard]] int compare(const Obj &rhs) const { return this->toString().compare(rhs.toString()); }

    bool operator>(const Obj &rhs) const {
      switch(this->otype) {
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
          throw fError("%s is not relational (>)", OTypes.to_chars(this->otype).c_str());
      }
    }

    bool operator<(const Obj &rhs) const {
      switch(this->otype) {
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
          throw fError("%s is not relational (<)", OTypes.to_chars(this->otype).c_str());
      }
    }

    bool operator<=(const Obj &rhs) const { return this->value_equals(rhs) || *this < rhs; }

    bool operator>=(const Obj &rhs) const { return this->value_equals(rhs) || *this > rhs; }

    Obj operator/(const Obj &rhs) const {
      switch(this->otype) {
        case OType::NOOBJ:
          return *to_noobj();
        case OType::INT:
          return Obj(this->int_value() / rhs.int_value(), OType::INT, this->tid, this->vid);
        case OType::REAL:
          return Obj(this->real_value() / rhs.real_value(), OType::REAL, this->tid, this->vid);
        default:
          throw fError("%s can not be divided (/)", OTypes.to_chars(this->otype).c_str());
      }
    }

    Obj operator-(const Obj &rhs) const {
      switch(this->otype) {
        case OType::NOOBJ:
          return *Obj::to_noobj();
        case OType::BOOL:
          return Bool(!this->bool_value(), OType::BOOL, this->tid, this->vid);
        case OType::INT:
          return Int(this->int_value() - rhs.int_value(), OType::INT, this->tid, this->vid);
        case OType::REAL:
          return Real(this->real_value() - rhs.real_value(), OType::REAL, this->tid, this->vid);
        case OType::URI:
          return Uri(this->uri_value().retract(), OType::URI, this->tid, this->vid);
        // case OType::STR:
        //  return Obj(string(this->str_value()).replace(string(rhs.str_value()), this->vid);
        case OType::LST: {
          auto list = std::make_shared<LstList>();
          for(const auto &obj: *this->lst_value()) {
            if(std::find(rhs.lst_value()->begin(), rhs.lst_value()->end(), obj) != std::end(*rhs.lst_value()))
              list->push_back(obj);
          }
          return Lst(list, OType::LST, this->tid, this->vid);
        }
        case OType::REC: {
          auto map = std::make_shared<RecMap<>>();
          for(const auto &pair: *this->rec_value()) {
            map->insert(pair);
          }
          for(const auto &pair: *rhs.rec_value()) {
            map->insert(pair);
          }
          return Rec(map, OType::REC, this->tid, this->vid);
        }
        default:
          throw fError("%s can not be subtracted (-)", OTypes.to_chars(this->otype).c_str());
      }
    }

    [[nodiscard]] bool value_equals(const Obj &other) const {
      if(this->otype != other.otype)
        return false;
      switch(this->otype) {
        case OType::NOOBJ:
          return true;
        case OType::OBJ:
          return !other.value_.has_value();
        case OType::TYPE:
          return this->type_value()->equals(*other.type_value());
        case OType::BOOL:
          return this->bool_value() == other.bool_value();
        case OType::INT:
          return this->int_value() == other.int_value();
        case OType::REAL:
          return this->real_value() == other.real_value();
        case OType::STR:
          return this->str_value() == other.str_value();
        case OType::URI:
          return this->uri_value() == other.uri_value();
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
            for(const auto &[b1, b2]: *b) {
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
        case OType::INST:
          return *this->inst_args() == *other.inst_args() && this->has_inst_f() == other.has_inst_f() &&
                 this->inst_f().index() == other.inst_f().index() &&
                 (this->inst_f().index() == 1 ||
                  (*std::get<Obj_p>(this->inst_f()) == *std::get<Obj_p>(other.inst_f()))) &&
                 this->domain_coefficient() == other.domain_coefficient() &&
                 this->range_coefficient() == other.range_coefficient() &&
                 *this->inst_seed_supplier() == *other.inst_seed_supplier();
        // TODO: Tuple equality
        default:
          throw fError("unknown obj type in ==: %s", OTypes.to_chars(this->otype).c_str());
      }
    }

    [[nodiscard]] bool equals(const Obj &other) const {
      this->resolve();
      other.resolve();
      return this->otype == other.otype && this->tid->no_query().equals(other.tid->no_query()) &&
             this->value_equals(other);
    }


    bool operator!=(const Obj &other) const { return !this->equals(other); }

    bool operator==(const Obj &other) const { return this->equals(other); }

    Obj &operator=(const Obj &other) {
      if(&other == this)
        return *this;
      this->tid = other.tid;
      this->vid = other.vid;
      this->otype = other.otype;
      switch(this->otype) {
        case OType::BOOL:
          this->value_ = std::any(std::any_cast<bool>(other.value_));
        case OType::INT:
          this->value_ = std::any(std::any_cast<FOS_INT_TYPE>(other.value_));
        case OType::REAL:
          this->value_ = std::any(std::any_cast<FOS_REAL_TYPE>(other.value_));
        case OType::STR:
          this->value_ = std::any(std::string(std::any_cast<std::string>(other.value_)));
        case OType::URI:
          this->value_ = std::any(fURI(std::any_cast<fURI>(other.value_)));
        case OType::OBJS:
        case OType::LST:
          this->value_ = std::any(std::any_cast<List_p<Obj_p>>(other.value_));
        case OType::REC:
          this->value_ = std::any(std::any_cast<RecMap_p<>>(other.value_));
        case OType::BCODE:
          this->value_ = std::any(std::any_cast<InstList_p>(other.value_));
        default:
          throw fError("unknown type during assignment operation: %s", OTypes.to_chars(this->otype));
      }
      return *this;
    }

    //  [[nodiscard]] Obj_p operator[](const char *key) const { return this->rec_get(key); }

    [[nodiscard]] bool is_type() const { return this->otype == OType::TYPE; }

    [[nodiscard]] bool is_noobj() const { return this->otype == OType::NOOBJ; }

    [[nodiscard]] bool is_bool() const { return this->otype == OType::BOOL; }

    [[nodiscard]] bool is_int() const { return this->otype == OType::INT; }

    [[nodiscard]] bool is_real() const { return this->otype == OType::REAL; }

    [[nodiscard]] bool is_uri() const { return this->otype == OType::URI; }

    [[nodiscard]] bool is_str() const { return this->otype == OType::STR; }

    [[nodiscard]] bool is_lst() const { return this->otype == OType::LST; }

    [[nodiscard]] bool is_poly() const {
      return this->is_lst() || this->is_rec(); // || this->is_objs() /*|| this->is_bcode() || this->is_inst()
    }

    [[nodiscard]] bool is_multi() const {
      return this->is_poly() || this->is_objs() || this->is_bcode() || this->is_inst();
    }

    [[nodiscard]] bool is_rec() const { return this->otype == OType::REC; }

    [[nodiscard]] bool is_inst() const { return this->otype == OType::INST; }

    [[nodiscard]] bool has_inst_f() const {
      return std::holds_alternative<Cpp_p>(this->inst_f())
                 ? std::get<Cpp_p>(this->inst_f()) != nullptr
                 : std::get<Obj_p>(this->inst_f()) != nullptr && !std::get<Obj_p>(this->inst_f())->is_noobj();
    }

    [[nodiscard]] bool is_applicable_inst() const { return this->is_inst() && this->has_inst_f(); }

    [[nodiscard]] bool is_objs() const { return this->otype == OType::OBJS; }

    [[nodiscard]] bool is_bcode() const { return this->otype == OType::BCODE; }

    [[nodiscard]] bool is_code() const { return this->is_bcode() || this->is_inst(); }

    [[nodiscard]] bool is_error() const { return this->otype == OType::ERROR; }

    [[nodiscard]] bool is_empty_bcode() const { return this->is_bcode() && this->bcode_value()->empty(); }

    [[nodiscard]] bool is_not_empty_bcode() const { return this->is_bcode() && !this->bcode_value()->empty(); }

    bool is_initial() const {
      const auto &[dmin, dmax] = this->domain_coefficient();
      return dmin == 0 && dmax == 0;
    }

    bool is_generative() const {
      const auto &[dmin, dmax] = this->domain_coefficient();
      return dmin == 0 && dmax > 0;
    }

    bool is_scatter() const {
      const auto &[rmin, rmax] = this->range_coefficient();
      return rmin == 0 && rmax > 1;
    };


    bool is_gather() const {
      const auto &[dmin, dmax] = this->domain_coefficient();
      return dmin == 0 && dmax > 1;
    }

    bool is_terminal() const {
      const auto &[rmin, rmax] = this->range_coefficient();
      return rmin == 0 && rmax == 0;
    }

    bool has_domain(const FOS_INT_TYPE min, const FOS_INT_TYPE max) const {
      return this->domain_coefficient().first == min && this->domain_coefficient().second == max;
    }

    bool has_range(const FOS_INT_TYPE min, const FOS_INT_TYPE max) const {
      return this->range_coefficient().first == min && this->range_coefficient().second == max;
    }

    bool is_map() const {
      const auto &[dmin, dmax] = this->domain_coefficient();
      const auto &[rmin, rmax] = this->range_coefficient();
      return 1 == dmin && 1 == dmax && 1 == rmin && 1 == rmax;
    }

    bool is_filter() const {
      const auto &[rmin, rmax] = this->range_coefficient();
      return !this->is_initial() && rmin == 0 && rmax > 0;
    }

    bool is_indexed_arg() const {
      if(!this->is_uri())
        return false;
      return StringHelper::is_integer(this->uri_value().toString());
    }

    bool is_indexed_args() const {
      if(this->is_inst())
        return this->inst_args()->is_indexed_args();
      if(!this->is_rec())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      int counter = 0;
      const auto r = *this->rec_value();
      for(const auto &[k, v]: r) {
        if(!(k->is_uri() && k->uri_value().toString() == to_string(counter++)))
          return false;
      }
      return true;
    }

    static bool has_code_block(const string &obj_string) {
      return obj_string.find("{{") != string::npos && obj_string.find("}}") != string::npos;
    }

    static string process_code_block(const Obj_p &lhs, string &to_format) {
      const string formatted = StringHelper::replace_groups(&to_format, "{{", "}}", [lhs](const string &match) {
        const Obj_p result = OBJ_PARSER(match)->apply(lhs)->none_one_all();
        if(result->is_str())
          return result->str_value();
        if(result->is_uri())
          return result->uri_value().toString();
        if(result->is_noobj())
          return string();
        return result->toString(NO_ANSI_PRINTER);
      });
      return formatted;
    }

    /////////////////////////////////////////////////////////////////
    //////////////////////////// APPLY //////////////////////////////
    /////////////////////////////////////////////////////////////////
    void resolve(/*const Obj_p &lhs = nullptr*/) const {
      const auto f = fURI(*this->tid);
      ID r = ROUTER_RESOLVE(f);
      /*if(lhs && f.equals(r)) {
        if(!ROUTER_READ(lhs->tid->extend(r))->is_noobj())
          r = lhs->tid->extend(r);
        else if(!ROUTER_READ(lhs->tid->add_component(r))->is_noobj())
          r = lhs->tid->add_component(r);
      }*/
      if(!f.equals(r))
        const_cast<Obj *>(this)->tid = id_p(r);
      ////////////////////////////////////////////////////////////////
      switch(this->otype) {
        case OType::REC: {
          for(const auto &[k, v]: *this->rec_value()) {
            k->resolve();
            v->resolve();
          }
          break;
        }
        case OType::LST: {
          for(const auto &e: *this->lst_value()) {
            e->resolve();
          }
          break;
        }
        case OType::BCODE: {
          for(const auto &e: *this->bcode_value()) {
            e->resolve();
          }
          break;
        }
        case OType::INST: {
          for(const auto &[k, v]: *std::get<0>(this->inst_value())->rec_value()) {
            k->resolve();
            v->resolve();
          }
          // if(std::holds_alternative<Obj_p>(this->inst_f()))
          //   std::get<Obj_p>(this->inst_f())->resolve();
          break;
        }
        case OType::OBJS: {
          for(const auto &e: *this->objs_value()) {
            e->resolve();
          }
          break;
        }
        default: {
          // do nothing
        }
      }
    }

    virtual void setup() {}

    template<typename T>
    T *get_model(const bool throw_on_miss = true) const {
      static Function<Obj_p, ptr<void>> GET_OR_CREATE_MODEL = [throw_on_miss](const Obj_p &model_obj) {
        if(model_obj->vid && MODEL_MAP->count(*model_obj->vid)) {
          return MODEL_MAP->at(*model_obj->vid);
        }
        if(MODEL_MAP->count(*model_obj->tid))
          return MODEL_MAP->at(*model_obj->tid);
        if(model_obj->vid && MODEL_CREATOR2->count(*model_obj->vid)) {
          const ptr<void> model_any = MODEL_CREATOR2->at(*model_obj->vid)(model_obj);
          MODEL_MAP->insert_or_assign(*model_obj->vid, model_any);
          return model_any;
        }
        if(MODEL_CREATOR2->count(*model_obj->tid)) {
          const ptr<void> model_any = MODEL_CREATOR2->at(*model_obj->tid)(model_obj);
          if(model_obj->vid) {
            MODEL_MAP->insert_or_assign(*model_obj->vid, model_any);
            return MODEL_MAP->at(*model_obj->vid);
          } else
            return model_any;
        }
        if(throw_on_miss)
          throw fError("no model for %s", model_obj->tid->toString().c_str());
        return ptr<void>(nullptr);
      };
      const ptr<void> ppp = GET_OR_CREATE_MODEL(this->shared_from_this());
      if(nullptr == ppp.get())
        return nullptr;
      T *t = static_cast<T *>(ppp.get());
      return t;
    }

    void delete_model() const {
      if(MODEL_MAP->count(*this->vid)) {
        MODEL_MAP->erase(*this->vid);
      }
    }

    [[nodiscard]] std::vector<Obj_p> values() const {
      if(this->is_objs())
        return *this->objs_value();
      else {
        const std::vector<Obj_p> ret = {this->shared_from_this()};
        return ret;
      }
    }

    [[nodiscard]] Obj_p apply(const Obj_p &lhs) const {
      switch(this->otype) {
        case OType::BOOL:
        case OType::INT:
        case OType::REAL:
        case OType::NOOBJ:
        case OType::ERROR:
          return this->shared_from_this();
        case OType::URI: {
          auto s = string(this->uri_value().toString());
          const Uri_p uri = has_code_block(s) ? Obj::to_uri(process_code_block(lhs, s), this->tid, this->vid)
                                              : this->shared_from_this();
          return lhs->deref(uri, true);
        }
        case OType::STR: {
          auto s = string(this->str_value());
          return has_code_block(s) ? Obj::to_str(process_code_block(lhs, s), this->tid, this->vid)
                                   : this->shared_from_this();
        }
        case OType::TYPE: {
          Obj_p new_value = this->type_value()->apply(lhs);
          return new_value; // Obj::create(new_value->value_, new_value->otype, this->tid, new_value->vid);
        }
        case OType::LST: {
          const auto new_values = make_shared<LstList>();
          int counter = 0;
          for(const auto &obj: *this->lst_value()) {
            new_values->emplace_back(obj->apply(lhs->is_poly() ? lhs->poly_get(Obj::to_int(counter++)) : lhs));
          }
          return Obj::to_lst(new_values, this->tid, this->vid);
        }
        case OType::REC: {
          const auto new_pairs = make_shared<RecMap<>>();
          for(const auto &[key, value]: *this->rec_value()) {
            const Obj_p key_apply = key->apply(lhs);
            new_pairs->insert_or_assign(key, value->apply(lhs->is_poly() ? key_apply : lhs));
          }
          return Obj::to_rec(new_pairs, this->tid, this->vid);
        }
        case OType::INST: {
          if(lhs->is_type()) {
            const Inst_p inst = Compiler().resolve_inst(lhs, this->shared_from_this());
            BCode_p body = lhs->type_value()->clone();
            body->add_inst(inst);
            return Obj::create(body, OType::TYPE, inst->range(), lhs->vid);
          }
          const Inst_p inst = Compiler().resolve_inst(lhs, this->shared_from_this());
          // TODO: type check should take coefficients into consideration
          //////////////////////////////////// DOMAIN TYPE CHECK ////////////////////////////////////
          if(!lhs->is_noobj() || !inst->range_coefficient().first == 0)
            Compiler(true).with_derivation_tree().type_check(lhs, *inst->domain());
          Obj_p result = ROUTER_EXEC_WITHIN_FRAME("#", inst->inst_args(), [this, inst, lhs]() {
            try {
              return std::holds_alternative<Obj_p>(inst->inst_f())
                         ? std::get<Obj_p>(inst->inst_f())->apply(lhs)
                         : (*std::get<Cpp_p>(inst->inst_f()))(lhs, inst->inst_args()->clone());
            } catch(const fError &e) {
              const string error_message =
                  fmt::format("{}\n\t  !rthrown at !yinst!! {} !g=>!! {} {}", e.what(), lhs->toString(),
                              this->toString(), ROUTER_GET_FRAME_DATA()->toString());
              throw fError("%s", error_message.c_str());
            }
          });
          //////////////////////////////////// RANGE TYPE CHECK ////////////////////////////////////
          if(!result->is_noobj() || !inst->range_coefficient().first == 0)
            Compiler(true).with_derivation_tree().type_check(result, *inst->range());
          return result;
        }
        case OType::BCODE: {
          if(this->is_empty_bcode())
            return lhs;
          if(!lhs->is_objs() && 1 == this->bcode_value()->size()) // TODO: this is because barriers
            return this->bcode_value()->front()->apply(lhs);
          return BCODE_PROCESSOR(this->bcode_starts(lhs->is_noobj() || lhs->is_objs() ? lhs : to_objs({lhs})))
              ->none_one_all();
        }
        case OType::OBJS: {
          Objs_p objs = Obj::to_objs();
          for(const Obj_p &obj: *this->objs_value()) {
            objs->add_obj(obj->apply(lhs));
          }
          return objs;
        }
        default:
          throw fError("unknown obj type in apply(): %s", OTypes.to_chars(this->otype).c_str());
      }
    }

    [[nodiscard]]
    bool is_base_type() const {
      return this->tid->equals(*OTYPE_FURI.at(this->otype));
    }

    [[nodiscard]] bool match(const Obj_p &type_obj, std::stack<string> *fail_reason = nullptr) const {
      // LOG(TRACE, "!ymatching!!: %s ~ %s\n", this->toString().c_str(), type->toString().c_str());
      if(type_obj->is_empty_bcode())
        return true;
      if(type_obj->is_type()) {
        if(this->is_code() && !type_obj->is_code())
          return true; /// TODO: check range
        const bool result = OTYPE_FURI.at(this->otype)->equals(*type_obj->tid);
        if(!result && fail_reason) {
          fail_reason->push(
              fmt::format("!b{}!! is !rnot!! a subtype of !b{}!!", this->tid->toString(), type_obj->tid->toString()));
        }
        return result;
      }

      if(type_obj->is_code() && !this->is_code()) {
        try {
          const Obj_p result = type_obj->apply(this->clone());
          const bool r = result->is_noobj() && type_obj->range_coefficient().first == 0 ? true : !result->is_noobj();
          if(!r and fail_reason) {
            fail_reason->push(
                fmt::format("!b{}!! is not applicable with !b{}!!", this->toString(), type_obj->tid->toString()));
          }
          return r;
        } catch(std::exception &e) {
          if(fail_reason)
            fail_reason->push(e.what());
          return false;
        }
      }
      if(!this->is_code() || !type_obj->is_code()) { // bcode and inst are compared in switch below
        if(this->otype != type_obj->otype) {
          if(fail_reason)
            fail_reason->push(fmt::format("!b{}!! is !rnot!! the same base type as !b{}!!",
                                          OTypes.to_chars(this->otype), OTypes.to_chars(type_obj->otype)));
          return false;
        }
        if(!this->is_base_type() && !type_obj->is_base_type() && *this->tid != *type_obj->tid) {
          if(!ROUTER_RESOLVE(fURI(*this->tid)).equals(ROUTER_RESOLVE(fURI(*type_obj->tid)))) {
            if(fail_reason)
              fail_reason->push(fmt::format("!b{}!! is !rnot!! the same type as !b{}!!", this->tid->toString(),
                                            type_obj->tid->toString()));
            return false;
          }
        }
      }
      static constexpr char *does_not_equal = "{} does !rnot!! equal {}";
      switch(this->otype) {
        case OType::TYPE:
          return this->type_value()->match(type_obj->is_type() ? type_obj->type_value() : type_obj, fail_reason);
        case OType::NOOBJ: // everything matches noobj
          return true;
        case OType::BOOL: {
          const bool match = this->bool_value() == type_obj->bool_value();
          if(!match && fail_reason)
            fail_reason->push(fmt::format(does_not_equal, this->toString(), type_obj->toString()));
          return match;
        }
        case OType::INT: {
          const bool match = this->int_value() == type_obj->int_value();
          if(!match && fail_reason)
            fail_reason->push(fmt::format(does_not_equal, this->toString(), type_obj->toString()));
          return match;
        }
        case OType::REAL: {
          const bool match = this->real_value() == type_obj->real_value();
          if(!match && fail_reason)
            fail_reason->push(fmt::format(does_not_equal, this->toString(), type_obj->toString()));
          return match;
        }
        case OType::URI: {
          const fURI this_no_query = this->uri_value().no_query();
          const fURI type_no_query = type_obj->uri_value().no_query();
          const bool match = this_no_query.matches(type_no_query);
          if(!match && fail_reason)
            fail_reason->push(fmt::format(does_not_equal, this->toString(), type_obj->toString()));
          return match;
        }
        case OType::STR: {
          const bool match = this->str_value() == type_obj->str_value();
          if(!match && fail_reason)
            fail_reason->push(fmt::format(does_not_equal, this->toString(), type_obj->toString()));
          return match;
        }
        case OType::LST: {
          const auto objs_a = this->lst_value();
          const auto objs_b = type_obj->lst_value();
          if(objs_a->size() != objs_b->size())
            return false;
          auto b = objs_b->begin();
          for(const auto &a: *objs_a) {
            if(!a->match(*b, fail_reason)) {
              if(fail_reason)
                fail_reason->push(fmt::format("{} does !rnot!! match {}", a->toString(), (*b)->toString()));
              return false;
            }
            ++b;
          }
          return true;
        }
        case OType::REC: {
          const auto pairs_a = this->rec_value();
          const auto pairs_b = type_obj->rec_value();
          for(const auto &[b_key, b_obj]: RecMap<>(*pairs_b)) {
            bool found = b_key->is_uri() && b_key->uri_value().toString().find(':') != string::npos;
            if(!found) {
              for(const auto &[a_key, a_obj]: RecMap<>(*pairs_a)) {
                if(a_key->match(b_key)) {
                  found = a_obj->match(b_obj, fail_reason);
                  if(found)
                    break;
                }
              }
            }
            if(!found) {
              if(fail_reason)
                fail_reason->push(fmt::format("{} does !rnot!! have a corresponding match in {}",
                                              Obj::to_rec({{b_key, b_obj}})->toString(), this->toString()));
              return false;
            }
          }
          return true;
        }
        case OType::INST: {
          if(type_obj->is_bcode() && type_obj->bcode_value()->size() == 1)
            return this->match(type_obj->bcode_value()->front());

          if(!type_obj) {
            if(fail_reason)
              fail_reason->push(fmt::format("!b{}!! is !rnot!! the same base type as !b{}!!",
                                            OTypes.to_chars(this->otype), OTypes.to_chars(type_obj->otype)));
            return false;
          }
          const auto args_a = this->inst_args();
          if(const auto args_b = type_obj->inst_args(); !args_a->match(args_b, fail_reason)) {
            if(fail_reason)
              fail_reason->push(
                  fmt::format("!yarguments!! {} do !rnot!! match {}", args_a->toString(), args_b->toString()));
            return false;
          }
          if(this->domain_coefficient() != type_obj->domain_coefficient()) {
            if(fail_reason)
              fail_reason->push(fmt::format("!ydomain coefficient!! {{{},{}}} do !rnot!! match {{{},{}}}",
                                            this->domain_coefficient().first, this->domain_coefficient().second,
                                            type_obj->domain_coefficient().first,
                                            type_obj->domain_coefficient().second));
            return false;
          }
          if(this->range_coefficient() != type_obj->range_coefficient()) {
            if(fail_reason)
              fail_reason->push(fmt::format("!yrange coefficient!! {{{},{}}} !rdoesn't match!! {{{},{}}}",
                                            this->range_coefficient().first, this->range_coefficient().second,
                                            type_obj->range_coefficient().first, type_obj->range_coefficient().second));
            return false;
          }
          return true;
        }
        case OType::BCODE: {
          /*if(type_obj->is_inst() && this->bcode_value()->size() == 1)
            return this->bcode_value()->front()->match(type_obj);*/
          return true;
        }
        default:
          throw fError("unknown obj type in match(): %s", OTypes.to_chars(this->otype).c_str());
      }
    }

    /*[[nodiscard]] Obj_p as(const char *furi) const {
      return this->as(id_p(furi));
    }*/

    [[nodiscard]] Obj_p as(const ID_p &type_id) const {
      return type_id->equals(*NOOBJ_FURI) ? Obj::to_noobj()
                                          : Obj::create(this->value_, this->otype, type_id, this->vid);
    }

    [[nodiscard]] Obj_p as(const Obj_p &type_obj) const {
      // if(this->tid->equals(*type_obj->tid))
      //   return this->shared_from_this();
      if(type_obj->is_noobj())
        return Obj::to_noobj();
      if(type_obj->is_code()) {
        if(type_obj->is_empty_bcode())
          return this->clone();
        const Obj_p new_value = type_obj->apply(this->shared_from_this());
        if(!new_value->is_noobj()) {
          return Obj::create(new_value->value_, new_value->otype, type_obj->tid, this->vid);
        } else {
          throw fError("%s is not a %s", this->toString().c_str(), type_obj->toString().c_str());
        }
      }
      if(!type_obj->is_type() && this->otype != type_obj->otype)
        throw fError("%s is not a %s", this->toString().c_str(), type_obj->toString().c_str());
      // if(type_obj->is_type()) {
      if(type_obj->is_type())
        return this->as(type_obj->tid);
      switch(this->otype) {
        case OType::NOOBJ:
          return Obj::to_noobj();
        case OType::BOOL:
        case OType::INT:
        case OType::REAL:
        case OType::STR:
        case OType::URI:
          return Obj::create(this->value_, this->otype, type_obj->tid, this->vid);
        case OType::LST: {
          const auto objs_a = this->lst_value();
          const auto objs_b = type_obj->lst_value();
          const auto objs_c = make_shared<std::vector<Obj_p>>();
          if(objs_a->size() != objs_b->size())
            throw fError("%s is not a %s", this->toString().c_str(), type_obj->toString().c_str());
          auto b = objs_b->begin();
          for(const auto &a: *objs_a) {
            objs_c->push_back(a->as(*b));
            ++b;
          }
          return Obj::to_lst(objs_c, type_obj->tid, this->vid);
        }
        case OType::REC: {
          const auto pairs_a = this->rec_value();
          const auto pairs_b = type_obj->rec_value();
          const auto pairs_c = make_shared<RecMap<>>();
          for(const auto &[b_id, b_obj]: *pairs_b) {
            bool found = false;
            for(const auto &[a_id, a_obj]: *pairs_a) {
              if(a_id->match(b_id) && (!b_id->is_uri() || b_id->uri_value().toString().find(':') == string::npos)) {
                pairs_c->insert_or_assign(a_id->as(b_id), a_obj->as(b_obj));
                found = true;
                break;
              }
            }
            if(!found)
              throw fError("%s is not a %s", this->toString().c_str(), type_obj->toString().c_str());
          }
          return Obj::to_rec(pairs_c, type_obj->tid, this->vid);
        }
        default:
          throw fError("%s is not a %s", this->toString().c_str(), type_obj->toString().c_str());
      }
      // }
    }

    Obj_p at(const ID_p &value_id) const {
      if(value_id == nullptr && this->vid == nullptr)
        return this->shared_from_this();
      return Obj::create(this->value_, this->otype, this->tid, value_id);
    }

    [[nodiscard]] Option<fURI> lock() const {
      return this->vid && this->vid->query_value("lock").has_value()
                 ? Option<fURI>(fURI(this->vid->query_value("lock").value().c_str()))
                 : Option<fURI>();
    }


    [[nodiscard]] Obj_p lock(const fURI &user) const {
      if(this->vid == nullptr)
        throw fError("only objs with a value id can be locked: %s\n", this->toString().c_str());
      if(this->lock().has_value())
        throw fError("obj currently locked by %s: %s\n", this->vid->query_value("lock").value().c_str());
      const string new_query = strlen(this->vid->query()) == 0
                                   ? string("lock=").append(user.toString())
                                   : string(this->vid->query()).append("&lock=").append(user.toString());
      const ID new_vid = this->vid->query(new_query.c_str());
      const Obj_p new_obj = this->at(id_p(new_vid));
      LOG_WRITE(
          INFO, this,
          L("!g[!r.!y.!c.!g]!m@!b{} !yobj!! locked by !b{}!!\n", this->vid->no_query().toString(), user.toString()));
      return new_obj;
    }

    [[nodiscard]] Obj_p unlock(const fURI &user) const {
      if(this->vid == nullptr)
        throw fError("only objs with a value id can be locked and unlocked: %s\n", this->toString().c_str());
      if(!this->lock().has_value())
        throw fError("obj is not locked: %s\n", this->toString().c_str());
      if(this->lock().value() == user) {
        const ID new_vid = this->vid->query(""); // TODO: selectively remove lock
        const Obj_p new_obj = this->at(id_p(new_vid));
        LOG_WRITE(INFO, this,
                  L("!g[!r.!y.!c.!g]!m@!b{} !yobj!! unlocked by !b{}!!\n", this->vid->no_query().toString(),
                    user.toString()));
        return new_obj;
      } else {
        throw fError("only the owner %s can unlock %s\n", this->vid->query_value("lock").value().c_str(),
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

    Obj_p or_else(const Obj_p &other) const { return this->is_noobj() ? other : this->shared_from_this(); }

    template<typename T>
    T or_else_(const T &other) const {
      return this->is_noobj() ? other : this->value<T>();
    }

    /// STATIC TYPE CONSTRAINED CONSTRUCTORS
    static Obj_p to_type(const ID_p &type_id,
                         const Obj_p &obj = Obj::create(make_shared<List<Inst_p>>(), OType::BCODE,
                                                        OTYPE_FURI.at(OType::BCODE)),
                         const ID_p &value_id = nullptr) {
      return Obj::create(obj, OType::TYPE, type_id, value_id);
    }

    static Obj_p to_noobj() {
      const static auto noobj = Obj::create(Any(nullptr), OType::NOOBJ, NOOBJ_FURI);
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
      const Lst_p list = to_lst(make_shared<LstList>(), type_id, value_id);
      list->lst_value()->reserve(FOS_PRE_ALLOCATED_ELEMENT_LIST_SIZE);
      return list;
    }

    static Lst_p to_lst(const std::initializer_list<Obj> &xlst, const ID_p &type_id = LST_FURI,
                        const ID_p &value_id = nullptr) {
      const auto list = make_shared<LstList>();
      list->reserve(xlst.size());
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
      const Rec_p record = to_rec(make_shared<RecMap<>>(), type_id, value_id);
      record->rec_value()->reserve(FOS_PRE_ALLOCATED_ELEMENT_LIST_SIZE);
      return record;
    }

    static Rec_p to_rec(const std::initializer_list<Pair<const Obj, Obj>> &xrec, const ID_p &type_id = REC_FURI,
                        const ID_p &value_id = nullptr) {
      const auto map = make_shared<Obj::RecMap<>>();
      map->reserve(xrec.size());
      for(const auto &[key, value]: xrec) {
        map->insert(make_pair(make_shared<Obj>(key), make_shared<Obj>(value)));
      }
      return to_rec(map, type_id, value_id);
    }

    static Rec_p to_rec(const std::initializer_list<Pair<const Obj_p, Obj_p>> &xrec, const ID_p &type_id = REC_FURI,
                        const ID_p &value_id = nullptr) {
      const auto map = make_shared<Obj::RecMap<>>();
      map->reserve(xrec.size());
      for(const auto &[key, value]: xrec) {
        map->insert(make_pair(key, value));
      }
      return to_rec(map, type_id, value_id);
    }

    static Rec_p to_rec(const std::initializer_list<Pair<const string, Obj_p>> &xrec, const ID_p &type_id = REC_FURI,
                        const ID_p &value_id = nullptr) {
      const auto map = make_shared<Obj::RecMap<>>();
      map->reserve(xrec.size());
      for(const auto &[key, value]: xrec) {
        map->insert(make_pair(Obj::to_uri(key), value));
      }
      return to_rec(map, type_id, value_id);
    }

    static Inst_p to_inst(const InstValue &value, const ID_p &type_id = INST_FURI, const ID_p &value_id = nullptr) {
      if(!std::get<0>(value)->is_rec())
        TYPE_ERROR(std::get<0>(value).get(), __FILE__, __LINE__);
      return Obj::create(value, OType::INST, type_id, value_id);
    }

    static Inst_p to_inst(const InstF &instf, const ID_p &type_id = INST_FURI, const ID_p &value_id = nullptr) {
      return Obj::create(InstValue(Obj::to_inst_args(), instf, Obj::to_noobj()), OType::INST, type_id, value_id);
    }

    static InstArgs to_inst_args() { return Obj::to_rec(); }

    static InstArgs to_inst_args(const List<Obj_p> &args) {
      const Rec_p a = Obj::to_rec();
      for(size_t i = 0; i < args.size(); i++) {
        a->rec_value()->insert({Obj::to_uri(to_string(i)), args.at(i)});
      }
      return a;
    }

    static InstArgs to_inst_args(const std::initializer_list<std::pair<const string, Obj_p>> &args) {
      const Rec_p a = Obj::to_rec(args);
      return a;
    }

    static Inst_p to_inst(const std::initializer_list<Obj_p> &args, const ID_p &type_id,
                          const ID_p &value_id = nullptr) {
      return to_inst(type_id->name(), Obj::to_inst_args(args), InstF(Obj::to_noobj()), to_noobj(), type_id, value_id);
    }

    static Inst_p to_inst(const List<Obj_p> &args, const ID_p &type_id, const ID_p &value_id = nullptr) {
      return to_inst(type_id->name(), Obj::to_inst_args(args), InstF(Obj::to_noobj()), to_noobj(), type_id, value_id);
    }

    static Inst_p to_inst(const InstArgs &args, const ID_p &type_id, const ID_p &value_id = nullptr) {
      return to_inst(type_id->name(), args, InstF(Obj::to_noobj()), to_noobj(), type_id, value_id);
    }

    static Inst_p to_inst(const string &opcode, const InstArgs &args, const InstF &function,
                          const Obj_p &seed = Obj::to_noobj(), const ID_p &type_id = nullptr,
                          const ID_p &value_id = nullptr) {
      const ID_p fix = type_id != nullptr ? type_id : id_p(opcode.c_str()); // id_p(ROUTER_RESOLVE(fURI(opcode)));
      return to_inst(make_tuple(args, function, seed), fix, value_id);
    }

    static Inst_p to_inst(const Obj_p &obj, const InstArgs &args = Obj::to_inst_args(), const ID_p &type_id = nullptr,
                          const ID_p &value_id = nullptr) {
      switch(obj->otype) {
        case OType::NOOBJ:
          return Obj::to_noobj();
        case OType::BOOL:
        case OType::INT:
        case OType::REAL:
        case OType::STR:
        case OType::URI:
        case OType::LST:
        case OType::REC:
        case OType::OBJS:
          return Obj::create(InstValue(args, InstF(obj), Obj::to_noobj()), OType::INST,
                             nullptr != type_id ? type_id : id_p(obj->vid_or_tid()->extend("inst")), value_id);
        case OType::INST:
          return obj;
        case OType::BCODE:
          if(obj->bcode_value()->size() == 1 && args->rec_value()->empty())
            return obj->bcode_value()->front();
          else
            return Obj::create(InstValue(args, InstF(obj), Obj::to_noobj()), OType::INST,
                               nullptr != type_id ? type_id : id_p(obj->vid_or_tid()->extend("inst")), value_id);
        default:
          throw fError("cannot create an inst from %s", obj->toString().c_str());
      }
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

    static Objs_p as_objs(const Obj_p &obj, const ID_p &type_id = OBJS_FURI, const ID_p &value_id = nullptr) {
      const Objs_p objs = Obj::to_objs(type_id, value_id);
      objs->add_obj(obj);
      return objs;
    }

    static Error_p to_error(const Obj_p &obj, const Inst_p &inst, const ID_p &type_id = ERROR_FURI) {
      return Obj::create(Pair<Obj_p, Inst_p>(obj, inst), OType::ERROR, type_id);
    }

    /*std::__allocator_base<Obj> allocator = std::allocator<Obj>()*/
    [[nodiscard]] Obj_p clone() const { return std::make_shared<Obj>(*this); }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    [[nodiscard]] BObj_p serialize() const {
      LOG_WRITE(DEBUG, this, L("serializing obj {}\n", this->toString()));
      const string serial = this->toString(SERIALIZER_PRINTER);
      return ptr<BObj>(new BObj(serial.length(), reinterpret_cast<fbyte *>(strdup(serial.c_str()))), bobj_deleter);
    }

    static Obj_p deserialize(const BObj_p &bobj) {
      // LOG_WRITE(DEBUG, nullptr, L("deserializing bytes {} (length {})\n", bobj->second, bobj->first));
      const Obj_p obj = OBJ_PARSER(string(reinterpret_cast<char *>(bobj->second), bobj->first));
      return obj;
    }


    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  protected:
    class ObjsSet {
    public:
      const unique_ptr<OrderedMap<Obj_p, long, objp_hash, objp_equal_to>> internal =
          make_unique<OrderedMap<Obj_p, long, objp_hash, objp_equal_to>>();

      void push_back(const Obj_p &obj, const long bulk = 1) const { this->add(obj, bulk); }

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
        for(const auto &[obj, bulk]: *this->internal) {
          bulk_total += bulk;
        }
        return bulk_total;
      }

      [[nodiscard]] unsigned long size() const { return this->internal->size(); }

      [[nodiscard]] bool empty() const { return this->internal->empty(); }
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  };

  static void load_logger() {
    LOG_WRITE = [](const LOG_TYPE log_type, const Obj *source, const std::function<string()> &message) {
      LOG_OBJ(log_type, source, message());
    };
  }

  [[maybe_unused]] static Uri_p vri(const fURI &xuri, const ID_p &type = URI_FURI, const ID_p &value_id = nullptr) {
    return Obj::to_uri(xuri, type, value_id);
  }

  [[maybe_unused]] static Uri_p vri(const ID_p &xuri, const ID_p &type_id = URI_FURI, const ID_p &value_id = nullptr) {
    return Obj::to_uri(ID(*xuri), type_id, value_id);
  }

  [[maybe_unused]] static Uri_p vri(const fURI_p &xuri, const ID_p &type_id = URI_FURI,
                                    const ID_p &value_id = nullptr) {
    return Obj::to_uri(fURI(*xuri), type_id, value_id);
  }

  [[maybe_unused]] static Uri_p vri(const char *xuri, const ID_p &type_id = URI_FURI, const ID_p &value_id = nullptr) {
    return Obj::to_uri(fURI(xuri), type_id, value_id);
  }

  [[maybe_unused]] static Uri_p vri(const string &xuri, const ID_p &type_id = URI_FURI,
                                    const ID_p &value_id = nullptr) {
    return Obj::to_uri(fURI(xuri), type_id, value_id);
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
                                    const ID_p &type = REC_FURI, const ID_p &vid = nullptr) {
    Obj::RecMap<> rec_map = Obj::RecMap<>();
    std::transform(map.begin(), map.end(), std::inserter(rec_map, rec_map.end()),
                   [](const auto &pair) { return std::make_pair(vri(pair.first), pair.second); });
    return rec(rec_map, type, vid);
  }

  [[maybe_unused]] static Objs_p objs() { return Obj::to_objs(make_shared<List<Obj_p>>()); }

  [[maybe_unused]] static Objs_p objs(const List<Obj_p> &list) { return Obj::to_objs(make_shared<List<Obj_p>>(list)); }

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

  inline std::ostream &operator<<(std::ostream &os, const Obj &value) {
    os << value.toString();
    return os;
  };
} // namespace fhatos
#endif
