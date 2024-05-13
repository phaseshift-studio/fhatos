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

    virtual const DATA &get() const { return this->value; }
    virtual OType type() const { return OBJ; }
    /*virtual const bool equals(const Obj &other) const {
      return this->type() == other.type() && this->value == other.get();
    }*/
    virtual string toString() const {
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
    virtual OType type() const override { return BOOL; }

    virtual string toString() const override {
      return this->value ? "true" : "false";
    }
  };

  /////////////////////////////////////////////////////////////
  class Int : public Obj<FL_INT_TYPE> {
  public:
    Int(const FL_INT_TYPE value) : Obj<FL_INT_TYPE>(value) {
    };
    virtual OType type() const override { return INT; }
    virtual string toString() const override { return std::to_string(value); }
  };

  /////////////////////////////////////////////////////////////
  class Real : public Obj<FL_REAL_TYPE> {
  public:
    Real(const FL_REAL_TYPE value) : Obj<FL_REAL_TYPE>(value) {
    };
    virtual OType type() const override { return REAL; }
    virtual string toString() const override { return std::to_string(value); }
  };

  /////////////////////////////////////////////////////////////
  class Str : public Obj<string> {
  public:
    Str(const string &value) : Obj<string>(value) {
    };
    virtual OType type() const override { return STR; }
    virtual string toString() const override { return value; }
  };

  /////////////////////////////////////////////////////////////
  class Lst : public Obj<List<ObjX> > {
  public:
    Lst(const List<ObjX> &value) : Obj<List<ObjX> >(value) {
    };
    virtual OType type() const override { return LST; }

    virtual string toString() const override {
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
    virtual OType type() const override { return REC; }
    virtual  string toString() const override { return "rec"; }
  };

  template<typename A, typename B>
  class Inst : public Obj<Triple<string, List<void *>, Function<A *, B *> > > {
  public:
    virtual ~Inst() {
    }

    Inst(const Triple<string, List<void *>, Function<A *, B *> > &value)
      : Obj<Triple<string, List<void *>, Function<A *, B *> > >(value) {
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
    virtual const string &opcode() const { return std::get<0>(this->value); }
    virtual const List<ObjX> &args() const { return std::get<1>(this->value); }

    virtual const Function<A *, B *> &func() const {
      return std::get<2>(this->value);
    }

    virtual const B *apply(const A *a) const { return (const B*)this->func()((A*)a); }
  };

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
      List<Inst<S, E> > list(this->get());
      list.push_back(inst);
      return Bytecode<S, E2>(list);
    }

    virtual string toString() const override {
      string s = "{";
      for (auto &inst: this->value) {
        s = s + inst.toString();
      }
      return s + "}";
    }
  };
} // namespace fhatos

#endif
