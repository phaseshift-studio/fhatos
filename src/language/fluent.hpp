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

namespace fhatos {
  template<typename ALGEBRA = FOS_DEFAULT_ALGEBRA, typename ROUTER = FOS_DEFAULT_ROUTER,
           typename PRINTER = FOS_DEFAULT_PRINTER>
  class Fluent {
    //////////////////////////////////////////////////////////////////////////////
    /////////////////////////    PUBLIC   ////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////
  public:
    explicit Fluent(const ptr<Bytecode> &bcode) : bcode(bcode) {}

    explicit Fluent(const ID &id = ID(*UUID::singleton()->mint(7))) :
        Fluent(share<Bytecode>(Bytecode(share<List<ptr<Inst>>>(List<ptr<Inst>>{})))) {}

    //////////////////////////////////////////////////////////////////////////////
    template<typename E = Obj>
    const ptr<E> next() const {
      static Processor<E> proc = Processor<E>(this->bcode);
      return proc.next();
    }

    template<typename E = Obj>
    void forEach(const Consumer<const ptr<E>> &consumer) const {
      Processor<E> proc = Processor<E>(this->bcode);
      proc.forEach(consumer);
    }

    template<typename E = Obj>
    List<ptr<E>> *toList() const {
      List<ptr<E>> *list = new List<ptr<E>>();
      this->template forEach<E>([list](const ptr<E> end) { list->push_back(end); });
      return list;
    }

    string toString() const {
      return string("!y<!!123")
          //.append(this->bcode->id().toString().c_str())
          .append("!y>!!")
          .append(this->bcode->toString());
    }

    //////////////////////////////////////////////////////////////////////////////
    ///////////////////////// PROTECTED  /////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////

    const ptr<Bytecode> bcode;

  protected:
    Fluent<> addInst(const ptr<Inst> &inst) const { return Fluent<>(this->bcode->addInst(inst)); }

  public:
    //////////////////////////////////////////////////////////////////////////////
    ///////////////////////// INSTRUCTIONS ///////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////

    explicit operator const Bytecode &() const { return *this->bcode; }

    const ptr<Bytecode> code() const { return this->bcode; }

    Fluent start(const ptr<List<ptr<Obj>>> &starts) const {
      /*auto castStarts = List<ptr<Obj>>();
      for (const auto &start: *starts) {
        castStarts.push_back(ptr<Obj>(ObjHelper::cast(*start)));
      }*/
      return this->addInst(ptr<Inst>(new StartInst(*starts)));
    }

    /////////////////////////////////////////////////////////////////////
    //////////////////////////// COMPOSITION ////////////////////////////
    /////////////////////////////////////////////////////////////////////

    Fluent plus(const Obj &rhs) {
      return this->addInst(
          ptr<Inst>(new CompositionInst<ALGEBRA>(ALGEBRA::COMPOSITION_OPERATOR::PLUS, ObjHelper::cast(rhs))));
    }

    Fluent mult(const Obj &rhs) {
      return this->addInst(
          ptr<Inst>(new CompositionInst<ALGEBRA>(ALGEBRA::COMPOSITION_OPERATOR::MULT, ObjHelper::cast(rhs))));
    }

    Fluent mod(const Obj &rhs) {
      return this->addInst(
          ptr<Inst>(new CompositionInst<ALGEBRA>(ALGEBRA::COMPOSITION_OPERATOR::MOD, ObjHelper::cast(rhs))));
    }

    ///////////////////////////////////////////////////////////////////
    //////////////////////////// RELATIONS ////////////////////////////
    ///////////////////////////////////////////////////////////////////

    Fluent eq(const Obj &rhs) {
      return this->addInst(
          ptr<Inst>(new RelationalInst<ALGEBRA>(ALGEBRA::RELATION_PREDICATE::EQ, ObjHelper::cast(rhs))));
    }

    Fluent neq(const Obj &rhs) {
      return this->addInst(
          ptr<Inst>(new RelationalInst<ALGEBRA>(ALGEBRA::RELATION_PREDICATE::NEQ, ObjHelper::cast(rhs))));
    }

    Fluent gt(const Obj &rhs) {
      return this->addInst(
          ptr<Inst>(new RelationalInst<ALGEBRA>(ALGEBRA::RELATION_PREDICATE::GT, ObjHelper::cast(rhs))));
    }


    Fluent gte(const Obj &rhs) {
      return this->addInst(
          ptr<Inst>(new RelationalInst<ALGEBRA>(ALGEBRA::RELATION_PREDICATE::GTE, ObjHelper::cast(rhs))));
    }


    Fluent lt(const Obj &rhs) {
      return this->addInst(
          ptr<Inst>(new RelationalInst<ALGEBRA>(ALGEBRA::RELATION_PREDICATE::LT, ObjHelper::cast(rhs))));
    }


    Fluent lte(const Obj &rhs) {
      return this->addInst(
          ptr<Inst>(new RelationalInst<ALGEBRA>(ALGEBRA::RELATION_PREDICATE::LTE, ObjHelper::cast(rhs))));
    }

