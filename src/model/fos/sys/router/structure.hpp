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
#ifndef fhatos_structure_hpp
#define fhatos_structure_hpp

#include "../../../../fhatos.hpp"
#include "../../../../lang/obj.hpp"
#include "../../../../structure/pubsub.hpp"
#include "../../../../structure/q_proc.hpp"
#include "../../q/q_default.hpp"
#include "../../q/q_sub.hpp"


#define FOS_TRY_META                                                                                                   \
  const Option<Obj_p> meta = this->try_meta(furi);                                                                     \
  if(meta.has_value())                                                                                                 \
    return meta.value();


namespace fhatos {
  using IdObjPairs = List<Pair<ID, Obj_p>>;

  class Router;

  /////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////
  class Structure;
  using Structure_p = ptr<Structure>;

  class Structure : public Rec {
  protected:
    Rec_p q_procs_;
    std::atomic_bool available_ = std::atomic_bool(false);
    Mutex mutex = Mutex();

    //////////////////////////////////
    void write_internal(const fURI &furi, const Obj_p &obj, bool retain = RETAIN);

    Obj_p read_internal(const fURI &furi);

  public:
    const Pattern_p pattern{};
    Structure_p shared_from_this() { return ptr<Structure>(this); }
    explicit Structure(const Pattern &span, const ID_p &tid, const ID_p &vid = nullptr,
                       const Rec_p &config = Obj::to_rec()) :
        Rec(config->rec_value()->empty()
                ? Obj::to_rec({
                                  {"pattern", vri(span)},
                                  {"q_proc", rec({{"default", QDefault::create(vid ? vid->extend("q/default") : "")},
                                                  {"sub", QSub::create(vid ? vid->extend("q/sub") : "")},
                                                  {"#", QType::create()}})},
                              })
                      ->rec_value()
                : Obj::to_rec({{"pattern", vri(span)},
                               {"q_proc", rec({{"default", QDefault::create(vid ? vid->extend("q/default") : "")},
                                               {"sub", QSub::create(vid ? vid->extend("q/sub") : "")},
                                               {"#", QType::create()}})},
                               {"config", config->clone()}})
                      ->rec_value(),
            OType::REC, tid, vid),
        pattern(p_p(span)) {
      this->q_procs_ = this->Obj::rec_get("q_proc");
    }

    void save() const override {
      this->rec_set("q_proc", this->q_procs_);
      Obj::save();
    }

    template<typename STRUCTURE>
    static ptr<STRUCTURE> create(const Pattern &span, const ID_p &vid = nullptr, const Rec_p &config = Obj::to_rec()) {
      static_assert(std::is_base_of_v<Structure, STRUCTURE>, "STRUCTURE should be derived from Structure");
      ptr<STRUCTURE> s = make_shared<STRUCTURE>(span, vid, config);
      s->save();
      return s;
    }

    /*Structure_p shared_from_this() {
      return std::dynamic_pointer_cast<Structure>(const_pointer_cast<Obj>(Obj::shared_from_this()));
    }*/

    static Structure_p add_qproc(const Structure_p &structure, const ptr<QProc> &qprocA,
                                 const ptr<QProc> &qprocB = nullptr, const ptr<QProc> &qprocC = nullptr) {
      structure->q_procs_->rec_set(vri(qprocA->q_key()), qprocA);
      LOG_WRITE(INFO, structure.get(), L("!yquery processor!! !b{}!! attached\n", qprocA->vid_or_tid()->toString()));
      if(qprocB) {
        structure->q_procs_->rec_set(vri(qprocB->q_key()), qprocB);
        LOG_WRITE(INFO, structure.get(), L("!yquery processor!! !b{}!! attached\n", qprocB->vid_or_tid()->toString()));
      }
      if(qprocC) {
        structure->q_procs_->rec_set(vri(qprocB->q_key()), qprocB);
        LOG_WRITE(INFO, structure.get(), L("!yquery processor!! !b{}!! attached\n", qprocC->vid_or_tid()->toString()));
      }
      structure->save();
      return structure;
    }

    [[nodiscard]] bool available() const { return this->available_.load(); }

    void setup() override {
      if(this->available_.load()) {
        LOG_WRITE(WARN, this,
                  L("!ystructure!! !b{} !yspanning !b{}!! already mounted\n",
                    this->vid ? this->vid->toString() : "<none>", this->pattern->toString()));
        return;
      }
      this->available_.store(true);
    }

    virtual void loop();

    virtual void stop() {
      if(!this->available_.load())
        LOG_WRITE(WARN, this,
                  L("!ystructure!! !b{} !yspanning !b{}!! already stopped\n",
                    this->vid ? this->vid->toString() : "<none>", this->pattern->toString()));
      this->available_ = false;
    }

