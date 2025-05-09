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

#include "../../../../fhatos.hpp"
#include "../../../../lang/obj.hpp"
#include "structure/frame.hpp"
#include "structure/structure.hpp"

#define FOS_ROUTER_STRUCTURE "structure"
#define FOS_ROUTER_QUERY "query"
#define FOS_ROUTER_QUERY_WRITE "query/write"
#define FOS_ROUTER_QUERY_READ "query/read"

namespace fhatos {

  static ID_p ROUTER_FURI = id_p("/sys/router_t");

  class Router final : public Rec {
  protected:
    const ptr<MutexDeque<Structure_p>> structures_;

  public:
    ~Router() override = default;

    explicit Router(const ID &id);

    static ptr<Router> &singleton(const ID &value_id = "/sys/router/");

    [[nodiscard]] fURI resolve(const fURI &furi) const;

    void loop() const;

    void stop();

    void attach(const Structure_p &structure) const;

    void save() const override;

    [[nodiscard]] Obj_p exec(const ID &bcode_id, const Obj_p &arg);

    [[nodiscard]] Objs_p read(const fURI &furi);

    void write(const fURI &furi, const Obj_p &obj, bool retain = RETAIN);

    void append(const fURI &furi, const Obj_p &obj) const;

    static void push_frame(const Pattern &pattern, const Rec_p &frame_data);

    static void pop_frame();

    static ptr<Frame<>> get_frame();

    static void *import();

    template<typename STRUCTURE>
    static void *import_structure(const ID &type_id) {
      static_assert(std::is_base_of_v<Structure, STRUCTURE>, "STRUCTURE should be derived from Structure");
      Typer::singleton()->save_type(type_id, Obj::to_rec({{"pattern?uri", __()}}));
      MODEL_CREATOR2->insert_or_assign(type_id, [](const Obj_p &structure_obj) {
        return Structure::create<STRUCTURE>(structure_obj->rec_get("pattern")->uri_value(), structure_obj->vid,
                                            structure_obj->rec_get("config")->or_else(Obj::to_rec()));
      });
      InstBuilder::build(Router::singleton()->vid->add_component("mount"))
          ->inst_args(rec({{"structure", Obj::to_bcode()}}))
          ->domain_range(OBJ_FURI, {0, 1}, OBJ_FURI, {1, 1})
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            const Obj_p structure_obj = args->arg("structure");
            const ptr<Structure> structure = structure_obj->get_model<Structure>()->shared_from_this();
            Router::singleton()->attach(structure);
            Router::singleton()->loop();
            return structure;
          })
          ->save();
      // LOG_WRITE(INFO, Router::singleton().get(), L("!b{}!! !ytype!! imported\n", type_id.toString()));
      return nullptr;
    }

  protected:
    [[nodiscard]] Structure_p get_structure(const Pattern &pattern, const Obj_p &to_write = nullptr,
                                            bool throw_on_error = true) const;
  };
} // namespace fhatos

#endif
