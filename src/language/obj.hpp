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
#endif
#ifndef FL_INT_TYPE
#define FL_INT_TYPE int
#endif

#include <fhatos.hpp>
#include <structure/furi.hpp>
#include <util/uuid.hpp>
#include <utility>
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
  static const Map<const char *, OType> STR_OTYPE = {{{"noobj", OType::NOOBJ},
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

  using BObj = Triple<OType, fbyte *, uint32_t>;
  class PtrSerializer;
  /// An mm-ADT obj represented in C++ as a class

  class Type;

  class Obj {
  protected:
    OType _otype;
    ptr<Type> _type;

  public:
    virtual ~Obj() = default;

    explicit Obj(const OType otype, const ptr<Type> type = nullptr) : _otype(otype), _type(type) {}

    virtual const OType otype() const { return this->_otype; }
    virtual const ptr<Type> type() const { return this->_type; }

    virtual const string toString() const { return "obj"; }

    const Obj *obj() const { return (Obj *) this; }

    virtual const Obj *apply(const Obj *obj) const { return this; }

    template<typename A>
    const A *as(const ptr<Type> type = nullptr) const {
      if (!type)
        return (A *) this;
      return new A(*(A *) this, type);
    }

    virtual bool operator==(const Obj &other) const {
      return this->_otype == other._otype && this->_type == other._type &&
             strcmp(this->toString().c_str(), other.toString().c_str()) == 0;
    }

    virtual bool operator!=(const Obj &other) const { return !(*this == other); }

    bool isNoObj() const { return this->_otype == OType::NOOBJ; }

    template<typename SERIALIZER = PtrSerializer>
    const ptr<BObj> serialize() const {
      return SERIALIZER::singleton()->serialize(this);
    }

    /*template <typename _OBJ>
    static const _OBJ* deserialize(const BObj* bobj) {

    }*/
  };


  class Type : public Obj {
  protected:
    const fURI _value;

  public:
    explicit Type(const fURI &furi) : Obj(STR_OTYPE.at(furi.segment(0).c_str())), _value(furi) {}
    const OType otype() const { return STR_OTYPE.at(_value.segment(0).c_str()); }
    const string var() const { return this->_value.user().value_or(""); }
    const string name() const { return this->_value.retract(false).path(); }
    const string location() const { return this->_value.host(); }
    const bool mutating() const { return this->var()[0] == '~'; }
    const string objString(const string &objValue) const {
      return (this->var().empty() ? "" : (this->var() + "@")) +
             (this->name().empty() ? objValue : (this->name() + "[" + objValue + "]"));
    }
    const fURI value() const { return this->_value; }
  };

  class NoObj : public Obj {
  public:
    static const NoObj *singleton() {
      static NoObj no = NoObj();
      return &no;
    }

    NoObj() : Obj(OType::NOOBJ) {}

    const NoObj *apply(const Obj *obj) const override { return this; }

    const string toString() const override { return "Ø"; }

    bool operator==(const Obj &other) const override { return other.isNoObj(); }
  };

  ///////////////////////////////////////////////// OBJS
  /////////////////////////////////////////////////////////////////

  class Objs final : public Obj {
  protected:
    const List<const Obj *> *_value;

  public:
    Objs(const List<const Obj *> *value) : Obj(OType::OBJS), _value(value) {}

    const List<const Obj *> *value() const { return this->_value; }

    const Objs *apply(const Obj *obj) const override { return this; }

    const string toString() const override {
      string t = "(";
      for (const Obj *obj: *this->_value) {
        t = t + obj->toString() + ",";
      }
      t[t.length() - 1] = ')';
      return t;
    }
  };

  ///////////////////////////////////////////////// BOOL
  /////////////////////////////////////////////////////////////////
  class Uri final : public Obj {
  protected:
    fURI _value;

  public:
    Uri(const fURI &value, const ptr<Type> &utype = nullptr) : Obj(OType::URI, utype), _value(value) {}

    Uri(const char *&value, const ptr<Type> &utype = nullptr) : Obj(OType::URI, utype), _value(fURI(value)) {}

    // TODO: move to ObjHeper as mutating needs to publish RETAIN
    const Uri *split(const fURI &newValue) const { return new Uri(newValue, this->_type); }

    const fURI value() const { return this->_value; }

    const Uri *apply(const Obj *obj) const override { return this; }

    const string toString() const override { return this->_type->objString(this->_value.toString()); }

    const Uri *cast(const ptr<Type> &type) const { return *this->_type == *type ? this : new Uri(this->_value, type); }

    bool operator==(const Obj &other) const override {
      return this->_type == other.type() && *this->_type == *other.type() &&
             ((Uri *) &other)->value().equals(this->value());
    }

    bool operator<(const Uri &other) const { return this->_value < other._value; }
  };

  ///////////////////////////////////////////////// BOOL
  /////////////////////////////////////////////////////////////////
  class Bool final : public Obj {
  protected:
    const bool _value;

  public:
    Bool(const bool value, const ptr<Type> &type = nullptr]) : Obj(OType::BOOL, type), _value(value) {}

    const bool value() const { return this->_value; }

    const Bool *split(const bool newValue) const { return new Bool(newValue, this->_type); }

    const Bool *apply(const Obj *obj) const override { return this; }

    const string toString() const override { return this->_type->objString(this->_value ? "true" : "false"); }

    bool operator==(const Obj &other) const override {
      return this->_type == other.type() && *this->_type == *other.type() && this->_value == ((Bool *) &other)->value();
    }

    const Bool *cast(const ptr<Type> &type) const {
      return *this->_type == *type ? this : new Bool(this->_value, type);
    }
  };

  ///////////////////////////////////////////////// INT //////////////////////////////////////////////////////////////
  class Int : public Obj {
  protected:
    const FL_INT_TYPE _value;

  public:
    Int(const FL_INT_TYPE value, const ptr<Type> &type = nullptr) : Obj(OType::INT, type), _value(value) {}

    const FL_INT_TYPE value() const { return this->_value; }

    const Int *split(const FL_INT_TYPE newValue) const { return new Int(newValue, this->_type); }

    const Obj *apply(const Obj *obj) const override { return this; }

    bool operator==(const Obj &other) const override {
      return this->_type == other.type() && *this->_type == *other.type() && this->_value == ((Int *) &other)->value();
    }

    const string toString() const override { return this->_type->objString(std::to_string(this->_value)); }

    const Int *cast(const ptr<Type> &type) const { return *this->_type == *type ? this : new Int(this->_value, type); }

    bool operator<(const Int &other) const { return this->_value < other._value; }

    bool operator>(const Int &other) const { return this->_value > other._value; }
  };

  ///////////////////////////////////////////////// REAL
  /////////////////////////////////////////////////////////////////
  class Real final : public Obj {
  protected:
    const FL_REAL_TYPE _value;

  public:
    Real(const FL_REAL_TYPE value, const ptr<Type> &type = nullptr) : Obj(OType::REAL, type), _value(value){};
    const FL_REAL_TYPE value() const { return this->_value; }

    const Real *apply(const Obj *obj) const override { return this; }

    const Real *split(const FL_REAL_TYPE newValue) const { return new Real(newValue, this->_type); }

    bool operator==(const Obj &other) const override {
      return this->_type == other.type() && *this->_type == *other.type() && this->_value == ((Real *) &other)->value();
    }

    const string toString() const override { return this->_type->objString(std::to_string(this->_value)); }

    const Real *cast(const ptr<Type> &type) const {
      return *this->_type == *type ? this : new Real(this->_value, type);
    }

    bool operator<(const Real &other) const { return this->_value < other._value; }

    bool operator>(const Real &other) const { return this->_value > other._value; }
  };

  ///////////////////////////////////////////////// STR //////////////////////////////////////////////////////////////
  class Str final : public Obj {
  protected:
    const string _value;

  public:
    Str(const string &value, const ptr<Type> &utype = nullptr) : Obj(OType::STR, utype), _value(value){};

    Str(const char *value, const ptr<UType> &utype = NO_UTYPE) : Str(string(value), utype) {}

    const string value() const { return this->_value; }

    const Str *apply(const Obj *obj) const override { return this; }

    const Str *split(const string &newValue) const { return new Str(newValue, this->_utype); }

    const string toString() const override { return this->wrapUType(this->_value); }

    int compare(const Str &other) const { return this->_value.compare(other._value); }

    bool operator==(const Obj &other) const override {
      return this->_type == other.type() && this->_utype == other.utype() && this->_value == ((Str *) &other)->value();
    }

    bool operator<(const Str &other) const { return this->_value < other._value; }

    const Str *cast(const UType &utype) const {
      return *this->_utype == utype ? this : new Str(this->_value, share(utype));
    }
  };

  ///////////////////////////////////////////////// LST //////////////////////////////////////////////////////////////
  class Lst final : public Obj {
  protected:
    const List<Obj> _value;

  public:
    Lst(const List<Obj> &value, const ptr<UType> &utype = NO_UTYPE) : Obj(OType::LST, utype), _value(value){};
    const List<Obj> value() const { return _value; }

    const Lst *apply(const Obj *obj) const override { return this; }

    const string toString() const override {
      string t = "[";
      for (const Obj &element: this->_value) {
        t = t + element.toString() + ",";
      }
      t[t.length() - 1] = ']';
      return t;
    }
  };

  ///////////////////////////////////////////////// REC //////////////////////////////////////////////////////////////
  struct obj_hash {
    size_t operator()(const Obj *obj) const {
      return static_cast<std::string::value_type>(obj->type()) ^
             (obj->utype() ? obj->utype()->toString().size() : 123) ^ obj->toString().length() ^ obj->toString()[0];
    }
  };

  struct obj_equal_to : std::binary_function<Obj *, Obj *, bool> {
    bool operator()(const Obj *a, const Obj *b) const { return *a == *b; }
  };

  template<typename K = Obj *, typename V = Obj *, typename H = obj_hash, typename Q = obj_equal_to>
  using RecMap = OrderedMap<K, V, H, Q>;

  class Rec final : public Obj {
  protected:
    RecMap<Obj *, Obj *> *_value;

  public:
    Rec(RecMap<Obj *, Obj *> *value, const ptr<UType> &utype = NO_UTYPE) : Obj(OType::REC, utype), _value(value){};

    Rec(RecMap<Obj *, Obj *> value, const ptr<UType> &utype = NO_UTYPE) :
        Obj(OType::REC, utype), _value(new RecMap<Obj *, Obj *>(std::move(value))){};

    Rec(const std::initializer_list<Pair<Obj *const, Obj *>> &keyValues, const ptr<UType> &utype = NO_UTYPE) :
        Obj(OType::REC, utype), _value(new RecMap<Obj *, Obj *>()) {
      for (const Pair<Obj *const, Obj *> pair: keyValues) {
        this->_value->insert(pair);
      }
    }

    /*Rec(const std::initializer_list<Pair<Obj const, Obj>>& keyValues, const ptr<UType>& utype = NO_UTYPE) :
        Obj(OType::REC, utype), _value(new RecMap<Obj *, Obj *>()) {
      for (const Pair<Obj const, Obj>& pair: keyValues) {
        this->_value->emplace(&pair.first, &pair.second);
      }
    }*/

    template<typename V>
    const V *get(Obj *key) const {
      return (V *) (this->_value->count(key) ? this->_value->at(key) : NoObj::singleton());
    }

    void set(Obj *key, Obj *val) { this->_value->insert({key, val}); }

    const Rec *apply(const Obj *obj) const override { return this; }

    const RecMap<Obj *, Obj *> *value() const { return this->_value; }

    bool operator==(const Obj &other) const override {
      if (this->_type != other.type() || this->_utype != other.utype())
        return false;
      if (this->_value->size() != ((Rec *) &other)->value()->size())
        return false;
      auto itB = ((Rec *) &other)->value()->begin();
      for (auto itA = this->_value->begin(); itA != this->_value->end(); ++itA) {
        if (*itA->first != *itB->first || *itA->second != *itB->second)
          return false;
        ++itB;
      }
      return true;
    }

    const string toString() const override {
      string t = "!m[!!";
      for (const auto &[k, v]: *this->_value) {
        t = t + k->toString() + "!r=>!!" + v->toString() + ",";
      }
      t[t.length() - 1] = '!';
      return this->wrapUType(t.append("m]!!"));
    }

    const Rec *cast(const UType &utype) const {
      return *this->_utype == utype ? this : new Rec(this->_value, share(utype));
    }
  };

  ///////////////////////////////////////////////// INST
  /////////////////////////////////////////////////////////////////
  enum class IType : uint8_t {
    NOINST = 0,
    ONE_TO_ONE,
    ONE_TO_MANY,
    MANY_TO_ONE,
    MANY_TO_MANY,
  }; // TYPE
  static const Map<IType, const char *> ITYPE_STR = {{
      {IType::NOINST, "noinst"},
      {IType::ONE_TO_ONE, "f(x)->y"},
      {IType::ONE_TO_MANY, "f(x)->y*"},
      {IType::MANY_TO_ONE, "f(x*)->y"},
      {IType::MANY_TO_MANY, "f(x*)->y*"},
  }

  };

  typedef Function<const Obj *, const Obj *> InstFunction;
  typedef Function<const Obj *, const Obj *> OneToOneFunction;
  typedef Function<const Obj *, const Objs *> OneToManyFunction;
  typedef Function<const Objs *, const Obj *> ManyToOneFunction;
  typedef Function<const Objs *, const Objs *> ManyToManyFunction;

  typedef List<Obj *> InstArgs;
  typedef string InstOpcode;
  typedef Pair<const InstOpcode, const InstArgs> InstPair;

  class Bytecode;

  class Inst : public Obj {
  protected:
    const InstPair _value;
    const IType _itype;
    Bytecode *_bcode = nullptr;

  public:
    explicit Inst(const InstOpcode &opcode, const InstArgs &args, const IType itype = IType::NOINST) :
        Obj(OType::INST), _value({opcode, args}), _itype(itype) {}

    bool isNoInst() const { return this->opcode() == "noinst"; }

    Inst *bcode(Bytecode *bcode) {
      this->_bcode = bcode;
      return this;
    }

    Bytecode *bcode() const { return this->_bcode; }

    virtual InstPair value() const { return this->_value; }

    const IType itype() const { return this->_itype; }

    virtual const Obj *apply(const Obj *obj) const override { return NoObj::singleton(); }

    const string toString() const override {
      string t = "!g[!!!b" + this->opcode() + "!! ";
      for (const auto arg: this->args()) {
        t = t + arg->toString() + ",";
      }
      t[t.length() - 1] = '!';
      return this->wrapUType(t.append("g]!!"));
    }

    ////////////////////////////////////
    const InstOpcode opcode() const { return std::get<0>(this->_value); }
    const InstArgs args() const { return std::get<1>(this->_value); }
    const Obj *arg(const uint8_t index) const { return this->args()[index]; }
  };

  //////// NO INST
  class NoInst final : public Inst {
  public:
    static const Inst *singleton() {
      static auto noInst = NoInst();
      return &noInst;
    }

  private:
    NoInst() : Inst("noinst", {}) {}
  };

  /////// ONE TO ONE
  class OneToOneInst : public Inst {
  protected:
    OneToOneFunction function;

  public:
    OneToOneInst(const InstOpcode &opcode, const InstArgs &args, OneToOneFunction function) :
        Inst(opcode, args, IType::ONE_TO_ONE), function(std::move(function)) {}

    const Obj *apply(const Obj *obj) const { return this->function(obj); }
  };

  /////// MANY TO ONE
  class ManyToOneInst : public Inst {
  protected:
    ManyToOneFunction function;

  public:
    ManyToOneInst(const InstOpcode &opcode, const InstArgs &args, ManyToOneFunction function) :
        Inst(opcode, args, IType::MANY_TO_ONE), function(std::move(function)) {}

    const Obj *apply(const Obj *obj) const override {
      assert(obj->type() == OType::OBJS);
      return this->function((const Objs *) obj);
    }
  };

  /////// ONE TO MANY
  class OneToManyInst : public Inst {
  protected:
    OneToManyFunction function;

  public:
    OneToManyInst(const InstOpcode &opcode, const InstArgs &args, OneToManyFunction function) :
        Inst(opcode, args, IType::ONE_TO_MANY), function(std::move(function)) {}

    const Objs *apply(const Obj *obj) const override { return this->function(obj); }
  };

  /////// ONE TO MANY
  class ManyToManyInst : public Inst {
  protected:
    ManyToManyFunction function;

  public:
    ManyToManyInst(const InstOpcode &opcode, const InstArgs &args, ManyToManyFunction function) :
        Inst(opcode, args, IType::MANY_TO_MANY), function(std::move(function)) {}

    const Objs *apply(const Obj *obj) const override { return this->function((Objs *) obj); }
  };

  ///////////////////////////////////////////////// BYTECODE
  /////////////////////////////////////////////////////////////////
  class Bytecode final : public IDed, public Obj {
  protected:
    const List<Inst *> *_value;
    Map<const fURI, const ptr<const Obj>> TYPE_CACHE;

  public:
    //~Bytecode() override {}
    explicit Bytecode(const List<Inst *> *list, const ID &id = ID(*UUID::singleton()->mint(7))) :
        IDed(id), Obj(OType::BYTECODE), _value(list) {
      for (auto *inst: *this->_value) {
        inst->bcode(this);
      }
    }

    explicit Bytecode(const ID &id = ID(*UUID::singleton()->mint(7))) : Bytecode(new List<Inst *>, id) {}

    void setId(const ID &id) {
      this->_id = id;
      for (const auto *inst: *this->value()) {
        for (const auto *arg: inst->args()) {
          if (arg->type() == OType::BYTECODE) {
            ((Bytecode *) arg)->setId(id);
          }
        }
      }
    }

    const Uri *relativeUri(const Uri *uri) const {
      if (uri->value().empty())
        return new Uri(this->id());
      if (uri->value().toString()[0] == ':')
        return new Uri(this->id().toString() + uri->value().toString());
      return uri;
    }

    const Inst *nextInst(const Inst *currentInst) const {
      if (currentInst->isNoInst())
        return currentInst;
      bool found = false;
      for (const Inst *inst: *this->value()) {
        if (found)
          return inst;
        if (inst == currentInst)
          found = true;
      }
      return NoInst::singleton();
    }

    const Obj *apply(const Obj *obj) const override {
      auto currentObj = const_cast<Obj *>(obj);
      if (currentObj->isNoObj())
        return currentObj;
      for (const Inst *inst: *this->_value) {
        currentObj = (Obj *) inst->apply(currentObj);
        if (currentObj->isNoObj())
          break;
      }
      return (currentObj->type() == OType::URI) ? relativeUri((Uri *) currentObj) : currentObj;
    }

    const List<Inst *> *value() const { return this->_value; }

    ptr<Bytecode> addInst(Inst *inst) const {
      auto *list = new List<Inst *>();
      const ptr<Bytecode> bcode = share(Bytecode(list, this->id()));
      for (const auto prevInst: *this->_value) {
        list->push_back(prevInst->bcode(bcode.get()));
      }
      list->push_back(inst->bcode(bcode.get()));
      return bcode;
    }

    template<typename ROUTER>
    void createType(const fURI &type, const ptr<const Obj> typeDefinition) {
      if (!ROUTER::singleton()->write(typeDefinition, this->id(), type)) {
        this->TYPE_CACHE.emplace(type, typeDefinition);
      }
    }

    template<typename ROUTER>
    const ptr<const Obj> getType(const fURI &type) {
      if (this->TYPE_CACHE.count(type)) {
        LOG(DEBUG, "Bytecode type cache hit: %s\n", type.toString().c_str());
        return this->TYPE_CACHE.at(type);
      }
      const ptr<const Obj> typeDefinition = ROUTER::singleton()->read(this->id(), type);
      if (typeDefinition) {
        this->TYPE_CACHE.emplace(type, typeDefinition);
      }
      return typeDefinition;
    }

    template<typename ROUTER>
    bool match(const Obj *obj, const fURI &type) const {
      return !((Obj *) this->getType<ROUTER>(type).apply(obj))->isNoObj();
    }

    const Inst *startInst() const { return this->value()->front(); }

    const Inst *endInst() const { return this->value()->back(); }

    const string toString() const override {
      string s;
      for (const auto *inst: *this->_value) {
        s.append(inst->toString());
      }
      return this->wrapUType(s);
    }
  };

  template<typename A>
  static const A *cast(const A *obj, const UType &utype) {
    switch (obj->type()) {
      case OType::BOOL:
        return *obj->utype() == utype ? obj : new Bool(((Bool *) obj)->value(), share(utype));
      case OType::INT:
        return *obj->utype() == utype ? obj : new Int(((Int *) obj)->value(), share(utype));
      case OType::REAL:
        return *obj->utype() == utype ? obj : new Real(((Real *) obj)->value(), share(utype));
      case OType::URI:
        return *obj->utype() == utype ? obj : new Uri(((Uri *) obj)->value(), share(utype));
      case OType::STR:
        return *obj->utype() == utype ? obj : new Str(((Str *) obj)->value(), share(utype));
      case OType::REC:
        return *obj->utype() == utype ? obj : new Rec(*((Rec *) obj)->value(), share(utype));
      // case OType::BYTECODE: return new Bytecode(new List<Inst*>(*((Bytecode *) obj)->value()), share(newUType));
      default:
        throw fError("Casting %s currently not supported\n", OTYPE_STR.at(obj->type()));
    }
  }

  /////////////////////////////////////////// UNIONS ///////////////////////////////////////////
  struct OBJ_OR_BYTECODE {
  public:
    virtual ~OBJ_OR_BYTECODE() = default;

    union OBJ_UNION {
      const Obj *objA;
      const Bytecode *bcodeB;
    };

    const OType _type;
    const OBJ_UNION data;

    bool isBytecode() const { return this->_type == OType::BYTECODE; }

    OBJ_OR_BYTECODE(const Obj *objA) :
        _type(objA->type()),
        data(objA->type() == OType::BYTECODE ? OBJ_UNION{.bcodeB = new Bytecode(*(Bytecode *) objA)}
                                             : OBJ_UNION{.objA = objA}) {}

    OBJ_OR_BYTECODE(const NoObj *objA) : _type(objA->type()), data(OBJ_UNION{.objA = objA}) {}

    template<class T, class = typename std::enable_if_t<std::is_same_v<bool, T>>>
    OBJ_OR_BYTECODE(const T &boolX) : _type(OType::BOOL), data(OBJ_UNION{.objA = new Bool(boolX)}) {}

    OBJ_OR_BYTECODE(const FL_INT_TYPE &intX) : _type(OType::INT), data(OBJ_UNION{.objA = new Int(intX)}) {}

    OBJ_OR_BYTECODE(const FL_REAL_TYPE &realX) : _type(OType::REAL), data(OBJ_UNION{.objA = new Real(realX)}) {}

    OBJ_OR_BYTECODE(const char *strX) : _type(OType::STR), data(OBJ_UNION{.objA = new Str(strX)}) {}

    OBJ_OR_BYTECODE(const string &strX) : _type(OType::STR), data(OBJ_UNION{.objA = new Str(strX)}) {}

    OBJ_OR_BYTECODE(const fURI &uriX) : _type(OType::URI), data(OBJ_UNION{.objA = new Uri(uriX)}) {}

    OBJ_OR_BYTECODE(const Bool *objA) : _type(OType::BOOL), data(OBJ_UNION{.objA = objA}) {}

    OBJ_OR_BYTECODE(const Int *objA) : _type(OType::INT), data(OBJ_UNION{.objA = objA}) {}

    OBJ_OR_BYTECODE(const Real *objA) : _type(OType::REAL), data(OBJ_UNION{.objA = objA}) {}

    OBJ_OR_BYTECODE(const Str *objA) : _type(OType::STR), data(OBJ_UNION{.objA = objA}) {}

    OBJ_OR_BYTECODE(const Uri *objA) : _type(OType::URI), data(OBJ_UNION{.objA = objA}) {}

    OBJ_OR_BYTECODE(const Rec *objA) : _type(OType::REC), data(OBJ_UNION{.objA = objA}) {}

    OBJ_OR_BYTECODE(const Bytecode *bcodeB) : _type(OType::BYTECODE), data(OBJ_UNION{.bcodeB = bcodeB}) {}

    OBJ_OR_BYTECODE(const Objs *objsA) : _type(OType::OBJS), data(OBJ_UNION{.objA = objsA}) {}

    OBJ_OR_BYTECODE(const Bool &objA) : _type(OType::BOOL), data(OBJ_UNION{.objA = new Bool(objA)}) {}

    OBJ_OR_BYTECODE(const Int &objA) : _type(OType::INT), data(OBJ_UNION{.objA = new Int(objA)}) {}

    OBJ_OR_BYTECODE(const Real &objA) : _type(OType::REAL), data(OBJ_UNION{.objA = new Real(objA)}) {}

    OBJ_OR_BYTECODE(const Str &objA) : _type(OType::STR), data(OBJ_UNION{.objA = new Str(objA)}) {}

    OBJ_OR_BYTECODE(const Uri &objA) : _type(OType::URI), data(OBJ_UNION{.objA = new Uri(objA)}) {}

    OBJ_OR_BYTECODE(const Rec &objA) : _type(OType::REC), data(OBJ_UNION{.objA = new Rec(objA)}) {}

    OBJ_OR_BYTECODE(const RecMap<Obj *, Obj *> &objA) : _type(OType::REC), data(OBJ_UNION{.objA = new Rec(objA)}) {}

    OBJ_OR_BYTECODE(const std::initializer_list<Pair<OBJ_OR_BYTECODE const, OBJ_OR_BYTECODE>> keyValues) :
        _type(OType::REC), data(OBJ_UNION{.objA = new Rec({})}) {
      for (auto it = std::begin(keyValues); it != std::end(keyValues); ++it) {
        const_cast<RecMap<Obj *, Obj *> *>(((Rec *) this->data.objA)->value())
            ->insert({it->first.cast<>(), it->second.cast<>()});
      }
    }

    /*OBJ_OR_BYTECODE(const std::initializer_list<OBJ_OR_BYTECODE> objs) : _type(OType::OBJS), data(OBJ_UNION{.objA=new
    Objs({})}) { for(auto it =std::begin(objs); it!=std::end(objs); ++it) {
        ((List<Obj*>*)((Objs*)this->data.objA)->value())->push_back({it->cast<>()});
      }
    }*/

    OBJ_OR_BYTECODE(const Bytecode &bcodeB) : _type(OType::BYTECODE), data(OBJ_UNION{.bcodeB = new Bytecode(bcodeB)}) {}


    const OType type() const { return this->_type; }

    template<typename T = Obj>
    T *cast() const {
      return this->isBytecode() ? (T *) (data.bcodeB) : (T *) (data.objA);
    }

    const string toString() const { return this->cast()->toString(); }

    virtual const Obj *apply(const Obj *input) const {
      return (this->isBytecode() ? data.bcodeB->apply(input) : data.objA->apply(input));
    }

    virtual operator Obj *() const { return const_cast<Obj *>(this->isBytecode() ? data.bcodeB : data.objA); }
  };

  struct URI_OR_BYTECODE : public OBJ_OR_BYTECODE {
    URI_OR_BYTECODE(const Obj *uriX) : OBJ_OR_BYTECODE(uriX) {}

    URI_OR_BYTECODE(const Uri &uriX) : OBJ_OR_BYTECODE(new Uri(uriX)) {}

    URI_OR_BYTECODE(const fURI &uriX) : OBJ_OR_BYTECODE(new Uri(uriX)) {}

    URI_OR_BYTECODE(const string &uriX) : OBJ_OR_BYTECODE(new Uri(uriX)) {}

    URI_OR_BYTECODE(const char *uriX) : OBJ_OR_BYTECODE(new Uri(uriX)) {}

    const Uri *apply(const Obj *input) const override {
      return this->isBytecode() ? (Uri *) data.bcodeB->apply(input) : (Uri *) data.objA->apply(input);
    }
  };

  ////////////////////////////
  //////// SERIALIZER ////////
  ////////////////////////////

  class PtrSerializer {
    static PtrSerializer *singleton() {
      static PtrSerializer serializer = PtrSerializer();
      return &serializer;
    }

    static const ptr<BObj> serialize(const Obj *obj) {
      auto *bytes = static_cast<fbyte *>(malloc(sizeof(*obj)));
      memcpy(bytes, reinterpret_cast<const fbyte *>(obj), sizeof(*obj));
      LOG(DEBUG, "[serialization] %i bytes allocated for %s\n", sizeof(*obj), obj->toString().c_str());
      return share<BObj>({obj->type(), bytes, sizeof(*obj)});
    }

    template<typename OBJ>
    ptr<OBJ> deserialize(BObj bobj) {
      const fbyte *bytes = std::get<1>(bobj);
      LOG(DEBUG, "[deserialization] %i bytes retrieved for %s\n", std::get<2>(bobj), OTYPE_STR.at(std::get<0>(bobj)));
      return ptr<OBJ>((OBJ *) bytes);
    }
  };

} // namespace fhatos

#endif
