#ifndef fhatos_fluent_hpp
#define fhatos_fluent_hpp

#include <fhatos.hpp>
//
#include <language/instructions.hpp>
#include <language/obj.hpp>
#include <language/processor.hpp>

namespace fhatos {
  //template<typename S, typename E>
  class S_E {
  public:
    virtual ~S_E() = default;

    OType type;
    Obj *obj;


    S_E(OType type, Obj *obj) : type(type), obj(obj) {
    }

    S_E(fURI furiX): type(URI), obj(new Uri(furiX)) {
    };

    S_E(bool boolX): type(BOOL), obj(new Bool(boolX)) {
    };

    S_E(int intX): type(INT), obj(new Int(intX)) {
    };

    S_E(float realX): type(REAL), obj(new Real(realX)) {
    }

    S_E(string strX): type(STR), obj(new Str(strX)) {
    };

    S_E(Bytecode *bcodeX): type(BYTECODE), obj(bcodeX) {
    };

    template<typename E>
    E *cast() const {
      return (E *) this->obj;
    }

    const string toString() const {
      return "S=>E[" + this->obj->toString() + "]";
    }
  };

  template<typename S, typename E, typename ALGEBRA = Algebra,
    typename PROCESSOR = Processor<S, E, Monad<E> > >
  class Fluent {
    //////////////////////////////////////////////////////////////////////////////
    /////////////////////////    PUBLIC   ////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////
  public:
    explicit Fluent(const Bytecode *bcode) : bcode(bcode) {
    }

    explicit Fluent(const ID context = ID("anonymous")) : Fluent(new Bytecode(context)) {
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

    const Bytecode *bcode;

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
      return *new S_E((Bytecode *) this->bcode);
    }

    Fluent<S, E> start(const List<S_E *> starts) const {
      List<E *> *castStarts = new List<E *>();
      for (S_E *se: starts) {
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

    Fluent<S, E> plus(const S_E &e) const {
      return this->addInst<E>((Inst *) new PlusInst<E, ALGEBRA>(e.cast<E>()));
    }

    Fluent<S, E> mult(const S_E &e) const {
      return this->addInst<E>((Inst *) new MultInst<E, ALGEBRA>(e.cast<E>()));
    }

    template <typename _PAYLOAD>
    Fluent<S, E> publish(const S_E &uri, const S_E &payload) const {
      return this->template addInst<E>(new PublishInst<E,_PAYLOAD>(uri.cast<E>(), payload.cast<_PAYLOAD>(), this->bcode->context));
    }

    template <typename _ONRECV>
    Fluent<S, E> subscribe(const S_E &pattern, const S_E &onRecv) const {
      return this->template addInst<E>(
        new SubscribeInst<E,_ONRECV>(pattern.cast<E>(), onRecv.cast<_ONRECV>(), this->bcode->context));
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
      new Bytecode(new List<Inst *>({new StartInst<S>(castStarts)})));
  };

  template<typename S>
  inline static Fluent<S, S> __(const S_E &start) {
    return Fluent<S, S>(new Bytecode(new List<Inst *>({
      new StartInst<S>(new List<S *>{(S *) start.obj})
    })));
  };

  template<typename S>
  inline static Fluent<S, S> __() { return Fluent<S, S>(); };
} // namespace fhatos

#endif
