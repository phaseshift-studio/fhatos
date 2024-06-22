#ifndef fhatos_bytecode_hpp
#define fhatos_bytecode_hpp

#include <fhatos.hpp>
#include <language/obj.hpp>

namespace fhatos {

  ///////////////////////////////////////////////// INST
  /////////////////////////////////////////////////////////////////
  enum class IType : uint8_t {
    NOINST = 0,
    ONE_TO_ONE,
    ONE_TO_MANY,
    MANY_TO_ONE,
    MANY_TO_MANY,
  }; // TYPE
  static const Map<IType, const char *> ITYPE_STR = {{
      {IType::NOINST, "noinst"},
      {IType::ONE_TO_ONE, "f(x)->y (map)"},
      {IType::ONE_TO_MANY, "f(x)->y* (flatmap)"},
      {IType::MANY_TO_ONE, "f(x*)->y (reduce)"},
      {IType::MANY_TO_MANY, "f(x*)->y* (barrier)"},
  }};

  template<typename A = Obj, typename B = Obj>
  using InstFunction = Function<ptr<A>, const ptr<B>>;
  typedef InstFunction<Obj, Obj> OneToOneFunction;
  typedef InstFunction<Obj, Objs> OneToManyFunction;
  typedef InstFunction<Objs, Obj> ManyToOneFunction;
  typedef InstFunction<Objs, Objs> ManyToManyFunction;

  using InstArgs = List<ptr<Obj>>;
  using InstOpcode = string;

  /////////////////////////////////////////////
  //////////////////// INST ///////////////////
  /////////////////////////////////////////////
  class Inst : public Obj {
  protected:
    const IType _itype;
    ptr<Bytecode> _bcode = ptr<Bytecode>(nullptr);
    const InstFunction<> _function;


  public:
    explicit Inst(const string &opcode, const InstArgs &value, const IType itype, const InstFunction<> &function) :
        Obj(value, share<fURI>(INST_FURI->extend(opcode.c_str()))), _itype(itype), _function(function) {}

    virtual bool isNoInst() const { return this->otype() == OType::NOINST; }

    void bcode(ptr<Bytecode> bcode) { this->_bcode = bcode; }

    ptr<Bytecode> bcode() const override { return this->_bcode; }

    InstArgs v_args() const { return std::any_cast<InstArgs>(std::get<0>(this->_var)); }

    IType itype() const { return this->_itype; }

    ptr<Obj> apply(const ptr<Obj> &obj) override {
      if (IType::MANY_TO_ONE == _itype || IType::MANY_TO_MANY == _itype)
        assert(obj->otype() == OType::OBJS);
      ptr<Obj> result = this->_function(obj);
      if (IType::ONE_TO_MANY == _itype || IType::MANY_TO_MANY == _itype)
        assert(result->otype() == OType::OBJS);
      return result;
    }

    ptr<Type> type() const override { return ptr<Type>(new Type(this->_type)); }

    string toString() const override {
      string t = "";
      for (const auto &arg: this->v_args()) {
        t += arg->toString() + ",";
      }
      return this->type()->objString(t.substr(0, t.length() - 1));
    }

    const InstOpcode opcode() const { return this->type()->name(); }
    const ptr<Obj> arg(const uint8_t index) const { return this->v_args().at(index); }
  };

  //////////////// NO INST ////////////////

  class NoInst : public Inst {
  public:
    template<typename _INST = Inst>
    static ptr<_INST> self_ptr() {
      return ptr<_INST>(new NoInst());
    }

    ptr<Obj> apply(const ptr<Obj> &obj) override { return NoObj::self_ptr(); }
    bool isNoObj() const override { return true; }

    bool isNoInst() const override { return true; }
    ptr<Type> type() const override { return ptr<Type>(new Type(this->_type)); }
    string toString() const override { return "Ø"; }

    bool operator==(const Obj &other) const override { return other.isNoObj(); }

  private:
    NoInst() : Inst("noinst", {}, IType::NOINST, [](ptr<Obj>) -> ptr<Obj> { return NoInst::self_ptr<Obj>(); }) {}
  };


  //////////////// ONE TO ONE ////////////////
  class OneToOneInst : public Inst {
  public:
    OneToOneInst(const string &opcode, const InstArgs &args, const OneToOneFunction &function) :
        Inst(opcode, args, IType::ONE_TO_ONE, function) {}
  };

  //////////////// MANY TO ONE ////////////////
  class ManyToOneInst : public Inst {
  public:
    ManyToOneInst(const string &opcode, const InstArgs &args, ManyToOneFunction function) :
        Inst(opcode, args, IType::MANY_TO_ONE, *(InstFunction<Obj, Obj> *) &function) {}
  };

  //////////////// ONE TO MANY ////////////////
  class OneToManyInst : public Inst {
  public:
    OneToManyInst(const string &opcode, const InstArgs &args, OneToManyFunction function) :
        Inst(opcode, args, IType::ONE_TO_MANY, *(InstFunction<Obj, Obj> *) &function) {}
  };


  //////////////// MANY TO MANY ////////////////
  class ManyToManyInst : public Inst {
  public:
    ManyToManyInst(const string &opcode, const InstArgs &args, ManyToManyFunction function) :
        Inst(opcode, args, IType::MANY_TO_MANY, *(InstFunction<Obj, Obj> *) &function) {}
  };


