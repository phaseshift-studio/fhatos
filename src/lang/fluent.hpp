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
#pragma once
#ifndef fhatos_fluent_hpp
#define fhatos_fluent_hpp

#include "../fhatos.hpp"
//
#include "../lang/obj.hpp"
#include "../lang/processor/processor.hpp"

namespace fhatos {
  class Fluent {

    //////////////////////////////////////////////////////////////////////////////
    /////////////////////////    PUBLIC   ////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////
  public:
    explicit Fluent(const Obj_p& bcode) :
      _bcode(bcode) {
    }

    explicit Fluent() :
      Fluent(Obj::to_bcode()) {
    }

    static Fluent __() {
      return Fluent();
    }

    //////////////////////////////////////////////////////////////////////////////
    Obj_p next() const {
      if(!this->_bcode->is_bcode())
        return this->_bcode;
      return Processor::compute(this->_bcode);
    }

    void forEach(const Consumer<const Obj_p> &consumer) const {
    }

    ptr<List<Obj_p>> toList() const {
      ptr<List<Obj_p>> list = make_shared<List<Obj_p>>();
      this->forEach([list](const Obj_p &end) { list->push_back(end); });
      return list;
    }

    Objs_p toObjs() const {
      Objs_p xobjs = objs();
      this->forEach([xobjs](const Obj_p &end) { xobjs->add_obj(end); });
      return xobjs;
    }

    void iterate() const {
      this->forEach([](const Obj_p &) {
      });
    }

    string toString() const { return this->_bcode->toString(); }

    //////////////////////////////////////////////////////////////////////////////
    ///////////////////////// PROTECTED  /////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////

    const BCode_p _bcode;

  protected:
    [[nodiscard]] Fluent extend(const Obj_p &inst) const {
      const auto insts = make_shared<List<Obj_p>>();
      for(const auto &old_inst: *this->_bcode->bcode_value()) {
        insts->push_back(old_inst);
      }
      insts->push_back(inst);
      return Fluent(Obj::to_bcode(insts));
    }

  public:
    //////////////////////////////////////////////////////////////////////////////
    ///////////////////////// INSTRUCTIONS ///////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////

    [[nodiscard]] Fluent as(const fURI_p &type) const {
      return this->extend(Obj::to_inst(Obj::to_inst_args({vri(type)}), id_p(MMADT_SCHEME "/as")));
    }

    static Inst_p as_(const fURI_p &type) {
      return Fluent().as(type).back_();
    }

    Inst_p front_() const {
      return this->_bcode->bcode_value()->front();
    }

    Inst_p back_() const {
      return this->_bcode->bcode_value()->back();
    }


    Fluent start(const List<Obj> &starts) const {
      return Fluent();
      // return this->addInst(Insts::start(objs(PtrHelper::clone<Obj>(starts))));
    }

    Fluent start(const List<Obj_p> &starts) const {
      return Fluent();
      //return this->addInst(Insts::start(objs(starts)));
    }

    Fluent map(const BCode &bcode) const {
      return Fluent();
      //return this->addInst(Insts::map(share(bcode)));
    }

    Fluent filter(const BCode &bcode) const {
      return Fluent();
      //     return this->addInst(Insts::filter(share(bcode)));
    }

    Fluent side(const BCode &bcode) const {
      return Fluent();
      //return this->addInst(Insts::side(share(bcode)));
    }

    Fluent get(const Obj &key) const {
      return Fluent();
      //  return this->addInst(Insts::get(share(key)));
    }

    Fluent set(const Obj &key, const Obj &value) const {
      return Fluent();
      //return this->addInst(Insts::set(share(key), share(value)));
    }

    /////////////////////////////////////////////////////////////////////
    //////////////////////////// COMPOSITION ////////////////////////////
    /////////////////////////////////////////////////////////////////////

    Fluent plus(const Obj &rhs) const {
      return Fluent();
      //return this->addInst(Insts::plus(share(rhs)));
    }

    Fluent mult(const Obj &rhs) const {
      return Fluent();
      //return this->addInst(Insts::mult(share(rhs)));
    }

    ///////////////////////////////////////////////////////////////////
    //////////////////////////// RELATIONS ////////////////////////////
    ///////////////////////////////////////////////////////////////////

