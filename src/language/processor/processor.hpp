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

#include <fhatos.hpp>
#include <language/obj.hpp>
#include <language/rewrite/rewriter.hpp>
#include <process/actor/actor.hpp>

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

    explicit Monad(const Obj_p &obj, const Inst_p &inst) : obj_(obj->clone()), inst_(inst) {
      // TODO: figure out how to not require a clone()
    }

    void split(const BCode_p &bcode, Deque<Monad_p> *running) const {
      const Obj_p next_obj = this->inst_->apply(this->obj_);
      const Inst_p next_inst = bcode->next_inst(this->inst_);
      if (!next_obj->is_noobj()) {
        LOG(DEBUG, FOS_TAB_2 "!mProcessing!! monad(s): %s\n", next_obj->toString().c_str());
        const List<Obj_p> objs = next_obj->is_objs() ? *next_obj->objs_value() : List<Obj_p>{next_obj};
        for (const auto &obj: objs) {
          if (!obj->is_noobj()) {
            if (this->inst_->inst_op() == "repeat") {
              const Obj_p until = this->inst_->inst_arg(1);
              const Obj_p emit = this->inst_->inst_arg(2);
              if (!emit->is_noobj() && !emit->apply(obj)->is_noobj()) {
                // repeat.emit
                const auto monad = make_shared<Monad>(obj, next_inst);
                running->push_back(monad);
                LOG(DEBUG, FOS_TAB_4 "!mEmitting!! monad: %s\n", monad->toString().c_str());
              }
              if (!until->is_noobj() && until->apply(obj)->is_noobj()) {
                // repeat.until
                const auto monad = make_shared<Monad>(obj, this->inst_);
                monad->loops_ = this->loops_ + 1;
                running->push_back(monad);
                LOG(DEBUG, FOS_TAB_4 "!mLooping!! monad: %s\n", monad->toString().c_str());
                continue;
              }
            }
            const auto monad = make_shared<Monad>(obj, next_inst);
            running->push_back(monad);
            LOG(DEBUG, FOS_TAB_4 "!mGenerating!! monad: %s\n", monad->toString().c_str());
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

  static Monad_p monad_p(const Obj_p &obj, const Inst_p &inst) {
    return share(Monad(obj, inst));
  }

  class Processor {
  protected:
    BCode_p bcode_;
    Deque<Monad_p> *running_;
    Deque<Monad_p> *barriers_;
    Deque<Obj_p> *halted_;

  public:
    ~Processor() {
      delete this->running_;
      delete this->barriers_;
      delete this->halted_;
    }

    explicit Processor(const BCode_p &bcode, const Obj_p &starts = noobj()) : bcode_(bcode),
                                                                              running_(new Deque<Monad_p>()),
                                                                              barriers_(new Deque<Monad_p>()),
                                                                              halted_(new Deque<Obj_p>()) {
      if (!this->bcode_->is_bcode())
        throw fError("Processor requires a !bbcode!! obj to execute: %s", bcode_->toString().c_str());
      this->bcode_ = Rewriter({Rewriter::starts(starts), Rewriter::by(), Rewriter::explain()}).apply(this->bcode_);
      for (const Inst_p &inst: *this->bcode_->bcode_value()) {
        const Obj_p seed_copy = inst->inst_seed(inst);
        if (is_barrier_out(inst->itype())) {
          const Monad_p m = monad_p(seed_copy, inst);
          this->barriers_->push_back(m);
          LOG(DEBUG, FOS_TAB_2 "!yBarrier!! monad: %s\n", m->toString().c_str());
        } else if (is_initial(inst->itype())) {
          const Monad_p m = monad_p(seed_copy, inst);
          this->running_->push_back(m);
          LOG(DEBUG, FOS_TAB_2 "!mStarting!!   monad: %s\n", m->toString().c_str());
        }
      }
      // start inst forced initial
      if (this->running_->empty()) {
        const Obj_p seed_copy = this->bcode_->bcode_value()->front()->inst_seed(this->bcode_->bcode_value()->front());
        this->running_->push_back(
          monad_p(seed_copy, this->bcode_->bcode_value()->front()));
      }
    }

    Obj_p next(const int steps = -1) {
      while (true) {
        if (this->halted_->empty()) {
          if (this->running_->empty())
            return nullptr;
          this->execute(steps);
        } else {
          const Obj_p end = this->halted_->front();
          this->halted_->pop_front();
          if (!end->is_noobj())
            return end;
        }
      }
    }

    Objs_p to_objs() {
      Objs_p objs = Obj::to_objs();
      Obj_p end;
      while (nullptr != (end = this->next())) {
        objs->add_obj(end);
      }
      return objs;
    }

    int execute(const int steps = -1) const {
      int counter = 0;
      while ((!this->running_->empty() || !this->barriers_->empty()) && (counter++ < steps || steps == -1)) {
        if (this->running_->empty() && !this->barriers_->empty()) {
          const Monad_p barrier = this->barriers_->front();
          this->barriers_->pop_front();
          LOG(DEBUG, "Processing barrier: %s\n", barrier->toString().c_str());
          barrier->split(this->bcode_, this->running_);
        } else {
          const Monad_p m = this->running_->front();
          this->running_->pop_front();
          if (m->halted()) {
            LOG(TRACE, FOS_TAB_5 "!gHalting!! monad: %s\n", m->toString().c_str());
            this->halted_->push_back(m->obj()->clone());
          } else {
            if (is_barrier_out(m->inst()->itype())) {
              /// MANY-TO-? BARRIER PROCESSING
              LOG(TRACE, "Adding to barrier: %s => %s\n", m->toString().c_str(), m->inst()->toString().c_str());
              this->barriers_->front()->obj()->objs_value()->push_back(m->obj()->clone());
            } else {
              LOG(TRACE, FOS_TAB_5 "!gSplitting!! monad: %s\n", m->toString().c_str());
              m->split(this->bcode_, this->running_);
            }
          }
        }
      }

      LOG(DEBUG, FOS_TAB_2 "Exiting current run with [!ghalted!!:%i] [!yrunning!!:%i]\n", this->running_->size(),
          this->halted_->size());
      return this->halted_->size();
    }

    void for_each(const Consumer<const Obj_p> &consumer, const int steps = -1) {
      while (true) {
        const Obj_p end = this->next(steps);
        if (!end) {
          break;
        } else if (!end->is_noobj()) {
          consumer(end);
        }
      }
    }
  };

  [[maybe_unused]] static Objs_p process(const BCode_p &bcode, const Obj_p &starts = noobj()) {
    return Processor(bcode, starts).to_objs();
  }

  [[maybe_unused]] static Objs_p process(const char *monoid_format, ...) {
    bool has_format = false;
    for (size_t i = 0; i < strlen(monoid_format); i++) {
      if (monoid_format[i] == '%') {
        has_format = true;
        break;
      }
    }
    if (has_format) {
      const size_t max_length = strlen(monoid_format) * 5;
      char monoid_format_formatted[max_length];
      va_list arg;
      va_start(arg, monoid_format);
      const size_t length = vsnprintf(monoid_format_formatted, max_length, monoid_format, arg);
      va_end(arg);
      if (length >= max_length) {
        throw fError("Monoid string longer than expected. Increase max_length");
      }
      return Processor(Options::singleton()->parser<Obj>(monoid_format_formatted), noobj()).to_objs();
    } else {
      return Processor(Options::singleton()->parser<Obj>(monoid_format), noobj()).to_objs();
    }
  }


  [[maybe_unused]] static void load_processor() {
    BCODE_PROCESSOR = [](const Objs_p &starts, const BCode_p &bcode) {
      return Processor(bcode, starts).to_objs();
    };
    Options::singleton()->processor<Obj, BCode, Objs>(
      [](const Obj_p &st, const BCode_p &bc) { return Processor(bc, st).to_objs(); });
  }
} // namespace fhatos

#endif
