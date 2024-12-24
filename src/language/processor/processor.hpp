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
#ifndef fhatos_processor_hpp
#define fhatos_processor_hpp

#include "../../fhatos.hpp"
#include "../../language/obj.hpp"
#include "../../language/rewrite/rewriter.hpp"

namespace fhatos {
  ///////////////////////////////////////////////////////////////////////////
  /////////////////////////////// PROCESSOR /////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  class Processor final : public Valued { // : public IDed, Typed or perhaps just full Obj`
    class Monad;
    using Monad_p = ptr<Processor::Monad>;

  protected:
    BCode_p bcode_;
    unique_ptr<Deque<Monad_p>> running_;
    unique_ptr<Deque<Monad_p>> barriers_;
    unique_ptr<Deque<Obj_p>> halted_;

    explicit Processor(const BCode_p &bcode) : Valued(ID(to_string(rand()).insert(0, "/sys/processor/"))),
                                               bcode_(bcode),
                                               running_(make_unique<Deque<Monad_p>>()),
                                               barriers_(make_unique<Deque<Monad_p>>()),
                                               halted_(make_unique<Deque<Obj_p>>()) {
      if(!this->bcode_->is_code()) {
        if(!this->bcode_->is_noobj()) {
          // monad halts immediately on a non-bcode submission
          this->halted_->push_back(bcode);
        }
      } else {
        // if a single inst, wrap in bcode
        if(this->bcode_->is_inst())
          this->bcode_ = Obj::to_bcode({this->bcode_},this->bcode_->tid());
        // process bcode inst pipeline
        this->bcode_ = Rewriter({Rewriter::by(), Rewriter::explain()}).apply(this->bcode_);
        // setup global behavior around barriers, initials, and terminals
        bool first = true;
        for(const Inst_p &inst: *this->bcode_->bcode_value()) {
          const Inst_p resolved = TYPE_INST_RESOLVER(Obj::to_type(OBJ_FURI), inst);
          const Obj_p seed_copy = resolved->inst_seed(resolved);
          if(is_gather(resolved->itype())) {
            // MANY_TO_??
            const Monad_p m = M(seed_copy, inst);
            this->barriers_->push_back(m);
            LOG_OBJ(DEBUG, this, FOS_TAB_2 "!ybarrier!! monad created: %s\n", m->toString().c_str());
          } else if(is_initial(resolved->itype()) || (first && is_maybe_initial(resolved->itype()))) {
            // ZERO/MAYBE*-TO_??
            const Monad_p m = M(noobj(), inst); // TODO: use seed
            this->running_->push_back(m);
            LOG_OBJ(DEBUG, this, FOS_TAB_2 "!ginitial!! monad created: %s\n", m->toString().c_str());
          }
          first = false;
        }
        // start inst forced initial TODO: remove this as it's not sound
        if(this->running_->empty()) {
          //const Obj_p seed_copy = Objs::to_objs();
          const Obj_p seed_copy = this->bcode_->bcode_value()->front()->inst_seed(this->bcode_->bcode_value()->front());
          this->running_->push_back(
            M(seed_copy, this->bcode_->bcode_value()->front()));
        }
      }
    }

    [[nodiscard]] ID_p vid_or_tid() const {
      return this->vid_;
    }

    [[nodiscard]] Monad_p M(const Obj_p &obj, const Inst_p &inst) const {
      return make_shared<Processor::Monad>(this, obj, inst);
    }


    [[nodiscard]] Obj_p next(const int steps = -1) const {
      while(true) {
        // Process::current_process()->feed_watchdog_via_counter();
        if(this->halted_->empty()) {
          if(this->running_->empty())
            return nullptr;
          this->execute(steps);
        } else {
          const Obj_p end = this->halted_->front();
          this->halted_->pop_front();
          LOG_OBJ(TRACE, this, FOS_TAB_2 "!ghalting!! monad at %s\n", end->toString().c_str());
          if(!end->is_noobj())
            return end;
        }
      }
    }

    [[nodiscard]] Objs_p to_objs() const {
      const Objs_p objs = Obj::to_objs();
      Obj_p end;
      while(nullptr != (end = this->next())) {
        objs->add_obj(end);
      }
      LOG_OBJ(TRACE, this, "%s\n", Ansi<>::singleton()->silly_print("processor shutting down",true,false).c_str());
      return objs;
    }

    int execute(const int steps = -1) const {
      uint16_t counter = 0;
      while((!this->running_->empty() || !this->barriers_->empty()) && (counter++ < steps || steps == -1)) {
        if(!this->running_->empty()) {
          const Monad_p m = this->running_->front();
          this->running_->pop_front();
          m->loop();
        } else if(!this->barriers_->empty()) {
          const Monad_p barrier = this->barriers_->front();
          this->barriers_->pop_front();
          LOG_OBJ(DEBUG, this, "processing barrier: %s\n", barrier->toString().c_str());
          barrier->loop();
        }
      }
      LOG_OBJ(TRACE, this, FOS_TAB_2 "exiting current run with [!ghalted!!:%i] [!yrunning!!:%i]: %s\n",
              this->running_->size(),
              this->halted_->size(), this->bcode_->toString().c_str());
      return this->halted_->size();
    }

