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
  class Monad {
  protected:
    const Obj_p _obj;
    const Obj_p _inst;
    const long _bulk = 1;

  public:
    explicit Monad(const Obj_p obj, const Inst_p &inst) : _obj(obj), _inst(inst) {}

    List<ptr<Monad>> split(const ptr<BCode> &bcode) const {
      if (this->_inst->isNoObj() || this->_obj->isNoObj()) {
        return List<ptr<Monad>>{};
      } else {
        const ptr<Obj> nextObj = this->_inst->apply(this->_obj);
        const Inst_p nextInst = bcode->nextInst(this->_inst);
        return List<ptr<Monad>>{ptr<Monad>(new Monad(nextObj, nextInst))};
      }
    }

    ptr<Obj> obj() const { return this->_obj; }
    ptr<Inst> inst() const { return this->_inst; }
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
    List<ptr<Monad>> *running = new List<ptr<Monad>>();
    List<Obj_p> *halted = new List<Obj_p>();
    Pair<Inst_p, List<Obj_p> *> *barrier;

  public:
    explicit Processor(const BCode_p &bcode) :
        bcode(bcode), barrier(new Pair<Inst_p, List<ptr<Obj>> *>(nullptr, nullptr)) {
      const Inst_p startInst = this->bcode->bcode_value().at(0);
      LOG(DEBUG, "startInst: %s in %s\n", startInst->toString().c_str(), this->bcode->toString().c_str());
      if (startInst->inst_op() == "start") {
        for (const ptr<Obj> &startObj: startInst->inst_args()) {
          const ptr<Monad> monad = ptr<Monad>(new Monad(startObj, this->bcode->nextInst(startInst)));
          this->running->push_back(monad);
          LOG(DEBUG, FOS_TAB_2 "!mStarting!! monad: %s\n", monad->toString().c_str());
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
      while ((!this->running->empty() || this->barrier->first) && (counter++ < steps || steps == -1)) {
        if (this->running->empty() && this->barrier->first) {
          const ptr<Objs> objA =
              share(Obj(12)); // TODO: ptr<Objs>(new Objs((List<const ptr<Obj>>) *this->barrier->second));
          LOG(DEBUG, "Processing barrier: %s\n", objA->toString().c_str());
          const ptr<Obj> objB = this->barrier->first->apply(objA);
          LOG(DEBUG, "Barrier reduction: %s\n", objB->toString().c_str());
          this->running->push_back(ptr<Monad>(new Monad(objB, bcode->nextInst(this->barrier->first))));
          this->barrier->second->clear();
          this->barrier->first = nullptr;
          delete this->barrier->second;
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
            if (false) { // parent->inst()->itype() == IType::MANY_TO_ONE) {
              /// MANY-TO-ONE BARRIER PROCESSING
              if (!this->barrier->first) {
                LOG(DEBUG, "!uCreating barrier!!: %s\n", parent->inst()->toString().c_str());
                this->barrier->first = share(Inst(*parent->inst()));
                this->barrier->second = new List<ptr<Obj>>();
              }
              LOG(DEBUG, "Adding to barrier: %s => %s\n", parent->toString().c_str(),
                  parent->inst()->toString().c_str());
              this->barrier->second->push_back(parent->obj());
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
