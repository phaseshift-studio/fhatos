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
  class Monad;

  using Monad_p = ptr<Monad>;

  class Monad {
  protected:
    const Obj_p obj_;
    const Inst_p inst_;
    const long bulk_ = 1;
    uint16_t loops_ = 0;

  public:
    Monad() = delete;

    explicit Monad(const Obj_p &obj, const Inst_p &inst) : obj_(obj), inst_(inst) {
      // TODO: figure out how to not require a clone()
    }

    static Monad_p M(const Obj_p &obj, const Inst_p &inst) {
      return make_shared<Monad>(obj, inst);
    }

    void split(const BCode_p &bcode, Deque<Monad_p> *running) const {
      const Obj_p next_obj = this->inst_->apply(this->obj_);
      const Inst_p next_inst = bcode->next_inst(this->inst_);
      if(next_obj->is_noobj()) {
        if(const Inst_p resolved = TYPE_INST_RESOLVER(next_obj, next_inst);
          resolved->is_inst() && is_initial(resolved->itype())) {
          const Monad_p monad = M(resolved->inst_seed(next_inst), next_inst);
          LOG(DEBUG, FOS_TAB_2 "!mre-initializing monad(s): %s\n", monad->toString().c_str());
          running->push_back(monad);
        }
      } else {
        LOG(DEBUG, FOS_TAB_2 "!mprocessing!! monad(s): %s\n", next_obj->toString().c_str());
        const List<Obj_p> objs = next_obj->is_objs() ? *next_obj->objs_value() : List<Obj_p>{next_obj};
        for(const auto &obj: objs) {
          if(!obj->is_noobj()) {
            if(this->inst_->inst_op() == "repeat") {
              if(const Obj_p emit = this->inst_->inst_args()->arg(2);
                !emit->is_noobj() && !emit->apply(obj)->is_noobj()) {
                // repeat.emit
                const auto monad = M(obj, next_inst);
                running->push_back(monad);
                LOG(DEBUG, FOS_TAB_4 "!memitting!! monad: %s\n", monad->toString().c_str());
              }
              if(const Obj_p until = this->inst_->inst_args()->arg(1);
                !until->is_noobj() && until->apply(obj)->is_noobj()) {
                // repeat.until
                const auto monad = M(obj, this->inst_);
                monad->loops_ = this->loops_ + 1;
                running->push_back(monad);
                LOG(DEBUG, FOS_TAB_4 "!mlooping!! monad: %s\n", monad->toString().c_str());
                continue;
              }
            }
            const auto monad = M(obj, next_inst);
            running->push_back(monad);
            LOG(DEBUG, FOS_TAB_4 "!mgenerating!! monad: %s\n", monad->toString().c_str());
          }
        }
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

  class Processor {
  protected:
    BCode_p bcode_;
    Deque<Monad_p> *running_;
    Deque<Monad_p> *barriers_;
    Deque<Obj_p> *halted_;

    ~Processor() {
      delete this->running_;
      delete this->barriers_;
      delete this->halted_;
    }

    explicit Processor(const BCode_p &bcode) : bcode_(bcode),
                                               running_(new Deque<Monad_p>()),
                                               barriers_(new Deque<Monad_p>()),
                                               halted_(new Deque<Obj_p>()) {
      if(bcode->is_inst()) { // wrap inst in bcode
        this->bcode_ = Obj::to_bcode({bcode}, bcode->tid());
      }
      if(!this->bcode_->is_bcode())
        throw fError("Processor requires a !bbcode!! obj to execute: %s", bcode_->toString().c_str());
      // this->bcode_ = Rewriter({
      // /*Rewriter::starts(starts), */Rewriter::by(), Rewriter::explain()}).apply(this->bcode_);
      for(const Inst_p &inst: *this->bcode_->bcode_value()) {
        const Inst_p resolved = TYPE_INST_RESOLVER(Obj::create(Any(), OType::OBJ, OBJ_FURI), inst);
        const Obj_p seed_copy = resolved->inst_seed(resolved);
        if(is_barrier_out(resolved->itype())) {
          const Monad_p m = Monad::M(seed_copy, inst);
          this->barriers_->push_back(m);
          LOG(DEBUG, FOS_TAB_2 "!ybarrier!! monad: %s\n", m->toString().c_str());
        } else if(is_initial(resolved->itype())) {
          const Monad_p m = Monad::M(seed_copy, inst);
          this->running_->push_back(m);
          LOG(DEBUG, FOS_TAB_2 "!mstarting!! monad: %s\n", m->toString().c_str());
        }
      }
      // start inst forced initial TODO: remove this as it's not sound
      if(this->running_->empty()) {
        //const Obj_p seed_copy = Objs::to_objs();
        const Obj_p seed_copy = this->bcode_->bcode_value()->front()->inst_seed(this->bcode_->bcode_value()->front());
        this->running_->push_back(
          Monad::M(seed_copy, this->bcode_->bcode_value()->front()));
      }
    }

    Obj_p next(const int steps = -1) const {
      while(true) {
        // Process::current_process()->feed_watchdog_via_counter();
        if(this->halted_->empty()) {
          if(this->running_->empty())
            return nullptr;
          this->execute(steps);
        } else {
          const Obj_p end = this->halted_->front();
          this->halted_->pop_front();
          if(!end->is_noobj())
            return end;
        }
      }
    }

    Objs_p to_objs() const {
      Objs_p objs = Obj::to_objs();
      Obj_p end;
      while(nullptr != (end = this->next())) {
        objs->add_obj(end);
      }
      return objs;
    }

    [[nodiscard]] int execute(const int steps = -1) const {
      uint16_t counter = 0;
      while((!this->running_->empty() || !this->barriers_->empty()) && (counter++ < steps || steps == -1)) {
        if(this->running_->empty() && !this->barriers_->empty()) {
          const Monad_p barrier = this->barriers_->front();
          this->barriers_->pop_front();
          LOG(DEBUG, "processing barrier: %s\n", barrier->toString().c_str());
          barrier->split(this->bcode_, this->running_);
        } else {
          const Monad_p m = this->running_->front();
          this->running_->pop_front();
          if(m->halted()) {
            LOG(TRACE, FOS_TAB_5 "!ghalting!! monad: %s\n", m->toString().c_str());
            this->halted_->push_back(m->obj());
          } else {
            if(const Inst_p resolved = TYPE_INST_RESOLVER(m->obj(), m->inst());
              resolved->is_inst() &&
              is_barrier_out(resolved->itype())) {
              /// MANY-TO-? BARRIER PROCESSING
              LOG(TRACE, "Adding to barrier: %s => %s\n", m->toString().c_str(), m->inst()->toString().c_str());
              this->barriers_->front()->obj()->objs_value()->push_back(m->obj());
            } else {
              LOG(TRACE, FOS_TAB_5 "!gSplitting!! monad: %s\n", m->toString().c_str());
              m->split(this->bcode_, this->running_);
            }
          }
        }
      }

      LOG(TRACE, FOS_TAB_2 "exiting current run with [!ghalted!!:%i] [!yrunning!!:%i]: %s\n", this->running_->size(),
          this->halted_->size(), this->bcode_->toString().c_str());
      return this->halted_->size();
    }

    void for_each(const Consumer<const Obj_p> &consumer, const int steps = -1) {
      while(true) {
        const Obj_p end = this->next(steps);
        if(!end)
          break;
        if(!end->is_noobj())
          consumer(end);
      }
    }

  public:
    static Objs_p compute(const BCode_p &bcode) {
      ROUTER_PUSH_FRAME("+", Obj::to_rec());
      const Objs_p results = Processor(bcode).to_objs();
      ROUTER_POP_FRAME();
      return results;
    }
  };

  [[maybe_unused]] static void load_processor() {
    BCODE_PROCESSOR = [](const BCode_p &bcode) {
      return Processor::compute(bcode);
    };
  }
} // namespace fhatos

#endif
