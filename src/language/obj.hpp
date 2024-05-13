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

  template<typename DATA>
  class Obj {
  protected:
    const DATA value;

  public:
    Obj(const DATA &value) : value(value) {
    }

    virtual const DATA get() const { return this->value; }
    virtual const OType type() const { return OBJ; }
    /*virtual const bool equals(const Obj &other) const {
      return this->type() == other.type() && this->value == other.get();
    }*/
    virtual const string toString() const {
      return string((char *) ((void *) (&(this->value))));
    }

    static const Algebra<Obj<DATA>, RING> algebra() {
      return Algebra<Obj<DATA>, RING>::singleton();
    }
  };

  using ObjX = void *; // wildcard object
  using ObjY = Obj<ObjX> *;

  /////////////////////////////////////////////////////////////
  class Bool : public Obj<bool> {
  public:
    Bool(const bool value) : Obj<bool>(value) {
    };
    virtual const OType type() const override { return BOOL; }

    virtual const string toString() const override {
      return this->value ? "true" : "false";
    }
  };

  /////////////////////////////////////////////////////////////
  class Int : public Obj<FL_INT_TYPE> {
  public:
    Int(const FL_INT_TYPE value) : Obj<FL_INT_TYPE>(value) {
    };
    virtual const OType type() const override { return INT; }
    virtual const string toString() const override { return std::to_string(value); }
  };

  /////////////////////////////////////////////////////////////
  class Real : public Obj<FL_REAL_TYPE> {
  public:
    Real(const FL_REAL_TYPE value) : Obj<FL_REAL_TYPE>(value) {
    };
    virtual const OType type() const override { return REAL; }
    virtual const string toString() const override { return std::to_string(value); }
  };

  /////////////////////////////////////////////////////////////
  class Str : public Obj<string> {
  public:
    Str(const string &value) : Obj<string>(value) {
    };
    virtual const OType type() const override { return STR; }
    virtual const string toString() const override { return value; }
  };

  /////////////////////////////////////////////////////////////
  class Lst : public Obj<List<ObjX> > {
  public:
    Lst(const List<ObjX> &value) : Obj<List<ObjX> >(value) {
    };
    virtual const OType type() const override { return LST; }

    virtual const string toString() const override {
      string t = "[";
      for (const ObjX &element: this->value) {
        t = t + ((ObjY) element)->toString() + ",";
      }
      t[t.length() - 1] = ']';
      return t;
    }
  };

  template<typename K, typename V>
  class Rec : public Obj<Map<K, V> > {
    Rec(const Map<K, V> &value) : Obj<Map<K, V> >(value) {
    };
    virtual const OType type() const override { return REC; }
    virtual const string toString() const override { return "rec"; }
  };

  template<typename A, typename B>
  class Inst : public Obj<Triple<string, List<void *>, Function<A *, B *> > > {
  public:
    Inst(const Triple<string, List<void *>, Function<A *, B *> > &value)
      : Obj<Triple<string, List<void *>, Function<A *, B *> > >(value) {
    }

    virtual const OType type() const override { return INST; }

    virtual const string toString() const override {
      string t = "[" + this->opcode() + ",";
      for (const ObjX &arg: this->args()) {
        t = t + ((ObjY) arg)->toString();
      }
      return t + "]";
    }

    ////////////////////////////////////
    virtual const string opcode() const { return std::get<0>(this->value); }
    virtual const List<ObjX> args() const { return std::get<1>(this->value); }

    virtual const Function<A *, B *> func() const {
      return std::get<2>(this->value);
    }

    virtual B *apply(A *a) const { return this->func()(a); }
  };

  template<typename S, typename E>
  class Bytecode : public Obj<List<Inst<S, E> > > {
  public:
    Bytecode() : Obj<List<Inst<S, E> > >(List<Inst<S, E> >()) {
    }

    Bytecode(const List<Inst<S, E> > &list) : Obj<List<Inst<S, E> > >(list) {
    }

    Bytecode(const std::initializer_list<Inst<S, E> > braces)
      : Obj<List<Inst<S, E> > >(List<Inst<S, E> >(braces)) {
    }

    Bytecode(const Inst<S, E> &inst) : Bytecode<S, E>({inst}) {
    }

    Bytecode(const E &obj)
      : Bytecode<S, E>(
        Inst<E, E>({"start", {new E(obj)}, [](E *e) { return e; }})) {
    }

    virtual const OType type() const override { return BYTECODE; }

    template<typename E2>
    const Bytecode<S, E2> addInst(const char *op, const List<void *> &args,
                                  const Function<E, E2> &function) const {
      return this->addInst(Inst<E, E2>({string(op), args, function}));
    }

    template<typename E2>
    const Bytecode<S, E2> addInst(const Inst<E, E2> &inst) const {
      List<Inst<S, E> > list(this->get());
      list.push_back(inst);
      return Bytecode<S, E2>(list);
    }

    const string toString() const {
      string s = "{";
      for (auto &inst: this->value) {
        s = s + inst.toString();
      }
      return s + "}";
    }
  };
} // namespace fhatos

#endif
