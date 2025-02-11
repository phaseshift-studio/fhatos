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
#include "../obj.hpp"
#include "../mmadt/rewriter.hpp"
#include "../mmadt/compiler.hpp"

namespace fhatos {
  ///////////////////////////////////////////////////////////////////////////
  /////////////////////////////// PROCESSOR /////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////

  class Processor final : public Obj { // : public IDed, Typed or perhaps just full Obj`

  public:
    class MonadSet;
    class Monad;
    using Monad_p = ptr<Processor::Monad>;
    uptr<Compiler> compiler_;

  protected:
    BCode_p bcode_;
    //unique_ptr<MonadSet> running_ = make_unique<MonadSet>();
    unique_ptr<Deque<Monad_p>> running_ = make_unique<Deque<Monad_p>>();
    unique_ptr<Deque<Monad_p>> barriers_ = make_unique<Deque<Monad_p>>();
    unique_ptr<Deque<Obj_p>> halted_ = make_unique<Deque<Obj_p>>();
    //unique_ptr<ObjsSet> halted_ = make_unique<ObjsSet>();

    explicit Processor(const BCode_p &bcode) : Obj(Any(), OType::OBJ, REC_FURI,
                                                   id_p(to_string(rand()).insert(0, "/sys/processor/").c_str())),
                                               compiler_(make_unique<Compiler>(true, false)), bcode_(bcode) {
      if(!this->bcode_->is_code()) {
        if(!this->bcode_->is_noobj()) {
          // monad halts immediately on a non-bcode submission
          this->halted_->push_back(bcode);
        }
      } else {
        // if a single inst, wrap in bcode
        if(this->bcode_->is_inst())
          this->bcode_ = Obj::to_bcode({this->bcode_});
        // process bcode inst pipeline
        this->bcode_ = Rewriter({Rewriter::by(), Rewriter::explain()}).apply(this->bcode_);
        // setup global behavior around barriers, initials, and terminals
        Log::LOGGER(DEBUG, this, FOS_TAB_2 "loading %s\n", this->bcode_->toString().c_str());
        bool first = true;
        for(const Inst_p &inst: *this->bcode_->bcode_value()) {
          try {
            const Inst_p resolved = TYPE_INST_RESOLVER(Obj::to_type(OBJ_FURI), inst);
            const Obj_p seed_copy = resolved->inst_seed(resolved);
            if(resolved->is_gather()) {
              // MANY_TO_??
              const Monad_p m = M(seed_copy, inst);
              this->barriers_->push_back(m);
              Log::LOGGER(DEBUG, this, FOS_TAB_2 "!ybarrier!! monad created: %s\n", m->toString().c_str());
            } else if(resolved->is_initial() || (first && resolved->is_map())) {
              // ZERO/MAYBE*-TO_??
              const Monad_p m = M(noobj(), inst); // TODO: use seed
              this->running_->push_back(m);
              Log::LOGGER(DEBUG, this, FOS_TAB_2 "!ginitial!! monad created: %s\n", m->toString().c_str());
            }
          } catch(const fError &e) {
            // throw e;
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
      return this->vid;
    }

    [[nodiscard]] Monad_p M(const Obj_p &obj, const Inst_p &inst, const long bulk = 1l) const {
      return make_shared<Monad>(this, obj, inst, bulk);
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
          Log::LOGGER(TRACE, this, FOS_TAB_2 "!ghalting!! monad at %s\n", end->toString().c_str());
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
      Log::LOGGER(TRACE, this, "%s\n",
                  Ansi<>::singleton()->silly_print("processor shutting down", true, false).c_str());
      return objs;
    }

    void execute(const int steps = -1) const {
      uint16_t counter = 0;
      while((!this->running_->empty() || !this->barriers_->empty()) && (counter++ < steps || steps == -1)) {
        if(!this->running_->empty()) {
          const Monad_p m = this->running_->front();
          this->running_->pop_front();
          m->loop();
        } else if(!this->barriers_->empty()) {
          const Monad_p barrier = this->barriers_->front();
          this->barriers_->pop_front();
          Log::LOGGER(DEBUG, this, "processing barrier: %s\n", barrier->toString().c_str());
          barrier->loop();
        }
      }
      Log::LOGGER(TRACE, this, FOS_TAB_2 "exiting current run with [!ghalted!!:%i] [!yrunning!!:%i]: %s\n",
                  this->running_->size(),
                  this->halted_->size(), this->bcode_->toString().c_str());
    }

  public:
    [[nodiscard]] MonadSet make_monad_set() {
      return MonadSet();
    }

    [[nodiscard]] Monad_p make_monad(const Obj_p &obj, const Inst_p &inst) const {
      return make_shared<Monad>(this, obj, inst);
    }

    static Objs_p compute(const BCode_p &bcode) {
      //ROUTER_PUSH_FRAME("+", Obj::to_inst_args());
      const Obj_p objs = Processor(bcode).to_objs();
      return objs;
    }

    static ptr<Processor> create(const BCode_p &bcode) {
      const auto proc = ptr<Processor>(new Processor(bcode));
      return proc;
    }


    ///////////////////////////////////////////////////////////////////////////
   ///////////////////////////////// MONAD ///////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////

  public:
    class MonadSet;

    class Monad : public enable_shared_from_this<Processor::Monad>, public Pair<Obj_p, Inst_p> { // OBJ as well?
      friend MonadSet;

    protected:
      const Processor *processor_;

    public:
      const Obj_p obj;
      const Inst_p inst;
      long bulk = 1;
      uint16_t loops = 0;

      Monad() = delete;

      explicit Monad(const Processor *processor, const Obj_p &obj, const Inst_p &inst,
                     const long bulk = 1l) : Pair<Obj_p, Inst_p>(obj, inst),
                                             processor_(processor),
                                             obj(obj),
                                             inst(inst),
                                             bulk(bulk) {
      };

      void loop() const {
        if(this->halted()) {
          if(!this->dead()) {
            LOG_OBJ(TRACE, this->processor_, "monad %s halting\n", this->toString().c_str());
            this->halt();
          }
        } else {
          //const Inst_p current_inst_resolved = TYPE_INST_RESOLVER(this->obj, this->inst);
          const Inst_p current_inst_resolved = this->processor_->compiler_->resolve_inst(this->obj, this->inst);
          LOG_OBJ(TRACE, this->processor_, "monad %s applying to resolved inst %s !m=>!! %s [!m%s!!]\n",
                  this->toString().c_str(),
                  this->inst->toString().c_str(),
                  current_inst_resolved->toString().c_str(),
                  "SIGNATURE HERE");
          this->domain_loop(current_inst_resolved);
        }
      }

      ///////////////////////////////////////////////////////////////////////////

      void domain_loop(const Inst_p &current_inst_resolved) const {
        LOG_OBJ(TRACE, this->processor_, FOS_TAB_2 "monad at !gdomain!! of %s !m=>!! %s [!m%s!m]\n",
                this->toString().c_str(),
                current_inst_resolved->toString().c_str(),
                "SIGNATURE HERE");
        if(current_inst_resolved->is_gather()) {
          if(this->obj->is_objs()) {
            LOG_OBJ(TRACE, this->processor_, "barrier monad [size: %i] fetch for processing by %s [!m%s!m]\n",
                    this->obj->objs_value()->size(),
                    current_inst_resolved->toString().c_str(),
                    "SIGNATURE HERE");
            range_loop(current_inst_resolved->apply(this->obj), current_inst_resolved);
          } else {
            this->processor_->barriers_->front()->obj->add_obj(this->obj);
            LOG_OBJ(TRACE, this->processor_, "monad %s stored in barrier [size: %i] [!m%s!m]\n",
                    this->toString().c_str(),
                    this->processor_->barriers_->front()->obj->objs_value()->size(),
                    "SIGNATURE HERE");
          }
        } else {
        //  this->obj->CHECK_OBJ_TO_INST_SIGNATURE(current_inst_resolved, true);
          range_loop(current_inst_resolved->apply(this->obj), current_inst_resolved);
        }
      }

      ///////////////////////////////////////////////////////////////////////////

      void range_loop(const Obj_p &next_obj, const Inst_p &current_inst_resolved) const {
        LOG_OBJ(TRACE, this->processor_, FOS_TAB_2 "monad at !grange!! of %s !m=>!! %s [%s]\n",
                this->processor_->M(next_obj,this->inst)->toString().c_str(),
                current_inst_resolved->toString().c_str(),
                "SIGNATURE HERE");
        next_obj->CHECK_OBJ_TO_INST_SIGNATURE(current_inst_resolved, false);
        const Inst_p next_inst = this->processor_->bcode_->next_inst(this->inst);
        if(next_inst->is_generative()) {
          LOG_OBJ(TRACE, this->processor_, "monad %s dying [%s]\n", this->toString().c_str(),
                  "SIGNATURE HERE");
        } else if(next_obj->is_objs() && !next_inst->is_gather()) {
          //   (is_scatter(current_inst_resolved->itype()) ||
          //    is_maybe_range(current_inst_resolved->itype()))) {
          LOG_OBJ(TRACE, this->processor_, "monad %s scattering [%s]\n",
                  this->toString().c_str(),
                  "SIGNATURE HERE");
          for(const Obj_p &barrier_next_obj: *next_obj->objs_value()) {
            const Monad_p m = this->processor_->M(barrier_next_obj, next_inst);
            LOG_OBJ(TRACE, this->processor_, "monad %s !r==!gmigrating!r==>!! %s\n",
                    this->toString().c_str(),
                    m->toString().c_str());
            this->processor_->running_->push_back(m);
          }
        } else {
          const Monad_p m = this->processor_->M(next_obj, next_inst);
          LOG_OBJ(TRACE, this->processor_, "monad %s !r==!gmigrating!r==>!! %s\n",
                  this->toString().c_str(),
                  m->toString().c_str());
          this->processor_->running_->push_back(m);
        }
      }

      bool operator==(const Monad &other) const {
        return this->equals(other);
      }

      [[nodiscard]] bool equals(const Monad &other) const {
        return this->inst->equals(*other.inst) && this->obj->equals(*other.obj);
      }

      void halt() const {
        this->processor_->halted_->push_back(this->obj);
        /*for(int i = 0; i < this->bulk_; i++) {
          this->processor_->halted_->push_back(std::move(this->obj_->clone()));
        }*/
      }

      bool operator<(const Monad &rhs) const {
        return this->obj->toString() < rhs.obj->toString() ||
               this->inst->toString() < rhs.inst->toString();
      }

      [[nodiscard]] bool halted() const { return this->inst->is_noobj(); }

      [[nodiscard]] bool dead() const { return this->obj->is_noobj(); }

      [[nodiscard]] string toString() const {
        return string("!MM!y[!!") + this->obj->toString() + "!g@!!" + this->inst->toString() + "!y]!!";
      }
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    struct TransparentCompare {
      using is_transparent = void;

      template<typename T, typename U>
      bool operator()(T &&lhs, U &&rhs) const {
        return *std::forward<T>(lhs) < *std::forward<U>(rhs);
      }
    };

    /* struct monadp_equal_to : std::binary_function<Monad_p &, Monad_p &, bool> {
       bool operator()(const Monad_p &a, const Monad_p &b) const { return a->equals(*b); }
     };*/

    class MonadSet {
    public:
      const unique_ptr<Set<Monad_p, TransparentCompare>> internal = make_unique<Set<Monad_p, TransparentCompare>>();

      [[nodiscard]] bool empty() const {
        return this->internal->empty();
      }

      [[nodiscard]] long bulk_of(const Pair_p<Obj_p, Inst_p> &monad) const {
        if(const auto it = this->internal->find(make_shared<Monad>(nullptr, monad->first, monad->second));
          it != this->internal->end()) {
          return (*it)->bulk;
        }
        return 0l;
      }

      [[nodiscard]] unsigned long size() const {
        return this->internal->size();
      }

      [[nodiscard]] unsigned long bulk_size() const {
        unsigned long bulk_total = 0;
        for(const auto &o: *this->internal) {
          bulk_total += o->bulk;
        }
        return bulk_total;
      }

      [[nodiscard]] Monad_p next() const {
        return std::move(this->internal->extract(this->internal->begin()).value());
      }

      [[nodiscard]] Monad_p front() const {
        return *this->internal->begin();
      }

      void pop_front() const {
        this->internal->erase(this->internal->begin());
        //auto node =
        // return move(node.value());
      }

      void push_back(const Monad_p &monad) const {
        if(const auto it = this->internal->find(monad); it != this->internal->end()) {
          // LOG(INFO, "FOUND: %s ==  %s\n", (*it)->toString().c_str(), monad->toString().c_str());
          (*it)->bulk = (*it)->bulk + monad->bulk;
        } else
          this->internal->insert(monad);
      }
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
