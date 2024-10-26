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
#ifndef fhatos_obj_structure_hpp
#define fhatos_obj_structure_hpp

#include <fhatos.hpp>
#include <language/obj.hpp>
#include <structure/router.hpp>
#include <structure/stype/computed.hpp>
#include <structure/stype/heap.hpp>
#include FOS_MQTT(mqtt.hpp)
#include "router.hpp"

namespace fhatos {
  template<typename STRUCTURE>
  class StructureObj : public STRUCTURE {
  protected:
    const Rec_p structure_rec_;
    const Uri_p pattern_uri_;

  public:
    explicit StructureObj(const Pattern &pattern, const Rec_p &structure_rec) :
        STRUCTURE(pattern), structure_rec_(structure_rec), pattern_uri_(Obj::to_uri(pattern)) {}

    void setup() override {
      try {
        STRUCTURE::setup();
        LOG_STRUCTURE(DEBUG, this, "Executing setup()-bcode: %s\n",
                      this->structure_rec_->rec_get(vri(this->pattern()->resolve(":setup")))->toString().c_str());
        process(this->structure_rec_->rec_get(vri(this->pattern()->resolve(":setup"))), this->pattern_uri_);
      } catch (const fError &error) {
        LOG_EXCEPTION(error);
        this->stop();
      }
    }

    void loop() override {
      try {
        if (this->available_.load()) {
          const BCode_p loop_bcode = this->structure_rec_->rec_get(vri(this->pattern()->resolve(":loop")));
          process(loop_bcode, this->pattern_uri_);
        }
      } catch (const fError &error) {
        LOG_EXCEPTION(error);
        this->stop();
      }
    }

    void stop() override {
      try {
        if (this->available_.load()) {
          LOG_STRUCTURE(DEBUG, this, "Executing stop()-bcode: %s\n",
                        this->structure_rec_->rec_get(vri(this->pattern()->resolve(":stop")))->toString().c_str());
          process(this->structure_rec_->rec_get(vri(this->pattern()->resolve(":stop"))), this->pattern_uri_);
          STRUCTURE::stop();
        }
      } catch (const fError &error) {
        LOG_EXCEPTION(error);
      }
    }
  };

  class HeapObj : public StructureObj<Heap> {
  public:
    explicit HeapObj(const Pattern &pattern, const Rec_p &structure_rec) : StructureObj(pattern, structure_rec) {}
  };

  /*class MqttObj : public StructureObj<Mqtt> {
  public:
    explicit MqttObj(const Pattern &pattern, const Rec_p &structure_rec) : StructureObj(pattern, structure_rec) {}
  };*/

  class ComputedObj : public StructureObj<Computed> {
  public:
    explicit ComputedObj(const Pattern &pattern, const Rec_p &structure_rec) : StructureObj(pattern, structure_rec) {}
  };

  inline void load_structure_attacher() {
    STRUCTURE_ATTACHER = [](const Pattern &structure_pattern, const Obj_p &structure_rec) {
      if (HEAP_FURI->equals(*structure_rec->type()))
        router()->attach(std::make_shared<HeapObj>(structure_pattern, structure_rec));
     // else if (MQTT_FURI->equals(*structure_rec->type()))
     //   router()->attach(std::make_shared<MqttObj>(structure_pattern, structure_rec));
      else if (COMPUTED_FURI->equals(*structure_rec->type()))
        router()->attach(std::make_shared<ComputedObj>(structure_pattern, structure_rec));
      else
        throw fError("Unknown structure type: %s", structure_rec->type()->toString().c_str());
    };
  }
} // namespace fhatos
#endif
