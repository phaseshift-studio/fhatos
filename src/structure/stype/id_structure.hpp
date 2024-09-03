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
#ifndef fhatos_id_structure_hpp
#define fhatos_id_structure_hpp

#include <fhatos.hpp>
#include <furi.hpp>
#include <structure/structure.hpp>

namespace fhatos {
  class IDStructure : public Structure {
  protected:
    Obj_p id_obj = noobj();

    explicit IDStructure(const ID_p &id) : Structure(Pattern(*id), SType::READWRITE) {
    }

  public:
    static ptr<IDStructure> create(const ID_p &id) {
      ptr<IDStructure> id_structure_p = ptr<IDStructure>(new IDStructure(id));
      return id_structure_p;
    }

    void write_retained(const Subscription_p &subscription) override {
      if (!this->id_obj->is_noobj() && this->pattern()->matches(subscription->pattern)) {
        subscription->onRecv(share(Message{
          .source = FOS_DEFAULT_SOURCE_ID, .target = ID(*this->pattern()), .payload = id_obj, .retain = RETAIN_MESSAGE
        }));
      }
    }

    void write(const ID_p &id, const Obj_p &obj, const ID_p &source, const bool retain) override {
      if (id->matches(*this->pattern()) && retain)
        this->id_obj = obj;
      distribute_to_subscriptions(share(Message{.source = *source, .target = *id, .payload = obj, .retain = retain}));
    }

    Obj_p read(const fURI_p &id, const ID_p &) override {
      return id->matches(*this->pattern()) ? this->id_obj : noobj();
    }
  };
}
#endif
