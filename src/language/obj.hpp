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
  enum OType { URI, OBJ, BOOL, INT, REAL, STR, LST, REC, INST, BYTECODE }; // TYPE
  enum AType { MONOID, SEMIRING, GROUP, RING, FIELD };

  static const Map<OType, string> OTYPE_STR = {
    {
      {OBJ, "obj"},
      {URI, "uri"},
      {BOOL, "bool"},
      {INT, "int"},
      {REAL, "real"},
      {STR, "str"},
      {LST, "lst"},
      {REC, "rec"},
      {BYTECODE, "bcode"}
    }
  };

  class Obj {
  protected:
    OType _type;

  public:
    virtual ~Obj() = default;

    explicit Obj(const OType type) : _type(type) {
    }

    virtual const OType type() const { return _type; }

    virtual const string toString() const {
      return "obj";
    }

    Obj *obj() const {
      return (Obj *) this;
    }

    virtual const Obj *apply(const Obj *obj) const {
      return this;
    }

    bool operator<(const Obj &other) const {
      return true; //this->_value < other._value;
    }

    bool operator==(const Obj &other) const {
      return strcmp(this->toString().c_str(), other.toString().c_str()) == 0;
    }
  };

  ///////////////////////////////////////////////// BOOL //////////////////////////////////////////////////////////////
  class Uri final : public Obj {
  protected:
    const fURI _value;

  public:
    using ptr = std::shared_ptr<Uri>;

    Uri(const fURI value) : Obj(URI), _value(value) {
    }

    Uri(const string &value) : Uri(fURI(value)) {
    }

    const fURI value() const { return this->_value; }

    virtual const string toString() const override {
      return this->_value.toString();
    }
  };

  ///////////////////////////////////////////////// BOOL //////////////////////////////////////////////////////////////
  class Bool final : public Obj {
  protected:
    const bool _value;

  public:
    using ptr = std::shared_ptr<Bool>;

    Bool(const bool value) : Obj(BOOL), _value(value) {
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
    using ptr = std::shared_ptr<Int>;

    Int(const FL_INT_TYPE value) : Obj(INT), _value(value) {
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
    Real(const FL_REAL_TYPE value) : Obj(REAL), _value(value) {
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
    Str(const string &value) : Obj(STR), _value(value) {
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
    Lst(const List<Obj> &value) : Obj(LST), _value(value) {
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
      return obj->type() + obj->toString().length();
    }
  };

  struct obj_equal_to : std::binary_function<Obj *, Obj *, bool> {
    bool operator()(const Obj *a, const Obj *b) const {
      return true; // strcmp(a->toString().c_str(), b->toString().c_str()) == 0;
    }
  };

  template<typename K, typename V>
  using RecMap = UnorderedMap<K, V, obj_hash, obj_equal_to>;

  class Rec final : public Obj {
  protected:
    RecMap<Obj *, Obj *> *_value;

  public:
    Rec(RecMap<Obj *, Obj *> *value) : Obj(REC), _value(value) {
    };

    /*Rec(const std::initializer_list<Pair<const Obj*, const Obj*> > init) : Rec(Map<const Obj*, Obj*>(init.begin())) {
    };*/

    template<typename V>
    const V *get(Obj *key) const {
      return (V *) (this->_value->count(key) ? this->_value->at(key) : nullptr);
    }

    void set(Obj *key, Obj *val) {
      this->_value->emplace(key, val);
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
        t = t + "!g" + pair.first->toString() + "!! !r=>!! !g" + pair.second->toString() + "!!,";
      }
      t[t.length() - 1] = ']';
      return t;
    }
  };

  ///////////////////////////////////////////////// INST //////////////////////////////////////////////////////////////
  typedef Function<Obj *, Obj *> InstFunction;
  typedef List<Obj *> InstArgs;
  typedef List<Obj *> InstStorage;

  class Inst : public Obj {
  protected:
    const Triple<const string, const InstArgs, const InstFunction> _value;
    InstStorage input;
    InstStorage output;

  public:
    explicit Inst(const Triple<const string, const InstArgs, const InstFunction> &value)
      : Obj(INST), _value(value) {
    }

    virtual Triple<const string, const InstArgs, const InstFunction> value() const {
      return this->_value;
    }

    const Obj *apply(const Obj *obj) const override {
      return this->func()((Obj *) obj);
    }

    InstStorage *getOutput() {
      return &this->output;
    }

    Obj *next() {
      while (true) {
        if (output.empty()) {
          if (input.empty())
            return nullptr;
          else {
            output.push_back((Obj *) (this->apply(input.back())));
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
      string t = "[" + this->opcode() + ",";
      for (const auto *arg: this->args()) {
        t = t.append(arg->toString()).append(",");
      }
      t[t.length() - 1] = ']';
      return t;
    }

    ////////////////////////////////////
    virtual const string opcode() const { return std::get<0>(this->_value); }
    virtual const InstArgs args() const { return std::get<1>(this->_value); }

    template<typename A>
    A *arg(const uint8_t index) const { return (A *) this->args()[index]; }

    virtual const InstFunction func() const {
      return std::get<2>(this->_value);
    }
  };

  ///////////////////////////////////////////////// BYTECODE //////////////////////////////////////////////////////////////
  class Bytecode final : public Obj {
  protected:
    const List<Inst *> *_value;

  public:
    using ptr = std::shared_ptr<Bytecode>;
    const ID context;

    explicit Bytecode(const List<Inst *> *list, const ID context = ID("anonymous")) : Obj(BYTECODE), _value(list),
      context(context) {
    }

    explicit Bytecode(const ID context = ID("anonymous")) : Bytecode(new List<Inst *>, context) {
    }

    explicit Bytecode() : Bytecode(new List<Inst *>()) {
    }

    const Obj *apply(const Obj *obj) const override {
      Obj *running = (Obj *) obj;
      for (const Inst *inst: *this->_value) {
        running = inst->func()(running);
      }
      return running;
    }

    const List<Inst *> *value() const { return this->_value; }

    Bytecode *addInst(const char *op, const List<Obj *> &args,
                      const Function<Obj *, Obj *> &function) const {
      return this->addInst(new Inst({string(op), args, function}));
    }

    Bytecode *addInst(Inst *inst) const {
      List<Inst *> *list = new List<Inst *>();
      for (Inst *i: *this->_value) {
        list->push_back((Inst *) i);
      }
      list->push_back(inst);
      return new Bytecode(list, this->context); //auto ret = std::make_shared<Bytecode<S, E2>>(
      //return ret;
    }

    Inst *startInst() const {
      return this->value()->front();
    }

    const string toString() const override {
      string s = "{";
      for (auto *inst: *this->_value) {
        s = s + inst->toString();
      }
      return s + "}";
    }
  };
} // namespace fhatos

#endif
