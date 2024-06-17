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
    const Obj *_obj;
    const Inst *_inst;
    const long _bulk = 1;

  public:
    explicit Monad(const Obj *obj, const Inst *inst) : _obj(obj), _inst(inst) {}

    List<const Monad *> split(const ptr<Bytecode> bcode) const {
      if (this->_inst->isNoInst() || this->_obj->isNoObj()) {
        return List<const Monad *>{};
      } else {
        const Obj *nextObj = this->_inst->apply(this->_obj);
        const Inst *nextInst = bcode->nextInst(this->_inst);
        return List<const Monad *>{new Monad(nextObj, nextInst)};
      }
    }

    const Obj *obj() const { return this->_obj; }
    const Inst *inst() const { return this->_inst; }
    const long bulk() const { return this->_bulk; }

    bool halted() const { return this->_inst->isNoInst(); }

    bool dead() const { return this->_obj->isNoObj(); }

    const string toString() const {
      return string("!bM!![") + this->obj()->toString() + "!g@!!" +
             (nullptr == this->_inst ? "Ø" : this->inst()->toString()) + "]";
    }

    // const Inst<Obj, A> *at() const { return this->inst; }
    // const bool equals(const Monad<ObjX> &other) const {
    //   return this->value.equals(other.get());
    // }
  };

  template<typename E>
  class Processor {
  protected:
    const ptr<Bytecode> bcode;
    List<const Monad *> *running = new List<const Monad *>();
    List<const E *> *halted = new List<const E *>();
    Pair<ManyToOneInst *, List<const Obj *> *> *barrier;

  public:
    explicit Processor(const ptr<Bytecode> bcode) :
        bcode(bcode), barrier(new Pair<ManyToOneInst *, List<const Obj *> *>(nullptr, nullptr)) {
      const Inst *startInst = this->bcode->startInst();
      LOG(DEBUG, "startInst: %s in %s\n", startInst->toString().c_str(), this->bcode->toString().c_str());
      if (startInst->opcode() == "start") {
        for (const Obj *startObj: startInst->args()) {
          const Monad *monad = new Monad(startObj, startInst);
          this->running->push_back(monad);
          LOG(DEBUG, FOS_TAB_2 "!mStarting!! monad: %s\n", monad->toString().c_str());
        }
      }
    }

    const E *next(const int steps = -1) {
      while (true) {
        if (this->halted->empty()) {
          if (this->running->empty()) {
            return (E *) (void *) NoObj::singleton();
          } else {
            this->execute(steps);
          }
        } else {
          const E *end = this->halted->back();
          this->halted->pop_back();
          return end;
        }
      }
    }

    int execute(const int steps = -1) {
      int counter = 0;

      while ((!this->running->empty() || this->barrier->first) && (counter++ < steps || steps == -1)) {
        if (this->running->empty() && this->barrier->first) {
          const Obj *objA = new Objs(this->barrier->second);
          LOG(DEBUG, "Processing barrier: %s\n", objA->toString().c_str());
          const Obj *objB = this->barrier->first->apply(objA);
          LOG(DEBUG, "Barrier reduction: %s\n", objB->toString().c_str());
          this->running->push_back(new Monad(objB, bcode->nextInst(this->barrier->first)));
          this->barrier->second->clear();
          this->barrier->first = nullptr;
          delete this->barrier->second;
        } else {
          const Monad *parent = this->running->back();
          this->running->pop_back();
          if (parent->dead()) {
            LOG(DEBUG, FOS_TAB_4 "!rKilling!! monad: %s\n", parent->toString().c_str());
          } else if (parent->halted()) {
            LOG(DEBUG, FOS_TAB_2 "!gHalting!! monad: %s\n", parent->toString().c_str());
            this->halted->push_back((const E *) parent->obj());
          } else {
            if (parent->inst()->itype() == IType::MANY_TO_ONE) {
              /// MANY-TO-ONE BARRIER PROCESSING
              if (!this->barrier->first) {
                LOG(DEBUG, "Creating barrier: %s\n", parent->inst()->toString().c_str());
                this->barrier->first = (ManyToOneInst *) parent->inst();
                this->barrier->second = new List<const Obj *>();
              }
              LOG(DEBUG, "Adding to barrier: %s => %s\n", parent->toString().c_str(),
                  parent->inst()->toString().c_str());
              this->barrier->second->push_back(parent->obj());
            } else {
              for (const Monad *child: parent->split(this->bcode)) {
                LOG(DEBUG, FOS_TAB_4 "!ySplitting!! monad : %s => %s\n", parent->toString().c_str(),
                    child->toString().c_str());
                this->running->push_back(child);
              }
            }
          }
          delete parent;
        }
      }

      LOG(DEBUG, FOS_TAB_2 "Exiting current run with [!yrunning!!:%i] [!ghalted!!:%i]\n", this->running->size(),
          this->halted->size());
      return this->halted->size();
    }

    void forEach(const Consumer<const E *> &consumer, const int steps = -1) {
      while (true) {
        const E *end = this->next();
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
