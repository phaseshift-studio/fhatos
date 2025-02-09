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
#ifndef mmadt_mmadt_hpp
#define mmadt_mmadt_hpp

#include "../../fhatos.hpp"
#include "../obj.hpp"

namespace mmadt {
  using namespace std;
  using namespace fhatos;

  class _mmADT {
    using _mmadt_p = shared_ptr<_mmADT>;
    //////////////////////////////////////////////////////////////////////////////
    ///////////////////////// PROTECTED  /////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////

    const BCode_p _bcode;
    fURI tid;
    fURI domain;
    IntCoefficient dc = {1, 1};
    fURI range;
    IntCoefficient rc = {1, 1};

    [[nodiscard]] _mmadt_p extend(const Obj_p &inst) const {
      const auto insts = make_shared<List<Obj_p>>();
      for(const auto &old_inst: *this->_bcode->bcode_value()) {
        insts->push_back(old_inst);
      }
      insts->push_back(inst);
      return make_shared<_mmADT>(this->tid, this->domain, this->range, Obj::to_bcode(insts));
    }

    [[nodiscard]] static BCode_p inst_to_bcode(const Inst_p &inst) {
      return inst->is_inst() ? std::get<Obj_p>(*inst->inst_f()) : inst;
    }

    //////////////////////////////////////////////////////////////////////////////
    /////////////////////////    PUBLIC   ////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////
  public:
    explicit _mmADT(const fURI &tid, const fURI &domain, const fURI &range, const BCode_p &bcode) :
      tid(tid), domain(domain), range(range), _bcode(bcode) {
    }

    string toString() const { return this->_bcode->toString(); }

    operator Obj_p() const {
      // Conversion logic here
      return InstBuilder::build(this->tid)
          ->domain_range(id_p(this->domain), this->dc, id_p(this->range), this->rc)
          ->inst_f(this->_bcode)
          ->create();
    }

    //////////////////////////////////////////////////////////////////////////////
    ///////////////////////// INSTRUCTIONS ///////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////

    [[nodiscard]] _mmadt_p as(const fURI_p &type) const {
      return this->extend(Obj::to_inst(Obj::to_inst_args({vri(type)}), id_p(MMADT_SCHEME "/as")));
    }


    /////////////////////////////////////////////////////////////////////
    //////////////////////////// COMPOSITION ////////////////////////////
    /////////////////////////////////////////////////////////////////////

    [[nodiscard]] _mmadt_p plus(const Obj_p &rhs) const {
      return this->extend(Obj::to_inst(Obj::to_inst_args({inst_to_bcode(rhs)}), id_p(MMADT_SCHEME "/plus")));
    }


    [[nodiscard]] _mmadt_p mult(const Obj_p &rhs) const {
      return this->extend(Obj::to_inst(Obj::to_inst_args({inst_to_bcode(rhs)}), id_p(MMADT_SCHEME "/mult")));
    }


    [[nodiscard]] _mmadt_p eq(const Obj_p &rhs) const {
      return this->extend(Obj::to_inst(Obj::to_inst_args({inst_to_bcode(rhs)}), id_p(MMADT_SCHEME "/eq")));
    }

    [[nodiscard]] _mmadt_p neq(const Obj_p &rhs) const {
      return this->extend(Obj::to_inst(Obj::to_inst_args({inst_to_bcode(rhs)}), id_p(MMADT_SCHEME "/neq")));
    }

    [[nodiscard]] _mmadt_p gt(const Obj_p &rhs) const {
      return this->extend(Obj::to_inst(Obj::to_inst_args({inst_to_bcode(rhs)}), id_p(MMADT_SCHEME "/gt")));
    }

    [[nodiscard]] _mmadt_p lt(const Obj_p &rhs) const {
      return this->extend(Obj::to_inst(Obj::to_inst_args({inst_to_bcode(rhs)}), id_p(MMADT_SCHEME "/lt")));
    }

