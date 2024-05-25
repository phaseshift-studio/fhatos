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
  template<typename S, typename E, typename ALGEBRA = Algebra,
    typename PROCESSOR = Processor<S, E, Monad<E> > >
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
    const E *next() const {
      static PROCESSOR proc = PROCESSOR(this->bcode);
      return proc.next();
    }

    const List<E *> &toList() const {
      static PROCESSOR proc = PROCESSOR(this->bcode);
      return proc.toList();
    }

    void forEach(const Consumer<const E *> &consumer) const {
      PROCESSOR proc = PROCESSOR(this->bcode);
      proc.forEach(consumer);
    }

    const string toString() const { return "f" + this->bcode->toString(); }

    //////////////////////////////////////////////////////////////////////////////
    ///////////////////////// PROTECTED  /////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////

    const ptr<Bytecode> bcode;

  protected:
    template<typename E2>
    Fluent<S, E2> addInst(const Inst *inst) const {
      return Fluent<S, E2>(this->bcode->addInst((Inst *) inst));
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

    operator const OBJ_OR_BYTECODE<Int> &() const {
      return *new INT_OR_BYTECODE(new Bytecode(this->bcode.get()->value()));
    }

    operator const INT_OR_BYTECODE &() const {
      return *new INT_OR_BYTECODE(new Bytecode(this->bcode.get()->value()));
    }

    Fluent<S, E> start(const List<ptr<S_E> > starts) const {
      List<E *> *castStarts = new List<E *>();
      for (auto se: starts) {
        castStarts->push_back((E *) se->obj);
      }
      return this->template addInst<E>(new StartInst<E>(castStarts));
    }

    Fluent<S, E> start(const List<S_E> starts) const {
      List<E *> *castStarts = new List<E *>();
      for (S_E se: starts) {
        castStarts->push_back((E *) se.obj);
      }
      return this->template addInst<E>(new StartInst<E>(castStarts));
    }

    Fluent<S, E> plus(const OBJ_OR_BYTECODE<E> &e) const {
      return this->template addInst<E>(new PlusInst<E, ALGEBRA>(e));
    }

    Fluent<S, E> mult(const S_E &e) const {
      return this->addInst<E>((Inst *) new MultInst<E, ALGEBRA>(e.cast<E>()));
    }

    Fluent<S, E> branch(const std::initializer_list<Pair<S_E const, S_E> > &recMap) {
      return this->addInst<E>((Inst *) new BranchInst<E>(S_E(recMap).cast<Rec>()));
    }

    Fluent<S, Bool> eq(const S_E &se) {
      return this->addInst<Bool>((Inst *) new EqInst<E>(se.cast<E>()));
    }

    Fluent<S, E> is(const BOOL_OR_BYTECODE &se) {
      return this->template addInst<E>(new IsInst<E>(se));
    }

    template<typename _PAYLOAD>
    Fluent<S, E> publish(const S_E &uri, const S_E &payload) const {
      return this->template addInst<E>(
        new PublishInst<E, _PAYLOAD>(uri.cast<E>(), payload.cast<_PAYLOAD>(), this->bcode->context));
    }

    template<typename _ONRECV>
    Fluent<S, E> subscribe(const S_E &pattern, const S_E &onRecv) const {
      return this->template addInst<E>(
        new SubscribeInst<E, _ONRECV>(pattern.cast<E>(), onRecv.cast<_ONRECV>(), this->bcode->context));
    }
  };

  /*class S_Ef : public S_E {
  public:
    S_Ef(Fluent<Obj, Obj> fluentX) : S_E(BYTECODE, (Obj *) fluentX.bcode) {
    }
  };*/

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////    STATIC HELPERS   ///////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////


  template<typename S>
  inline static Fluent<S, S> __(const List<S_E> &starts) {
    List<S *> *castStarts = new List<S *>();
    for (S_E se: starts) {
      castStarts->push_back((S *) se.obj);
    }
    return Fluent<S, S>(
      share<Bytecode>(Bytecode(new List<Inst *>({new StartInst<S>(castStarts)}))));
  };

  template<typename S>
  inline static Fluent<S, S> __(const S_E &start) {
    return Fluent<S, S>(share<Bytecode>(Bytecode(new List<Inst *>({
      new StartInst<S>(new List<S *>{(S *) start.obj})
    }))));
  };

  template<typename S>
  inline static Fluent<S, S> __() { return Fluent<S, S>(); };

  template<typename S>
  inline static Fluent<S, S> _ = __<S>();
} // namespace fhatos

#endif