    Fluent eq(const Obj &rhs) const {
      return Fluent();
      //return this->addInst(Insts::eq(share(rhs)));
    }

    Fluent lte(const Obj &rhs) const {
      return Fluent();
      //return this->addInst(Insts::lte(share(rhs)));
    }

    Fluent lt(const Obj &rhs) const {
      return Fluent();
      //return this->addInst(Insts::lt(share(rhs)));
    }

    Fluent gte(const Obj &rhs) const {
      return Fluent();
      // return this->addInst(Insts::gte(share(rhs)));
    }

    Fluent gt(const Obj &rhs) const {
      return Fluent();
      //return this->addInst(Insts::gt(share(rhs)));
    }

    Fluent neq(const Obj &rhs) const {
      return Fluent();
      //return this->addInst(Insts::neq(share(rhs)));
    }

    Fluent mod(const Obj &rhs) const {
      return Fluent();
      //return this->addInst(Insts::mod(share(rhs)));
    }

    /////////////////////////////////////////////////////////////////////
    //////////////////////////// SIDE-EFFECT ////////////////////////////
    /////////////////////////////////////////////////////////////////////

    Fluent count() const {
      return Fluent();
      //return this->addInst(Insts::count());
    };

    Fluent to(const Uri &uri) const {
      return Fluent();
      //return this->addInst(Insts::to(share(uri)));
    }

    Fluent from(const Uri &uri) const {
      return Fluent();
      //return this->addInst(Insts::from(share(uri)));
    }

    /*Fluent explain() { return this->addInst(new ExplainInst()); }*/

    Fluent print(const Obj &toPrint) const {
      return Fluent();
      //return this->addInst(Insts::print(share(toPrint)));
    }

    ///////////////////////////////////////////////////////////////////
    //////////////////////////// BRANCHING ////////////////////////////
    ///////////////////////////////////////////////////////////////////

    /*Fluent bswitch(const std::initializer_list<Pair<const Obj, Obj>> &recPairs) const {
      return this->bswitch(Rec(recPairs));
    }*/

    //Fluent bswitch(const Rec &branches) const { return this->addInst(Insts::bswitch(share(branches))); }

    ///////////////////////////////////////////////////////////////////
    //////////////////////////// FILTERING ////////////////////////////
    ///////////////////////////////////////////////////////////////////

    Fluent is(const Obj &test) const {
      return Fluent();
      //return this->addInst(Insts::is(share(test)));
    }

    //  Fluent where(const OBJ_OR_BYTECODE &test) { return this->addInst(new WhereInst(test)); }

    /* Fluent pub(const Obj &target, const Obj &payload, const Bool &retain = true) const {
       return this->addInst(Insts::pub(share(target), share(payload), share(retain)));
     }

     Fluent sub(const Obj &pattern, const Obj &onRecv) const {
       return this->addInst(Insts::sub(share(pattern), share(onRecv)));
     }*/

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

    Fluent block(const Obj &obj) const {
      return Fluent();
      //return this->addInst(Insts::block(share(obj)));
    }

    Fluent as(const Obj &utype) const {
      return Fluent();
      //return this->addInst(Insts::as(share(utype)));
    }

    Fluent type() const {
      return Fluent();
      //return this->addInst(Insts::type());
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////    STATIC HELPERS   ///////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  [[maybe_unused]] static Fluent __(const initializer_list<Obj> &starts) {
    List<Obj> s = List<Obj>(starts);
    return Fluent();
    /*return Fluent(
            Obj::to_bcode(
                    s.empty() ? List<Obj_p>{} : List<Obj_p>{Insts::start(objs(PtrHelper::clone<Obj>(s)))}));*/
  }

  [[maybe_unused]] static Fluent __(const List<Obj> &starts) {
    return Fluent();
    /*  return Fluent(Obj::to_bcode(
              starts.empty() ? List<Obj_p>{} : List<Obj_p>{Insts::start(objs(PtrHelper::clone<Obj>(starts)))}));*/
  }

  [[maybe_unused]] static Fluent __(const Obj &start) {
    return start.is_noobj() ? __(List<Obj>{}) : __(List<Obj>{start});
  }

  [[maybe_unused]] static Fluent __() { return __(List<Obj>{}); }

  [[maybe_unused]] inline static Fluent _ = __();
} // namespace fhatos

#endif
