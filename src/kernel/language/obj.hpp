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

enum OType { OBJ, BOOL, INT, REAL, STR, LST, REC, TYPE, INST };
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
  Obj(const DATA value) : value(value) {}
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
class Int : public Obj<int> {
public:
  Int(const FL_INT_TYPE value) : Obj<FL_INT_TYPE>(value) {};
  virtual const OType type() const override { return INT; }
  virtual const String toString() const override { return String(value); }
  //
  const Int plus(const Int &other) const {
    return Int(this->value + other.value);
  }
  const Int mult(const Int &other) const {
    return Int(this->value * other.value);
  }
  const Int neg() const { return Int(-this->value); }

  const Int minus(const Int &other) const {
    return Int(this->value - other.value);
  }
};
/////////////////////////////////////////////////////////////
class Real : public Obj<FL_REAL_TYPE> {
public:
  Real(const FL_REAL_TYPE value) : Obj<FL_REAL_TYPE>(value) {};
  virtual const OType type() const override { return REAL; }
  virtual const String toString() const override { return String(value); }
  //
  const Real plus(const Real &other) const {
    return Real(this->value + other.value);
  }
  const Real mult(const Real &other) const {
    return Real(this->value * other.value);
  }
  const Real neg() const { return Real(-this->value); }

  const Real minus(const Real &other) const {
    return Real(this->value - other.value);
  }
};
/////////////////////////////////////////////////////////////
class Str : public Obj<String> {
public:
  Str(const String value) : Obj<String>(value) {};
  virtual const OType type() const override { return STR; }
  virtual const String toString() const override { return value; }
  //
  const Str plus(const Str &other) const {
    return Str(this->value + other.value);
  }
};
/////////////////////////////////////////////////////////////
class Lst : public Obj<List<ObjX>> {
public:
  Lst(const List<ObjX> value) : Obj<List<ObjX>>(value) {};
  virtual const OType type() const override { return LST; }
  virtual const String toString() const override {
    String t = "[";
    for (ObjX element : this->value) {
      if (((ObjY)element)->type() == BOOL) {
        t = t + Bool(*(bool *)((ObjY)element)->get()).toString() + ",";
      } else if (((ObjY)element)->type() == INT) {
        t = t + Int(*(FL_INT_TYPE *)((ObjY)element)->get()).toString() + ",";
      } /*else if (element.type() == REAL) {
         t = t + Real(*(FL_REAL_TYPE *)element.get()).toString() + ",";
       } else if (element.type() == STR) {
         t = t + Str(*(String *)element.get()).toString() + ",";
       }*/
    }
    t[t.length() - 1] = ']';
    return t;
  }
};
template <typename K, typename V> class Rec : public Obj<Map<K, V>> {
  Rec(const Map<K, V> value) : Obj<Map<K, V>>(value) {};
  virtual const OType type() const override { return REC; }
  virtual const String toString() const override { return "rec"; }
};

/*template <typename A, typename B>
using Inst_t = Pair<Pair<Str, Lst<Obj<void *>>>, Function<A, B>>;*/

template <typename A, typename B>
class Inst : public Obj<Triple<String, List<void *>, Function<A, B>>> {
public:
  Inst(const Triple<String, List<void *>, Function<A, B>> value)
      : Obj<Triple<String, List<void *>, Function<A, B>>>(value) {}
  virtual const OType type() const override { return INST; }
  virtual const String opcode() const { return std::get<0>(this->value); }
  virtual const List<void *> args() const { return std::get<1>(this->value); }
  virtual const String toString() const override {
    String t = "[" + this->opcode() + ",";
    for (const void *arg : this->args()) {
      Obj<void *> *arg2 = (Obj<void *> *)arg;
      switch (arg2->type()) {
      case BOOL: {
        t = t + ((Bool *)arg2)->toString();
        break;
      }
      case INT: {
        t = t + ((Int *)arg2)->toString();
        break;
      }
      default:
        break;
      }
    }
    return t + "]";
  }
};

/*template <typename S, typename E> class Bytecode : public Inst<S, E> {

  // Bytecode(const Bytecode<S, E> copy) : Inst<S,E>(copy.get()){}
  Bytecode(const List<Inst<void*, void*>> insts)
      : Inst<S, E>(
            {"=>", insts, [this](const S &s) { return (E)s; }}) {}
};*/

} // namespace fhatos::kernel

#endif