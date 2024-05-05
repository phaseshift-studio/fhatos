#ifndef fhatos_kernel__obj_hpp
#define fhatos_kernel__obj_hpp

#include <fhatos.hpp>

namespace fhatos::kernel {

enum OType { OBJ, BOOL, INT, REAL, STR, LST, REC, TYPE };

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

class Type : public Obj<Lst<Type>> {
  Type(const Obj<Lst<Type>> value) : Obj(value) {};
  virtual const OType type() const override { return TYPE; }
  virtual const String toStr() const override { return value.toStr(); }
  //
  const Type plus(const Type &other) { return Type(Lst<Type>({*this, other})); }
};

class Bool : public Obj<bool> {
public:
  Bool(const bool value) : Obj(value) {};
  virtual const OType type() const override { return BOOL; }
  virtual const String toStr() const override { return FP_BOOL_STR(value); }
  //
  const Bool plus(const Bool &other) {
    return Bool(this->value || other.value);
  }
};
class Int : public Obj<int> {
public:
  Int(const int value) : Obj(value) {};
  virtual const OType type() const override { return INT; }
  virtual const String toStr() const override { return String(value); }
  //
  const Int plus(const Int &other) { return Int(this->value + other.value); }
};
class Real : public Obj<float> {
public:
  Real(const float value) : Obj(value) {};
  virtual const OType type() const override { return REAL; }
  virtual const String toStr() const override { return String(value); }
  //
  const Real plus(const Real &other) { return Real(this->value + other.value); }
};
class Str : public Obj<String> {
public:
  Str(const String value) : Obj(value) {};
  virtual const OType type() const override { return STR; }
  virtual const String toStr() const override { value; }
  //
  const Str plus(const Str &other) { return Str(this->value + other.value); }
};
template <typename V> class Lst : public Obj<List<V>> {
public:
  Lst(const List<V> value) : Obj(value) {};
  virtual const OType type() const override { return LST; }
  virtual const String toStr() const override { return "lst" }
  //
  const Lst plus(const Lst &other) {
    return Lst(this->value.emplace_back(other.value));
  }
};
template <typename K, typename V> class Rec : public Obj<Map<K, V>> {
  Rec(const Map<K, V> value) : Obj(value) {};
  virtual const OType type() const override { return REC; }
  virtual const String toStr() const override { return "rec"; }
  //
  const Rec plus(const Rec &other) { return Rec(this->value.insert(other)); }
};

} // namespace fhatos::kernel

#endif