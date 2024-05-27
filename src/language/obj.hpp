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


namespace fhatos {
  enum class OType : uint8_t { NOOBJ = 0, URI, OBJ, BOOL, INT, REAL, STR, LST, REC, INST, BYTECODE }; // TYPE
  enum AType { MONOID, SEMIRING, GROUP, RING, FIELD };

  static const Map<OType, string> OTYPE_STR = {
    {
      {OType::NOOBJ, "noobj"},
      {OType::OBJ, "obj"},
      {OType::URI, "uri"},
      {OType::BOOL, "bool"},
      {OType::INT, "int"},
      {OType::REAL, "real"},
      {OType::STR, "str"},
      {OType::LST, "lst"},
      {OType::REC, "rec"},
      {OType::INST, "inst"},
      {OType::BYTECODE, "bcode"}
    }

  };

  using UType = string;

  class Obj {
  protected:
    OType _type;

  public:
    virtual ~Obj() = default;

    explicit Obj(const OType type) : _type(type) {
    }

    virtual OType type() const { return _type; }
    virtual UType utype() const { return OTYPE_STR.at(_type); }

    virtual const string toString() const {
      return "obj";
    }

    Obj *obj() const {
      return (Obj *) this;
    }

    virtual const Obj *apply(const Obj *obj) const {
      return this;
    }


    virtual bool operator==(const Obj &other) const {
      return !this->isNoObj() &&
             !other.isNoObj() &&
             strcmp(this->toString().c_str(), other.toString().c_str()) == 0;
    }

    bool isNoObj() const {
      return this->type() == OType::NOOBJ;
    }
  };

  class NoObj : public Obj {
  public:
    static NoObj *singleton() {
      static NoObj no = NoObj();
      return &no;
    }

    NoObj() : Obj(OType::NOOBJ) {
    }

    const NoObj *apply(const Obj *obj) const override {
      return this;
    }

    virtual const string toString() const override {
      return "Ø";
    }

    virtual bool operator==(const Obj &other) const override {
      return other.isNoObj();
    }
  };

  ///////////////////////////////////////////////// BOOL //////////////////////////////////////////////////////////////
  class Uri final : public Obj {
  protected:
    fURI _value;

  public:
    Uri() : Obj(OType::URI), _value(nullptr) {
    }

    Uri(const fURI &value) : Obj(OType::URI), _value(value) {
    }

    Uri(const string &value) : Obj(OType::URI), _value(fURI(value)) {
    }

    const fURI value() const { return this->_value; }

    const Uri *apply(const Obj *obj) const override {
      return this;
    }

    /* virtual bool isType() const {
       return this->_value == nullptr;
     }*/

    virtual const string toString() const override {
      return this->_value.toString();
    }
  };

  ///////////////////////////////////////////////// BOOL //////////////////////////////////////////////////////////////
  class Bool final : public Obj {
  protected:
    const bool _value;

  public:
    Bool(const bool value) : Obj(OType::BOOL), _value(value) {
    }

    const bool value() const { return this->_value; }

    const Bool *apply(const Obj *obj) const override {
      return this;
    }

    const string toString() const override {
      return this->_value ? "true" : "false";
    }
  };

  ///////////////////////////////////////////////// INT //////////////////////////////////////////////////////////////
  class Int : public Obj {
  protected:
    const FL_INT_TYPE _value;

  public:
    Int(const FL_INT_TYPE value) : Obj(OType::INT), _value(value) {
    }

    const FL_INT_TYPE value() const { return this->_value; }

    virtual const Int *apply(const Obj *obj) const override {
      return new Int(this->_value);
    }

    const string toString() const override { return std::to_string(_value); }
  };

  ///////////////////////////////////////////////// REAL //////////////////////////////////////////////////////////////
  class Real final : public Obj {
  protected:
    const FL_REAL_TYPE _value;

  public:
    Real(const FL_REAL_TYPE value) : Obj(OType::REAL), _value(value) {
    };
    const FL_REAL_TYPE value() const { return this->_value; }

    const Real *apply(const Obj *obj) const override {
      return this;
    }

