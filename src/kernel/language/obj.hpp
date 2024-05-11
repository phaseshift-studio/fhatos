#ifndef fhatos_kernel__obj_hpp
#define fhatos_kernel__obj_hpp

#ifndef FL_REAL_TYPE
#define FL_REAL_TYPE float
#endif
#ifndef FL_INT_TYPE
#define FL_INT_TYPE int
#endif

#include <fhatos.hpp>

namespace fhatos::kernel {

enum OType { OBJ, BOOL, INT, REAL, STR, LST, REC, INST, BYTECODE }; // TYPE
enum AType { MONOID, SEMIRING, GROUP, RING, FIELD };

template <typename OBJ, AType atype> struct Algebra {
  const AType _atype;

  Algebra(const AType _atype) : _atype(_atype) {}
  static const Algebra<OBJ, atype> singleton() {
    const static Algebra<OBJ, atype> *instance = Algebra<OBJ, atype>(atype);
    return &instance;
  }
};

template <typename DATA> class Obj {

protected:
  const DATA value;

public:
  Obj(const DATA &value) : value(value) {}
  virtual const DATA get() const { return this->value; }
  virtual const OType type() const { return OBJ; }
  // virtual const String utype() const { return "obj"; }
  virtual const String toString() const {
    return String((char *)((void *)(&(this->value))));
  }
  static const Algebra<Obj<DATA>, RING> algebra() {
    return Algebra<Obj<DATA>, RING>::singleton();
  }
};

using ObjX = void *; // wildcard object
using ObjY = Obj<ObjX> *;
using ObjZ = Obj<ObjX>;

class Bool : public Obj<bool> {
public:
  /////////////////////////////////////////////////////////////
  Bool(const bool value) : Obj<bool>(value) {};
  virtual const OType type() const override { return BOOL; }
  virtual const String toString() const override {
    return this->value ? "true" : "false";
  }
};
/////////////////////////////////////////////////////////////
class Int : public Obj<FL_INT_TYPE> {
public:
  Int(const FL_INT_TYPE value) : Obj<FL_INT_TYPE>(value) {};
  virtual const OType type() const override { return INT; }
  virtual const String toString() const override { return String(value); }
};
/////////////////////////////////////////////////////////////
class Real : public Obj<FL_REAL_TYPE> {
public:
  Real(const FL_REAL_TYPE value) : Obj<FL_REAL_TYPE>(value) {};
  virtual const OType type() const override { return REAL; }
  virtual const String toString() const override { return String(value); }
};
/////////////////////////////////////////////////////////////
class Str : public Obj<String> {
public:
  Str(const String &value) : Obj<String>(value) {};
  virtual const OType type() const override { return STR; }
  virtual const String toString() const override { return value; }
};
/////////////////////////////////////////////////////////////
class Lst : public Obj<List<ObjX>> {
public:
  Lst(const List<ObjX> &value) : Obj<List<ObjX>>(value) {};
  virtual const OType type() const override { return LST; }
  virtual const String toString() const override {
    String t = "[";
    for (ObjX element : this->value) {
      t = t + ((ObjY)element)->toString();
    }
    t[t.length() - 1] = ']';
    return t;
  }
};
template <typename K, typename V> class Rec : public Obj<Map<K, V>> {
  Rec(const Map<K, V> &value) : Obj<Map<K, V>>(value) {};
  virtual const OType type() const override { return REC; }
  virtual const String toString() const override { return "rec"; }
};

template <typename A, typename B>
class Inst : public Obj<Triple<String, List<void *>, Function<A, B>>> {

public:
  Inst(const Triple<String, List<void *>, Function<A, B>> &value)
      : Obj<Triple<String, List<void *>, Function<A, B>>>(value) {}
  virtual const OType type() const override { return INST; }
  virtual const String toString() const override {
    String t = "[" + this->opcode() + ",";
    for (const ObjX arg : this->args()) {
      t = t + ((ObjY)arg)->toString();
    }
    return t + "]";
  }
  //////
  virtual const String opcode() const { return std::get<0>(this->value); }
  virtual const List<ObjX> args() const { return std::get<1>(this->value); }
};

template <typename S, typename E>
class Bytecode : public Obj<List<Inst<S, E>>> {
public:
  Bytecode() : Obj<List<Inst<S, E>>>(List<Inst<S, E>>()) {}
  Bytecode(const List<Inst<S, E>> &list) : Obj<List<Inst<S, E>>>(list) {}
  Bytecode(const std::initializer_list<Inst<S, E>> braces)
      : Obj<List<Inst<S, E>>>(List<Inst<S, E>>(braces)) {}
  Bytecode(const Inst<S, E> &inst) : Bytecode<S, E>({inst}) {}
  Bytecode(const E &obj)
      : Bytecode<S, E>(Inst<E, E>(
            {"identity", {new E(obj)}, [](const E &e) { return e; }})) {}

  virtual const OType type() const override { return BYTECODE; }
  template <typename E2>
  const Bytecode<S, E2> addInst(const char *op, const List<void *> &args,
                                const Function<E, E2> &function) const {
    List<Inst<S, E>> list(this->get());
    list.push_back(Inst<E, E2>({String(op), args, function}));
    return Bytecode<S, E2>(list);
  }
  const String toString() const {
    String s;
    for (auto &inst : this->value) {
      s = s + inst.toString();
    }
    return s;
  }
};

} // namespace fhatos::kernel

#endif