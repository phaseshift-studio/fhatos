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

    S_E(bool boolX): type(BOOL), obj(new Bool(boolX)) {
    };

    S_E(int intX): type(INT), obj(new Int(intX)) {
    };

    S_E(float realX): type(REAL), obj(new Real(realX)) {
    }

    S_E(string strX): type(STR), obj(new Str(strX)) {
    };


    S_E(Bytecode<Obj,Obj>* bcodeX): type(BYTECODE), obj(new Bytecode<Obj,Obj>(*bcodeX)) {
    };

    template<typename E>
     E *cast() const {
      return (E *) this->obj;
    }

    const string toString() const {
      return "S=>E[" + this->obj->toString() + "]";
    }
  };

  template<typename S, typename E,
    typename PROCESSOR = Processor<S, E, Monad<E> > >
  class Fluent {
    //////////////////////////////////////////////////////////////////////////////
    /////////////////////////    PUBLIC   ////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////
  public:
    explicit Fluent() : bcode(new Bytecode<S, E>()) {
    }

    explicit Fluent(const Bytecode<S, E> *bcode) : bcode(bcode) {
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

    const Bytecode<S, E> *bcode;

  protected:
    template<typename E2>
    Fluent<S, E2> addInst(const Inst<E, E2> *inst) const {
      return Fluent<S, E2>(this->bcode->addInst(inst));
    }

  public:
    //////////////////////////////////////////////////////////////////////////////
    ///////////////////////// INSTRUCTIONS ///////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////

    operator const S_E&() const {

      LOG(INFO,"%s!!\n",OTYPE_STR.at(S_E((Bytecode<Obj,Obj>*)this->bcode).obj->type()).c_str());
      return *new S_E((Bytecode<Obj,Obj>*)this->bcode);
    }

    Fluent<S, E> plus(const S_E &e) const {
      return this->template addInst<E>(new PlusInst<E>(e.cast<E>()));
    }

    Fluent<S, E> mult(const S_E &e) const {
      LOG(INFO,"%s!!\n",OTYPE_STR.at(e.obj->type()).c_str());
      return this->template addInst<E>(new MultInst<E>(e.cast<E>()));
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
    List<S *>* castStarts = new List<S*>();
    for (S_E se: starts) {
      castStarts->push_back((S*)se.obj);
    }
    return Fluent<S, S>(new Bytecode<S, S>(new List<Inst<Obj, Obj> *>({(Inst<Obj,Obj>*)new StartInst<S>(castStarts)})));
  };

  template<typename S>
  inline static Fluent<S, S> __(const S_E &start) {
    return Fluent<S, S>(new Bytecode<S, S>(new List<Inst<Obj, Obj> *>({(Inst<Obj,Obj>*)new StartInst<S>(new List<S*>{(S*)start.obj})})));
  };

  template<typename S>
  inline static Fluent<S, S> __() { return Fluent<S, S>(); };
} // namespace fhatos

#endif
