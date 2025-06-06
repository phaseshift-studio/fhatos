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

#define CUSTOM_STACK_NAME "custom_stack"

#include "../../fhatos.hpp"
#include "../../model/fos/sys/scheduler/thread/thread.hpp"
#include "../mmadt/compiler.hpp"
#include "../mmadt/rewriter.hpp"
#include "../obj.hpp"

#define PROCESSOR_TID "/mmadt/util/proc"

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
    // unique_ptr<MonadSet> running_ = make_unique<MonadSet>();
    unique_ptr<Deque<Monad_p>> running_ = make_unique<Deque<Monad_p>>();
    unique_ptr<Deque<Monad_p>> barriers_ = make_unique<Deque<Monad_p>>();
    unique_ptr<Deque<Obj_p>> halted_ = make_unique<Deque<Obj_p>>();
    // unique_ptr<ObjsSet> halted_ = make_unique<ObjsSet>();

  public:
    explicit Processor(const BCode_p &bcode) :
        Obj(Any(), OType::OBJ, REC_FURI, id_p(Processor::get_core_id())), compiler_(make_unique<Compiler>()),
        bcode_(bcode) {
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
        LOG_WRITE(DEBUG, this, L(FOS_TAB_2 "loading {}\n", this->bcode_->toString()));
        bool first = true;
        for(const Inst_p &inst: *this->bcode_->bcode_value()) {
          try {
            const Inst_p resolved = TYPE_INST_RESOLVER(Obj::to_type(OBJ_FURI), inst);
            const Obj_p seed_copy = resolved->inst_seed(resolved);
            if(resolved->is_gather()) {
              // MANY_TO_??
              const Monad_p m = M(seed_copy, inst);
              this->barriers_->push_back(m);
              LOG_WRITE(DEBUG, this, L(FOS_TAB_2 "!ybarrier!! monad created: {}\n", m->toString()));
            } else if(resolved->is_initial() || (first && resolved->is_map())) {
              // ZERO/MAYBE*-TO_??
              const Monad_p m = M(noobj(), inst); // TODO: use seed
              this->running_->push_back(m);
              LOG_WRITE(DEBUG, this, L(FOS_TAB_2 "!ginitial!! monad created: {}\n", m->toString()));
            }
          } catch(const fError &e) {
            // throw e;
          }
          first = false;
        }
        // start inst forced initial TODO: remove this as it's not sound
        if(this->running_->empty()) {
          // const Obj_p seed_copy = Objs::to_objs();
          const Obj_p seed_copy = this->bcode_->bcode_value()->front()->inst_seed(this->bcode_->bcode_value()->front());
          this->running_->push_back(M(seed_copy, this->bcode_->bcode_value()->front()));
        }
      }
    }

    static fURI get_core_id(const string &postfix = "") {
      const fURI i = ID("/sys/vm/core").append(to_string(Thread::core_current_thread()));
      return postfix.empty() ? i : i.extend(postfix);
    }

    [[nodiscard]] ID_p vid_or_tid() const { return this->vid; }

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
          LOG_WRITE(TRACE, vri(this->vid_or_tid()).get(), L(FOS_TAB_2 "!ghalting!! monad at {}\n", end->toString()));
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
      LOG_WRITE(TRACE, vri(this->vid_or_tid()).get(),
                L("{}\n", Ansi<>::singleton()->silly_print("processor shutting down", true, false)));
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
          LOG_WRITE(DEBUG, this, L("processing barrier: {}\n", barrier->toString()));
          barrier->loop();
        }
      }
      LOG_WRITE(TRACE, this,
                L(FOS_TAB_2 "exiting current run with [!ghalted!!:{}] [!yrunning!!:{}]: {}\n", this->running_->size(),
                  this->halted_->size(), this->bcode_->toString()));
    }

  public:
    [[nodiscard]] MonadSet make_monad_set() { return MonadSet(); }

    [[nodiscard]] Monad_p make_monad(const Obj_p &obj, const Inst_p &inst) const {
      return make_shared<Monad>(this, obj, inst);
    }

    static Objs_p compute(const BCode_p &bcode) {
      ////////////////////////////////////////////////////////////////////
      if(const int custom_stack_size = Memory::get_stack_size(bcode, "config/stack_size", 0); custom_stack_size <= 0) {
        const Obj_p objs = Processor(bcode).to_objs();
        return objs;
      } else {
        return Memory::singleton()->use_custom_stack(
            InstBuilder::build("process_custom_stack")
                ->inst_f([](const Obj_p &bcode, const InstArgs &) { return Processor(bcode).to_objs(); })
                ->create(),
            bcode, custom_stack_size);
      }
    }

    static Objs_p compute(const string &bcode) { return Processor::compute(OBJ_PARSER(bcode)); }

    static void register_module() {
      BCODE_PROCESSOR = [](const BCode_p &bcode) -> Objs_p {
        const Objs_p objs = Processor::compute(bcode);
        return objs;
        // return nullptr == objs ? Obj::to_noobj() : objs;
      };
      REGISTERED_MODULES->insert_or_assign(
          PROCESSOR_TID, InstBuilder::build(Typer::singleton()->vid->add_component(PROCESSOR_TID))
                             ->domain_range(OBJ_FURI, {0, 1}, REC_FURI, {1, 1})
                             ->inst_f([](const Obj_p &, const InstArgs &) {
                               return Obj::to_rec({{Obj::to_uri(PROCESSOR_TID), Obj::to_rec()},
                                                   {Obj::to_uri(ID(PROCESSOR_TID).add_component("eval")),
                                                    InstBuilder::build(ID(PROCESSOR_TID).add_component("eval"))
                                                        ->domain_range(OBJ_FURI, {0, 1}, OBJ_FURI, {0, 1})
                                                        ->inst_args(rec({{"code?str", Obj::to_bcode()}}))
                                                        ->inst_f([](const Obj_p &, const InstArgs &args) {
                                                          Objs_p result = Processor::compute(
                                                              OBJ_PARSER(args->arg("code")->str_value()));
                                                          return result;
                                                        })
                                                        ->create()}});
                             })
                             ->create());
    }


    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////// MONAD ///////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////

  public:
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

      explicit Monad(const Processor *processor, const Obj_p &obj, const Inst_p &inst, const long bulk = 1l) :
          Pair<Obj_p, Inst_p>(obj, inst), processor_(processor), obj(obj), inst(inst), bulk(bulk) {};

      void loop() const {
        if(this->halted()) {
          if(!this->dead()) {
            LOG_WRITE(TRACE, this->processor_, L("monad {} halting\n", this->toString()));
            this->halt();
          }
        } else {
          // const Inst_p current_inst_resolved = TYPE_INST_RESOLVER(this->obj, this->inst);
          const Inst_p current_inst_resolved = this->processor_->compiler_->resolve_inst(this->obj, this->inst);
          LOG_WRITE(TRACE, this->processor_,
                    L("monad {} applying to resolved inst {} !m=>!! {} [!m{}!!]\n", this->toString(),
                      this->inst->toString(), current_inst_resolved->toString(), "SIGNATURE HERE"));
          this->domain_loop(current_inst_resolved);
        }
      }

      ///////////////////////////////////////////////////////////////////////////

      void domain_loop(const Inst_p &current_inst_resolved) const {
        LOG_WRITE(TRACE, this->processor_,
                  L(FOS_TAB_2 "monad at !gdomain!! of {} !m=>!! {} [!m{}!!]\n", this->toString(),
                    current_inst_resolved->toString(), "SIGNATURE HERE"));
        if(current_inst_resolved->is_gather()) {
          if(this->obj->is_objs()) {
            LOG_WRITE(TRACE, this->processor_,
                      L("barrier monad [size: {}] fetch for processing by {} [!m{}!m]\n",
                        this->obj->objs_value()->size(), current_inst_resolved->toString(), "SIGNATURE HERE"));
            range_loop(current_inst_resolved->apply(this->obj), current_inst_resolved);
          } else {
            this->processor_->barriers_->front()->obj->add_obj(this->obj);
            LOG_WRITE(TRACE, this->processor_,
                      L("monad {} stored in barrier [size: {}] [!m{}!m]\n", this->toString(),
                        this->processor_->barriers_->front()->obj->objs_value()->size(), "SIGNATURE HERE"));
          }
        } else {
          //  this->obj->CHECK_OBJ_TO_INST_SIGNATURE(current_inst_resolved, true);
          range_loop(current_inst_resolved->apply(this->obj), current_inst_resolved);
        }
      }

      ///////////////////////////////////////////////////////////////////////////

      void range_loop(const Obj_p &next_obj, const Inst_p &current_inst_resolved) const {
        LOG_WRITE(TRACE, this->processor_,
                  L(FOS_TAB_2 "monad at !grange!! of %s !m=>!! %s [%s]\n",
                    this->processor_->M(next_obj, this->inst)->toString(), current_inst_resolved->toString(),
                    "SIGNATURE HERE"));
        //    next_obj->CHECK_OBJ_TO_INST_SIGNATURE(current_inst_resolved, false);
        const Inst_p next_inst = this->processor_->bcode_->next_inst(this->inst);
        if(next_inst->is_generative()) {
          LOG_WRITE(TRACE, this->processor_, L("monad {} dying [{}]\n", this->toString().c_str(), "SIGNATURE HERE"));
        } else if(next_obj->is_objs() && !next_inst->is_gather()) {
          //   (is_scatter(current_inst_resolved->itype()) ||
          //    is_maybe_range(current_inst_resolved->itype()))) {
          LOG_WRITE(TRACE, this->processor_,
                    L("monad {} scattering [{}]\n", this->toString().c_str(), "SIGNATURE HERE"));
          for(const Obj_p &o: *next_obj->objs_value()) {
            const Monad_p m = this->processor_->M(o, next_inst);
            LOG_WRITE(TRACE, this->processor_,
                      L("monad %s !r==!gmigrating!r==>!! %s\n", this->toString(), m->toString()));
            this->processor_->running_->push_back(m);
          }
        } else {
          const Monad_p m = this->processor_->M(next_obj, next_inst);
          LOG_WRITE(TRACE, this->processor_,
                    L("monad {} !r==!gmigrating!r==>!! {}\n", this->toString(), m->toString()));
          this->processor_->running_->push_back(m);
        }
      }

      bool operator==(const Monad &other) const { return this->equals(other); }

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
        return this->obj->toString() < rhs.obj->toString() || this->inst->toString() < rhs.inst->toString();
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

      [[nodiscard]] bool empty() const { return this->internal->empty(); }

      [[nodiscard]] long bulk_of(const Pair_p<Obj_p, Inst_p> &monad) const {
        if(const auto it = this->internal->find(make_shared<Monad>(nullptr, monad->first, monad->second));
           it != this->internal->end()) {
          return (*it)->bulk;
        }
        return 0l;
      }

      [[nodiscard]] unsigned long size() const { return this->internal->size(); }

      [[nodiscard]] unsigned long bulk_size() const {
        unsigned long bulk_total = 0;
        for(const auto &o: *this->internal) {
          bulk_total += o->bulk;
        }
        return bulk_total;
      }

      [[nodiscard]] Monad_p next() const { return std::move(this->internal->extract(this->internal->begin()).value()); }

      [[nodiscard]] Monad_p front() const { return *this->internal->begin(); }

      void pop_front() const {
        this->internal->erase(this->internal->begin());
        // auto node =
        //  return move(node.value());
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
      // return nullptr == objs ? Obj::to_noobj() : objs;
    };
  }
} // namespace fhatos

#endif
