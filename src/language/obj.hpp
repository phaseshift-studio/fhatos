#ifndef fhatos_obj_hpp
#define fhatos_obj_hpp

#ifndef FL_REAL_TYPE
#define FL_REAL_TYPE float
#endif
#ifndef FL_INT_TYPE
#define FL_INT_TYPE int
#endif

#include <fhatos.hpp>

namespace fhatos {
  enum OType { URI, OBJ, BOOL, INT, REAL, STR, LST, REC, INST, BYTECODE }; // TYPE
  enum AType { MONOID, SEMIRING, GROUP, RING, FIELD };

  static const Map<OType, string> OTYPE_STR = {
    {
      {OBJ, "obj"},
      {BOOL, "bool"},
      {INT, "int"},
      {REAL, "real"},
      {STR, "str"},
      {LST, "lst"},
      {REC, "rec"}
    }
  };

  template<typename OBJ, AType atype>
  struct Algebra {
    const AType _atype;

    Algebra(const AType _atype) : _atype(_atype) {
    }

    static const Algebra<OBJ, atype> singleton() {
      const static Algebra<OBJ, atype> *instance = Algebra<OBJ, atype>(atype);
      return &instance;
    }
  };

  /*template<typename S, typename E>
  static const List<E *> ptr_list(const List<S> ts) {
    List<E *> pts = List<E *>();
    for (const auto &t: ts) {
      pts.push_back((E *) t.self());
    }
    return pts;
  }*/

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


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

    const Obj *obj() const {
      return (Obj *) this;
    }

    virtual const Obj *apply(const Obj *obj) const {
      return this;
    }

    bool operator<(const Obj &other) const {
      return true; //this->_value < other._value;
    }
  };

  ///////////////////////////////////////////////// BOOL //////////////////////////////////////////////////////////////
  /*class Uri final : public Obj<fURI> {
  public:
    Uri(const fURI value) : Obj(value) {
    }

    virtual OType type() const override { return URI; }

    virtual string toString() const override {
      return string(this->_value.toString().c_str());
    }
  };*/

  ///////////////////////////////////////////////// BOOL //////////////////////////////////////////////////////////////
  class Bool final : public Obj {
  protected:
    const bool _value;

  public:
    Bool(const bool value) : Obj(BOOL), _value(value) {
    }

    virtual const bool value() const { return this->_value; }

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
  class Rec final : public Obj {
  protected:
    const Map<const Obj, Obj> _value;

  public:
    Rec(const Map<const Obj, Obj> &value) : Obj(REC), _value(value) {
    };

    Rec(const std::initializer_list<Pair<const Obj, Obj> > init) : Obj(REC),
                                                                   _value(Map<const Obj, Obj>(init)) {
    };

    /* template <typename V>
     V get(const ObjZ &key) {
       return  this->_value[key];
     }

     void set(const ObjZ &key, const ObjZ &val) {
       return this->_value[key] = val;
     }*/

    const Rec *apply(const Obj *obj) const override {
      return this;
    }

    virtual const Map<const Obj, Obj> value() const {
      return this->_value;
    }

    const string toString() const override {
      string t = "[";
      for (const Pair<const Obj, Obj> &pair: this->_value) {
        t = t + pair.first.toString() + ":" + pair.second.toString() + ",";
      }
      t[t.length() - 1] = ']';
      return t;
    }
  };

  ///////////////////////////////////////////////// INST //////////////////////////////////////////////////////////////
  template<typename S, typename E>
  class Inst : public Obj {
  protected:
    const Triple<const string, const List<Obj *>, const Function<S *, E *>> _value;

  public:
    Inst(const Triple<const string, const List<Obj *>, const Function<S *, E *>> &value)
      : Obj(INST), _value(value) {
    }

    virtual Triple<const string, const List<Obj *>, const Function<S *, E *>> value() const {
      return this->_value;
    }

    static const string makeString(const string &opcode, const string arg1 = "", const string arg2 = "",
                                   const string arg3 = "") {
      string t = "[" + opcode + ",";
      if (!arg1.empty())
        t = t + arg1 + ",";
      if (!arg2.empty())
        t = t + arg2 + ",";
      if (!arg3.empty())
        t = t + arg3 + ",";
      t[t.length() - 1] = ']';
      return t;
    }

    const E *apply(const Obj *obj) const override {
      return this->func()((S *) obj);
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
    virtual const List<Obj *> args() const { return std::get<1>(this->_value); }

    template<typename A>
    A *arg(const uint8_t index) const { return (A *) this->args()[index]; }

    virtual const Function<S *, E *> func() const {
      return std::get<2>(this->_value);
    }
  };

  ///////////////////////////////////////////////// BYTECODE //////////////////////////////////////////////////////////////
  template<typename S, typename E>
  class Bytecode final : public Obj {
  protected:
    const List<Inst<S, E> *> _value;

  public:
    ~Bytecode() override {
      //  todo clear list and delete instructions
    }

    explicit Bytecode(const List<Inst<S, E> *> &list) : Obj(BYTECODE), _value(list) {
    }

    explicit Bytecode() : Obj(BYTECODE), _value(List<Inst<S, E> *>()) {
    }

    const E *apply(const Obj *obj) const override {
      const E *running = (E *) obj;
      for (const auto *inst: this->_value) {
        running = inst->func()((S *) running);
      }
      return running;
    }

    const List<Inst<S, E> *> value() const { return this->_value; }

    template<typename E2>
    Bytecode<S, E2> *addInst(const char *op, const List<Obj *> &args,
                             const Function<E *, E2 *> &function) const {
      return this->addInst(std::make_shared<Inst<E, E2> >(Inst<E, E2>({string(op), args, function})));
    }

    template<typename E2>
    Bytecode<S, E2> *addInst(const Inst<E, E2> *inst) const {
      List<Inst<S, E2> *> list;
      for (Inst<S, E> *i: this->_value) {
        list.push_back((Inst<S, E2> *) i);
      }
      list.push_back((Inst<S, E2> *) inst);
      return new Bytecode<S, E2>(list); //auto ret = std::make_shared<Bytecode<S, E2>>(
      //return ret;
    }

    const string toString() const override {
      string s = "{";
      for (auto *inst: this->_value) {
        s = s + inst->toString();
      }
      return s + "}";
    }
  };


} // namespace fhatos

#endif