  public:
    static Objs_p compute(const BCode_p &bcode) {
      //ROUTER_PUSH_FRAME("+", Obj::to_inst_args());
      const Obj_p objs = Processor(bcode).to_objs();
      return objs;
    }

    ///////////////////////////////////////////////////////////////////////////
   ///////////////////////////////// MONAD ///////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  private:
    class Monad : public enable_shared_from_this<Processor::Monad> { // OBJ as well?
    protected:
      const Processor *processor_;
      const Obj_p obj_;
      const Inst_p inst_;
      const long bulk_ = 1;
      uint16_t loops_ = 0;

    public:
      Monad() = delete;

      explicit Monad(const Processor *processor, const Obj_p &obj, const Inst_p &inst) : processor_(processor),
        obj_(obj->clone()), inst_(inst) {
      };

      void loop() const {
        if(this->halted()) {
          if(!this->dead()) {
            LOG_OBJ(TRACE, this->processor_, "monad %s halting\n", this->toString().c_str());
            this->processor_->halted_->push_back(this->obj_);
          }
        } else {
          const Inst_p current_inst_resolved = TYPE_INST_RESOLVER(this->obj_, this->inst_);
          LOG_OBJ(TRACE, this->processor_, "monad %s applying to resolved inst %s !m=>!! %s [!m%s!!]\n",
                  this->toString().c_str(),
                  this->inst_->toString().c_str(),
                  current_inst_resolved->toString().c_str(),
                  ITypeSignatures.to_chars(current_inst_resolved->itype()).c_str());
          this->domain_loop(current_inst_resolved);
        }
      }

      ///////////////////////////////////////////////////////////////////////////

      void domain_loop(const Inst_p &current_inst_resolved) const {
        LOG_OBJ(TRACE, this->processor_, FOS_TAB_2 "monad at !gdomain!! of %s !m=>!! %s [!m%s!m]\n",
                this->toString().c_str(),
                current_inst_resolved->toString().c_str(),
                ITypeDescriptions.to_chars(current_inst_resolved->itype()).c_str());
        if(is_gather(current_inst_resolved->itype())) {
          if(this->obj_->is_objs()) {
            LOG_OBJ(TRACE, this->processor_, "barrier monad [size: %i] fetch for processing by %s [!m%s!m]\n",
                    this->obj_->objs_value()->size(),
                    current_inst_resolved->toString().c_str(),
                    ITypeDescriptions.to_chars(current_inst_resolved->itype()).c_str());
            range_loop(current_inst_resolved->apply(this->obj_), current_inst_resolved);
          } else {
            LOG_OBJ(TRACE, this->processor_, "monad %s stored in barrier [size: %i] [!m%s!m]\n",
                    this->toString().c_str(),
                    this->processor_->barriers_->front()->obj_->objs_value()->size(),
                    ITypeDescriptions.to_chars(current_inst_resolved->itype()).c_str());
            this->processor_->barriers_->front()->obj_->add_obj(this->obj_);
          }
        } else {
          this->obj_->CHECK_OBJ_TO_INST_SIGNATURE(current_inst_resolved, true);
          range_loop(current_inst_resolved->apply(this->obj_), current_inst_resolved);
        }
      }

      ///////////////////////////////////////////////////////////////////////////

      void range_loop(const Obj_p &next_obj, const Inst_p &current_inst_resolved) const {
        LOG_OBJ(TRACE, this->processor_, FOS_TAB_2 "monad at !grange!! of %s !m=>!! %s [%s]\n",
                this->processor_->M(next_obj,this->inst_)->toString().c_str(),
                current_inst_resolved->toString().c_str(),
                ITypeSignatures.to_chars(current_inst_resolved->itype()).c_str());
        next_obj->CHECK_OBJ_TO_INST_SIGNATURE(current_inst_resolved, false);
        if(is_scatter(current_inst_resolved->itype())) {
          for(const Obj_p &obj: *next_obj->objs_value()) {
            const Monad_p m = this->processor_->M(obj, this->processor_->bcode_->next_inst(this->inst_));
            this->processor_->running_->push_back(m);
          }
        } else {
          const Monad_p m = this->processor_->M(next_obj, this->processor_->bcode_->next_inst(this->inst_));
          LOG_OBJ(TRACE, this->processor_, "monad %s !r==!gmigrating!r==>!! %s\n", this->toString().c_str(),
                  m->toString().c_str());
          this->processor_->running_->push_back(m);
        }
      }

      [[nodiscard]] Obj_p obj() const { return this->obj_; }

      [[nodiscard]] Inst_p inst() const { return this->inst_; }

      [[nodiscard]] long bulk() const { return this->bulk_; }

      [[nodiscard]] uint16_t loops() const { return this->loops_; }

      [[nodiscard]] bool halted() const { return this->inst_->is_noobj(); }

      [[nodiscard]] bool dead() const { return this->obj_->is_noobj(); }

      [[nodiscard]] string toString() const {
        return string("!MM!y[!!") + this->obj_->toString() + "!g@!!" + this->inst_->toString() + "!y]!!";
      }
    };
  };

  [[maybe_unused]] static void load_processor() {
    BCODE_PROCESSOR = [](const BCode_p &bcode) -> Objs_p {
      const Objs_p objs = Processor::compute(bcode);
      return objs;
      //return nullptr == objs ? Obj::to_noobj() : objs;
    };
  }
} // namespace fhatos

#endif
