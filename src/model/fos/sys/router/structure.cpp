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

#include "structure.hpp"

namespace fhatos {

  /* Obj_p Structure::read(const ID &source, const fURI &pattern) {
    return this->read(pattern);
  }*/

  Obj_p Structure::read(const fURI &furi) {
    auto lock = shared_lock<Mutex>(mutex);
    return this->read_internal(furi);
  }

  /* void Structure::write(const ID &source, const fURI &target, const Obj_p &obj) {
     this->write(target, obj, true);
   }*/

  void Structure::write(const fURI &furi, const Obj_p &obj, const bool retain) {
    auto lock = lock_guard<Mutex>(this->mutex);
    this->write_internal(furi, obj, retain);
  }


  bool Structure::has(const fURI &furi) {
    if(furi.is_node())
      return !this->read(furi)->is_noobj();
    const fURI plus_extension = furi.extend("+");
    auto lock = shared_lock<Mutex>(mutex);
    return !this->read_raw_pairs(plus_extension).empty();
  }

  void Structure::loop() {
    if(!this->available_.load())
      throw fError(FURI_WRAP " !ystructure!! is closed", this->pattern->toString().c_str());
    for(const auto &[k, o]: *this->q_procs_->rec_value()) {
      FEED_WATCHDOG();
      ((QProc *) o.get())->loop();
    }
  }

  void Structure::append(const fURI &furi, const Obj_p &obj) {
    if(const Obj_p collection = this->read(furi); collection->is_lst()) {
      collection->lst_add(obj);
      this->write(furi, collection, true);
    } else if(collection->is_rec() && obj->is_rec()) {
      collection->rec_add(obj);
      this->write(furi, collection, true);
    } else if(collection->is_bcode()) {
      this->write(furi, collection->add_bcode(obj->is_code() ? obj : __().map(obj).bcode_), true);
    } else if(collection->is_objs()) {
      collection->add_obj(obj);
      this->write(furi, collection, true);
    } else if(collection->is_noobj()) {
      this->write(furi, obj, true);
    } else {
      const Objs_p objs = Obj::to_objs({collection, obj});
      this->write(furi, objs, true);
    }
  }

