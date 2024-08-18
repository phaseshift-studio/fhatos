//  FhatOS: A Distributed Operating System
//  Copyright (c) 2024 PhaseShift Studio, LLC
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Affero General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Affero General Public License for more details.
//
//  You should have received a copy of the GNU Affero General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#pragma once
#ifndef fhatos_processor_hpp
#define fhatos_processor_hpp

#include <fhatos.hpp>
#include <language/obj.hpp>
#include <language/rewrite/rewriter.hpp>
#include <utility>
#include FOS_PROCESS(coroutine.hpp)
#include <process/actor/actor.hpp>
#include <structure/stype/key_value.hpp>

namespace fhatos {
  class Monad;
  using Monad_p = ptr<Monad>;
  class Monad {
  protected:
    const Obj_p _obj;
    const Inst_p _inst;
    const long _bulk = 1;

  public:
    explicit Monad(const Obj_p &obj, const Inst_p &inst) : _obj(obj), _inst(inst) {}

    void split(const BCode_p &bcode, Deque<Monad_p> *running) const {
      const Obj_p nextObj = this->_inst->apply(this->_obj);
      const Inst_p nextInst = bcode->nextInst(this->_inst);
      if (nextObj->isObjs()) {
        LOG(DEBUG, FOS_TAB_2 "!mUnrolling!! objs monad: %s\n", nextObj->toString().c_str());
        const List_p<Obj_p> objs = nextObj->objs_value();
        for (const auto &obj: *objs) {
          if (!obj->isNoObj()) {
            const Monad_p monad = share<Monad>(Monad(obj, nextInst));
            running->push_back(monad);
            LOG(DEBUG, FOS_TAB_4 "!mGenerating!! monad: %s\n", monad->toString().c_str());
          }
        }
      } else if (!nextObj->isNoObj()) {
        const Monad_p monad = share<Monad>(Monad(nextObj, nextInst));
        running->push_back(monad);
        LOG(DEBUG, FOS_TAB_2 "!mGenerating!! monad: %s\n", monad->toString().c_str());
      }
    }

    [[nodiscard]] Obj_p obj() const { return this->_obj; }
    [[nodiscard]] Inst_p inst() const { return this->_inst; }
    [[nodiscard]] long bulk() const { return this->_bulk; }

    [[nodiscard]] bool halted() const { return this->_inst->isNoObj(); }
    [[nodiscard]] bool dead() const { return this->_obj->isNoObj(); }

    [[nodiscard]] string toString() const {
      return string("!MM!y[!!") + this->obj()->toString() + "!g@!!" + this->inst()->toString() + "!y]!!";
    }
  };

  template<typename E>
  class Processor {
  protected:
    BCode_p bcode;
    Deque<Monad_p> *running = new Deque<Monad_p>();
    Deque<Monad_p> *barriers = new Deque<Monad_p>();
    Deque<Obj_p> *halted = new Deque<Obj_p>();

  public:
    ~Processor() {
      FOS_SAFE_DELETE(this->running);
      FOS_SAFE_DELETE(this->barriers);
      FOS_SAFE_DELETE(this->halted);
    }

    explicit Processor(const BCode_p &bcode, const Obj_p &starts = noobj()) : bcode(bcode) {
      if (!this->bcode->isBytecode())
        throw fError("Processor requires a !bbcode!! obj to execute: %s\n", bcode->toString().c_str());
      this->bcode = Rewriter({Rewriter::starts(starts), Rewriter::by(), Rewriter::explain()}).apply(this->bcode);
      for (const Inst_p &inst: *this->bcode->bcode_value()) {
        if (Insts::isBarrier(inst)) {
          const Monad_p monad = share(Monad(inst->inst_seed(), inst));
          this->barriers->push_back(monad);
          LOG(DEBUG, FOS_TAB_2 "!yBarrier!! monad: %s\n", monad->toString().c_str());
        } else if (Insts::isInitial(inst)) {
          const Monad_p monad = share(Monad(inst->inst_seed(), inst));
          this->running->push_back(monad);
          LOG(DEBUG, FOS_TAB_2 "!mStarting!!   monad: %s\n", monad->toString().c_str());
        }
      }
      // start inst forced initial
      if (this->running->empty())
        this->running->push_back(
            share(Monad(this->bcode->bcode_value()->front()->inst_seed(), this->bcode->bcode_value()->front())));
    }

    ptr<E> next(const int steps = -1) {
      while (true) {
        if (this->halted->empty()) {
          if (this->running->empty())
            return nullptr;
          this->execute(steps);
        } else {
          const ptr<E> end = std::dynamic_pointer_cast<E>(this->halted->front());
          this->halted->pop_front();
          return end;
        }
      }
    }

    Objs_p toObjs() {
      Objs_p objs = Obj::to_objs(List<Obj_p>{});
      Obj_p end;
      while (nullptr != (end = this->next())) {
        objs->add_obj(end);
      }
      return objs;
    }

    int execute(const int steps = -1) {
      int counter = 0;
      while ((!this->running->empty() || !this->barriers->empty()) && (counter++ < steps || steps == -1)) {
        if (this->running->empty() && !this->barriers->empty()) {
          const Monad_p barrier = this->barriers->front();
          this->barriers->pop_front();
          LOG(DEBUG, "Processing barrier: %s\n", barrier->toString().c_str());
          barrier->split(this->bcode, this->running);
        } else {
          const Monad_p monad = this->running->front();
          this->running->pop_front();
          if (monad->halted()) {
            LOG(TRACE, FOS_TAB_5 "!gHalting!! monad: %s\n", monad->toString().c_str());
            this->halted->push_back(monad->obj());
          } else {
            if (Insts::isBarrier(monad->inst())) {
              /// MANY-TO-? BARRIER PROCESSING
              LOG(TRACE, "Adding to barrier: %s => %s\n", monad->toString().c_str(), monad->inst()->toString().c_str());
              this->barriers->front()->obj()->objs_value()->push_back(monad->obj());
            } else {
              LOG(TRACE, FOS_TAB_5 "!gSplitting!! monad: %s\n", monad->toString().c_str());
              monad->split(this->bcode, this->running);
            }
          }
        }
      }

      LOG(DEBUG, FOS_TAB_2 "Exiting current run with [!ghalted!!:%i] [!yrunning!!:%i]\n", this->running->size(),
          this->halted->size());
      return this->halted->size();
    }

    void forEach(const Consumer<const ptr<E>> &consumer, const int steps = -1) {
      while (true) {
        const ptr<E> end = this->next(steps);
        if (!end) {
          break;
        } else if (!end->isNoObj()) {
          consumer(end);
        }
      }
    }
  };

  static Objs_p process(const BCode_p &bcode, const Obj_p &starts = noobj()) {
    return Processor<Obj>(bcode, starts).toObjs();
  }

  static void load_processor() {
    Options::singleton()->processor<Obj, BCode, Objs>(
        [](const Obj_p &st, const BCode_p &bc) { return Processor<Obj>(bc, st).toObjs(); });
  }

} // namespace fhatos

#endif
