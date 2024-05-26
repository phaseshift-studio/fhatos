#ifndef fhatos_fluent_hpp
#define fhatos_fluent_hpp

#include <fhatos.hpp>
//
#include <language/instructions.hpp>
#include <language/obj.hpp>
#include <language/processor.hpp>

namespace fhatos {
  class S_E {
  public:
    virtual ~S_E() = default;

    OType type;
    Obj *obj;


    S_E(OType type, Obj *obj) : type(type), obj(obj) {
    }

    S_E(fURI furiX): type(OType::URI), obj(new Uri(furiX)) {
    };

    S_E(NoObj *noobj): type(OType::NOOBJ), obj(noobj) {
    };

    S_E(bool boolX): type(OType::BOOL), obj(new Bool(boolX)) {
    };

    S_E(int intX): type(OType::INT), obj(new Int(intX)) {
    };

    S_E(float realX): type(OType::REAL), obj(new Real(realX)) {
    }

    S_E(string strX): type(OType::STR), obj(new Str(strX)) {
    };

    S_E(RecMap<Obj *, Obj *> *recX): type(OType::REC), obj(new Rec(recX)) {
    };

    S_E(const std::initializer_list<Pair<S_E const, S_E> > &init) : type(OType::REC),
                                                                    obj(new Rec(new RecMap<Obj *, Obj *>())) {
      for (auto iter = rbegin(init); iter != rend(init); ++iter) {
        ((Rec *) this->obj)->value()->insert({iter->first.obj, iter->second.obj});
        LOG(INFO, "%s => %s\n", iter->first.obj->toString().c_str(), iter->second.cast<Obj>()->toString().c_str());
      }
      LOG(INFO, "size => %i\n", ((Rec*)this->obj)->value()->size());
    };

    S_E(Bytecode *bcodeX): type(OType::BYTECODE), obj(bcodeX) {
    };

    bool isNoObj() const {
      return this->obj->isNoObj();
    }

    template<typename E>
    E *cast() const {
      return (E *) this->obj;
    }

    const string toString() const {
      return "S=>E[" + this->obj->toString() + "]";
    }
  };

  class S_E_BOOL final : public S_E {
  public:
    S_E_BOOL(Bytecode *bcodeX): S_E(bcodeX) {
    };

    S_E_BOOL(bool boolX): S_E(boolX) {
    };
  };


  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  template<typename ALGEBRA = Algebra>
  class Fluent {
    //////////////////////////////////////////////////////////////////////////////
    /////////////////////////    PUBLIC   ////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////
  public:
    explicit Fluent(const ptr<Bytecode> bcode) : bcode(bcode) {
    }

    explicit Fluent(const ID context = ID("anonymous")) : Fluent(share<Bytecode>(Bytecode(context))) {
    }

    //////////////////////////////////////////////////////////////////////////////
    template<typename E = Obj>
    const E *next() const {
      static Processor<E> proc = Processor<E>(this->bcode);
      return proc.next();
    }

    template<typename E = Obj>
    const List<E *> &toList() const {
      static Processor<E> proc = Processor<E>(this->bcode);
      return proc.toList();
    }

    template<typename E = Obj>
    void forEach(const Consumer<const E *> &consumer) const {
      Processor<E> proc = Processor<E>(this->bcode);
      proc.forEach(consumer);
    }

    const string toString() const { return "f" + this->bcode->toString(); }

    //////////////////////////////////////////////////////////////////////////////
    ///////////////////////// PROTECTED  /////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////

    const ptr<Bytecode> bcode;

  protected:
    Fluent<> addInst(Inst *inst) const {
      return Fluent<>(this->bcode->addInst(inst));
    }

  public:
    //////////////////////////////////////////////////////////////////////////////
    ///////////////////////// INSTRUCTIONS ///////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////

    operator const S_E &() const {
      return *(new S_E(new Bytecode(this->bcode.get()->value())));
    }

    operator const BOOL_OR_BYTECODE &() const {
      return *new BOOL_OR_BYTECODE(new Bytecode(this->bcode.get()->value()));
    }

    operator const OBJ_OR_BYTECODE<Bytecode> &() const {
      return *new OBJ_OR_BYTECODE<Bytecode>(new Bytecode(this->bcode.get()->value()));
    }

    operator const OBJ_OR_BYTECODE<Obj> &() const {
      return *new OBJ_OR_BYTECODE<Obj>(new Bytecode(this->bcode.get()->value()));
    }

    operator const OBJ_OR_BYTECODE<Int> &() const {
      return *new INT_OR_BYTECODE(new Bytecode(this->bcode.get()->value()));
    }

    operator const OBJ_OR_BYTECODE<Bool> &() const {
      return *new OBJ_OR_BYTECODE<Bool>(new Bytecode(this->bcode.get()->value()));
    }

    operator const INT_OR_BYTECODE &() const {
      return *new INT_OR_BYTECODE(new Bytecode(this->bcode.get()->value()));
    }

    Fluent start(const List<ptr<S_E> > starts) const {
      List<Obj *> *castStarts = new List<Obj *>();
      for (auto se: starts) {
        castStarts->push_back((Obj *) se->obj);
      }
      return this->addInst(new StartInst(castStarts));
    }

    Fluent start(const List<S_E> starts) const {
      List<Obj *> *castStarts = new List<Obj *>();
      for (S_E se: starts) {
        castStarts->push_back(se.obj);
      }
      return this->addInst(new StartInst(castStarts));
    }

    Fluent plus(const OBJ_OR_BYTECODE<Obj> &e) const {
      return this->addInst(new PlusInst<ALGEBRA>(e));
    }

    Fluent mult(const S_E &e) const {
      return this->addInst(new MultInst<ALGEBRA>(e.cast<Obj>()));
    }

    Fluent branch(const std::initializer_list<Pair<S_E const, S_E> > &recMap) {
      return this->addInst(new BranchInst(S_E(recMap).cast<Rec>()));
    }

    Fluent eq(const S_E &se) {
      return this->addInst(new EqInst(se.cast<Obj>()));
    }

    Fluent is(const BOOL_OR_BYTECODE &se) {
      return this->addInst(new IsInst(se));
    }

    template<typename _PAYLOAD>
    Fluent publish(const S_E &uri, const S_E &payload) const {
      return this->addInst(
        new PublishInst<Obj, _PAYLOAD>(uri.cast<Obj>(), payload.cast<_PAYLOAD>(), this->bcode->context));
    }

    template<typename _ONRECV>
    Fluent subscribe(const S_E &pattern, const S_E &onRecv) const {
      return this->addInst(
        new SubscribeInst<Obj, _ONRECV>(pattern.cast<Obj>(), onRecv.cast<_ONRECV>(), this->bcode->context));
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////    STATIC HELPERS   ///////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////


  inline static Fluent<> __(const List<S_E> &starts) {
    List<Obj *> *castStarts = new List<Obj *>();
    for (S_E se: starts) {
      castStarts->push_back((Obj *) se.obj);
    }
    return Fluent(
      share<Bytecode>(Bytecode(new List<Inst *>({new StartInst(castStarts)}))));
  };


  inline static Fluent<> __(const S_E &start) {
    return Fluent(share<Bytecode>(Bytecode(new List<Inst *>({
      new StartInst(new List<Obj *>{start.obj})
    }))));
  };

  inline static Fluent<> __() { return Fluent(); };

  inline static Fluent<> _ = __();
} // namespace fhatos

#endif