    [[nodiscard]] _mmadt_p gte(const Obj_p &rhs) const {
      return this->extend(Obj::to_inst(Obj::to_inst_args({inst_to_bcode(rhs)}), id_p(MMADT_SCHEME "/gte")));
    }

    [[nodiscard]] _mmadt_p lte(const Obj_p &rhs) const {
      return this->extend(Obj::to_inst(Obj::to_inst_args({inst_to_bcode(rhs)}), id_p(MMADT_SCHEME "/lte")));
    }

    [[nodiscard]] _mmadt_p is(const Obj_p &rhs) const {
      return this->extend(Obj::to_inst(Obj::to_inst_args({inst_to_bcode(rhs)}), id_p(MMADT_SCHEME "/is")));
    }

    [[nodiscard]] _mmadt_p mod(const Obj_p &rhs) const {
      return this->extend(Obj::to_inst(Obj::to_inst_args({inst_to_bcode(rhs)}), id_p(MMADT_SCHEME "/mod")));
    }

    [[nodiscard]] _mmadt_p count() const {
      return this->extend(Obj::to_inst(Obj::to_inst_args(), id_p(MMADT_SCHEME "/count")));
    }

    [[nodiscard]] _mmadt_p to(const Obj_p &rhs) const {
      return this->extend(Obj::to_inst(Obj::to_inst_args({inst_to_bcode(rhs)}), id_p(MMADT_SCHEME "/to")));
    }

    [[nodiscard]] _mmadt_p from(const Obj_p &rhs) const {
      return this->extend(Obj::to_inst(Obj::to_inst_args({inst_to_bcode(rhs)}), id_p(MMADT_SCHEME "/from")));
    }

    [[nodiscard]] _mmadt_p print(const Obj_p &rhs) const {
      return this->extend(Obj::to_inst(Obj::to_inst_args({inst_to_bcode(rhs)}), id_p(MMADT_SCHEME "/print")));
    }

    [[nodiscard]] _mmadt_p as(const Obj_p &rhs) const {
      return this->extend(Obj::to_inst(Obj::to_inst_args({inst_to_bcode(rhs)}), id_p(MMADT_SCHEME "/as")));
    }

    [[nodiscard]] _mmadt_p block(const Obj_p &rhs) const {
      return this->extend(Obj::to_inst(Obj::to_inst_args({inst_to_bcode(rhs)}), id_p(MMADT_SCHEME "/block")));
    }

    [[nodiscard]] _mmadt_p split(const Poly_p &rhs) const {
      return this->extend(Obj::to_inst(Obj::to_inst_args({inst_to_bcode(rhs)}), id_p(MMADT_SCHEME "/split")));
    }

    [[nodiscard]] _mmadt_p merge(const Int_p &rhs = jnt(INT_MAX)) const {
      return this->extend(Obj::to_inst(Obj::to_inst_args({inst_to_bcode(rhs)}), id_p(MMADT_SCHEME "/merge")));
    }

    [[nodiscard]] _mmadt_p ref(const Obj_p &rhs) const {
      return this->extend(Obj::to_inst(Obj::to_inst_args({inst_to_bcode(rhs)}), id_p(MMADT_SCHEME "/to_inv")));
    }


    ///////////////////////////////////////////////////////////////////
    //////////////////////////// BRANCHING ////////////////////////////
    ///////////////////////////////////////////////////////////////////

    /*Fluent bswitch(const std::initializer_list<Pair<const Obj, Obj>> &recPairs) const {
      return this->bswitch(Rec(recPairs));
    }*/

    //Fluent bswitch(const Rec &branches) const { return this->addInst(Insts::bswitch(share(branches))); }
  };

  static ptr<_mmADT> __() {
    return make_shared<_mmADT>(*INST_FURI, *OBJ_FURI, *OBJ_FURI, Obj::to_bcode());
  }

  static ptr<_mmADT> __(const fURI &tid, const fURI &range, const fURI &domain) {
    return make_shared<_mmADT>(tid, domain, range, Obj::to_bcode());
  }
} // namespace fhatos

#endif
