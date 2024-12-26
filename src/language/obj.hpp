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

#define C_INST_C STR(::)
#define MMADT_INST_SCHEME C_INST_C MMADT_SCHEME
#define MMADT_SCHEME "/mmadt"
#define FOS_SCHEME "/fos"
// #define FOS_BASE_TYPE_INDEX 1

#include "../fhatos.hpp"
#include <utility>
#include <variant>
#include "../furi.hpp"
#include <tsl/ordered_map.h>

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


  using Obj_p = shared_ptr<Obj>;
  using Obj_wp = weak_ptr<Obj>;
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
  using Error = Obj;
  using Error_p = Obj_p;
  using BObj = Pair<uint32_t, fbyte *>;
  using BObj_p = ptr<Pair<uint32_t, fbyte *>>;

  enum class IType : uint16_t {
    ZERO_TO_ZERO = (1 << 0),
    ZERO_TO_ONE = (1 << 1),
    ZERO_TO_MANY = (1 << 2),
    ZERO_TO_MAYBE = (1 << 3),
    ONE_TO_ZERO = (1 << 4),
    ONE_TO_ONE = (1 << 5),
    ONE_TO_MANY = (1 << 6),
    ONE_TO_MAYBE = (1 << 7),
    MANY_TO_ONE = (1 << 8),
    MANY_TO_MANY = (1 << 9),
    MANY_TO_MAYBE = (1 << 10),
    MANY_TO_ZERO = (1 << 11),
    MAYBE_TO_ONE = (1 << 12),
    MAYBE_TO_MANY = (1 << 13),
    MAYBE_TO_ZERO = (1 << 14),
    MAYBE_TO_MAYBE = (1 << 15),
  }; // TYPE

  enum class Cardinality : unsigned char {
    ZERO = '.',
    ONE = 'o',
    MANY = 'O',
    MAYBE = '?'
  };

  //Enums<Cardinality> Cardinalities = Enums<Cardinality>({{Cardinality::ZERO,"."},})

  inline bool operator&(IType a, IType b) {
    // Implement bitwise OR logic
    return (bool) (IType) (static_cast<std::underlying_type_t<IType>>(a) & static_cast<std::underlying_type_t<IType>>(
                             b));
  }

  inline IType operator|(IType a, IType b) {
    // Implement bitwise OR logic
    return (IType) (static_cast<std::underlying_type_t<IType>>(a) | static_cast<std::underlying_type_t<IType>>(b));
  }


  inline bool contains_itype(IType check, IType check_set) {
    return (static_cast<uint8_t>(check) & static_cast<uint8_t>(check_set)) == static_cast<uint8_t>(check_set);
  }

  [[maybe_unused]] static bool is_initial(const IType itype) {
    return itype == IType::ZERO_TO_ONE || itype == IType::ZERO_TO_MANY || itype == IType::ZERO_TO_ZERO || itype ==
           IType::ZERO_TO_MAYBE;
  }

  [[maybe_unused]] static bool is_maybe_initial(const IType itype) {
    return itype == IType::MAYBE_TO_ONE || itype == IType::MAYBE_TO_MAYBE || itype == IType::MAYBE_TO_MANY || itype ==
           IType::MAYBE_TO_ZERO;
  }

  [[maybe_unused]] static bool is_scatter(const IType itype) {
    return itype == IType::ONE_TO_MANY || itype == IType::MANY_TO_MANY || itype == IType::ZERO_TO_MANY || itype ==
           IType::MAYBE_TO_MANY;
  }

  [[maybe_unused]] static bool is_gather(const IType itype) {
    return itype == IType::MANY_TO_ONE || itype == IType::MANY_TO_MANY || itype == IType::MANY_TO_ZERO || itype ==
           IType::MANY_TO_MAYBE;
  }

  [[maybe_unused]] static bool is_terminal(const IType itype) {
    return itype == IType::ONE_TO_ZERO || itype == IType::MANY_TO_ZERO || itype == IType::ZERO_TO_ZERO || itype ==
           IType::MAYBE_TO_ZERO;
  }

  [[maybe_unused]] static bool is_map(const IType itype) {
    return itype == IType::ONE_TO_ONE || itype == IType::MAYBE_TO_ONE;
  }

  [[maybe_unused]] static bool is_filter(const IType itype) {
    return itype == IType::ONE_TO_MAYBE || itype == IType::MAYBE_TO_MAYBE;
  }

  static Consumer<BObj *> bobj_deleter = [](const BObj *bobj) {
    free(bobj->second);
    delete bobj;
  };
  static const auto ITypeDomains = Enums<IType>({
    {IType::ZERO_TO_ZERO, "."},
    {IType::ZERO_TO_ONE, "."},
    {IType::ZERO_TO_MANY, "."},
    {IType::ONE_TO_ZERO, "o"},
    {IType::MANY_TO_ZERO, "O"},
    {IType::ONE_TO_ONE, "o"},
    {IType::ONE_TO_MANY, "o"},
    {IType::MAYBE_TO_MANY, "?"},
    {IType::MAYBE_TO_ONE, "?"},
    {IType::MAYBE_TO_ZERO, "?"},
    {IType::MAYBE_TO_MAYBE, "?"},
    {IType::ONE_TO_MAYBE, "o"},
    {IType::ZERO_TO_MAYBE, "."},
    {IType::MANY_TO_MAYBE, "O"},
    {IType::MANY_TO_ONE, "O"},
    {IType::MANY_TO_MANY, "O"}});
  static const auto ITypeRanges = Enums<IType>({
    {IType::ZERO_TO_ZERO, "."},
    {IType::ZERO_TO_ONE, "o"},
    {IType::ZERO_TO_MANY, "O"},
    {IType::ONE_TO_ZERO, "."},
    {IType::MANY_TO_ZERO, "."},
    {IType::ONE_TO_ONE, "o"},
    {IType::ONE_TO_MANY, "O"},
    {IType::MAYBE_TO_MANY, "?"},
    {IType::MAYBE_TO_ONE, "o"},
    {IType::MAYBE_TO_ZERO, "."},
    {IType::MAYBE_TO_MAYBE, "?"},
    {IType::ONE_TO_MAYBE, "?"},
    {IType::ZERO_TO_MAYBE, "?"},
    {IType::MANY_TO_MAYBE, "?"},
    {IType::MANY_TO_ONE, "o"},
    {IType::MANY_TO_MANY, "O"}});
  static const auto ITypeSignatures = Enums<IType>({
    {IType::ZERO_TO_ZERO, ".->."},
    {IType::ZERO_TO_ONE, ".->o"},
    {IType::ZERO_TO_MANY, ".->O"},
    {IType::ONE_TO_ZERO, "o->."},
    {IType::MANY_TO_ZERO, "O->."},
    {IType::ONE_TO_ONE, "o->o"},
    {IType::ONE_TO_MANY, "o->O"},
    {IType::MAYBE_TO_MANY, "?"},
    {IType::MAYBE_TO_ONE, "o"},
    {IType::MAYBE_TO_ZERO, "."},
    {IType::MAYBE_TO_MAYBE, "?"},
    {IType::MANY_TO_MAYBE, "?"},
    {IType::ONE_TO_MAYBE, "o"},
    {IType::ZERO_TO_MAYBE, "."},
    {IType::MANY_TO_ONE, "O->o"},
    {IType::MANY_TO_MANY, "O->O"}});
  static const auto ITypeDescriptions = Enums<IType>({
    {IType::ZERO_TO_ZERO, ".->. (transient)"},
    {IType::ZERO_TO_ONE, ".->o (supplier)"},
    {IType::ZERO_TO_MANY, ".->O (initial)"},
    {IType::ONE_TO_ZERO, "o->. (consumer)"},
    {IType::MANY_TO_ZERO, "O->. (terminal)"},
    {IType::ONE_TO_ONE, "o->o (map)"},
    {IType::ONE_TO_MANY, "o->O (flatmap)"},
    {IType::MAYBE_TO_MANY, "?->O (potential)"},
    {IType::MAYBE_TO_ONE, "?->o (flip)"},
    {IType::MAYBE_TO_ZERO, "?->. (spark)"},
    {IType::MAYBE_TO_MAYBE, "?->? (flux)"},
    {IType::ONE_TO_MAYBE, "o->? (filter)"},
    {IType::ZERO_TO_MAYBE, ".->? (check)"},
    {IType::MANY_TO_MAYBE, "?-? (strain)"},
    {IType::MANY_TO_ONE, "O->o (reduce)"},
    {IType::MANY_TO_MANY, "O->O (barrier)"},
  });
  using InstArgs = Rec_p;

  struct InstF {
    bool pure;
    Any f;

    InstF() = default;

    explicit InstF(const BiFunction<const Obj_p, const InstArgs, Obj_p> &ff) : pure(false),
                                                                               f(make_shared<BiFunction<Obj_p, InstArgs,
                                                                                 Obj_p>>(ff)) {
    }

    InstF(const Obj_p &obj_f, const bool pure) : pure(pure), f(obj_f) {
    }

    static ptr<InstF> wrap(const Obj_p &obj) {
      return make_shared<InstF>(obj, true);
    }

    Obj_p obj_f() {
      if(!this->pure)
        throw fError("inst function written in native cxx");
      return std::any_cast<Obj_p>(this->f);
    }

    ptr<BiFunction<const Obj_p, const InstArgs, Obj_p>> cpp_f() {
      if(this->pure)
        throw fError("inst function written in obj");
      return std::any_cast<ptr<BiFunction<const Obj_p, const InstArgs, Obj_p>>>(this->f);
    }
  };

  using InstF_p = ptr<InstF>;
  using InstValue = Quad<InstArgs, InstF_p, IType, Obj_p>;
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

  static Runnable ROUTER_POP_FRAME = []() {
    LOG(TRACE, "!ROUTER_POP_FRAME!! undefined at this point in bootstrap\n");
  };
  static BiConsumer<Pattern, Rec_p> ROUTER_PUSH_FRAME = [](const Pattern &pattern, const Rec_p &frame_data) {
    LOG(TRACE, "!ROUTER_PUSH_FRAME!! undefined at this point in bootstrap: %s\n", pattern.toString().c_str());
  };
  static TriFunction<const ID_p &, const ID_p &, List<ID_p> *, const bool> IS_TYPE_OF =
      [](const ID_p &is_type_id, const ID_p &type_of_id, List<ID_p> *derivations) {
    LOG(TRACE, "!IS_TYPE_OF!! undefined at this point in bootstrap: %s\n", is_type_id->toString().c_str());
    return false;
  };
  static TriFunction<const Obj *, const ID_p &, const bool, const bool> TYPE_CHECKER =
      [](const Obj *, const ID_p &type_id, const bool = true) -> bool {
    LOG(TRACE, "!yTYPE_CHECKER!! undefined at this point in bootstrap: %s\n", type_id->toString().c_str());
    return true;
  };
  static BiFunction<Obj_p, ID_p, Obj_p> TYPE_MAKER = [](Obj_p, const ID_p &) {
    LOG(TRACE, "!yTYPE_MAKER!! undefined at this point in bootstrap.\n");
    return nullptr;
  };
  static ptr<Ansi<>> PRINTER = nullptr;

  static Function<const string &, Obj_p> OBJ_PARSER = [](const string &) {
    LOG(TRACE, "!yOBJ_PARSER!! undefined at this point in bootstrap.\n");
    return nullptr;
  };
  static Function<BCode_p, Objs_p> BCODE_PROCESSOR = [](const BCode_p &) {
    LOG(TRACE, "!yBCODE_PROCESSOR!! undefined at this point in bootstrap.\n");
    return nullptr;
  };
  static Function<const fURI, const fURI_p> ROUTER_RESOLVE = [](const fURI &furi) {
    LOG(TRACE, "!yROUTER_RESOLVE!! undefined at this point in bootstrap.\n");
    return id_p(furi);
  };
  static TriConsumer<const fURI_p &, const Obj_p &, const bool> ROUTER_WRITE =
      [](const fURI_p &, const Obj_p &, const bool retain) -> void {
    LOG(TRACE, "!yROUTER_WRITE!! undefined at this point in bootstrap.\n");
  };
  static Function<const fURI_p &, const Obj_p> ROUTER_READ = [](const fURI_p &) -> Obj_p {
    LOG(TRACE, "!yROUTER_READ!! undefined at this point in bootstrap.\n");
    return nullptr;
  };
  static BiConsumer<const ID_p, const Obj_p> TYPE_SAVER = [](const ID_p &type_id, const Obj_p &obj) {
    LOG(TRACE, "!yTYPE_SAVER!! undefined at this point in bootstrap: %s\n", type_id->toString().c_str());
    ROUTER_WRITE(type_id, obj, true);
  };
  struct Subscription;
  static Consumer<const ptr<Subscription> &> ROUTER_SUBSCRIBE = [](const ptr<Subscription> &) {
    LOG(TRACE, "!yROUTER_SUBSCRIBE!! undefined at this point in bootstrap.\n");
  };
  static BiFunction<const Obj_p &, const Inst_p &, Inst_p> TYPE_INST_RESOLVER = [
      ](const Obj_p &lhs, const Inst_p &old_inst) {
    LOG(TRACE, "!RESOLVE_INST!! undefined at this point in bootstrap.\n");
    return nullptr;
  };

  //////////////////////////////////////////////////
  ////////////////////// OBJ //////////////////////
  /////////////////////////////////////////////////
  /// An mm-ADT abstract object from which all other types derive
  class Obj : public Typed,
              public Valued,
              public Function<Obj_p, Obj_p>,
              public BiFunction<Obj_p, InstArgs, Obj_p>,
              public enable_shared_from_this<Obj> {
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
                                                   Valued(value_id), otype_(otype),
                                                   value_(value) {
      if(value.has_value()) { // value token
        TYPE_CHECKER(this, type_id, true);
        this->tid_ = type_id;
        if(value_id) {
          const Obj_p strip = this->clone();
          strip->vid_ = nullptr;
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
      return fError(FURI_WRAP " %s !yaccessed!! as !b%s!! L%i", obj->vid_or_tid()->toString().c_str(),
                    obj->toString().c_str(),
                    string(function).replace(string(function).find("_value"), 6, "").c_str(),
                    line_number);
    }

    //////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////
    [[nodiscard]] ID_p vid_or_tid() const {
      return this->vid_ ? this->vid_ : this->tid_;
    }

    virtual void save(const ID_p &id = nullptr) {
      if(id) {
        this->vid_ = id;
        ROUTER_WRITE(id, shared_from_this(), true);
      } else if(this->vid_)
        ROUTER_WRITE(this->vid_, shared_from_this(), true);
    }

    [[nodiscard]] OType o_type() const { return this->otype_; }

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

    void lst_add(const Obj_p &obj) const {
      if(!this->is_lst())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      this->lst_value()->push_back(obj);
    }

    [[nodiscard]] Obj_p deref(const Obj_p &uri) const {
      if(this->is_rec())
        return this->rec_get(uri);
      if(this->is_lst())
        return this->lst_get(uri);
      if(this->is_objs()) {
        const Objs_p transform = Obj::to_objs(make_shared<List<Obj_p>>());
        for(const auto &o: *this->objs_value()) {
          transform->add_obj(o->deref(uri));
        }
        return transform;
      }
      return uri;
    }

    [[nodiscard]] Int_p rec_size() const { return Obj::to_int(this->rec_value()->size()); }

    [[nodiscard]] Int_p lst_size() const { return Obj::to_int(this->lst_value()->size()); }

    [[nodiscard]] Obj_p lst_get(const uint16_t &index) const { return this->lst_get(Obj::to_int(index)); }

    [[nodiscard]] Obj_p lst_get(const Obj_p &index) const {
      if(!this->is_lst())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      if(index->is_uri()) {
        const auto segment = string(index->uri_value().path(0));
        const int i = StringHelper::is_integer(segment)
                        ? std::stoi(segment)
                        : (StringHelper::has_wildcards(segment) ? -1 : -100);
        if(-100 == i)
          throw fError("segment !b%s!! of !b%s!! !ris not!! an !yint!! or wildcard", segment.c_str(),
                       index->uri_value().toString().c_str());
        if(i >= this->lst_value()->size())
          return to_noobj();
        Obj_p segment_value = Obj::to_objs();
        if(i == -1) {
          for(const auto &e: *this->lst_value()) {
            segment_value->add_obj(e);
          }
        } else {
          segment_value->add_obj(this->lst_value()->at(i));
        }
        segment_value = segment_value->none_one_all();
        if(segment_value->is_noobj())
          return to_noobj();
        return index->uri_value().path_length() <= 1
                 ? segment_value
                 : segment_value->deref(Obj::to_uri(index->uri_value().path(1, 255)));
      }
      return (static_cast<size_t>(index->int_value()) >= this->lst_value()->size())
               ? Obj::to_noobj()
               : this->lst_value()->at(index->int_value());
    }

    void lst_set(const Int_p &index, const Obj_p &obj) const {
      if(!this->is_lst())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      this->lst_value()->insert(this->lst_value()->begin() + index->int_value(), obj);
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
      size_t counter = 0;
      for(const auto &[k,v]: *this->rec_value()) {
        if(index == counter)
          return v;
        counter++;
      }
      return Obj::to_noobj();
    }

    [[nodiscard]] Obj_p arg(const Obj_p &key) const {
      return this->rec_value()->count(key) ? this->rec_value()->at(key) : Obj::to_noobj();
    }

    [[nodiscard]] Obj_p rec_get(const Obj_p &key, const Runnable &on_error = nullptr) const {
      if(!this->is_rec())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      if(key->is_uri()) {
        const auto segment = 0 == key->uri_value().path_length()
                               ? key->uri_value().toString()
                               : string(key->uri_value().path(0));
        Objs_p segment_value = Obj::to_objs();
        const Uri_p segment_uri = Obj::to_uri(fURI(segment));
        const bool match_all = StringHelper::has_wildcards(segment);
        for(const auto &[k,v]: *this->rec_value()) {
          if(match_all || k->match(segment_uri))
            segment_value->add_obj(v);
        }
        segment_value = segment_value->none_one_all();
        return key->uri_value().path_length() <= 1
                 ? segment_value
                 : segment_value->deref(to_uri(key->uri_value().path(1, 255)));
      }
      const Objs_p segment_value = Obj::to_objs();
      for(const auto &[k, v]: *this->rec_value()) {
        if(k->match(key))
          segment_value->add_obj(v);
      }
      return segment_value->none_one_all();
    }

    [[nodiscard]] Obj_p rec_get(const fURI_p &key, const Runnable &on_error = nullptr) const {
      return rec_get(to_uri(*key), on_error);
    }

    [[nodiscard]] Obj_p rec_get(const char *uri_key, const Runnable &on_error = nullptr) const {
      return rec_get(to_uri(fURI(uri_key)), on_error);
    }

    [[nodiscard]] Rec_p rec_merge(const RecMap_p<> &rmap) {
      this->rec_value()->insert(rmap->cbegin(), rmap->cend());
      return shared_from_this();
    }

    virtual void rec_set(const char *uri_key, const Obj_p &val, const bool nest = true) const {
      this->rec_set(Obj::to_uri(uri_key), val, nest);
    }

    virtual void rec_set(const Obj_p &key, const Obj_p &val, const bool nest = true) const {
      if(nest && key->is_uri() && key->uri_value().path_length() > 1) {
        const Rec *current_rec = const_cast<Rec *>(this);
        for(int i = 0; i < key->uri_value().path_length(); i++) {
          const auto p = string(key->uri_value().path(i));
          if(!current_rec->rec_value()->count(to_uri(p)))
            break;
          if(i == key->uri_value().path_length() - 1)
            current_rec->rec_value()->erase(to_uri(p));
          else {
            const Rec *next_rec = current_rec->rec_value()->at(to_uri(p)).get();
            if(!next_rec->is_rec())
              throw fError("path %s of %s is not a rec", p.c_str(), key->toString().c_str(),
                           next_rec->toString().c_str());
            current_rec = next_rec;
          }
        }
        if(!val->is_noobj()) {
          for(int i = 0; i < key->uri_value().path_length(); i++) {
            const auto p = string(key->uri_value().path(i));
            if(i == key->uri_value().path_length() - 1)
              current_rec->rec_value()->insert({to_uri(p), val});
            else {
              if(!current_rec->rec_value()->count(to_uri(p)))
                current_rec->rec_value()->insert({to_uri(p), Obj::to_rec()});
              current_rec = current_rec->rec_value()->at(to_uri(p)).get();
            }
          }
        }
      } else {
        this->rec_value()->erase(key);
        if(!val->is_noobj())
          this->rec_value()->insert({key, val});
        if(this->vid_)
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
      for(const auto &[k, v]: *other->rec_value()) {
        if(this->rec_value()->count(k))
          this->rec_value()->erase(k);
        this->rec_set(k, v);
      }
      if(this->vid_)
        ROUTER_WRITE(this->vid_, Obj::to_rec(make_shared<RecMap<>>(*this->rec_value()), id_p(*this->tid_)), true);
    }

    void rec_delete(const Obj &key) const { Obj::rec_set(make_shared<Obj>(key), Obj::to_noobj()); }

    [[nodiscard]] InstValue inst_value() const {
      if(!this->is_inst())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->value<InstValue>();
    }

    [[nodiscard]] string inst_op() const {
      if(!this->is_inst())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->tid_->name();
    }

    [[nodiscard]] InstArgs inst_args() const {
      if(!this->is_inst())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return std::get<0>(this->inst_value());
    }

    [[nodiscard]] bool has_arg(const Obj_p &key) {
      return this->inst_args()->rec_value()->count(key) != 0;
    }

    [[nodiscard]] InstF_p inst_f() const {
      if(!this->is_inst())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return std::get<1>(this->inst_value());
    }

    [[nodiscard]] Obj_p inst_seed_supplier() const {
      if(!this->is_inst())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return std::get<3>(this->inst_value());
    }

    [[nodiscard]] Obj_p inst_seed(const Obj_p &arg) const { return this->inst_seed_supplier()->apply(arg); }

    [[nodiscard]] Pair<Obj_p, Inst_p> error_value() const {
      if(!this->is_error())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      return this->value<Pair<Obj_p, Inst_p>>();
    }

    [[nodiscard]] ID_p domain() const {
      return this->tid_->has_query("domain")
               ? id_p(this->tid_->query_value("domain")->c_str())
               : (this->is_bcode() && !this->bcode_value()->empty()
                    ? this->bcode_value()->front()->domain()
                    : OBJ_FURI);
    }

    [[nodiscard]] IType itype2() const {
      if(this->is_inst())
        return this->itype();
      if(!this->tid_->has_query("ftype"))
        return IType::ONE_TO_ONE;
      const auto dom_rng = this->tid_->query_values("ftype");
      return ITypeSignatures.to_enum(string(dom_rng[0]).append("->").append(dom_rng[1]));
    }

    [[nodiscard]] ID_p range() const {
      return this->tid_->has_query("range")
               ? id_p(this->tid_->query_value("range")->c_str())
               : this->is_code()
                   ? (this->is_bcode() && !this->bcode_value()->empty()
                        ? this->bcode_value()->back()->range()
                        : OBJ_FURI)
                   : this->tid_;
    }

    [[nodiscard]] Obj_p type() const {
      return ROUTER_READ(this->tid_);
    }

    [[nodiscard]] IType itype() const {
      if(this->is_inst())
        return std::get<2>(this->inst_value());
      if(this->is_bcode()) {
        const IType domain = this->bcode_value()->front()->itype();
        const IType range = this->bcode_value()->back()->itype();
        return ITypeSignatures.to_enum(string(ITypeDomains.to_chars(domain) + "->" + ITypeRanges.to_chars(range)));
      }
      if(this->is_objs())
        return IType::ONE_TO_MANY;
      return IType::ONE_TO_ONE;
    }

    bool CHECK_OBJ_TO_INST_SIGNATURE(const Inst_p &resolved, const bool domain_or_range,
                                     const bool throw_exception = true) const {
      if(domain_or_range) {
        if(this->is_noobj()) {
          if(!(resolved->itype() & (IType::ZERO_TO_ONE | IType::ZERO_TO_MANY | IType::ZERO_TO_ZERO |
                                    IType::ZERO_TO_MAYBE | IType::MAYBE_TO_ONE | IType::MAYBE_TO_MANY |
                                    IType::MAYBE_TO_ZERO | IType::MAYBE_TO_MAYBE))) {
            if(!throw_exception) return false;
            throw fError("%s [%s] not in domain of %s [!y%s!!]",
                         this->toString().c_str(),
                         Obj::to_type(NOOBJ_FURI)->toString().c_str(),
                         resolved->toString().c_str(),
                         ITypeDescriptions.to_chars(resolved->itype()).c_str());
          }
        } else if(this->is_objs()) {
          if(!(resolved->itype() & (IType::MANY_TO_ONE | IType::MANY_TO_MANY | IType::MANY_TO_ZERO |
                                    IType::MANY_TO_MAYBE))) {
            if(!throw_exception) return false;
            throw fError("%s [%s] not in domain of %s [!y%s!!]",
                         this->toString().c_str(),
                         Obj::to_type(OBJS_FURI)->toString().c_str(),
                         resolved->toString().c_str(),
                         ITypeDescriptions.to_chars(resolved->itype()).c_str());
          }
        } else {
          if(!(resolved->itype() & (IType::ONE_TO_ONE | IType::ONE_TO_MANY | IType::ONE_TO_ZERO |
                                    IType::ONE_TO_MAYBE | IType::MAYBE_TO_ONE | IType::MAYBE_TO_MANY |
                                    IType::MAYBE_TO_ZERO | IType::MAYBE_TO_MAYBE))) {
            if(!throw_exception) return false;
            throw fError("%s [%s] not in domain of %s [!y%s!!]",
                         this->toString().c_str(),
                         Obj::to_type(OBJ_FURI)->toString().c_str(),
                         resolved->toString().c_str(),
                         ITypeDescriptions.to_chars(resolved->itype()).c_str());
          }
        }
      } else {
        if(this->is_noobj()) {
          if(!(resolved->itype() & (IType::ONE_TO_ZERO | IType::MANY_TO_ZERO | IType::ZERO_TO_ZERO |
                                    IType::MAYBE_TO_ZERO | IType::ONE_TO_MAYBE | IType::MANY_TO_MAYBE |
                                    IType::ZERO_TO_MAYBE | IType::MAYBE_TO_MAYBE))) {
            if(!throw_exception) return false;
            throw fError("%s [%s] not in range of %s [!y%s!!]",
                         this->toString().c_str(),
                         Obj::to_type(NOOBJ_FURI)->toString().c_str(),
                         resolved->toString().c_str(),
                         ITypeDescriptions.to_chars(resolved->itype()).c_str());
          }
        } else if(this->is_objs()) {
          if(!(resolved->itype() & (IType::ONE_TO_MANY | IType::MANY_TO_MANY | IType::ZERO_TO_MANY |
                                    IType::MAYBE_TO_MANY))) {
            if(!throw_exception) return false;
            throw fError("%s [%s] not in range of %s [!y%s!!]",
                         this->toString().c_str(),
                         Obj::to_type(OBJS_FURI)->toString().c_str(),
                         resolved->toString().c_str(),
                         ITypeDescriptions.to_chars(resolved->itype()).c_str());
          }
        } else {
          if(!(resolved->itype() & (IType::ONE_TO_ONE | IType::MANY_TO_ONE | IType::ZERO_TO_ONE |
                                    IType::MAYBE_TO_ONE | IType::ONE_TO_MAYBE | IType::MANY_TO_MAYBE |
                                    IType::ZERO_TO_MAYBE | IType::MAYBE_TO_MAYBE))) {
            if(!throw_exception) return false;
            throw fError("%s [%s] not in range of %s [!y%s!!]",
                         this->toString().c_str(),
                         Obj::to_type(OBJ_FURI)->toString().c_str(),
                         resolved->toString().c_str(),
                         ITypeDescriptions.to_chars(resolved->itype()).c_str());
          }
        }
      }
      return true;
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

    Obj_p this_add(const ID &relative_id, const Obj_p &inst, const bool at_type = true) {
      if(!at_type && !this->vid_)
        throw fError("only objs with a value id can have properties and insts");
      //if(inst->is_code())
      ROUTER_WRITE(id_p(this->vid_->append(relative_id)), inst, true);
      return this->shared_from_this();
    }

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

    [[nodiscard]] Obj_p none_one_all() {
      if(this->is_objs()) {
        if(this->objs_value()->empty())
          return Obj::to_noobj();
        if(this->objs_value()->size() == 1)
          return this->objs_value()->front();
      }
      return this->shared_from_this();
    }

    void add_obj(const Obj_p &obj, [[maybe_unused]] const bool mutate = true) {
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

    [[nodiscard]] string toString(const ObjPrinter *obj_printer = nullptr) const {
      if(!obj_printer)
        obj_printer = GLOBAL_PRINTERS.at(this->o_type());
      string obj_string;
      if(this->is_noobj())
        return "!r" STR(FOS_NOOBJ_TOKEN) "!!";
      if(!this->is_type()) {
        switch(this->o_type()) {
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
            obj_string = "!m[!!";
            bool first = true;
            for(const auto &obj: *this->lst_value()) {
              if(first) {
                first = false;
              } else {
                obj_string += "!m,!!";
              }
              obj_string += obj->toString(obj_printer);
            }
            obj_string += "!m]!!";
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
                obj_string += v->toString();
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
                obj_string += k->toString(obj_printer->next());
                obj_string += "=>";
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
          case OType::OBJ: {
            obj_string = "!g" STR(FOS_OBJ_TOKEN) "!!";
            break;
          }
          default:
            throw fError("unknown obj type in toString(): %s", OTypes.to_chars(this->o_type()).c_str());
        }
      }
      if(!this->is_noobj() &&
         ////
         (this->is_type() ||
          this->is_inst() ||
          (obj_printer->show_type && this->is_code() &&
           (this->itype2() != IType::ONE_TO_ONE || !this->domain()->equals(*OBJ_FURI) || !this->range()->
            equals(*OBJ_FURI)))) ||
         ////
         (obj_printer->show_type && this->is_base_type()) ||
         ////
         (obj_printer->strict && this->is_uri())) {
        string typing = this->is_base_type() && !this->is_code() && !this->is_type()
                          ? ""
                          : string("!B").append(this->is_bcode() ? "!!" : this->tid_->name()).append("!!");
        // TODO: remove base_type check
        if(obj_printer->show_domain_range &&
           !this->is_base_type() &&
           (this->itype2() != IType::ONE_TO_ONE ||
            !this->domain()->equals(*OBJ_FURI) ||
            !this->range()->equals(*OBJ_FURI))) {
          typing = typing.append("!m?!!")
              .append("!c").append(this->range()->name()).append("!m{!c").append(ITypeRanges.to_chars(this->itype2())).
              append("!m}!!")
              .append("!m<=!!")
              .append("!c").append(this->domain()->name()).append("!m{!c").append(ITypeDomains.to_chars(this->itype2()))
              .append("!m}!!");
        }
        obj_string = this->is_base_type() && !this->is_inst()
                       ? typing.append(obj_string)
                       : typing.append(this->is_inst() ? "!g(!!" : "!g[!!")
                       .append(obj_string)
                       .append(this->is_inst() ? "!g)!!" : "!g]!!");

        if(this->is_inst() && this->inst_f() && this->inst_f()->pure) {
          obj_string = obj_string
              .append("!g[!!")
              .append(this->inst_f()->obj_f()->toString())
              .append("!g]!!");
        }
      }
      if(obj_printer->show_id && this->vid_) {
        obj_string += "!m@!b";
        obj_string += this->vid_->toString();
        obj_string += "!!";
      }

      return obj_printer->ansi ? obj_string : Ansi<>::strip(obj_string);
    }

    [[nodiscard]] int compare(const Obj &rhs) const { return this->toString().compare(rhs.toString()); }

    bool operator>(const Obj &rhs) const {
      switch(this->o_type()) {
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
      switch(this->o_type()) {
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

    Obj operator/(const Obj &rhs) const {
      switch(this->o_type()) {
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

    Obj operator-(const Obj &rhs) const {
      switch(this->o_type()) {
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
          throw fError("%s can not be subtracted (-)", OTypes.to_chars(this->o_type()).c_str());
      }
    }

    [[nodiscard]] bool equals(const Obj &other) const {
      if(this->otype_ != other.otype_ || !this->tid_->equals(*other.tid_))
        return false;
      switch(this->otype_) {
        case OType::NOOBJ: return true;
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
        case OType::INST: return this->toString() == other.toString(); // TODO: Tuple equality
        default: throw fError("Unknown obj type in ==: %s", OTypes.to_chars(this->o_type()).c_str());
      }
    }


    bool operator!=(const Obj &other) const { return !(*this == other); }

    bool operator==(const Obj &other) const {
      return this->equals(other);
    }

    /*[[nodiscard]] Obj operator[](const Obj &key) const {
      switch(this->o_type()) {
        case OType::STR:
          return *this->str_get(share(key));
        case OType::LST:
          return *this->lst_get(share(key));
        case OType::REC:
          return *this->rec_get(share(key));
        default:
          throw fError("Unknown obj type in []: %s", OTypes.to_chars(this->o_type()).c_str());
      }
    }*/

    [[nodiscard]] Obj_p operator[](const char *id) const { return this->rec_get(id_p(id)); }

    [[nodiscard]] bool is_type() const { return !this->value_.has_value() && this->otype_ == OType::OBJ; }

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

    [[no_discard]] bool is_indexed_arg() const {
      if(!this->is_uri()) return false;
      const string s = this->uri_value().toString();
      return s[0] == '_' && StringHelper::is_integer(s.substr(1));
    }


    [[no_discard]] bool is_indexed_args() const {
      if(this->is_inst())
        return this->inst_args()->is_indexed_args();
      if(!this->is_rec())
        throw TYPE_ERROR(this, __FUNCTION__, __LINE__);
      int counter = 0;
      for(const auto &[k,v]: *this->rec_value()) {
        // LOG(INFO, "%s == %s : %i", k->uri_value().toString().c_str(), to_string(counter).insert(0,"_").c_str(),
        //    k->uri_value().toString() == to_string(counter).insert(0,"_"));
        if(!(k->is_uri() &&
             k->uri_value().toString() == to_string(counter++).insert(0, "_")))
          return false;
      }
      return true;
    }

    /////////////////////////////////////////////////////////////////
    //////////////////////////// APPLY //////////////////////////////
    /////////////////////////////////////////////////////////////////

    auto operator()(const Obj_p &obj, const InstArgs &args) {
      return this->apply(obj, args);
    }

    Obj_p apply(const Obj_p &lhs, const InstArgs &args) {
      ROUTER_PUSH_FRAME("+", args);
      const Obj_p result = this->apply(lhs);
      ROUTER_POP_FRAME();
      return result;
    }

    Obj_p apply() {
      return this->apply(Obj::to_noobj());
    }

    Obj_p apply(const Obj_p &lhs) {
      if(lhs->is_error())
        return lhs;
      //  if(!lhs->is_bcode() || !this->is_bcode()) {
      //   return shared_from_this()->apply(lhs->apply(shared_from_this()));
      //}
      if(lhs->is_type() && this->is_code()) {
        TYPE_CHECKER(this, lhs->tid(), true);
        return this->range() == OBJ_FURI ? lhs : Obj::to_type(this->range());
      }
      switch(this->o_type()) {
        case OType::OBJ: // type token
          return lhs->is_noobj() ? shared_from_this() : lhs->as(this->tid());
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
            new_values->push_back(obj->apply(lhs));
          }
          return Obj::to_lst(new_values);
        }
        case OType::REC: {
          const auto new_pairs = make_shared<RecMap<>>();
          for(const auto &[key, value]: *this->rec_value()) {
            if(const Obj_p key_apply = key->apply(lhs); !key_apply->is_noobj())
              new_pairs->insert({key_apply, value/*->apply(key_apply)*/});
          }
          return Obj::to_rec(new_pairs, this->tid_);
        }
        //case OType::BCODE: {
        //  return lhs->is_bcode() ? this->shared_from_this() : this->apply(lhs);
        //}
        case OType::INST: {
          //// dynamically fetch inst implementation if no function body exists (stub inst)
          const Inst_p inst = TYPE_INST_RESOLVER(lhs, this->shared_from_this());
          if(!lhs->is_code())
            TYPE_CHECKER(lhs.get(), inst->domain(), true);
          // compute args
          InstArgs remake;
          if(this->inst_op() == "block" ||
             this->inst_op() == "each" ||
             this->inst_op() == "within") {
            //// don't evaluate args for block()-inst -- TODO: don't have these be a 'special inst'
            remake = inst->inst_args()->clone();
          } else {
            remake = Obj::to_inst_args();
            //// apply lhs to args
            for(const auto &[k,v]: *inst->inst_args()->rec_value()) {
              remake->rec_value()->insert({k, v->apply(lhs)});
            }
          }
          ROUTER_PUSH_FRAME("+", remake);
          //// TODO: type check lhs-based on inst type_id domain
          //// TODO: don't evaluate inst for type objs for purpose of compilation
          try {
            if(nullptr == inst->inst_f())
              throw fError("!runable to resolve!! %s relative to !b%s!g[!!%s!g]!!", inst->toString().c_str(),
                           lhs->tid_->name().c_str(),
                           lhs->toString().c_str());
            const Obj_p result = inst->inst_f()->pure
                                   ? (*inst->inst_f()->obj_f())(lhs, remake)
                                   : (*inst->inst_f()->cpp_f())(lhs, remake);
            if(!result->is_code())
              TYPE_CHECKER(result.get(), inst->range(), true);
            ROUTER_POP_FRAME();
            return result;
          } catch(std::exception &e) {
            ROUTER_POP_FRAME(); // TODO: does this clear all frames automatically through exception recurssion?
            throw fError("%s\n\t\t!rthrown at !yinst!!  %s !g=>!! %s", e.what(),
                         lhs->toString().c_str(),
                         this->toString().c_str());
          }
        }
        case OType::BCODE: {
          return BCODE_PROCESSOR(this->shared_from_this()->bcode_starts(to_objs({lhs})))->none_one_all();
        }
        case OType::OBJS: {
          Objs_p objs = Obj::to_objs();
          for(const Obj_p &obj: *this->objs_value()) {
            objs->add_obj(obj->apply(lhs));
          }
          return objs;
        }
        default:
          throw fError("unknown obj type in apply(): %s", OTypes.to_chars(this->o_type()).c_str());
      }
    }

    [[nodiscard]]
    bool is_base_type() const { return this->tid_->equals(*OTYPE_FURI.at(this->otype_)); }

    [[nodiscard]] bool match(const Obj_p &type_obj, const bool require_same_type_id = true) const {
      // LOG(TRACE, "!ymatching!!: %s ~ %s\n", this->toString().c_str(), type->toString().c_str());
      if(type_obj->is_noop_bcode())
        return true;
      if(type_obj->is_bcode() && !this->is_bcode())
        return !type_obj->apply(this->clone())->is_noobj();
      if(!type_obj->value_.has_value() &&
         (type_obj->tid()->equals(*OBJ_FURI) || FURI_OTYPE.at(*type_obj->tid()) == this->otype_))
        return true;
      if(this->o_type() != type_obj->o_type())
        return false;
      if(require_same_type_id && (*this->tid_ != *type_obj->tid_))
        return false;

      switch(this->o_type()) {
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
          if(this->itype() != type_obj->itype())
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
          throw fError("Unknown obj type in match(): %s", OTypes.to_chars(this->o_type()).c_str());
      }
    }

    [[nodiscard]] Obj_p as(const ID_p &type_id) const {
      Obj_p obj = TYPE_MAKER(make_shared<Obj>(*this), type_id); // TODO: expensive.
      return obj;
    }

    Obj_p as(const char *furi) const {
      return this->as(id_p(furi));
    }

    Obj_p at(const ID_p &value_id) const {
      return Obj::create(this->value_, this->otype_, this->tid_, value_id);
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

    /// STATIC TYPE CONSTRAINED CONSTRUCTORS
    static Obj_p to_type(const ID_p &type_id) {
      return Obj::create(Any(), OType::OBJ, type_id);
    }

    static Obj_p to_noobj() {
      static auto noobj = Obj::create(Any(nullptr), OType::NOOBJ, NOOBJ_FURI);
      return noobj;
    }

    static Obj_p to_obj() {
      static auto noobj = Obj::create(Any(nullptr), OType::OBJ, OBJ_FURI);
      return noobj;
    }

    static Bool_p to_bool(const bool value, const ID_p &typed_id = BOOL_FURI, const ID_p &value_id = nullptr) {
      return Obj::create(value, OType::BOOL, typed_id, value_id);
    }

    static Int_p to_int(const FOS_INT_TYPE value, const ID_p &type_id = INT_FURI, const ID_p &value_id = nullptr) {
      return Obj::create(value, OType::INT, type_id, value_id);
    }

    static Real_p to_real(const FOS_REAL_TYPE value, const ID_p &furi = REAL_FURI) {
      return Obj::create(value, OType::REAL, furi);
    }

    static Str_p to_str(const string &value, const ID_p &furi = STR_FURI) {
      return Obj::create(value, OType::STR, furi);
    }

    static Str_p to_str(const char *value, const ID_p &furi = STR_FURI) {
      return Obj::create(string(value), OType::STR, furi);
    }

    static Uri_p to_uri(const fURI &value, const ID_p &furi = URI_FURI) {
      return Obj::create(value, OType::URI, furi);
    }

    static Uri_p to_uri(const char *value, const ID_p &furi = URI_FURI) {
      return Obj::create(fURI(value), OType::URI, furi);
    }

    static Lst_p to_lst(const ID_p &furi = LST_FURI) {
      return Obj::create(make_shared<LstList>(), OType::LST, furi);
    }

    static Lst_p to_lst(const LstList_p &xlst, const ID_p &furi = LST_FURI) {
      return Obj::create(xlst, OType::LST, furi);
    }

    static Lst_p to_lst(const std::initializer_list<Obj> &xlst, const ID_p &furi = LST_FURI) {
      const auto list = make_shared<LstList>();
      for(const auto &obj: xlst) {
        list->push_back(share(obj));
      }
      return to_lst(list, furi);
    }

    static Lst_p to_lst(const std::initializer_list<Obj_p> &xlst, const ID_p &furi = LST_FURI) {
      return to_lst(make_shared<LstList>(xlst), furi);
    }

    static Rec_p to_rec(const ID_p &type = REC_FURI, const ID_p &id = nullptr) {
      return Obj::create(make_shared<RecMap<>>(), OType::REC, type, id);
    }

    static Rec_p to_rec(const RecMap_p<> &map, const ID_p &type = REC_FURI, const ID_p &id = nullptr) {
      return Obj::create(map, OType::REC, type, id);
    }

    static Rec_p to_rec(const std::initializer_list<Pair<const Obj, Obj>> &xrec, const ID_p &type = REC_FURI,
                        const ID_p &id = nullptr) {
      const auto map = make_shared<Obj::RecMap<>>();
      for(const auto &[key, value]: xrec) {
        map->insert(make_pair(make_shared<Obj>(key), make_shared<Obj>(value)));
      }
      return to_rec(map, type, id);
    }

    static Rec_p to_rec(const std::initializer_list<Pair<const Obj_p, Obj_p>> &xrec, const ID_p &type = REC_FURI,
                        const ID_p &id = nullptr) {
      const auto map = make_shared<Obj::RecMap<>>();
      for(const auto &[key, value]: xrec) {
        map->insert(make_pair(key, value));
      }
      return to_rec(map, type, id);
    }

    static Rec_p to_rec(const std::initializer_list<Pair<const string, Obj_p>> &xrec, const ID_p &type = REC_FURI,
                        const ID_p &id = nullptr) {
      const auto map = make_shared<Obj::RecMap<>>();
      for(const auto &[key, value]: xrec) {
        map->insert(make_pair(Obj::to_uri(key), value));
      }
      return to_rec(map, type, id);
    }

    static Inst_p to_inst(const InstValue &value, const ID_p &type_id = INST_FURI, const ID_p &value_id = nullptr) {
      if(!std::get<0>(value)->is_rec())
        TYPE_ERROR(std::get<0>(value).get(),__FILE__,__LINE__);
      return Obj::create(value, OType::INST, type_id, value_id);
    }

    static InstArgs to_inst_args() {
      return Obj::to_rec();
    }

    static InstArgs to_inst_args(const List<Obj_p> &args) {
      const Rec_p inst_args = Obj::to_rec();
      for(size_t i = 0; i < args.size(); i++) {
        inst_args->rec_value()->insert({Obj::to_uri(string("_").append(to_string(i))), args.at(i)});
      }
      return inst_args;
    }

    static Inst_p to_inst(const std::initializer_list<Obj_p> &args, const ID_p &type_id,
                          const ID_p &value_id = nullptr) {
      return to_inst(type_id->name(), Obj::to_inst_args(args), nullptr, IType::ONE_TO_ONE, to_noobj(), type_id,
                     value_id);
    }

    static Inst_p to_inst(const List<Obj_p> &args, const ID_p &type_id, const ID_p &value_id = nullptr) {
      return to_inst(type_id->name(), Obj::to_inst_args(args), nullptr, IType::ONE_TO_ONE, to_noobj(), type_id,
                     value_id);
    }

    static Inst_p to_inst(const InstArgs &args, const ID_p &type_id, const ID_p &value_id = nullptr) {
      return to_inst(type_id->name(), args, nullptr, IType::ONE_TO_ONE, to_noobj(), type_id, value_id);
    }

    static Inst_p to_inst(const string &opcode, const InstArgs &args, const InstF_p &function,
                          const IType itype, const Obj_p &seed = Obj::to_noobj(), const ID_p &type_id = nullptr,
                          const ID_p &value_id = nullptr) {
      const ID_p fix = type_id != nullptr ? type_id : id_p(*ROUTER_RESOLVE(fURI(opcode)));
      return to_inst({args, function, itype, seed}, fix, value_id);
    }

    static BCode_p to_bcode(const InstList_p &insts, const ID_p &furi = BCODE_FURI) {
      return Obj::create(insts, OType::BCODE, furi);
    }

    static BCode_p to_bcode(const InstList &insts, const ID_p &furi = BCODE_FURI) {
      return Obj::to_bcode(make_shared<InstList>(insts), furi);
    }

    static BCode_p to_bcode(const ID_p &furi = BCODE_FURI) {
      return Obj::to_bcode(share<InstList>({}), furi);
    }

    static Objs_p to_objs(const ID_p &type_id = OBJS_FURI) {
      // fError::OTYPE_CHECK(furi->path(FOS_BASE_TYPE_INDEX), OTypes.to_chars(OType::OBJS));
      return to_objs(make_shared<List<Obj_p>>(), type_id);
    }

    static Objs_p to_objs(const List_p<Obj_p> &objs, const ID_p &type_id = OBJS_FURI) {
      // fError::OTYPE_CHECK(furi->path(FOS_BASE_TYPE_INDEX), OTypes.to_chars(OType::OBJS));
      return Obj::create(objs, OType::OBJS, type_id);
    }

    static Objs_p to_objs(const std::initializer_list<Obj_p> &objs, const ID_p &type_id = OBJS_FURI) {
      const auto list = make_shared<List<Obj_p>>();
      for(const auto &obj: objs) {
        list->push_back(obj);
      }
      return Obj::create(list, OType::OBJS, type_id);
    }

    static Error_p to_error(const Obj_p &obj, const Inst_p &inst, const ID_p &type_id = ERROR_FURI) {
      return Obj::create(Pair<Obj_p, Inst_p>(obj, inst), OType::ERROR, type_id);
    }

    /*std::__allocator_base<Obj> allocator = std::allocator<Obj>()*/
    Obj_p clone() const {
      const ID_p type_id_clone = id_p(*this->tid_);
      if(this->is_rec()) {
        // REC
        const auto new_map = make_shared<RecMap<>>();
        for(const auto &[k, v]: *this->rec_value()) {
          new_map->insert({k->clone(), v->clone()});
        }
        auto r = Rec::create(new_map, OType::REC, type_id_clone);
        r->vid_ = this->vid_;
        return r;
      }
      if(this->is_lst()) {
        // LST
        const auto new_list = make_shared<LstList>();
        for(const auto &e: *this->lst_value()) {
          new_list->push_back(e->clone());
        }
        auto r = Lst::create(new_list, OType::LST, type_id_clone);
        r->vid_ = this->vid_;
        return r;
      }
      if(this->is_inst()) {
        // INST
        Inst_p r = to_inst(string(this->inst_op()), this->inst_args()->clone(), this->inst_f(), this->itype(),
                           this->inst_seed_supplier(),
                           this->tid());
        r->vid_ = this->vid_;
        return r;
      }
      if(this->is_bcode()) {
        // BCODE
        InstList new_insts;
        for(const auto &inst: *this->bcode_value()) {
          new_insts.push_back(inst->clone());
        }
        BCode_p r = to_bcode(new_insts, type_id_clone);
        r->vid_ = this->vid_;
        return r;
      }
      if(this->is_objs()) {
        // OBJS
        const auto new_list = make_shared<List<Obj_p>>();
        for(const auto &e: *this->objs_value()) {
          new_list->push_back(e->clone());
        }
        auto r = Objs::create(new_list, OType::OBJS, type_id_clone);
        r->vid_ = this->vid_;
        return r;
      }
      auto r = Obj::create(any(this->value_), this->otype_, type_id_clone);
      r->vid_ = this->vid_;
      return r;
    }

    BObj_p serialize() const {
      static ObjPrinter *DEFAULT_SERIALIZATION_PRINTER = new ObjPrinter{.show_id = true, .show_type = true,
        .show_domain_range = true, .strict = true, .ansi = false, .propagate = true};
      LOG(DEBUG, "Serializing obj %s\n", this->toString().c_str());
      const string serial = this->toString(DEFAULT_SERIALIZATION_PRINTER);
      return ptr<BObj>(new BObj{serial.length(), reinterpret_cast<fbyte *>(strdup(serial.c_str()))}, bobj_deleter);
    }

    static Obj_p deserialize(const BObj_p &bobj) {
      LOG(DEBUG, "Deserializing bytes %s (length %i)\n", bobj->second, bobj->first);
      return OBJ_PARSER(string(reinterpret_cast<char *>(bobj->second), bobj->first));
    }
  };

  [[maybe_unused]] static Uri_p vri(const fURI &xuri, const ID_p &type = URI_FURI) { return Obj::to_uri(xuri, type); }

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

  [[maybe_unused]] static Str_p str(const char *xstr, const ID_p &type_id = STR_FURI) {
    return Obj::to_str(xstr, type_id);
  }

  [[maybe_unused]] static Str_p str(const string &xstr, const ID_p &type_id = STR_FURI) {
    return Obj::to_str(xstr, type_id);
  }

  [[maybe_unused]] static Real_p real(const FOS_REAL_TYPE &xreal, const ID_p &type_id = REAL_FURI) {
    return Obj::to_real(xreal, type_id);
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

  static Obj::RecMap_p<> rmap(const initializer_list<Pair<ID_p, Obj_p>> &pairs) {
    auto m = make_shared<Obj::RecMap<>>();
    for(const auto &[id, obj]: pairs) {
      m->insert({vri(id), obj});
    }
    return m;
  }

  static Obj_p from(const Uri_p &uri, const Obj_p &default_arg = noobj()) {
    return Obj::to_inst(
      "from", Obj::to_inst_args({uri, default_arg}),
      make_shared<InstF>([](const Uri_p &, const InstArgs &args) {
        const Obj_p result = ROUTER_READ(furi_p(args->arg(0)->uri_value()));
        return result->is_noobj() ? args->arg(1) : result;
      }),
      (uri->is_uri() && uri->uri_value().is_pattern()) ? IType::ONE_TO_MANY : IType::ONE_TO_ONE);
  }

  [[maybe_unused]] static Inst_p x(const uint8_t arg_num, const Obj_p &default_arg = noobj()) {
    return from(Obj::to_uri(string("_") + to_string(arg_num)), default_arg);
  }

  [[maybe_unused]] static Inst_p x(const uint8_t arg_num, const char *arg_name, const Obj_p &default_arg = noobj()) {
    return from(Obj::to_uri(ID(string("_") + to_string(arg_num)).query(arg_name)), default_arg);
  }

  static BCode_p ___ = Obj::to_bcode();
  static NoObj_p _noobj_ = Obj::to_noobj();
} // namespace fhatos
#endif