    /////////////////////////////////////////////////////////////////////
    //////////////////////////// SIDE-EFFECT ////////////////////////////
    /////////////////////////////////////////////////////////////////////

    Fluent ref(const Uri &uri) { return this->addInst(ptr<Inst>(new ReferenceInst<ROUTER>(share<Uri>(uri)))); }

    Fluent dref(const Uri &uri) { return this->addInst(ptr<Inst>(new DereferenceInst<ROUTER>(share<Uri>(uri)))); }

    /*Fluent explain() { return this->addInst(new ExplainInst()); }

    Fluent count() { return this->addInst(new CountInst()); }*/

    Fluent print(const Obj &toPrint) { return this->addInst(ptr<Inst>(new PrintInst<PRINTER>(share<Obj>(toPrint)))); }

    ///////////////////////////////////////////////////////////////////
    //////////////////////////// BRANCHING ////////////////////////////
    ///////////////////////////////////////////////////////////////////

    Fluent bswitch(const std::initializer_list<Pair<Obj const, Obj>> &recPairs) {
      RecMap<> recMap = RecMap<>();
      for (const auto &[key, value]: recPairs) {
        recMap.insert({share<Obj>(key), share<Obj>(value)});
      }
      return this->addInst(
          ptr<Inst>(new BranchInst<ALGEBRA>(ALGEBRA::BRANCH_SEMANTIC::SWITCH, share<Rec>(Rec(recMap)))));
    }

    Fluent bswitch(const Rec &branches) {
      return this->addInst(ptr<Inst>(new BranchInst<ALGEBRA>(ALGEBRA::BRANCH_SEMANTIC::SWITCH, share<Rec>(branches))));
    }

    ///////////////////////////////////////////////////////////////////
    //////////////////////////// FILTERING ////////////////////////////
    ///////////////////////////////////////////////////////////////////

    Fluent is(const Bool &test) { return this->addInst(ptr<Inst>(new IsInst(share<Bool>(test)))); }

    //  Fluent where(const OBJ_OR_BYTECODE &test) { return this->addInst(new WhereInst(test)); }

    Fluent publish(const ptr<Uri> &target, const ptr<Obj> &payload) {
      return this->addInst(ptr<Inst>(new PublishInst<ROUTER>(target, payload, ID("123") /*this->bcode->id()*/)));
    }

    Fluent subscribe(const ptr<Uri> &pattern, const ptr<Bytecode> &onRecv) {
      return this->addInst(ptr<Inst>(new SubscribeInst<ROUTER>(pattern, onRecv, ID("123") /*this->bcode->id()*/)));
    }

    Fluent select(const List<ptr<Uri>> &uris) {
      List<ptr<Obj>> castObjs = List<ptr<Obj>>();
      for (ptr<Uri> uri: uris) {
        castObjs.push_back(uri);
      }
      return this->addInst(new SelectInst<ROUTER>(castObjs));
    }

    /*Fluent select(const List<OBJ_OR_BYTECODE> &uris) {
      List<Obj *> *castObjs = new List<Obj *>();
      for (OBJ_OR_BYTECODE uri: uris) {
        castObjs->push_back(uri.cast<>());
      }
      return this->addInst(new SelectInst<ROUTER>(castObjs));
    }

    Fluent select(const OBJ_OR_BYTECODE &branches) {
      return this->addInst(new SelectInst<ROUTER>(branches.cast<Rec>()));
    }*/

    /*Fluent as(const OBJ_OR_BYTECODE &utype) { return this->addInst(new AsInst<ROUTER>(utype)); }

    Fluent as() { return this->addInst(new AsInst<ROUTER>(NoObj::singleton())); }*/

    Fluent define(const ptr<Type> &type, const ptr<Bytecode> &typeDefinition) {
      return this->addInst(ptr<Inst>(new DefineInst<ROUTER>(type, typeDefinition)));
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////    STATIC HELPERS   ///////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  static Fluent<> __(const List<Obj> &starts) {
    if (starts.empty()) {
      return Fluent<>(ptr<Bytecode>(new Bytecode(ptr<List<ptr<Inst>>>(new List<ptr<Inst>>{}))));
    } else {
      List<ptr<Obj>> castStarts = List<ptr<Obj>>();
      for (const Obj &start: starts) {
        castStarts.push_back(ObjHelper::cast(start));
      }
      return Fluent(ptr<Bytecode>(
          new Bytecode(ptr<List<ptr<Inst>>>(new List<ptr<Inst>>{ptr<StartInst>(new StartInst(castStarts))}))));
    }
  };

  static Fluent<> __(const Obj &start) { return start.isNoObj() ? __(List<Obj>{}) : __(List<Obj>{start}); };

  static Fluent<> __() { return __(List<Obj>{}); };

  // inline static const Fluent<> _ = __();
} // namespace fhatos

#endif
