#include "scheduler.hpp"

namespace fhatos {
  Scheduler::Scheduler(const ID &id) :
      Rec(rmap({{"spawn", lst()}, {"bundle", lst()}}), OType::REC, REC_FURI, id_p(id)) {
    SCHEDULER_ID = this->vid;
    LOG_WRITE(INFO, this, L("!g[!y{}!g] !yscheduler!! started\n", id.toString()));
    // ROUTER_WRITE(this->vid->extend("halt"), Subscription::create(
  }
  ptr<Scheduler> &Scheduler::singleton(const ID &id) {
    static auto scheduler = std::make_shared<Scheduler>(id);
    return scheduler;
  }
  bool Scheduler::shutting_down() {
    return ROUTER_READ(SCHEDULER_ID->extend("halt"))->or_else(dool(false))->bool_value();
  }
  void Scheduler::stop() {
    for(const Uri_p &bundle_uri: *this->obj_get("bundle")->or_else(lst())->lst_value()) {
      LOG_WRITE(INFO, this, L("!b{} !yfiber!! closing\n", bundle_uri->toString()));
      ROUTER_WRITE(bundle_uri->uri_value(), Obj::to_noobj(), true);
      Router::singleton()->loop();
    }
    std::vector<Uri_p> list = *this->obj_get("spawn")->or_else(lst())->lst_value();
    while(!list.empty()) {
      if(list.back()->is_uri()) {
        const Obj_p thread_obj = Obj::load(list.back());
        LOG_WRITE(INFO, this, L("!b{} !ythread!! closing\n", thread_obj->vid_or_tid()->toString()));
        thread_obj->obj_set("halt", dool(true));
      }
      list.pop_back();
      Router::singleton()->loop();
    }
    const auto timestamp = std::chrono::system_clock::now();
    while((std::chrono::system_clock::now() - timestamp) < std::chrono::milliseconds(250)) {
      // do nothing (waiting for threads to close)
    }
    Router::singleton()->stop(); // ROUTER SHUTDOWN (DETACHMENT ONLY)
    LOG_WRITE(INFO, this, L("!yscheduler !b{}!! stopped\n", this->vid->toString()));
  }
  void Scheduler::loop() {
    try {
      Thread::current_thread() = std::nullopt;
      this->handle_bundle();
      // this->handle_spawn();
    } catch(const fError &e) {
      LOG_WRITE(ERROR, this, L("scheduling error: {}\n", e.what()));
    }
  }
  void Scheduler::handle_memory() {
    // check memory and if low, warn logging.
  }
  void Scheduler::spawn_thread(const Obj_p &thread_obj) {
    if(!thread_obj->vid)
      fError::create(this->vid->toString(), "!ythread !rmust have!y a vid!!: %s",
                     thread_obj->toString().c_str());
    auto lock = std::lock_guard<Mutex>(mutex);
    thread_obj->obj_set("halt", dool(false));
    const Lst_p thread_uris = this->obj_get("spawn")->or_else(lst());
    /*for(const auto &id: *thread_uris->lst_value()) {
      if(thread_obj->vid->equals(id->uri_value()))
        throw fError("!g[!b%s!g] !ralready spawned!!: %s", thread_obj->vid->toString().c_str(),
                     thread_obj->toString().c_str());
    }*/
    thread_uris->lst_value()->push_back(Obj::to_uri(*thread_obj->vid));
    this->obj_set("spawn", thread_uris);
    Thread::get_state(thread_obj);
    LOG_WRITE(INFO, this, L("!b{} !ythread!! spawned\n", thread_obj->vid->toString()));
    /*Subscription::create(this->vid, p_p(thread_obj->vid->extend("halt")),
                         [](const Obj_p &obj, const InstArgs &args) { // target = thread_id/halt
                           if(obj->is_bool() && obj->bool_value()) {
                             // LOG_WRITE(INFO,Scheduler::singleton().get(),L("HERE: {} {} -> {}\n", obj->toString(),
                             // this->rec_get("thread")->toString(),
                             // args->arg("target")->toString()));
                             const Lst_p new_thread_ids = Obj::to_lst();
                             for(const Uri_p &thread_id:
                                 *Scheduler::singleton()->obj_get("spawn")->or_else(lst())->lst_value()) {
                               if(!thread_id->uri_value().matches(args->arg("target")->uri_value().retract())) {
                                 new_thread_ids->lst_add(thread_id);
                               } else {
                                 ROUTER_WRITE(args->arg("target")->uri_value().query("sub"), Obj::to_noobj(), true);
                                 LOG_WRITE(INFO, Scheduler::singleton().get(),
                                           L("!ythread !b{}!! unscheduled\n", thread_id->toString()));
                               }
                             }
                             Scheduler::singleton()->obj_set("spawn", new_thread_ids);
                           }
                           return Obj::to_noobj();
                         })
        ->post();*/

  }
  void Scheduler::bundle_fiber(const Obj_p &fiber_obj) {
    if(!fiber_obj->vid)
      fError::create(this->vid->toString(), "!yfiber !rmust have!y a vid!!: %s",
                     fiber_obj->toString().c_str());
    auto lock = std::lock_guard<Mutex>(mutex);
    const Lst_p fiber_uris = this->obj_get("bundle")->or_else(lst());
    fiber_uris->lst_value()->push_back(Obj::to_uri(*fiber_obj->vid));
    this->obj_set("bundle", fiber_uris);
    LOG_WRITE(INFO, this, L("!b{} !yfiber!! bundled\n", fiber_obj->vid->toString()));
  }
  void Scheduler::handle_bundle() {
    auto lock = std::lock_guard<Mutex>(mutex);
    const Lst_p bundle_uris = this->obj_get("bundle")->or_else(lst());
    if(bundle_uris->lst_value()->empty())
      return;
    const size_t count = bundle_uris->lst_value()->size();
    bundle_uris->lst_value()->erase(
        std::remove_if<>(
            bundle_uris->lst_value()->begin(), bundle_uris->lst_value()->end(),
            [this](const Uri_p &fiber_id) -> bool {
              if(!fiber_id->is_uri()) {
                LOG_WRITE(ERROR, this,
                          L("fiber bundles can only store uris: {}\n", OTypes.to_chars(fiber_id->otype).c_str()));
                return true;
              }
              const Obj_p fiber = Obj::load(fiber_id);
              if(fiber->is_noobj()) {
                LOG_WRITE(INFO, this, L("!b{} !yfiber!! removed\n", fiber_id->uri_value().toString()));
                return true;
              }
              try {
                const Inst_p fiber_loop_inst =
                    Compiler(true, true).resolve_inst(fiber, Obj::to_inst(Obj::to_inst_args(), id_p("loop")));
                // LOG_WRITE(INFO, this,L("{}\n", fiber_loop_inst->toString()));
                mmADT::delift(fiber_loop_inst)->apply(fiber);
                return false;
              } catch(const fError &e) {
                LOG_WRITE(ERROR, this,
                          L("!b{} !yfiber !rloop error!!: {}\n", fiber->vid_or_tid()->toString(), e.what()));
                return true;
              }
            }),
        bundle_uris->lst_value()->end());
    if(bundle_uris->lst_value()->size() != count)
      this->obj_set("bundle", bundle_uris);
  }
  void *Scheduler::import() {
    Scheduler::singleton()->save();
    Scheduler::singleton()->obj_set("config", ROUTER_READ(FOS_BOOT_CONFIG_VALUE_ID "/scheduler")->or_else(noobj()));
    InstBuilder::build(Scheduler::singleton()->vid->add_component("spawn"))
        ->inst_args(rec({{"thread", Obj::to_bcode()}}))
        ->domain_range(OBJ_FURI, {0, 1}, THREAD_FURI, {1, 1})
        ->inst_f([](const Obj_p &, const InstArgs &args) {
          const Obj_p thread_obj = args->arg("thread");
          Scheduler::singleton()->spawn_thread(thread_obj);
          return thread_obj;
        })
        ->save();
    InstBuilder::build(Scheduler::singleton()->vid->add_component("bundle"))
        ->domain_range(OBJ_FURI, {0, 1}, OBJ_FURI, {1, 1})
        ->inst_args(rec({{"fiber", Obj::to_bcode()}}))
        ->inst_f([](const Obj_p &, const InstArgs &args) {
          const Obj_p fiber = args->arg("fiber");
          Scheduler::singleton()->bundle_fiber(fiber);
          return fiber;
        })
        ->save();
    /* Subscription::create(Scheduler::singleton()->vid, p_p(Scheduler::singleton()->vid->extend("spawn")),
                          [](const Obj_p &thread_uri, const InstArgs &) {
                            if(thread_uri->is_uri()) {
                              const Obj_p thread_obj = ROUTER_READ(thread_uri->uri_value());
                              if(!thread_obj->is_noobj())
                                Scheduler::singleton()->spawn_thread(thread_obj);
                            }
                            return Obj::to_noobj();
                          })
         ->post();*/
    return nullptr;
  }
} // namespace fhatos