  Obj_p Structure::read_internal(const fURI &furi) {
    if(!this->available_.load()) {
      LOG_WRITE(ERROR, this, L("!yunable to read!! {}\n", furi.toString()));
      return noobj();
    }
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////// READ QUERY PROCESSORS ////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    try {
      const Objs_p results = Obj::to_objs();
      if(furi.has_query()) {
        auto [on_result, q_obj] = this->process_query_read(QProc::POSITION::PRE, furi, nullptr);
        if(on_result == QProc::ON_RESULT::ONLY_Q)
          return q_obj;
        results->add_obj(q_obj);
      }
      //////////////////////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////// READ BRANCH PATTERN/ID ///////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////////////////////////
      const fURI furi_no_query = furi.no_query();
      const fURI furi_no_query_node = furi_no_query.as_node();
      IdObjPairs matches = this->read_raw_pairs(furi_no_query_node);
      if(furi_no_query.is_branch()) {
        const Rec_p rec = Obj::to_rec();
        rec->rec_value()->reserve(matches.size());
        // BRANCH ID AND PATTERN
        for(const auto &[key, value]: matches) {
          rec->rec_set(vri(key), value, false);
        }
        results->add_obj(rec);
      } else {
        //////////////////////////////////////////////////////////////////////////////////////////////////////////
        /////////////////////////////////////// READ NODE PATTERN/ID /////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////////////////////////////
        if(matches.empty()) {
          // LOG_WRITE(TRACE, this, L("searching for base poly of: {}\n", furi_no_query.toString()));
          const fURI furi_no_query_retract = furi_no_query.retract();
          if(const auto pair = this->locate_base_poly(furi_no_query_retract); pair.has_value()) {
            /*LOG_WRITE(TRACE, this, L("base poly found at {}: {}\n",
                                     pair->first.toString(),
                                     pair->second->toString())                  );*/
            const fURI furi_subpath = furi_no_query.remove_subpath(pair->first.as_branch().toString(), true);
            const Poly_p poly_read = pair->second; //->clone();
            const Obj_p read_obj = poly_read->poly_get(vri(furi_subpath));
            results->add_obj(read_obj);
          } else if(matches.empty()) {
            results->add_obj(Obj::to_noobj());
          }
        } else if(!furi_no_query.is_pattern())
          results->add_obj(matches.front().second);
        else {
          const Objs_p objs = Obj::to_objs();
          for(const auto &o: matches) {
            objs->add_obj(o.second);
          }
          results->add_obj(objs);
        }
      }
      const Obj_p new_results = Obj::to_objs();
      new_results->add_obj(
          furi.has_query() ? this->process_query_read(QProc::POSITION::POST, furi, results).second->none_one_all()
                           : this->process_query_read(QProc::POSITION::Q_LESS, furi, results).second->none_one_all());
      new_results->add_obj(results);
      return new_results->none_one_all();
    } catch(const std::exception &e) {
      throw fError("unable to read from %s\n\t %s", furi.toString().c_str(), e.what());
    }
  }

  void Structure::write_internal(const fURI &furi, const Obj_p &obj, const bool retain) {
    if(!this->available_.load()) {
      throw fError::create(this->vid_or_tid()->toString(), "!yunable to write!! %s to !b%s!!", obj->toString().c_str(),
                           furi.toString().c_str());
    }
    if(furi.has_query()) {
      if(const QProc::ON_RESULT result = this->process_query_write(QProc::POSITION::PRE, furi, obj, retain);
         result == QProc::ON_RESULT::ONLY_Q)
        return;
    }
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////// READ QUERY PROCESSORS ////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    try {
      //// WRITES
      if(const fURI new_furi = !furi.has_query() ? furi : furi.no_query(); new_furi.is_branch()) {
        // BRANCH (POLYS)
        if(obj->is_noobj()) {
          const fURI new_furi_reader =
              new_furi.ends_with("#") || new_furi.ends_with("#/") ? new_furi : new_furi.append("+");
          const IdObjPairs ids = this->read_raw_pairs(new_furi_reader);
          // noobj
          for(const auto &[key, value]: ids) {
            this->write_raw_pairs(key, obj, retain);
          }
        } else if(obj->is_rec()) {
          // rec
          const auto remaining = Obj::to_rec();
          for(const auto &[key, value]: RecMap<>(*obj->rec_value())) {
            if(key->is_uri()) {
              // uri key
              const fURI new_new_furi = new_furi.extend(key->uri_value());
              this->write_raw_pairs(new_new_furi, value, retain);
            } else // non-uri key
              remaining->rec_value()->insert({key, value});
          }
          if(!remaining->rec_value()->empty()) {
            // non-uri keyed pairs written to /0
            const fURI new_new_furi = new_furi.extend("0");
            this->write_raw_pairs(new_new_furi, remaining->clone(), retain);
          }
        } else if(obj->is_lst()) {
          // lst /0,/1,/2 indexing
          const List_p<Obj_p> list = obj->lst_value();
          for(size_t i = 0; i < list->size(); i++) {
            const fURI new_new_furi = new_furi.extend(to_string(i));
            this->write_raw_pairs(new_new_furi, list->at(i), retain);
          }
        } else {
          // BRANCH (MONOS)
          // monos written to /0
          const fURI new_new_furi = new_furi.extend("0");
          this->write_raw_pairs(new_new_furi, obj, retain);
        }
      } else {
        // NODE PATTERN
        if(new_furi.is_pattern()) {
          const IdObjPairs matches = this->read_raw_pairs(new_furi);
          for(const auto &[key, value]: matches) {
            this->write_internal(key, obj, retain);
          }
        }
        //////////////////////////////////////////////////////////////////////////////////////////////////////////
        /////////////////////////////////////// WRITE NODE ID ////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////////////////////////////
        else {
          // LOG_WRITE(TRACE, this,L("searching for base poly of: {}\n", new_furi.toString()));
          if(const auto pair = this->locate_base_poly(new_furi.retract()); pair.has_value()) {
            /*LOG_WRITE(TRACE, this,L("base poly found at {}: {}\n",
                                    pair->first.toString(), pair->second->toString()));*/
            if(const Poly_p poly_insert = pair->second; poly_insert->is_poly()) {
              const string furi_branch = pair->first.as_branch().toString();
              const fURI id_insert = new_furi.remove_subpath(furi_branch, true).as_node();
              if(obj->is_noobj())
                poly_insert->poly_drop(vri(id_insert));
              else
                poly_insert->poly_set(vri(id_insert), obj);
              // distribute_to_subscribers(Message::create(id_p(new_furi), obj, retain));
              /* LOG_WRITE(TRACE, this, L("base poly reinserted into structure at {}: {}\n",
                                        pair->first.toString(), poly_insert->toString()));*/
              this->write_internal(pair->first, poly_insert, retain); // NOTE: using write() so poly recursion happens
            } else {
              this->write_raw_pairs(new_furi, obj, retain);
            }
          } else {
            this->write_raw_pairs(new_furi, obj, retain);
          }
        }
      }
      this->process_query_write(QProc::POSITION::POST, furi, obj, retain);
      this->process_query_write(QProc::POSITION::Q_LESS, furi, obj, retain);
    } catch(const std::exception &e) {
      throw fError("!runable to write!! %s to !b%s!!\n\t %s", obj->toString().c_str(), furi.toString().c_str(),
                   e.what());
    }
  }
} // namespace fhatos
