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

    List<Monad_p> split(const ptr<BCode> &bcode) const {
      if (this->_inst->isNoObj() || this->_obj->isNoObj()) {
        return List<ptr<Monad>>{};
      } else {
        const ptr<Obj> nextObj = this->_inst->apply(this->_obj);
        const Inst_p nextInst = bcode->nextInst(this->_inst);
        return List<ptr<Monad>>{ptr<Monad>(new Monad(nextObj, nextInst))};
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

    // const Inst<Obj, A> *at() const { return this->inst; }
    // const bool equals(const Monad<ObjX> &other) const {
    //   return this->value.equals(other.get());
    // }
  };

  template<typename E>
  class Processor {
  protected:
    const BCode_p bcode;
    List<Monad_p> *running = new List<ptr<Monad>>();
    List<Obj_p> *halted = new List<Obj_p>();
    Deque<Pair<Inst_p, Objs_p>> *barriers = new Deque<Pair<Inst_p, Objs_p>>();

  public:
    explicit Processor(const BCode_p &bcode) : bcode(bcode) {
      if (!this->bcode->bcode_value().empty()) {
        for (Inst_p &inst: bcode->bcode_value()) {
          if (inst->inst_itype() == IType::MANY_TO_MANY || inst->inst_itype() == IType::MANY_TO_ONE) {
            this->barriers->push_back({inst, inst->inst_seed()});
            LOG(DEBUG_MORE, "Adding barrier from %s\n", inst->toString().c_str());
          }
        }
        const Inst_p startInst = this->bcode->bcode_value().at(0);
        LOG(DEBUG, "startInst: %s in %s\n", startInst->toString().c_str(), this->bcode->toString().c_str());
        Inst_p nextInst = this->bcode->nextInst(startInst);
        if (startInst->inst_op() == "start") {
          for (const Obj_p &startObj: startInst->inst_args()) {
            const Monad_p monad = Monad_p(new Monad(startObj, nextInst));
            this->running->push_back(monad);
            LOG(DEBUG, FOS_TAB_2 "!mStarting!! monad: %s\n", monad->toString().c_str());
          }
        } else {
          if (startInst->inst_itype() == IType::ZERO_TO_ONE || startInst->inst_itype() == IType::ZERO_TO_MANY) {
            const Monad_p monad = Monad_p(new Monad(startInst->apply(startInst->inst_seed()), nextInst));
            this->running->push_back(monad);
            LOG(DEBUG, FOS_TAB_2 "!mGenerating!! monad: %s\n", monad->toString().c_str());
          }
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
      int killed = 0;
      int counter = 0;
      while ((!this->running->empty() || !this->barriers->empty()) && (counter++ < steps || steps == -1)) {
        if (this->running->empty() && !this->barriers->empty()) {
          Inst_p barrier = this->barriers->front().first;
          Objs_p objs = this->barriers->front().second;
          this->barriers->pop_front();
          LOG(DEBUG, "Processing barrier: %s (%s)\n", barrier->toString().c_str(), objs->toString().c_str());
          const Obj_p objB = barrier->apply(objs);
          LOG(DEBUG, "Barrier reduction: %s\n", objB->toString().c_str());
          this->running->push_back(ptr<Monad>(new Monad(objB, bcode->nextInst(barrier))));
        } else {
          const ptr<Monad> parent = this->running->back();
          this->running->pop_back();
          if (parent->dead()) {
            killed++;
            LOG(DEBUG, FOS_TAB_5 "!rKilling!! monad: %s\n", parent->toString().c_str());
          } else if (parent->halted()) {
            LOG(DEBUG, FOS_TAB_5 "!gHalting!! monad: %s\n", parent->toString().c_str());
            this->halted->push_back(parent->obj());
          } else {
            if (parent->inst()->inst_itype() == IType::MANY_TO_MANY ||
                parent->inst()->inst_itype() == IType::MANY_TO_ONE) {
              /// MANY-TO-? BARRIER PROCESSING
              LOG(DEBUG, "Adding to barrier: %s => %s\n", parent->toString().c_str(),
                  parent->inst()->toString().c_str());
              this->barriers->front().second->objs_value()->push_back(parent->obj());
            } else {
              for (const ptr<Monad> &child: parent->split(this->bcode)) {
                LOG(DEBUG, FOS_TAB_3 "!ySplitting!! monad : %s => %s\n", parent->toString().c_str(),
                    child->toString().c_str());
                this->running->push_back(child);
              }
            }
          }
        }
      }

      LOG(DEBUG, FOS_TAB_2 "Exiting current run with [!ghalted!!:%i] [!yrunning!!:%i] [!rkilled!!:%i]\n",
          this->running->size(), this->halted->size(), killed);
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
