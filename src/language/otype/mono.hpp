#ifndef fhatos_mono_hpp
#define fhatos_mono_hpp

#include <fhatos.hpp>
#include <language/obj.hpp>
#include <language/otype/bytecode.hpp>
#include <structure/furi.hpp>

namespace fhatos {

  template<typename OBJ, OType OTYPE, typename VALUE>
  class XObj : public Obj, public std::enable_shared_from_this<OBJ> {
  public:
    explicit XObj(const std::variant<Any, ptr<Bytecode>> &value, const ptr<fURI> &type) : Obj(value, type) {}
    virtual OType otype() const override { return OTYPE; }
    ptr<Type> type() const override { return ptr<Type>(new Type(this->_type)); }
    VALUE value() const { return std::any_cast<VALUE>(std::get<0>(this->_var)); }
    ptr<Obj> apply(const ptr<Obj> &obj) override {
      if (this->isBytecode()) {
        ptr<Obj> result = this->bcode()->apply(obj);
        result->_type = this->_type;
        return result;
      } else {
        return this->shared_from_this();
      }
    }
    ptr<OBJ> as(const char *utype) const { return this->Obj::as<OBJ>(Type::obj_t(OTYPE, utype)); }
    bool operator==(const Obj &other) const override {
      return this->_type->equals(*other._type) && (this->isBytecode() == other.isBytecode()) &&
             (this->isBytecode() ? (this->bcode() == other.bcode()) : (this->value() == (*(XObj *) &other).value()));
    }
    static ptr<fURI> _t(const char *utype) { return Type::obj_t(OTYPE, utype); }
  };

  ////////////////////////////////////////////////
  ///////////////////// BOOL ////////////////////
  ///////////////////////////////////////////////
  class Bool;
  using BoolP = ptr<Bool>;
  class Bool final : public XObj<Bool, OType::BOOL, bool> {

  public:
    explicit Bool(const std::variant<Any, ptr<Bytecode>> &value, const ptr<fURI> &type = BOOL_FURI) :
        XObj(value, type) {}
    explicit Bool(const bool value, const ptr<fURI> &type = BOOL_FURI) :
        Bool(std::variant<Any, ptr<Bytecode>>(value), type) {}
    string toString() const override { return this->type()->objString(this->value() ? "true" : "false"); }
    // operators
    bool operator&&(const Bool &other) const { return this - value() && other.value(); }
    bool operator||(const Bool &other) const { return this - value() || other.value(); }
  };

  ///////////////////////////////////////////////
  ///////////////////// INT /////////////////////
  ///////////////////////////////////////////////
  class Int;
  using IntP = ptr<Int>;
  class Int final : public XObj<Int, OType::INT, FL_INT_TYPE> {

  public:
    explicit Int(const std::variant<Any, ptr<Bytecode>> &value, const ptr<fURI> &type = INT_FURI) : XObj(value, type) {}
    string toString() const override { return this->type()->objString(std::to_string(this->value())); }
    // operators
    bool operator<(const Int &other) const { return this->value() < other.value(); }
    bool operator>(const Int &other) const { return this->value() > other.value(); }
  };

  ///////////////////////////////////////////////
  ///////////////////// REAL ////////////////////
  ///////////////////////////////////////////////
  class Real final : public XObj<Real, OType::REAL, FL_REAL_TYPE> {

  public:
    explicit Real(const std::variant<Any, ptr<Bytecode>> &value, const ptr<fURI> &type = REAL_FURI) :
        XObj(value, type) {}
    string toString() const override { return this->type()->objString(std::to_string(this->value())); }
    bool operator<(const Real &other) const { return this->value() < other.value(); }
    bool operator>(const Real &other) const { return this->value() > other.value(); }
  };

  ///////////////////////////////////////////////
  ///////////////////// STR /////////////////////
  ///////////////////////////////////////////////
  class Str final : public XObj<Str, OType::STR, string> {

