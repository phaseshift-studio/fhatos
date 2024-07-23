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
#include <language/rewriter.hpp>
//
#include <language/obj.hpp>
#ifdef NATIVE
#include <assert.h>
#else
#include <esp_assert.h>
#endif

namespace fhatos {
  class Monad;
  using Monad_p = ptr<Monad>;
  class Monad {
  protected:
    const Obj_p _obj;
    const Inst_p _inst;
    const long _bulk = 1;

  public:
    explicit Monad(const Obj_p obj, const Inst_p &inst) : _obj(obj), _inst(inst) {}

    void split(const BCode_p &bcode, Deque<Monad_p> *running) const {
      const Obj_p nextObj = this->_inst->apply(this->_obj);
      const Inst_p nextInst = bcode->nextInst(this->_inst);
      if (nextObj->isObjs()) {
        for (const auto &obj: *nextObj->objs_value()) {
          if (!obj->isNoObj()) {
            const Monad_p monad = Monad_p(new Monad(obj, nextInst));
            running->push_back(monad);
            LOG(DEBUG, FOS_TAB_2 "!mGenerating!! monad: %s\n", monad->toString().c_str());
          }
        }
      } else {
        /*^..if (!nextObj->isNoObj() || (!nextInst->isNoObj() && strcmp("Ø",IDomain.toChars(nextInst->itype())) == 0))*/
        const Monad_p monad = Monad_p(new Monad(nextObj, nextInst));
        running->push_back(monad);
        LOG(DEBUG, FOS_TAB_2 "!mGenerating!! monad: %s\n", monad->toString().c_str());
      }
    }

    Obj_p obj() const { return this->_obj; }
    Inst_p inst() const { return this->_inst; }
    long bulk() const { return this->_bulk; }

    bool halted() const { return this->_inst->isNoObj(); }
    bool dead() const { return this->_obj->isNoObj(); }

    string toString() const {
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
    explicit Processor(const BCode_p &bcode) : bcode(bcode) {
      if(!this->bcode->isBytecode())
        throw fError("Processor requires a bcode obj to execute: %s\n", bcode->toString().c_str());
      this->bcode = Rewriter({Rewriter::by()}).apply(this->bcode);
      bool first = true; // TODO HACK:: ZERO_TO_Y OR X_TO_Y instructions need to be determined based on location in bcode
      for (const Inst_p &inst: this->bcode->bcode_value()) {
        if (Insts::isBarrier(inst)) {
          const Monad_p monad = share(Monad(inst->inst_seed(), inst));
          this->barriers->push_back(monad);
          LOG(DEBUG, FOS_TAB_2 "!yBarrier!! monad: %s\n", monad->toString().c_str());
        } else if (Insts::isInitial(inst) && first) {
          const Monad_p monad = share(Monad(inst->inst_seed(), inst));
          this->running->push_back(monad);
          LOG(DEBUG, FOS_TAB_2 "!mStarting!!   monad: %s\n", monad->toString().c_str());
        }
        first = false;
      }
    }

    const ptr<E> next(const int steps = -1) {
      while (true) {
        if (this->halted->empty()) {
          if (this->running->empty())
            return nullptr; // ptr<E>((E*)new NoObj());
          this->execute(steps);
        } else {
          const ptr<E> end = std::dynamic_pointer_cast<E>(this->halted->front());
          this->halted->pop_front();
          return end;
        }
      }
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
        if (end && !end->isNoObj()) {
          consumer(end);
        } else {
          break;
        }
      }
    }
  };


} // namespace fhatos

#endif
