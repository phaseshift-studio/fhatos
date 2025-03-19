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

#include <utility>

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
    fURI tid;
    fURI domain;
    IntCoefficient dc = {1, 1};
    fURI range;
    IntCoefficient rc = {1, 1};

    [[nodiscard]] _mmadt_p extend(const Obj_p &inst) const {
      const auto insts = make_shared<List<Obj_p>>();
      for(const auto &old_inst: *this->bcode_->bcode_value()) {
        insts->push_back(old_inst);
      }
      insts->push_back(inst);
      return make_shared<_mmADT>(this->tid, this->domain, this->range, Obj::to_bcode(insts));
    }

    [[nodiscard]] _mmadt_p extend(const fURI &inst_op, const Obj_p &rhs = nullptr, const Obj_p &rhs1 = nullptr) const {
      const Obj_p rhs_final = rhs
                                ? (rhs->is_bcode() && rhs->bcode_value()->size() == 1
                                     ? rhs->bcode_value()->front()
                                     : rhs)
                                : nullptr;
      const Obj_p rhs1_final = rhs1
                                 ? (rhs1->is_bcode() && rhs1->bcode_value()->size() == 1
                                      ? rhs1->bcode_value()->front()
                                      : rhs1)
                                 : nullptr;
      const InstArgs args = rhs1_final
                              ? Obj::to_inst_args({rhs_final, rhs1_final})
                              : (rhs_final
                                   ? Obj::to_inst_args({rhs_final})
                                   : Obj::to_inst_args());
      return this->extend(Obj::to_inst(args, id_p(inst_op)));
    }

    [[nodiscard]] static BCode_p inst_to_bcode(const Inst_p &inst) {
      return inst->is_inst() && inst->has_inst_f() && std::holds_alternative<Obj_p>(inst->inst_f())
               ? std::get<Obj_p>(inst->inst_f())
               : inst;
    }

    //////////////////////////////////////////////////////////////////////////////
    /////////////////////////    PUBLIC   ////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////
  public:
    const BCode_p bcode_;

    explicit _mmADT(const fURI &tid, const fURI &domain, const fURI &range, BCode_p bcode) :
      tid(tid), domain(domain), range(range), bcode_(std::move(bcode)) {
    }

    [[nodiscard]] string toString() const { return this->bcode_->toString(); }

    operator Obj_p() const {
      if(this->tid.equals(*BCODE_FURI))
        return this->bcode_;
      // Conversion logic here
      return InstBuilder::build(this->tid)
          ->domain_range(id_p(this->domain), this->dc, id_p(this->range), this->rc)
          ->inst_f(this->bcode_)
          ->create();
    }

    static _mmadt_p __() {
      return make_shared<_mmADT>(*BCODE_FURI, *OBJ_FURI, *OBJ_FURI, Obj::to_bcode());
    }

    //////////////////////////////////////////////////////////////////////////////
    ///////////////////////// INSTRUCTIONS ///////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////

    [[nodiscard]] _mmadt_p isa(const Obj_p &rhs) const {
      return this->extend("isa", rhs);
    }

    [[nodiscard]] _mmadt_p isa(const fURI &rhs) const {
      return this->extend("isa", vri(rhs));
    }

    [[nodiscard]] _mmadt_p a(const Obj_p &rhs) const {
      return this->extend("a", rhs);
    }

    [[nodiscard]] _mmadt_p as(const Obj_p &rhs) const {
      return this->extend("as", rhs);
    }

    [[nodiscard]] _mmadt_p as(const fURI &rhs) const {
      return this->extend("as", vri(rhs));
    }

    [[nodiscard]] _mmadt_p start(const Obj_p &rhs) const {
      return this->extend("start", rhs);
    }


    /////////////////////////////////////////////////////////////////////
    //////////////////////////// COMPOSITION ////////////////////////////
    /////////////////////////////////////////////////////////////////////

    [[nodiscard]] _mmadt_p plus(const Obj_p &rhs) const {
      return this->extend("plus", rhs);
    }


    [[nodiscard]] _mmadt_p mult(const Obj_p &rhs) const {
      return this->extend("mult", rhs);
    }


    [[nodiscard]] _mmadt_p eq(const Obj_p &rhs) const {
      return this->extend("eq", rhs);
    }

    [[nodiscard]] _mmadt_p neq(const Obj_p &rhs) const {
      return this->extend("neq", rhs);
    }

    [[nodiscard]] _mmadt_p gt(const Obj_p &rhs) const {
      return this->extend("gt", rhs);
    }

    [[nodiscard]] _mmadt_p lt(const Obj_p &rhs) const {
      return this->extend("lt", rhs);
    }

    [[nodiscard]] _mmadt_p gte(const Obj_p &rhs) const {
      return this->extend("gte", rhs);
    }

    [[nodiscard]] _mmadt_p lte(const Obj_p &rhs) const {
      return this->extend("lte", rhs);
    }

    [[nodiscard]] _mmadt_p is(const Obj_p &rhs) const {
      return this->extend("is", rhs);
    }

    [[nodiscard]] _mmadt_p mod(const Obj_p &rhs) const {
      return this->extend("mod", rhs);
    }

    [[nodiscard]] _mmadt_p count() const {
      return this->extend("count");
    }

    [[nodiscard]] _mmadt_p to(const Obj_p &rhs) const {
      return this->extend("to", rhs);
    }

    [[nodiscard]] _mmadt_p to(const fURI &rhs) const {
      return this->extend("to", vri(rhs));
    }

    [[nodiscard]] _mmadt_p from(const Obj_p &rhs) const {
      return this->extend("from", rhs);
    }

    [[nodiscard]] _mmadt_p from(const fURI &rhs) const {
      return this->from(vri(rhs));
    }

    [[nodiscard]] _mmadt_p map(const Obj_p &rhs) const {
      return this->extend("map", rhs);
    }

    [[nodiscard]] _mmadt_p print(const Obj_p &rhs) const {
      return this->extend("print", rhs);
    }

    [[nodiscard]] _mmadt_p block(const Obj_p &rhs) const {
      return this->extend("block", rhs);
    }

    [[nodiscard]] _mmadt_p split(const Poly_p &rhs) const {
      return this->extend("split", rhs);
    }

    [[nodiscard]] _mmadt_p merge(const Int_p &rhs = jnt(INT_MAX)) const {
      return this->extend("merge", rhs);
    }

    [[nodiscard]] _mmadt_p each(const Poly_p &rhs) const {
      return this->extend("each", rhs);
    }

    [[nodiscard]] _mmadt_p ref(const Obj_p &rhs) const {
      return this->extend("ref", rhs);
    }

    [[nodiscard]] _mmadt_p inst(const fURI &inst_furi, const Obj_p &rhs = nullptr, const Obj_p &rhs1 = nullptr) const {
      return this->extend(inst_furi, rhs, rhs1);
    }


    ///////////////////////////////////////////////////////////////////
    //////////////////////////// BRANCHING ////////////////////////////
    ///////////////////////////////////////////////////////////////////

    /*Fluent bswitch(const std::initializer_list<Pair<const Obj, Obj>> &recPairs) const {
      return this->bswitch(Rec(recPairs));
    }*/

    //Fluent bswitch(const Rec &branches) const { return this->addInst(Insts::bswitch(share(branches))); }

    class Result {
      const List_p<Obj_p> results{};

    public:
      explicit Result(const _mmADT *mmadt):
        results{BCODE_PROCESSOR(mmadt->bcode_)->objs_value()} {
      }

      [[nodiscard]] bool exists() const {
        return !results->empty() && !results->front()->is_noobj();
      }

      [[nodiscard]] std::vector<Obj_p>::iterator begin() const {
        return this->results->begin();
      }

      [[nodiscard]] std::vector<Obj_p>::iterator end() const {
        return this->results->end();
      }

      [[nodiscard]] Obj_p next() const {
        const Obj_p o = this->results->back();
        this->results->pop_back();
        return o;
      }
    };

    ///////////////////////////////////////////////////////////////////
    //////////////////////////// EVALUATE /////////////////////////////
    ///////////////////////////////////////////////////////////////////

    Result compute() {
      return Result(this);
    }

  };

  static ptr<_mmADT> __() {
    return make_shared<_mmADT>(*BCODE_FURI, *OBJ_FURI, *OBJ_FURI, Obj::to_bcode());
  }

  static ptr<_mmADT> __(const fURI &tid, const fURI &range, const fURI &domain) {
    return make_shared<_mmADT>(tid, domain, range, Obj::to_bcode());
  }

  static ptr<_mmADT> __(const Obj_p &start) {
    return __()->map(start);
  }
} // namespace fhatos

#endif