  /////////////////////////////////////////////
  //////////////////// OBJS ///////////////////
  /////////////////////////////////////////////
  class Objs final : public Obj {

  public:
    Objs(const List<const ptr<Obj>> &value, const ptr<fURI> &type = OBJS_FURI) : Obj(value, type) {}

    const List<const ptr<Obj>> v_stream() const { return std::any_cast<List<const ptr<Obj>>>(std::get<0>(this->_var)); }

    ptr<Obj> apply(const ptr<Obj> &obj) override { return ptr<Objs>((Objs *) this); }

    virtual string toString() const override {
      string t = "(";
      for (const ptr<Obj> &obj: std::any_cast<List<ptr<Obj>>>(std::get<0>(this->_var))) {
        t = t + obj->toString() + ",";
      }
      t[t.length() - 1] = ')';
      return this->type()->objString(t);
    }
  };

  /////////////////////////////////////////////
  ////////////////// BYTECODE /////////////////
  /////////////////////////////////////////////
  class Bytecode final : public Obj {
  protected:
    Map<const fURI, const ptr<const Obj>> TYPE_CACHE;

  public:
    //~Bytecode() override {}
    Bytecode() = delete;
    explicit Bytecode(const std::variant<Any, ptr<Bytecode>> &value, const ptr<fURI> &type = BCODE_FURI) :
        Obj(value, type) {
      // for (auto &inst: this->v_insts()) {
      //  inst->bcode(this);
      // }
    }

    /*void setId(const ID &id) {
      this->_id = id;
      for (const ptr<Inst> inst: this->_value) {
        for (const auto &arg: inst->args()) {
          if (arg->otype() == OType::BYTECODE) {
            (*(ptr<Bytecode> *) &arg)->setId(id);
          }
        }
      }
    }*/

    /*const ptr<Uri> relativeUri(const ptr<Uri> uri) const {
      if (uri->value().empty())
        return new Uri(this->id());
      if (uri->value().toString()[0] == ':')
        return new Uri(this->id().toString() + uri->value().toString());
      return uri;
    }*/

    const ptr<Inst> nextInst(ptr<Inst> currentInst) const {
      if (currentInst->isNoInst())
        return currentInst;
      bool found = false;
      for (const auto &inst: *this->v_insts()) {
        if (found)
          return inst;
        if (inst == currentInst)
          found = true;
      }
      return NoInst::self_ptr<>();
    }

    ptr<Obj> apply(const ptr<Obj> &obj) override {
      ptr<Obj> currentObj = obj;
      for (const ptr<Inst> &currentInst: *this->v_insts()) {
        if (currentInst->isNoInst() || currentObj->isNoObj())
          break;
        LOG(DEBUG, "Applying %s => %s\n", currentObj->toString().c_str(), currentInst->toString().c_str());
        currentObj = currentInst->apply(currentObj);
      }
      return currentObj; //(currentObj->type() == OType::URI) ? relativeUri((ptr<Uri>currentObj) : currentObj;
    }


    ptr<List<ptr<Inst>>> v_insts() const { return std::any_cast<ptr<List<ptr<Inst>>>>(std::get<0>(this->_var)); }

    ptr<Bytecode> addInst(const ptr<Inst> inst) const {
      ptr<List<ptr<Inst>>> list = ptr<List<ptr<Inst>>>(new List<ptr<Inst>>());
      for (const ptr<Inst> &prevInst: *this->v_insts()) {
        list->push_back(prevInst); //->bcode(bcode));
      }
      list->push_back(inst); //->bcode(bcode));
      return ptr<Bytecode>(new Bytecode(list, this->_type));
    }

    /*template<typename ROUTER>
    void createType(const fURI &type, const ptr<const Obj> typeDefinition) {
      if (!ROUTER::singleton()->write(typeDefinition, this->id(), type)) {
        this->TYPE_CACHE.emplace(type, typeDefinition);
      }
    }

    template<typename ROUTER>
    const ptr<const Obj> getType(const fURI &type) {
      if (this->TYPE_CACHE.count(type)) {
        LOG(DEBUG, "Bytecode type cache hit: %s\n", type.toString().c_str());
        return this->TYPE_CACHE.at(type);
      }
      const ptr<const Obj> typeDefinition = ROUTER::singleton()->read(this->id(), type);
      if (typeDefinition) {
        this->TYPE_CACHE.emplace(type, typeDefinition);
      }
      return typeDefinition;
    }

    template<typename ROUTER>
    bool match(const Obj *obj, const fURI &type) const {
      return !((Obj *) this->getType<ROUTER>(type).apply(obj))->isNoObj();
    }*/

    ptr<Type> type() const override { return ptr<Type>(new Type(this->_type)); }

    const ptr<Inst> startInst() const { return this->v_insts()->front(); }

    const ptr<Inst> endInst() const { return this->v_insts()->back(); }

    virtual string toString() const override {
      string s;
      for (const auto &inst: *this->v_insts()) {
        s.append(inst->toString() + ',');
      }
      return this->type()->objString(s.substr(0, s.length() - 1));
    }
  };
} // namespace fhatos
#endif