    const string toString() const override { return std::to_string(_value); }
  };

  ///////////////////////////////////////////////// STR //////////////////////////////////////////////////////////////
  class Str final : public Obj {
  protected:
    const string _value;

  public:
    Str(const string &value) : Obj(OType::STR), _value(value) {
    };
    const string value() const { return this->_value; }

    const Str *apply(const Obj *obj) const override {
      return this;
    }

    const string toString() const override { return _value; }

    int compare(const Str &other) const {
      return this->_value.compare(other._value);
    }

    bool operator<(const Str &other) const {
      return this->_value < other._value;
    }
  };

  ///////////////////////////////////////////////// LST //////////////////////////////////////////////////////////////
  class Lst final : public Obj {
  protected:
    const List<Obj> _value;

  public:
    Lst(const List<Obj> &value) : Obj(OType::LST), _value(value) {
    };
    const List<Obj> value() const { return _value; }

    const Lst *apply(const Obj *obj) const override {
      return this;
    }

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
      return static_cast<std::string::value_type>(obj->type()) * obj->toString().length() * obj->toString()[0];
    }
  };

  struct obj_equal_to : std::binary_function<Obj *, Obj *, bool> {
    bool operator()(const Obj *a, const Obj *b) const {
      return a->type() == b->type() && strcmp(a->toString().c_str(), b->toString().c_str()) == 0;
    }
  };

  template<typename K, typename V, typename H=obj_hash, typename Q=obj_equal_to>
  using RecMap = UnorderedMap<K, V, H, Q>;

  class Rec final : public Obj {
  protected:
    RecMap<Obj *, Obj *> *_value;

  public:
    Rec(RecMap<Obj *, Obj *> *value) : Obj(OType::REC), _value(value) {
    };

    template<typename V>
    const V *get(Obj *key) const {
      return (V *) (this->_value->count(key) ? this->_value->at(key) : NoObj::singleton());
    }

    void set(Obj *key, Obj *val) {
      this->_value->insert({key, val});
    }

    const Rec *apply(const Obj *obj) const override {
      return this;
    }

    RecMap<Obj *, Obj *> *value() const {
      return this->_value;
    }

    const string toString() const override {
      string t = "[";
      for (const auto &pair: *this->_value) {
        t = t + pair.first->toString() + "!r=>!!" + pair.second->toString() + ",";
      }
      t[t.length() - 1] = ']';
      return t;
    }
  };

  ///////////////////////////////////////////////// INST //////////////////////////////////////////////////////////////
  typedef Function<const Obj *, const Obj *> InstFunction;
  typedef List<Obj *> InstArgs;
  typedef List<Obj *> InstStorage;

  class Inst : public Obj {
  protected:
    const Triple<const string, const InstArgs, const InstFunction> _value;
    InstStorage input;
    InstStorage output;

  public:
    explicit Inst(const Triple<const string, const InstArgs, const InstFunction> &value)
      : Obj(OType::INST), _value(value) {
    }

    virtual Triple<const string, const InstArgs, const InstFunction> value() const {
      return this->_value;
    }

    const Obj *apply(const Obj *obj) const override {
      return this->func()(obj);
    }

    InstStorage *getOutput() {
      return &this->output;
    }

    Obj *next() {
      while (true) {
        if (output.empty()) {
          if (input.empty())
            return NoObj::singleton();
          else {
            output.push_back(const_cast<Obj *>(this->apply(input.back())));
            input.pop_back();
          }
        } else {
          Obj *obj = output.back();
          output.pop_back();
          return obj;
        }
      }
    }

    const string toString() const override {
      string t = "[!b" + this->opcode() + "!! ";
      for (const auto arg: this->args()) {
        t = t + arg->toString() + ",";
      }
      t[t.length() - 1] = ']';
      return t;
    }

    ////////////////////////////////////
    virtual const string opcode() const { return std::get<0>(this->_value); }
    virtual const InstArgs args() const { return std::get<1>(this->_value); }

    template<typename A>
    A *arg(const uint8_t index) const { return static_cast<A *>(this->args()[index]); }

    virtual const InstFunction func() const {
      return std::get<2>(this->_value);
    }
  };

  ///////////////////////////////////////////////// BYTECODE //////////////////////////////////////////////////////////////
  class Bytecode final : public Obj {
  protected:
    const List<Inst *> *_value;

  public:
    const ID context;

    explicit Bytecode(const List<Inst *> *list, const ID context = ID("anonymous")) : Obj(OType::BYTECODE),
      _value(list),
      context(context) {
    }

    explicit Bytecode(const ID context = ID("anonymous")) : Bytecode(new List<Inst *>, context) {
    }

    explicit Bytecode() : Bytecode(new List<Inst *>()) {
    }

    const Obj *apply(const Obj *obj) const override {
      Obj *running = const_cast<Obj *>(obj);
      for (const Inst *inst: *this->_value) {
        running = (Obj *) inst->func()(running);
        if (running->isNoObj())
          break;
      }
      return running;
    }

    const List<Inst *> *value() const { return this->_value; }

    ptr<Bytecode> addInst(const char *op, const List<Obj *> &args,
                          const Function<const Obj *, const Obj *> &function) const {
      return this->addInst(new Inst({string(op), args, function}));
    }

    ptr<Bytecode> addInst(Inst *inst) const {
      List<Inst *> *list = new List<Inst *>();
      for (const auto i: *this->_value) {
        list->push_back(i);
      }
      list->push_back(inst);
      return share<Bytecode>(Bytecode(list, this->context));
    }

    Inst *startInst() const {
      return this->value()->front();
    }

    template<typename S>
    void addStarts(const std::initializer_list<S> &starts) {
      for (const S &s: starts) {
        this->startInst()->args().push_back(s);
      }
    }

    const string toString() const override {
      string s = "{";
      for (const auto *inst: *this->_value) {
        s = s + inst->toString();
      }
      return s + "}";
    }
  };

  /////////////////////////////////////////// UNIONS ///////////////////////////////////////////
  struct OBJ_OR_BYTECODE {
    union OBJ_UNION {
      const Obj *objA;
      const Bytecode *bcodeB;
    };

    const OType _type;
    const OBJ_UNION data;

    bool isBytecode() const {
      return this->_type == OType::BYTECODE;
    }

    bool isType() const {
      return this->_type != OType::BYTECODE && data.objA == NoObj::singleton();
    }

    OBJ_OR_BYTECODE(const int objA) : _type(OType::INT), data(OBJ_UNION{.objA = new Int(objA)}) {
    }

    OBJ_OR_BYTECODE(const Obj *objA) : _type(objA->type()), data(OBJ_UNION{.objA = objA}) {
    }

    OBJ_OR_BYTECODE(const Bool *objA) : _type(objA->type()), data(OBJ_UNION{.objA = objA}) {
    }


    OBJ_OR_BYTECODE(const Int *objA) : _type(objA->type()), data(OBJ_UNION{.objA = objA}) {
    }


    OBJ_OR_BYTECODE(const Real *objA) : _type(objA->type()), data(OBJ_UNION{.objA = objA}) {
    }


    OBJ_OR_BYTECODE(const Str *objA) : _type(objA->type()), data(OBJ_UNION{.objA = objA}) {
    }


    OBJ_OR_BYTECODE(const Uri *objA) : _type(objA->type()), data(OBJ_UNION{.objA = objA}) {
    }


    OBJ_OR_BYTECODE(const Rec *objA) : _type(objA->type()), data(OBJ_UNION{.objA = objA}) {
    }


    OBJ_OR_BYTECODE(const Bytecode *bcodeB) : _type(OType::BYTECODE), data(OBJ_UNION{.bcodeB = bcodeB}) {
    }


    template<typename T = Obj>
    T *cast() const {
      return (T *) (this->isBytecode() ? (T *) data.bcodeB : (T *) data.objA);
    }

    const string toString() const {
      return this->cast()->toString();
    }

    const Obj *apply(const Obj *input) const {
      return (this->isBytecode() ? data.bcodeB->apply(input) : data.objA->apply(input));
    }
  };
} // namespace fhatos

#endif
