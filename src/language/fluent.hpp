/*******************************************************************************
 FhatOS: A Distributed Operating System
 Copyright (c) 2024 PhaseShift Studio, LLC

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Affero General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Affero General Public License for more details.

 You should have received a copy of the GNU Affero General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#ifndef fhatos_fluent_hpp
#define fhatos_fluent_hpp

#include <fhatos.hpp>
//
#include <language/instructions.hpp>
#include <language/obj.hpp>
#include <language/processor.hpp>
#include <process/router/local_router.hpp>

namespace fhatos {
  template<typename ALGEBRA = FOS_DEFAULT_ALGEBRA, typename ROUTER = FOS_DEFAULT_ROUTER,
           typename PRINTER = FOS_DEFAULT_PRINTER>
  class Fluent {
    //////////////////////////////////////////////////////////////////////////////
    /////////////////////////    PUBLIC   ////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////
  public:
    explicit Fluent(const ptr<Bytecode> &bcode) : bcode(bcode) {}

    explicit Fluent(const ID &id = ID(*UUID::singleton()->mint(7))) : Fluent(share<Bytecode>(Bytecode(id))) {}

    //////////////////////////////////////////////////////////////////////////////
    template<typename E = Obj>
    const E *next() const {
      static Processor<E> proc = Processor<E>(this->bcode);
      return proc.next();
    }

    template<typename E = Obj>
    void forEach(const Consumer<const E *> &consumer) const {
      Processor<E> proc = Processor<E>(this->bcode);
      proc.forEach(consumer);
    }

    template<typename E = Obj>
    List<const E *> *toList() const {
      List<const E *> *list = new List<const E *>();
      this->template forEach<E>([list](const E *end) { list->push_back(end); });
      return list;
    }

    [[nodiscard]] string toString() const {
      return string("!y<!!")
          .append(this->bcode->id().toString().c_str())
          .append("!y>!!")
          .append(this->bcode->toString());
    }

    //////////////////////////////////////////////////////////////////////////////
    ///////////////////////// PROTECTED  /////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////

    const ptr<Bytecode> bcode;

  protected:
    Fluent<> addInst(Inst *inst) const { return Fluent<>(this->bcode->addInst(inst)); }

  public:
    //////////////////////////////////////////////////////////////////////////////
    ///////////////////////// INSTRUCTIONS ///////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////

    operator const OBJ_OR_BYTECODE &() const { return *new OBJ_OR_BYTECODE(new Bytecode(this->bcode.get()->value())); }

    Fluent start(const List<ptr<OBJ_OR_BYTECODE>> &starts) const {
      auto *castStarts = new List<Obj *>();
      for (const auto &start: starts) {
        castStarts->push_back(start->cast<Obj>());
      }
      return this->addInst(new StartInst(castStarts));
    }

    /////////////////////////////////////////////////////////////////////
    //////////////////////////// COMPOSITION ////////////////////////////
    /////////////////////////////////////////////////////////////////////

    Fluent plus(const OBJ_OR_BYTECODE &rhs) {
      return this->addInst(new CompositionInst<ALGEBRA>(ALGEBRA::COMPOSITION_OPERATOR::PLUS, rhs));
    }

    Fluent mult(const OBJ_OR_BYTECODE &rhs) {
      return this->addInst(new CompositionInst<ALGEBRA>(ALGEBRA::COMPOSITION_OPERATOR::MULT, rhs));
    }

    ///////////////////////////////////////////////////////////////////
    //////////////////////////// RELATIONS ////////////////////////////
    ///////////////////////////////////////////////////////////////////

    Fluent eq(const OBJ_OR_BYTECODE &rhs) {
      return this->addInst(new RelationalInst<ALGEBRA>(ALGEBRA::RELATION_PREDICATE::EQ, rhs));
    }

    Fluent neq(const OBJ_OR_BYTECODE &rhs) {
      return this->addInst(new RelationalInst<ALGEBRA>(ALGEBRA::RELATION_PREDICATE::NEQ, rhs));
    }

    Fluent gt(const OBJ_OR_BYTECODE &rhs) {
      return this->addInst(new RelationalInst<ALGEBRA>(ALGEBRA::RELATION_PREDICATE::GT, rhs));
    }


    Fluent gte(const OBJ_OR_BYTECODE &rhs) {
      return this->addInst(new RelationalInst<ALGEBRA>(ALGEBRA::RELATION_PREDICATE::GTE, rhs));
    }


    Fluent lt(const OBJ_OR_BYTECODE &rhs) {
      return this->addInst(new RelationalInst<ALGEBRA>(ALGEBRA::RELATION_PREDICATE::LT, rhs));
    }


    Fluent lte(const OBJ_OR_BYTECODE &rhs) {
      return this->addInst(new RelationalInst<ALGEBRA>(ALGEBRA::RELATION_PREDICATE::LTE, rhs));
    }

    /////////////////////////////////////////////////////////////////////
    //////////////////////////// SIDE-EFFECT ////////////////////////////
    /////////////////////////////////////////////////////////////////////

    Fluent ref(const URI_OR_BYTECODE &uri) { return this->addInst(new ReferenceInst<ROUTER>(uri)); }

    Fluent dref(const URI_OR_BYTECODE &uri) { return this->addInst(new DereferenceInst<ROUTER>(uri)); }

    Fluent explain() { return this->addInst(new ExplainInst()); }

    Fluent count() { return this->addInst(new CountInst()); }

    Fluent print(const OBJ_OR_BYTECODE &toPrint) { return this->addInst(new PrintInst<PRINTER>(toPrint)); }

    ///////////////////////////////////////////////////////////////////
    //////////////////////////// BRANCHING ////////////////////////////
    ///////////////////////////////////////////////////////////////////

    Fluent bswitch(const std::initializer_list<Pair<OBJ_OR_BYTECODE const, OBJ_OR_BYTECODE>> &recPairs) {
      const auto recMap = new RecMap<Obj *, Obj *>;
      for (const auto &[key, value]: recPairs) {
        recMap->insert({key.cast(), value.cast()});
      }
      return this->addInst(new BranchInst<ALGEBRA>(ALGEBRA::BRANCH_SEMANTIC::SWITCH, OBJ_OR_BYTECODE(new Rec(recMap))));
    }

    Fluent bswitch(const OBJ_OR_BYTECODE &branches) {
      return this->addInst(new BranchInst<ALGEBRA>(ALGEBRA::BRANCH_SEMANTIC::SWITCH, branches.cast<Rec>()));
    }

    ///////////////////////////////////////////////////////////////////
    //////////////////////////// FILTERING ////////////////////////////
    ///////////////////////////////////////////////////////////////////

    Fluent is(const OBJ_OR_BYTECODE &test) { return this->addInst(new IsInst(test)); }

    Fluent where(const OBJ_OR_BYTECODE &test) { return this->addInst(new WhereInst(test)); }

    Fluent publish(const URI_OR_BYTECODE &target, const OBJ_OR_BYTECODE &payload) {
      return this->addInst(new PublishInst<ROUTER>(target, payload, this->bcode->id()));
    }

    Fluent subscribe(const URI_OR_BYTECODE &pattern, const OBJ_OR_BYTECODE &onRecv) {
      return this->addInst(new SubscribeInst<ROUTER>(pattern, onRecv, this->bcode->id()));
    }

    Fluent as(const OBJ_OR_BYTECODE &utype) { return this->addInst(new AsInst<ROUTER>(utype)); }

    Fluent define(const URI_OR_BYTECODE &utype, const OBJ_OR_BYTECODE &typeDefinition) {
      return this->addInst(new DefineInst<ROUTER>(utype, typeDefinition));
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////    STATIC HELPERS   ///////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  static Fluent<> __(const List<OBJ_OR_BYTECODE> &starts) {
    if (starts.empty()) {
      return Fluent<>(share<Bytecode>(Bytecode(new List<Inst *>()))); // TODO: remove unnecesary [start]?
    } else {
      List<Obj *> *castStarts = new List<Obj *>();
      for (OBJ_OR_BYTECODE start: starts) {
        castStarts->push_back(start.cast<>());
      }
      return Fluent<>(share<Bytecode>(Bytecode(new List<Inst *>({new StartInst(castStarts)}))));
    }
  };

  static Fluent<> __(const OBJ_OR_BYTECODE &start) {
    return start.cast<>()->isNoObj() ? __(List<OBJ_OR_BYTECODE>({})) : __(List<OBJ_OR_BYTECODE>({start.cast<>()}));
  };

  static Fluent<> __() { return __(List<OBJ_OR_BYTECODE>{}); };

  inline static Fluent<> _ = __();
} // namespace fhatos

#endif
