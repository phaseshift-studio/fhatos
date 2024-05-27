#ifndef fhatos_fluent_hpp
#define fhatos_fluent_hpp

#include <fhatos.hpp>
//
#include <language/instructions.hpp>
#include <language/obj.hpp>
#include <language/processor.hpp>

namespace fhatos {
  template<typename ALGEBRA = Algebra>
  class Fluent {
    //////////////////////////////////////////////////////////////////////////////
    /////////////////////////    PUBLIC   ////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////
  public:
    explicit Fluent(const ptr<Bytecode> bcode) : bcode(bcode) {
    }

    explicit Fluent(const ID context = ID("anon")) : Fluent(share<Bytecode>(Bytecode(context))) {
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

    operator const OBJ_OR_BYTECODE &() const {
      return *new OBJ_OR_BYTECODE(new Bytecode(this->bcode.get()->value()));
    }

    Fluent start(const List<ptr<OBJ_OR_BYTECODE> > &starts) const {
      List<Obj *> *castStarts = new List<Obj *>();
      for (const auto &se: starts) {
        castStarts->push_back(se->cast<>());
      }
      return this->addInst(new StartInst(castStarts));
    }

    Fluent plus(const OBJ_OR_BYTECODE &rhs) const {
      return this->addInst(new PlusInst<ALGEBRA>(rhs));
    }

    Fluent mult(const OBJ_OR_BYTECODE &rhs) const {
      return this->addInst(new MultInst<ALGEBRA>(rhs));
    }

    Fluent branch(const std::initializer_list<Pair<OBJ_OR_BYTECODE const, OBJ_OR_BYTECODE> > &recPairs) {
      RecMap<Obj *, Obj *> *recMap = new RecMap<Obj *, Obj *>;
      for (auto pair: recPairs) {
        recMap->insert({pair.first.cast<>(), pair.second.cast<>()});
      }
      return this->addInst(new BranchInst(OBJ_OR_BYTECODE(new Rec(recMap)).cast<Rec>()));
    }

    Fluent branch(const OBJ_OR_BYTECODE &branches) {
      return this->addInst(new BranchInst(branches));
    }

    Fluent eq(const OBJ_OR_BYTECODE &rhs) {
      return this->addInst(new EqInst(rhs));
    }

    Fluent is(const OBJ_OR_BYTECODE &test) {
      return this->addInst(new IsInst(test));
    }

    template<typename _PAYLOAD>
    Fluent publish(const OBJ_OR_BYTECODE &uri, const OBJ_OR_BYTECODE &payload) const {
      return this->addInst(
        new PublishInst<Obj, _PAYLOAD>(uri.cast<Obj>(), payload.cast<_PAYLOAD>(), this->bcode->context));
    }

    template<typename _ONRECV>
    Fluent subscribe(const OBJ_OR_BYTECODE &pattern, const OBJ_OR_BYTECODE &onRecv) const {
      return this->addInst(
        new SubscribeInst<Obj, _ONRECV>(pattern.cast<Obj>(), onRecv.cast<_ONRECV>(), this->bcode->context));
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////    STATIC HELPERS   ///////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////


  static Fluent<> __(const List<OBJ_OR_BYTECODE> &starts) {
    List<Obj *> *castStarts = new List<Obj *>();
    for (OBJ_OR_BYTECODE se: starts) {
      castStarts->push_back(se.cast<>());
    }
    return Fluent<>(
      share<Bytecode>(Bytecode(new List<Inst *>({new StartInst(castStarts)}))));
  };


  static Fluent<> __(const OBJ_OR_BYTECODE &start) {
    return Fluent<>(share<Bytecode>(Bytecode(new List<Inst *>({
      new StartInst(new List<Obj *>{start.cast<>()})
    }))));
  };

  static Fluent<> __() { return Fluent<>(); };

  inline static Fluent<> _ = __();
} // namespace fhatos

#endif
