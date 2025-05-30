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
#include "../../s/frame.hpp"
#include "memory/memory.hpp"
#include "structure.hpp"

#define FOS_ROUTER_STRUCTURE "structure"
#define FOS_ROUTER_QUERY "query"
#define FOS_ROUTER_QUERY_WRITE "query/write"
#define FOS_ROUTER_QUERY_READ "query/read"

namespace fhatos {

  class Router final : public Rec {
  protected:
    const ptr<MutexDeque<Structure_p>> structures_;
    std::vector<Uri_p> auto_prefixes_;

  public:
    ~Router() override = default;

    explicit Router(const ID &id);

    static ptr<Router> &singleton(const ID &vid = "/sys/router");

    [[nodiscard]] fURI resolve(const fURI &furi) const;

    void loop() const;

    void stop();

    void attach(const Structure_p &structure) const;

    void save() const override;

    [[nodiscard]] Objs_p read(const fURI &furi) const;

    void write(const fURI &furi, const Obj_p &obj, bool retain = RETAIN);

    void append(const fURI &furi, const Obj_p &obj) const;

    static void push_frame(const Pattern &pattern, const Rec_p &frame_data);

    static void pop_frame();

    static ptr<Frame<>> get_frame();

    static void *import();

    template<typename STRUCTURE>
    static void *register_structure_module(const ID &type_id) {
      static_assert(std::is_base_of_v<Structure, STRUCTURE>, "STRUCTURE should be derived from Structure");
      MODEL_CREATOR2->insert_or_assign(type_id, [](const Obj_p &structure_obj) {
        return STRUCTURE::create(structure_obj->rec_get("pattern")->uri_value(), structure_obj->vid,
                                 structure_obj->rec_get("config")->or_else(Obj::to_rec()));
      });
      REGISTERED_MODULES->insert_or_assign(
          type_id, InstBuilder::build(Typer::singleton()->vid->add_component(type_id))
                       ->domain_range(OBJ_FURI, {0, 1}, REC_FURI, {1, 1})
                       ->inst_f([type_id](const Obj_p &, const InstArgs &) {
                         return Obj::to_rec({{vri(type_id), Obj::to_rec({{"pattern?uri", __()}})},
                                             {vri(type_id.add_component("mount")),
                                              InstBuilder::build(type_id.add_component("mount"))
                                                  ->domain_range(id_p(type_id), {1, 1}, id_p(type_id), {1, 1})
                                                  ->inst_f([](const Obj_p &structure, const InstArgs &) {
                                                    Router::singleton()->attach(
                                                        structure->get_model<Structure>()->shared_from_this());
                                                    return structure;
                                                  })
                                                  ->create()}});
                       })
                       ->create());
      return nullptr;
    }

  protected:
    [[nodiscard]] Structure_p get_structure(const Pattern &pattern, const Obj_p &to_write = nullptr,
                                            bool throw_on_error = true) const;
  };
} // namespace fhatos

#endif
