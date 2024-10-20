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
#ifndef fhatos_redirect_hpp
#define fhatos_redirect_hpp

#include "fhatos.hpp"
#include "language/obj.hpp"
#include "structure/structure.hpp"

namespace fhatos {
  class Redirect : public KeyValue {
  protected:
    List<Pair<fURI_p, BCode_p>> mappings_;

    fURI_p base_subscription_;
    BCode_p uri_mapper_;

    explicit Redirect(const Pattern &pattern,
                      const List<Pair<fURI_p, BCode_p>> &mappings = {}) : KeyValue(pattern), mappings_(mappings) {
    }

  public:
    static ptr<Redirect> create(const Pattern &pattern, const List<Pair<fURI_p, BCode_p>> &mappings = {}) {
      auto redirect_p = ptr<Redirect>(new Redirect(pattern, mappings));
      return redirect_p;
    }

    void setup() override {
      KeyValue::setup();
     /* for(Pair<fURI_p,BCode_p>& pair : this->mappings_) {
        router()->route_subscription(subscription_p(ID(this->pattern()->retract_pattern()), Pattern(*this->base_subscription_), Insts::to_bcode(
                                                      [this](const Message_p &message) {
                                                        router()->write(
                                                          furi_p(
                                                            this->uri_mapper_->apply(vri(message->target))->uri_value()),
                                                          message->payload, message->retain);
                                                      })));
      }*/
    }

    /* void stop() override {
       Thread::stop();
     }*/

      public:
        void write_raw_pairs(const ID_p &id, const Obj_p &obj, const bool retain) override {
         const auto base = ID(id->toString().substr(this->pattern_->retract_pattern().toString().length()));
          if(this->data_->count(id)) {
            if(obj->is_noobj()) {
              router()->route_unsubscribe(id_p(this->pattern()->retract_pattern()),p_p(*id));
            } else {
              //router()->route_subscription(subscription_p(id_p(this->pattern()->retract_pattern()),))
            }
          }

        }

        //List<Pair<ID_p, Obj_p>> read_raw_pairs(const fURI_p &match) override {
         // return this->base_structure_->read_raw_pairs(furi_p(this->uri_mapper_->apply(vri(match))->uri_value()));
        //}
  };
} // namespace fhatos

#endif
