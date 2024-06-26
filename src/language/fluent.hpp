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
    explicit Fluent(const Obj_p &bcode) : bcode(bcode) {}

    explicit Fluent(const ID &id = ID(*UUID::singleton()->mint(7))) : Fluent(Obj::to_bcode(List<Obj_p>({}))) {}

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
    ptr<List<ptr<E>>> toList() const {
      List<ptr<E>> *list = new List<ptr<E>>();
      this->template forEach<E>([list](const ptr<E> end) { list->push_back(end); });
      return ptr<List<ptr<E>>>(list);
    }

    string toString() const { return this->bcode->toString(); }

    //////////////////////////////////////////////////////////////////////////////
    ///////////////////////// PROTECTED  /////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////

    const Obj_p bcode;

  protected:
    Fluent<> addInst(const Obj_p &inst) const {
      List<Obj_p> newList = List<Obj_p>();
      for (const auto &oldInst: this->bcode->bcode_value()) {
        newList.push_back(share(Obj(*oldInst)));
      }
      newList.push_back(share(Obj(*inst)));
      return Fluent<>(Obj::to_bcode(newList));
    }

  public:
    //////////////////////////////////////////////////////////////////////////////
    ///////////////////////// INSTRUCTIONS ///////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////

    // operator const Obj &() const { return *ObjHelper::clone<Bytecode>(this->bcode.get()); }

    operator const Obj &() const { return *this->bcode; }
    //  operator const ptr<Bytecode> &() const { return this->bcode; }

    Fluent start(const List<Obj> &starts) const { return this->addInst(Insts::start(Obj::cast(starts))); }
    Fluent start(const List<Obj_p> &starts) const { return this->addInst(Insts::start(starts)); }

    /////////////////////////////////////////////////////////////////////
    //////////////////////////// COMPOSITION ////////////////////////////
    /////////////////////////////////////////////////////////////////////

    Fluent plus(const Obj &rhs) { return this->addInst(Insts::plus(share(rhs))); }
    Fluent mult(const Obj &rhs) { return this->addInst(Insts::mult(share(rhs))); }

    ///////////////////////////////////////////////////////////////////
    //////////////////////////// RELATIONS ////////////////////////////
    ///////////////////////////////////////////////////////////////////

    Fluent eq(const Obj &rhs) { return this->addInst(Insts::eq(share(rhs))); }
    Fluent lte(const Obj &rhs) { return this->addInst(Insts::lte(share(rhs))); }
    Fluent lt(const Obj &rhs) { return this->addInst(Insts::lt(share(rhs))); }
    Fluent gte(const Obj &rhs) { return this->addInst(Insts::gte(share(rhs))); }
    Fluent gt(const Obj &rhs) { return this->addInst(Insts::gt(share(rhs))); }
    Fluent neq(const Obj &rhs) { return this->addInst(Insts::neq(share(rhs))); }
    Fluent mod(const Obj &rhs) { return this->addInst(Insts::mod(share(rhs))); }

    /////////////////////////////////////////////////////////////////////
    //////////////////////////// SIDE-EFFECT ////////////////////////////
    /////////////////////////////////////////////////////////////////////

    Fluent to(const Uri &uri) { return this->addInst(Insts::to(share(uri))); }
    Fluent from(const Uri &uri) { return this->addInst(Insts::from(share(uri))); }

    /*Fluent explain() { return this->addInst(new ExplainInst()); }

    Fluent count() { return this->addInst(new CountInst()); }*/

    Fluent print(const Obj &toPrint) { return this->addInst(Insts::print(share(toPrint))); }

    ///////////////////////////////////////////////////////////////////
    //////////////////////////// BRANCHING ////////////////////////////
    ///////////////////////////////////////////////////////////////////

    Fluent bswitch(const std::initializer_list<Pair<const Obj, Obj>> &recPairs) {
      return this->addInst(Insts::bswitch(share(Rec(recPairs))));
    }

    Fluent bswitch(const Rec &branches) { return this->addInst(Insts::bswitch(share(branches))); }

    ///////////////////////////////////////////////////////////////////
    //////////////////////////// FILTERING ////////////////////////////
    ///////////////////////////////////////////////////////////////////

    Fluent is(const Obj &test) { return this->addInst(Insts::is(share(test))); }

    //  Fluent where(const OBJ_OR_BYTECODE &test) { return this->addInst(new WhereInst(test)); }

    Fluent publish(const Obj &target, const Obj &payload) {
      return this->addInst(Insts::publish(share(target), share(payload)));
    }

    Fluent subscribe(const Obj &pattern, const Obj &onRecv) {
      return this->addInst(Insts::subscibe(share(pattern), share(onRecv)));
    }

    /*Fluent select(const List<ptr<Uri>> &uris) { return this->addInst(ptr<Inst>(new SelectInst<ROUTER>(uris))); }

    Fluent select(const List<Uri> &uris) {
      List<ptr<Uri>> castObjs = List<ptr<Uri>>();
      for (auto &uri: uris) {
        castObjs.push_back(share<Uri>(uri));
      }
      return this->addInst(ptr<Inst>(new SelectInst<ROUTER>(castObjs)));
    }

    Fluent select(const Rec &branches) {
      return this->addInst(ptr<Inst>(new SelectInst<ROUTER>(share<Rec>(branches))));
    }*/

    Fluent as(const Obj &utype) { return this->addInst(Insts::as(share(utype))); }

    Fluent type(const Obj &obj) { return this->addInst(Insts::type(share(obj))); }

    Fluent define(const Obj &type, const Obj &typeDefinition) {
      return this->addInst(Insts::define(share(type), share(typeDefinition)));
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////    STATIC HELPERS   ///////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  static Fluent<> __(const List<Obj> &starts) {
    if (starts.empty()) {
      return Fluent(Obj::to_bcode(List<Obj_p>{}));
    } else {
      return Fluent(Obj::to_bcode(List<Obj_p>{Insts::start(Obj::cast(starts))}));
    }
  };

  static Fluent<> __(const Obj &start) { return start.isNoObj() ? __(List<Obj>{}) : __(List<Obj>{start}); };

  static Fluent<> __() { return __(List<Obj>{}); };

  inline static Fluent<> _ = __();
} // namespace fhatos

#endif
