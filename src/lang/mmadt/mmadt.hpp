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
#include "../../util/obj_helper.hpp"

namespace mmadt {
  using namespace std;
  using namespace fhatos;


  class _mmADT {
    //////////////////////////////////////////////////////////////////////////////
    ///////////////////////// PROTECTED  /////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////
    fURI tid;
    fURI domain;
    IntCoefficient dc = {1, 1};
    fURI range;
    IntCoefficient rc = {1, 1};

    [[nodiscard]] _mmADT extend(const Obj_p &inst) const {
      const auto insts = make_shared<List<Obj_p>>();
      for(const auto &old_inst: *this->bcode_->bcode_value()) {
        insts->push_back(old_inst);
      }
      insts->push_back(inst);
      return _mmADT(this->tid, this->domain, this->range, Obj::to_bcode(insts));
    }

    [[nodiscard]] _mmADT extend(const fURI &inst_op, const Obj_p &rhs = nullptr, const Obj_p &rhs1 = nullptr) const {
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

    static _mmADT __() {
      return _mmADT(*BCODE_FURI, *OBJ_FURI, *OBJ_FURI, Obj::to_bcode());
    }

    //////////////////////////////////////////////////////////////////////////////
    ///////////////////////// INSTRUCTIONS ///////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////

    [[nodiscard]] _mmADT and_(const Obj_p &rhs1, const Obj_p &rhs2, const Obj_p &rhs3 = nullptr,
                              const Obj_p &rhs4 = nullptr) const {
      const Lst_p ands = lst();
      ands->lst_add(rhs1);
      ands->lst_add(rhs2);
      if(rhs3)
        ands->lst_add(rhs3);
      if(rhs4)
        ands->lst_add(rhs4);
      return this->extend("and", ands);
    }

    [[nodiscard]] _mmADT or_(const Obj_p &rhs1, const Obj_p &rhs2, const Obj_p &rhs3 = nullptr,
                             const Obj_p &rhs4 = nullptr,const Obj_p &rhs5 = nullptr) const {
      const Lst_p ors = lst();
      ors->lst_add(rhs1);
      ors->lst_add(rhs2);
      if(rhs3)
        ors->lst_add(rhs3);
      if(rhs4)
        ors->lst_add(rhs4);
      if(rhs5)
        ors->lst_add(rhs5);
      return this->extend("or", ors);
    }

    [[nodiscard]] _mmADT isa(const Obj_p &rhs) const {
      return this->extend("isa", rhs);
    }

    [[nodiscard]] _mmADT isa(const fURI &rhs) const {
      return this->isa(vri(rhs));
    }

    [[nodiscard]] _mmADT as(const Obj_p &rhs) const {
      return this->extend("as", rhs);
    }

    [[nodiscard]] _mmADT as(const fURI &rhs) const {
      return this->as(vri(rhs));
    }

    [[nodiscard]] _mmADT at(const fURI &rhs) const {
      return this->at(vri(rhs));
    }

    [[nodiscard]] _mmADT at(const Uri_p &rhs) const {
      return this->extend("at", rhs);
    }


    [[nodiscard]] _mmADT start(const Obj_p &rhs) const {
      return this->extend("start", rhs);
    }


    /////////////////////////////////////////////////////////////////////
    //////////////////////////// COMPOSITION ////////////////////////////
    /////////////////////////////////////////////////////////////////////

    [[nodiscard]] _mmADT plus(const Obj_p &rhs) const {
      return this->extend("plus", rhs);
    }


    [[nodiscard]] _mmADT mult(const Obj_p &rhs) const {
      return this->extend("mult", rhs);
    }


    [[nodiscard]] _mmADT eq(const Obj_p &rhs) const {
      return this->extend("eq", rhs);
    }

    [[nodiscard]] _mmADT neq(const Obj_p &rhs) const {
      return this->extend("neq", rhs);
    }

    [[nodiscard]] _mmADT gt(const Obj_p &rhs) const {
      return this->extend("gt", rhs);
    }

    [[nodiscard]] _mmADT lt(const Obj_p &rhs) const {
      return this->extend("lt", rhs);
    }

    [[nodiscard]] _mmADT gte(const Obj_p &rhs) const {
      return this->extend("gte", rhs);
    }

    [[nodiscard]] _mmADT lte(const Obj_p &rhs) const {
      return this->extend("lte", rhs);
    }

    [[nodiscard]] _mmADT is(const Obj_p &rhs) const {
      return this->extend("is", rhs);
    }

    [[nodiscard]] _mmADT mod(const Obj_p &rhs) const {
      return this->extend("mod", rhs);
    }

    [[nodiscard]] _mmADT count() const {
      return this->extend("count");
    }

    [[nodiscard]] _mmADT to(const Obj_p &rhs) const {
      return this->extend("to", rhs);
    }

    [[nodiscard]] _mmADT to(const fURI &rhs) const {
      return this->extend("to", vri(rhs));
    }

    [[nodiscard]] _mmADT from(const Obj_p &rhs, const Obj_p &default_obj = Obj::to_noobj()) const {
      return this->extend("from", rhs, default_obj);
    }

    [[nodiscard]] _mmADT from(const fURI &rhs, const Obj_p &default_obj = Obj::to_noobj()) const {
      return this->from(vri(rhs), default_obj);
    }

    [[nodiscard]] _mmADT map(const Obj_p &rhs) const {
      return this->extend("map", rhs);
    }

    [[nodiscard]] _mmADT print(const Obj_p &rhs) const {
      return this->extend("print", rhs);
    }

    [[nodiscard]] _mmADT block(const Obj_p &rhs) const {
      return this->extend("block", rhs);
    }

    [[nodiscard]] _mmADT split(const Poly_p &rhs) const {
      return this->extend("split", rhs);
    }

    [[nodiscard]] _mmADT merge(const Int_p &rhs = jnt(INT_MAX)) const {
      return this->extend("merge", rhs);
    }

    [[nodiscard]] _mmADT each(const Poly_p &rhs) const {
      return this->extend("each", rhs);
    }

    [[nodiscard]] _mmADT else_(const Obj_p &rhs) const {
      return this->extend("else", rhs);
    }

    [[nodiscard]] _mmADT ref(const Obj_p &rhs) const {
      return this->extend("ref", rhs);
    }

    [[nodiscard]] _mmADT inst(const fURI &inst_furi, const Obj_p &rhs = nullptr, const Obj_p &rhs1 = nullptr) const {
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
      List_p<Obj_p> results{};

    public:
      explicit Result(const _mmADT &mmadt) {
        if(const Obj_p output = BCODE_PROCESSOR(mmadt.bcode_); output->is_objs())
          results = output->objs_value();
        else {
          results = make_shared<List<Obj_p>>();
          results->push_back(output);
        }
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
        if(this->results->empty())
          return Obj::to_noobj();
        const Obj_p o = this->results->back();
        //this->results->pop_back();
        return o;
      }

      [[nodiscard]] Objs_p to_objs() const {
        return Obj::to_objs(this->results);
      }
    };

    ///////////////////////////////////////////////////////////////////
    //////////////////////////// EVALUATE /////////////////////////////
    ///////////////////////////////////////////////////////////////////

    [[nodiscard]] Result compute() const {
      return Result(*this);
    }

  };

  static _mmADT __() {
    return _mmADT(*BCODE_FURI, *OBJ_FURI, *OBJ_FURI, Obj::to_bcode());
  }

  static _mmADT __(const fURI &tid, const fURI &range, const fURI &domain) {
    return _mmADT(tid, domain, range, Obj::to_bcode());
  }

  static _mmADT __(const Obj_p &start) {
    return __().map(start); // try start()
  }
} // namespace fhatos

#endif