  public:
    explicit Str(const std::variant<Any, ptr<Bytecode>> &value, const ptr<fURI> &type = STR_FURI) : XObj(value, type) {}
    string toString() const override { return this->type()->objString(this->value()); }
    bool operator<(const Str &other) const { return this->value() < other.value(); }
    bool operator>(const Str &other) const { return this->value() > other.value(); }
    int compare(const Str &other) const { return this->value().compare(other.value()); }
  };

  ///////////////////////////////////////////////
  ///////////////////// URI /////////////////////
  ///////////////////////////////////////////////
  class Uri final : public XObj<Uri, OType::URI, fURI> {
  public:
    explicit Uri(const std::variant<Any, ptr<Bytecode>> &value, const ptr<fURI> &type = URI_FURI) : XObj(value, type) {}
    string toString() const override { return this->type()->objString(this->value().toString()); }
    bool operator<(const Uri &other) const { return this->value().toString() < other.value().toString(); }
    bool operator>(const Uri &other) const { return this->value().toString() > other.value().toString(); }
    int compare(const Uri &other) const { return this->value().toString().compare(other.value().toString()); }
  };

  ///////////////////////////////////////////////
  ///////////////////// REC /////////////////////
  ///////////////////////////////////////////////

  struct obj_hash {
    size_t operator()(const ptr<Obj> &obj) const {
      return static_cast<std::string::value_type>(obj->otype()) ^ (obj->type() ? obj->type()->toString().size() : 123) ^
             obj->toString().length() ^ obj->toString()[0];
    }
  };

  struct obj_equal_to : std::binary_function<ptr<Obj>, ptr<Obj>, bool> {
    bool operator()(const ptr<Obj> &a, const ptr<Obj> &b) const { return *a == *b; }
  };
  /////
  template<typename K = ptr<Obj>, typename V = ptr<Obj>, typename H = obj_hash, typename Q = obj_equal_to>
  using RecMap = OrderedMap<K, V, H, Q>;
  class Rec final : public XObj<Rec, OType::REC, RecMap<>> {
  public:
    explicit Rec(const std::variant<Any, ptr<Bytecode>> &value, const ptr<fURI> &type = REC_FURI) : XObj(value, type) {}
    explicit Rec(const std::initializer_list<Pair<Obj const, Obj>> &keyValues, const ptr<fURI> &type = REC_FURI) :
        XObj(RecMap<>(), type) {
      RecMap<> map = this->value();
      for (const Pair<Obj const, Obj> &pair: keyValues) {
        map.insert({share<Obj>(pair.first), share<Obj>(pair.second)});
      }
      this->_var.emplace<Any>(Any(RecMap<>(map)));
    }
    template<typename V>
    ptr<V> get(const ptr<Obj> &key) const {
      return ptr<V>((V *) (this->value().count(key) ? this->value().at(key).get() : NoObj::self_ptr().get()));
    }
    void set(ptr<Obj> &key, ptr<Obj> &val) { this->value().insert({key, val}); }
    string toString() const override {
      string t = "!m[!!";
      for (const auto &[k, v]: this->value()) {
        t = t + k->toString() + "!r=>!!" + v->toString() + ",";
      }
      t[t.length() - 1] = '!';
      return this->type()->objString(t.append("m]!!"));
    }
    bool operator==(const Obj &other) const override {
      if (!this->_type->equals(*other._type) || (this->isBytecode() != other.isBytecode()) ||
          (this->isBytecode() && *this->bcode() != *other.bcode()) ||
          (this->value().size() != ((Rec *) &other)->value().size()))
        return false;
      auto itB = ((Rec *) &other)->value().begin();
      for (auto itA = this->value().begin(); itA != this->value().end(); ++itA) {
        if (*itA->first != *itB->first || *itA->second != *itB->second)
          return false;
        ++itB;
      }
      return true;
    }
  };
} // namespace fhatos

#endif
