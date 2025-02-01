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
    bool active = true;
    const unique_ptr<MutexDeque<Structure_p>> structures_;

  public:
    ~Router() override = default;

    explicit Router(const ID &id);

    void load_config(const ID &config_id);

    static ptr<Router> singleton(const ID &value_id = "/sys/router/");

    [[nodiscard]] fURI_p resolve(const fURI &furi) const;

    void loop() const;

    void stop();

    void attach(const Structure_p &structure) const;

    void save() const override;

    [[nodiscard]] Obj_p exec(const ID_p &bcode_id, const Obj_p &arg);

    [[nodiscard]] Objs_p read(const fURI_p &furi);

    //[[nodiscard]] Objs_p read(const vID& &variant);

    void write(const fURI_p &furi, const Obj_p &obj, bool retain = RETAIN);

    void unsubscribe(const ID_p &subscriber, const Pattern_p &pattern = p_p("#"));

    void subscribe(const Subscription_p &subscription);

    static void push_frame(const Pattern &pattern, const Rec_p &frame_data);

    static void pop_frame();

    static void *import();

    template<typename STRUCTURE>
    static void *import_structure(const ID &import_id, const ID &type_id) {
      static_assert(std::is_base_of_v<Structure, STRUCTURE>, "STRUCTURE should be derived from Structure");
      Router::singleton()->write(id_p(type_id), Obj::to_rec({{"pattern", Obj::to_type(URI_FURI)}}));
      InstBuilder::build(id_p(import_id.extend(":create")))
          ->inst_args(Obj::to_rec({
              {"pattern", Obj::to_type(URI_FURI)},
              {"id", Obj::to_noobj()},
              {"config", Obj::to_rec()}}))
          ->domain_range(OBJ_FURI, {0, 1}, REC_FURI, {1, 1})
          ->inst_f([type_id](const Obj_p &, const InstArgs &args) {
            const Pattern pattern = args->arg("pattern")->uri_value();
            const ID_p id = args->arg("id")->is_noobj() ? nullptr : id_p(args->arg("id")->uri_value());
            const Rec_p config = args->arg("config");
            const ptr<STRUCTURE> structure = Structure::create<STRUCTURE>(pattern, id, config);
            Router::singleton()->attach(structure);
            return structure;
          })->save();
      LOG_KERNEL_OBJ(INFO, Router::singleton(), "!b%s!! !ytype!! imported\n", type_id.toString().c_str());
      return nullptr;
    }

  protected:
    [[nodiscard]] Structure_p get_structure(const Pattern_p &pattern, bool throw_on_error = true) const;
  };
} // namespace fhatos

#endif