    virtual bool has(const fURI &furi);

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    [[nodiscard]] std::pair<QProc::ON_RESULT, Obj_p> process_query_read(const QProc::POSITION pos, const fURI &furi,
                                                                        const Obj_p &obj) const {
      const Objs_p results = Obj::to_objs();
      bool found = false;
      if(!furi.has_query() && QProc::POSITION::Q_LESS == pos) {
        for(const auto &[k, o]: *this->q_procs_->rec_value()) {
          const auto q = (QProc *) o.get();
          if(const QProc::ON_RESULT on_result = q->is_q_less_read(); QProc::ON_RESULT::NO_Q != on_result) {
            found = true;
            Obj_p result = q->read(pos, furi, obj);
            if(QProc::ON_RESULT::ONLY_Q == on_result)
              return {QProc::ON_RESULT::ONLY_Q, result};
            if(QProc::ON_RESULT::INCLUDE_Q == on_result)
              results->add_obj(result);
          }
        }
      } else if(furi.has_query()) {
        for(const auto &[k, o]: *this->q_procs_->rec_value()) {
          const auto q = (QProc *) o.get();
          if(const QProc::ON_RESULT on_result = QProc::POSITION::PRE == pos ? q->is_pre_read() : q->is_post_read();
             QProc::ON_RESULT::NO_Q != on_result &&
             (furi.has_query(q->q_key().toString().c_str()) || q->q_key().toString() == "#")) {
            found = true;
            const Obj_p q_obj = q->read(pos, furi, obj);
            if(QProc::ON_RESULT::ONLY_Q == on_result)
              return {on_result, q_obj};
            if(QProc::ON_RESULT::INCLUDE_Q == on_result)
              results->add_obj(q_obj);
          }
          FEED_WATCHDOG();
        }
      } else {
        return {QProc::ON_RESULT::NO_Q, obj};
      }
      if(!found) {
        // TODO: should a non-find qproc read just fail silently (WARN message)?
        if(!furi.has_query(FOS_DOMAIN) && !furi.has_query(FOS_RANGE)) {
          LOG_WRITE(WARN, this, L("!rno query processor!! for !y%s!! on read\n", furi.query()));
          /* throw fError::create(this->vid_or_tid()->toString(), "!rno query processor!! for !y%s!! on read",
                                furi.query());*/
        }
      }
      return {QProc::ON_RESULT::INCLUDE_Q, results->none_one_all()};
    }

    [[nodiscard]] QProc::ON_RESULT process_query_write(const QProc::POSITION position, const fURI &furi,
                                                       const Obj_p &obj, const bool retain) const {
      if(QProc::POSITION::Q_LESS == position) {
        for(const auto &[k, o]: *this->q_procs_->rec_value()) {
          auto q = (QProc *) o.get();
          if(QProc::ON_RESULT::NO_Q != q->is_q_less_write()) {
            q->write(position, furi, obj, retain);
          }
        }
      } else if(furi.has_query()) {
        bool found = false;
        for(const auto &[k, o]: *this->q_procs_->rec_value()) {
          const auto q = (QProc *) o.get();
          QProc::ON_RESULT on_result = position == QProc::POSITION::PRE ? q->is_pre_write() : q->is_post_write();
          if(QProc::ON_RESULT::NO_Q != on_result && (furi.has_query(q->q_key().toString().c_str()))) {
            found = true;
            q->write(position, furi, obj, retain);
            if(QProc::ON_RESULT::ONLY_Q == on_result)
              return on_result;
          }
          FEED_WATCHDOG();
        }
        if(!found) {
          if(position == QProc::POSITION::PRE) {
            const auto q = (QProc *) this->q_procs_->rec_value()->at(vri("#")).get();
            q->write(position, furi, obj, retain);
          } /*else {
            throw fError::create(this->vid_or_tid()->toString(), "!rno query processor!! for !y%s!! on write",
                                 furi.query());
          }*/
        }
      }
      return QProc::ON_RESULT::INCLUDE_Q;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    // virtual Obj_p read(const ID &source, const fURI &pattern);

    virtual Obj_p read(const fURI &furi);

    // virtual void write(const ID &source, const fURI &target, const Obj_p &obj);

    virtual void write(const fURI &furi, const Obj_p &obj, bool retain);

    virtual void append(const fURI &furi, const Obj_p &obj);

    /////////////////////////////////////////////////////////////////////////////
    Option<Pair<ID, Poly_p>>
    locate_base_poly(const fURI &furi, const Predicate<Obj_p> &poly_filter = [](const Poly_p &) { return true; }) {
      auto old_furi = fURI(furi);
      auto pc_furi = make_unique<fURI>(old_furi); // force it to be a node. good or bad?
      Obj_p obj = Obj::to_noobj();
      while(pc_furi->path_length() > 0) {
        obj = this->read_internal(*pc_furi);
        if(!obj->is_noobj() && poly_filter(obj)) // obj->is_poly() || obj->is_objs() || obj->is_code())
          break;
        const fURI new_furi = pc_furi->retract().as_node();
        auto pc_new_furi = make_unique<fURI>(new_furi);
        pc_furi.swap(pc_new_furi);
      }
      return obj->is_poly() || obj->is_objs() ? Option<Pair<ID, Poly_p>>(Pair<ID, Poly_p>(ID(*pc_furi), obj))
                                              : Option<Pair<ID, Poly_p>>();
    }

  protected:
    virtual void write_raw_pairs(const ID &id, const Obj_p &obj, bool retain) = 0;

    virtual IdObjPairs read_raw_pairs(const fURI &match) = 0;
  };
} // namespace fhatos

#endif
