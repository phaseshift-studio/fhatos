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
  enum OType { OBJ, BOOL, INT, REAL, STR, LST, REC, INST, BYTECODE }; // TYPE
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

  template<typename T>
  static const List<T *> *ptr_list(const List<T> ts) {
    List<T *> *pts = new List<T *>();
    for (const auto &t: ts) {
      pts->push_back(new T(t));
    }
    return pts;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  template<typename DATA>
  class Obj {
  protected:
    const DATA _value;

    //~Obj() {
    //}

  public:
    explicit Obj(const DATA &value) : _value(value) {
    }

    virtual const DATA &value() const { return this->_value; }
    virtual OType type() const { return OBJ; }
    /*virtual const bool equals(const Obj &other) const {
      return this->type() == other.type() && this->value == other.get();
    }*/
    virtual string toString() const {
      return string((char *) ((void *) (&(this->_value))));
    }
    bool operator<(const Obj &other) const {
      return true; //this->_value < other._value;
    }

    static const Algebra<Obj<DATA>, RING> algebra() {
      return Algebra<Obj<DATA>, RING>::singleton();
    }
  };

  using ObjX = void *; // wildcard object
  using ObjY = Obj<ObjX> *;
  using ObjZ = Obj<ObjX>;

  ///////////////////////////////////////////////// BOOL //////////////////////////////////////////////////////////////
  class Bool final : public Obj<bool> {
  public:
    Bool(const bool value) : Obj(value) {
    }

    virtual OType type() const override { return BOOL; }

    virtual string toString() const override {
      return this->_value ? "true" : "false";
    }
  };

  ///////////////////////////////////////////////// INT //////////////////////////////////////////////////////////////
  class Int final : public Obj<FL_INT_TYPE> {
  public:
    Int(const FL_INT_TYPE value) : Obj(value) {
    }

    virtual OType type() const override { return INT; }
    virtual string toString() const override { return std::to_string(_value); }
  };

  ///////////////////////////////////////////////// REAL //////////////////////////////////////////////////////////////
  class Real final : public Obj<FL_REAL_TYPE> {
  public:
    Real(const FL_REAL_TYPE value) : Obj(value) {
    };
    virtual OType type() const override { return REAL; }
    virtual string toString() const override { return std::to_string(_value); }
  };

  ///////////////////////////////////////////////// STR //////////////////////////////////////////////////////////////
  class Str final : public Obj<string> {
  public:
    Str(const string &value) : Obj<string>(value) {
    };
    virtual OType type() const override { return STR; }
    virtual string toString() const override { return _value; }

    int compare(const Str &other) const {
      return this->_value.compare(other._value);
    }

    bool operator<(const Str &other) const {
      return this->_value < other._value;
    }
  };

  ///////////////////////////////////////////////// LST //////////////////////////////////////////////////////////////
  class Lst final : public Obj<List<ObjX> > {
  public:
    Lst(const List<ObjX> &value) : Obj<List<ObjX> >(value) {
    };
    virtual OType type() const override { return LST; }

    virtual string toString() const override {
      string t = "[";
      for (const ObjX &element: this->_value) {
        t = t + ((ObjY) element)->toString() + ",";
      }
      t[t.length() - 1] = ']';
      return t;
    }
  };

  ///////////////////////////////////////////////// REC //////////////////////////////////////////////////////////////
  class Rec final : public Obj<Map<const ObjZ, ObjZ> > {
  public:
    Rec(const Map<const ObjZ, ObjZ> &_value) : Obj<Map<const ObjZ, ObjZ> >(_value) {
    };

    Rec(const std::initializer_list<Pair<const ObjZ, ObjZ> > init) : Obj<Map<const ObjZ, ObjZ> >(Map<const ObjZ,ObjZ>(init)) {
    };

   /* template <typename V>
    V get(const ObjZ &key) {
      return  this->_value[key];
    }

    void set(const ObjZ &key, const ObjZ &val) {
      return this->_value[key] = val;
    }*/

    virtual OType type() const override { return REC; }

    virtual string toString() const override {
      string t = "[";
      for (const Pair<const ObjZ, ObjZ> &pair: this->_value) {
        t = t + pair.first.toString() + ":" + pair.second.toString() + ",";
      }
      t[t.length() - 1] = ']';
      return t;
    }
  };

  ///////////////////////////////////////////////// INST //////////////////////////////////////////////////////////////
  template<typename A, typename B>
  class Inst : public Obj<Triple<string, List<void *>, Function<A *, B *> > > {
  public:
    virtual ~Inst() {
    }

    Inst(const Triple<string, List<void *>, Function<A *, B *> > &_value)
      : Obj<Triple<string, List<void *>, Function<A *, B *> > >(_value) {
    }

    virtual OType type() const override { return INST; }

    virtual string toString() const override {
      string t = "[" + this->opcode() + ",";
      for (const ObjX &arg: this->args()) {
        t = t + ((ObjY) arg)->toString();
      }
      return t + "]";
    }

    ////////////////////////////////////
    virtual const string &opcode() const { return std::get<0>(this->_value); }
    virtual const List<ObjX> &args() const { return std::get<1>(this->_value); }

    virtual const Function<A *, B *> &func() const {
      return std::get<2>(this->_value);
    }

    virtual const B *apply(const A *a) const { return (const B *) this->func()((A *) a); }
  };

  ///////////////////////////////////////////////// BYTECODE //////////////////////////////////////////////////////////////
  template<typename S, typename E>
  class Bytecode final : public Obj<List<Inst<S, E> > > {
  public:
    virtual ~Bytecode() {
      //  todo clear list and delete instructions
    }

    explicit Bytecode() : Obj<List<Inst<S, E> > >(List<Inst<S, E> >()) {
    }

    explicit Bytecode(const List<Inst<S, E> > &list) : Obj<List<Inst<S, E> > >(list) {
    }

    explicit Bytecode(const std::initializer_list<Inst<S, E> > braces)
      : Obj<List<Inst<S, E> > >(List<Inst<S, E> >(braces)) {
    }

    virtual OType type() const override { return BYTECODE; }

    template<typename E2>
    Bytecode<S, E2> addInst(const char *op, const List<void *> &args,
                            const Function<E, E2> &function) const {
      return this->addInst(Inst<E, E2>({string(op), args, function}));
    }

    template<typename E2>
    Bytecode<S, E2> addInst(const Inst<E, E2> &inst) const {
      List<Inst<S, E> > list(this->value());
      list.push_back(inst);
      return Bytecode<S, E2>(list);
    }

    virtual string toString() const override {
      string s = "{";
      for (auto &inst: this->_value) {
        s = s + inst.toString();
      }
      return s + "}";
    }
  };
} // namespace fhatos

#endif
