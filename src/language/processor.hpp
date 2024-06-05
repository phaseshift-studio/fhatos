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
    explicit Monad(const Obj *obj, const Inst *inst) : _obj(obj), _inst(inst) {
    }

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

    bool halted() const {
      return this->_inst->isNoInst();
    }

    bool dead() const {
      return this->_obj->isNoObj();
    }

    const string toString() const {
      return string("!bM!![") + this->obj()->toString() + "!g@!!" +
             (nullptr == this->_inst
                ? "Ø"
                : this->inst()->toString()) + "]";
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

  public:
    explicit Processor(const ptr<Bytecode> bcode) : bcode(bcode) {
      const Inst *startInst = this->bcode->startInst();
      assert(startInst->opcode() == "start");
      for (const Obj *startObj: startInst->args()) {
        const Monad *monad = new Monad(startObj, startInst);
        this->running->push_back(monad);
        LOG(DEBUG, FOS_TAB_2 "!mStarting!! monad: %s\n", monad->toString().c_str());
      }
    }

    const E *next(const int steps = -1) {
      while (true) {
        if (this->halted->empty()) {
          if (this->running->empty()) {
            return nullptr;
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
      while (!this->running->empty() && (counter++ < steps || steps == -1)) {
        const Monad *parent = this->running->back();
        this->running->pop_back();
        if (parent->dead()) {
          LOG(DEBUG, FOS_TAB_4 "!rKilling!! monad: %s\n", parent->toString().c_str());
        } else if (parent->halted()) {
          LOG(DEBUG, FOS_TAB_2 "!gHalting!! monad: %s\n", parent->toString().c_str());
          this->halted->push_back((const E *) parent->obj());
        } else {
          for (const Monad *child: parent->split(this->bcode)) {
            LOG(DEBUG, FOS_TAB_4 "!ySplitting!! monad : %s => %s\n", parent->toString().c_str(),
                child->toString().c_str());
            this->running->push_back(child);
          }
        }
        delete parent;
      }
      LOG(DEBUG, FOS_TAB_2 "Exiting current run with [!yrunning!!:%i] [!ghalted!!:%i]\n", this->running->size(),
          this->halted->size());
      return this->halted->size();
    }

    void forEach(const Consumer<const E *> &consumer, const int steps = -1) {
      while (true) {
        const E *end = this->next();
        if (end) {
          consumer(end);
        } else {
          break;
        }
      }
    }
  };
} // namespace fhatos

#endif
