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
    const Obj_p _inst;
    const long _bulk = 1;

  public:
    explicit Monad(const Obj_p obj, const Inst_p &inst) : _obj(obj), _inst(inst) {}

    void split(const ptr<BCode> &bcode, List<Monad_p> *running) const {
      const ptr<Obj> nextObj = this->_inst->apply(this->_obj);
      const Inst_p nextInst = bcode->nextInst(this->_inst);
      if (nextObj->isObjs()) {
        for (const auto &obj: *nextObj->objs_value()) {
          if (!obj->isNoObj()) {
            const Monad_p monad = Monad_p(new Monad(obj, nextInst));
            running->push_back(monad);
            LOG(DEBUG, FOS_TAB_2 "!mGenerating!! monad: %s\n", monad->toString().c_str());
          }
        }
      } else if (!nextObj->isNoObj()) {
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
    const BCode_p bcode;
    List<Monad_p> *running = new List<Monad_p>();
    Deque<Monad_p> *barriers = new Deque<Monad_p>();
    List<Obj_p> *halted = new List<Obj_p>();

  public:
    explicit Processor(const BCode_p &bcode) : bcode(bcode) {
      for (const Inst_p &inst: bcode->bcode_value()) {
        const Monad_p monad = share(Monad(inst->inst_seed(), inst));
        if (Insts::isBarrier(inst)) {
          this->barriers->push_back(monad);
          LOG(DEBUG, "Adding barrier monad: %s\n", monad->toString().c_str());
        } else if (Insts::isInitial(inst)) {
          this->running->push_back(monad);
          LOG(DEBUG, "Adding initial monad: %s\n", monad->toString().c_str());
        }
      }
    }

    const ptr<E> next(const int steps = -1) {
      while (true) {
        if (this->halted->empty()) {
          if (this->running->empty()) {
            return nullptr; // ptr<E>((E*)new NoObj());
          } else {
            this->execute(steps);
          }
        } else {
          const ptr<E> end = std::dynamic_pointer_cast<E>(this->halted->back());
          this->halted->pop_back();
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
          const ptr<Monad> monad = this->running->back();
          this->running->pop_back();
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
