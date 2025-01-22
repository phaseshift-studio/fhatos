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
#ifndef fhatos_router_hpp
#define fhatos_router_hpp

#include "../fhatos.hpp"
#include "structure.hpp"
#include "../lang/obj.hpp"

namespace fhatos {
  class Router final : public Rec {
  protected:
    const unique_ptr<MutexDeque<Structure_p>> structures_;

  public:
    ~Router() override = default;

    explicit Router(const ID &id);

    void load_config(const ID& config_id);

    static ptr<Router> singleton(const ID &value_id = "/sys/router/");

    [[nodiscard]] fURI_p resolve(const fURI &furi) const;

    void loop() const;

    void stop() const;

    void attach(const Structure_p &structure) const;

    void save() const override;

    [[nodiscard]] Obj_p exec(const ID_p &bcode_id, const Obj_p &arg);

    [[nodiscard]] Objs_p read(const fURI_p &furi);

    void write(const fURI_p &furi, const Obj_p &obj, bool retain = RETAIN);

    void unsubscribe(const ID_p &subscriber, const Pattern_p &pattern = p_p("#"));

    void subscribe(const Subscription_p &subscription);

    static void push_frame(const Pattern &pattern, const Rec_p &frame_data);

    static void pop_frame();

    static void *import();

  protected:
    [[nodiscard]] Structure_p get_structure(const Pattern_p &pattern, bool throw_on_error = true) const;
  };
} // namespace fhatos

#endif
