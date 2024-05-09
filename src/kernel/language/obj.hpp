#ifndef fhatos_kernel__obj_hpp
#define fhatos_kernel__obj_hpp

#include <fhatos.hpp>

namespace fhatos::kernel {

enum OType { OBJ, BOOL, INT, REAL, STR, LST, REC, TYPE, INST };

template <typename DATA> class Obj {
protected:
  const DATA value;

public:
  Obj(const DATA value) : value(value) {}
  virtual const DATA get() const { return this->value; }
  virtual const OType type() const { return OBJ; }
  virtual const String utype() const { return "obj"; }
  virtual const String toStr() const { return "obj"; }
};

using ObjX = void *;

/*class Type : public Obj<Lst<Type>> {
  Type(const Obj<Lst<Type>> value) : Obj(value) {};
  virtual const OType type() const override { return TYPE; }
  virtual const String toStr() const override { return value.toStr(); }
  //
  const Type plus(const Type &other) { return Type(Lst<Type>({*this, other})); }
};*/

class Bool : public Obj<bool> {
public:
  Bool(const bool value) : Obj<bool>(value) {};
  virtual const OType type() const override { return BOOL; }
  virtual const String toStr() const override { return FP_BOOL_STR(value); }
  //
  const Bool plus(const Bool &other) const {
    return Bool(this->value || other.value);
  }
};
class Int : public Obj<int> {
public:
  Int(const int value) : Obj<int>(value) {};
  virtual const OType type() const override { return INT; }
  virtual const String toStr() const override { return String(value); }
  //
  const Int plus(const Int &other) const {
    return Int(this->value + other.value);
  }
};
class Real : public Obj<float> {
public:
  Real(const float value) : Obj<float>(value) {};
  virtual const OType type() const override { return REAL; }
  virtual const String toStr() const override { return String(value); }
  //
  const Real plus(const Real &other) const {
    return Real(this->value + other.value);
  }
};
class Str : public Obj<String> {
public:
  Str(const String value) : Obj<String>(value) {};
  virtual const OType type() const override { return STR; }
  virtual const String toStr() const override { return value; }
  //
  const Str plus(const Str &other) const {
    return Str(this->value + other.value);
  }
};
class Lst : public Obj<List<ObjX>> {
public:
  Lst(const List<ObjX> value) : Obj<List<void *>>(value) {};
  virtual const OType type() const override { return LST; }
  virtual const String toStr() const override {
    String t = "[";
    // List<Obj<void *>> temp = reinterpret_cast<List<Obj<void
    // *>>>(this->value);
    for (ObjX element : this->value) {
      t = t + ((Int *)element)->toStr() + ",";
    }
    t[t.length() - 1] = ']';
    return t;
  }
  //
  //// const Lst plus(const Lst &other) {
  // return Lst(this->value.emplace_back(other.value));
  // }
};
template <typename K, typename V> class Rec : public Obj<Map<K, V>> {
  Rec(const Map<K, V> value) : Obj<Map<K, V>>(value) {};
  virtual const OType type() const override { return REC; }
  virtual const String toStr() const override { return "rec"; }
  //
  const Rec plus(const Rec &other) { return Rec(this->value.insert(other)); }
};

/*template <typename A, typename B>
using Inst_t = Pair<Pair<Str, Lst<Obj<void *>>>, Function<A, B>>;*/

template <typename A, typename B>
class Inst : public Obj<Pair<Lst, Function<A, B>>> {
public:
  Inst(const Pair<Lst, Function<A, B>> value)
      : Obj<Pair<Lst, Function<A, B>>>(value) {}
  virtual const Str opcode() const {
    return *((Str *)this->get().first.get().front());
  }
  virtual const List<ObjX> args() const {
    List<ObjX> list;
    int i = 0;
    for (const ObjX arg : this->get().first.get()) {
      if (i++ > 0) {
        list.push_back(arg);
      }
    }
    return list;
  }
  virtual const OType type() const override { return INST; }
  virtual const String toStr() const override {
    String t = "[" + this->opcode().toStr() + ",";
    for (const ObjX arg : this->args()) {
      t = t + ((Int *)arg)->toStr();
    }
    return t + "]";
  }
};

} // namespace fhatos::kernel

#endif