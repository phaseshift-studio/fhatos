#include "scheduler.hpp"

#include "bundler.hpp"

namespace fhatos {
  Scheduler::Scheduler(const ID &id) :
      Rec(rmap({{"spawn", lst()}, {"bundle", lst()}}), OType::REC, REC_FURI, id_p(id)) {
    SCHEDULER_ID = this->vid;
    LOG_WRITE(INFO, this, L("!gscheduler!! started\n"));
  }
  ptr<Scheduler> &Scheduler::singleton(const ID &id) {
    static auto scheduler = std::make_shared<Scheduler>(id);
    if(BOOTING && !scheduler->vid->equals(id) && scheduler->vid->path().find("boot") != std::string::npos) {
      scheduler->vid = id_p(id);
      SCHEDULER_ID = scheduler->vid;
      scheduler->save();
      LOG_WRITE(INFO, scheduler.get(), L("!gscheduler!! !bid!! reassigned\n"));
    }
    return scheduler;
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
      if(!this->for_scheduler.empty())
        this->for_scheduler.pop_back().value()();
      Thread::current_thread() = std::nullopt;
      this->handle_bundle();
      // this->handle_spawn();
    } catch(const fError &e) {
      LOG_WRITE(ERROR, this, L("scheduling error: {}\n", e.what()));
    }
  }

  void Scheduler::spawn_thread(const Obj_p &thread_obj) {
    if(!thread_obj->vid)
      fError::create(this->vid->toString(), "!ythread !rmust have!y a vid!!: %s", thread_obj->toString().c_str());
    auto lock = std::lock_guard<Mutex>(mutex);
    thread_obj->obj_set("halt", dool(false));
    const Lst_p thread_uris = this->obj_get("spawn")->or_else(lst());
    const auto itty =
        std::find_if(thread_uris->lst_value()->begin(), thread_uris->lst_value()->end(),
                     [&thread_obj](const Uri_p &u) -> bool { return thread_obj->vid->equals(u->uri_value()); });
    if(itty != thread_uris->lst_value()->end()) {
      throw fError::create(this->tid->toString(), "!ythread !b%s !ralready!! spawned",
                           thread_obj->vid->toString().c_str());
    }
    thread_uris->lst_value()->push_back(Obj::to_uri(*thread_obj->vid));
    this->obj_set("spawn", thread_uris);
    const Thread *thread = new Thread(thread_obj);
    LOG_WRITE(INFO, this, L("!b{} !ythread!! spawned\n", thread_obj->vid->toString()));
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    Subscription::create(
        this->vid, p_p(thread_obj->vid->extend("halt")),
        [](const Obj_p &obj, const InstArgs &args) { // target = thread_id/halt
          if(obj->is_bool() && obj->bool_value()) {
            const Lst_p new_thread_ids = Obj::to_lst();
            auto lock = std::lock_guard<Mutex>(Scheduler::singleton()->mutex);
            for(const Uri_p &thread_id: *Scheduler::singleton()->obj_get("spawn")->or_else(lst())->lst_value()) {
              if(!thread_id->uri_value().matches(args->arg("target")->uri_value().retract())) {
                new_thread_ids->lst_add(thread_id);
              } else {
                ROUTER_WRITE(args->arg("target")->uri_value().query("sub"), Obj::to_noobj(), true);
                LOG_WRITE(INFO, Scheduler::singleton().get(), L("!ythread !b{}!! stopped\n", thread_id->toString()));
              }
            }
            Scheduler::singleton()->obj_set("spawn", new_thread_ids);
          }
          return Obj::to_noobj();
        })
        ->post();
  }
  void Scheduler::bundle_fiber(const Obj_p &fiber_obj) {
    auto lock = std::lock_guard<Mutex>(mutex);
    Bundler::bundle_fiber(this, fiber_obj);
  }
  void Scheduler::handle_bundle() {
    auto lock = std::lock_guard<Mutex>(mutex);
    Bundler::handle_fibers(this);
  }

  void *Scheduler::import() {
    InstBuilder::build(Scheduler::singleton()->vid->add_component("spawn"))
        ->inst_args(rec({{"thread", Obj::to_bcode()}}))
        ->domain_range(OBJ_FURI, {0, 1}, OBJ_FURI, {1, 1})
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
