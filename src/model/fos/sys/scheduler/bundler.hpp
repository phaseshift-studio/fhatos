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
#ifndef fhatos_bundler_hpp
#define fhatos_bundler_hpp

#include "../../../../fhatos.hpp"
//
#include "../../../../furi.hpp"
#include "../../../../lang/mmadt/mmadt_obj.hpp"
#include "../../../../lang/obj.hpp"

namespace fhatos {

  class Bundler final {

  public:
    explicit Bundler() = delete;

    template<typename T>
    static void bundle_fiber(const T *bundler, const Obj_p &fiber_obj) {
      if(!fiber_obj->vid)
        fError::create(bundler->vid->toString(), "!yfiber vid !rrequired!!: %s", fiber_obj->toString().c_str());
      const Lst_p fiber_uris = bundler->obj_get("bundle")->or_else(lst());
      const auto itty =
          std::find_if(fiber_uris->lst_value()->begin(), fiber_uris->lst_value()->end(),
                       [&fiber_obj](const Uri_p &u) -> bool { return fiber_obj->vid->equals(u->uri_value()); });
      if(itty != fiber_uris->lst_value()->end()) {
        throw fError::create(bundler->tid->toString(), "!yfiber !b%s !ralready bundled!!",
                             fiber_obj->vid->toString().c_str());
      }
      fiber_uris->lst_value()->push_back(Obj::to_uri(*fiber_obj->vid));
      bundler->obj_set("bundle", fiber_uris);
      LOG_WRITE(INFO, bundler, L("!b{} !yfiber!! bundled\n", fiber_obj->vid->toString()));
    }

    template<typename T>
    static void handle_fibers(const T *bundler) {
      const Lst_p bundle_uris = bundler->obj_get("bundle")->or_else(lst());
      if(bundle_uris->lst_value()->empty())
        return;
      const size_t count = bundle_uris->lst_value()->size();
      bundle_uris->lst_value()->erase(
          std::remove_if<>(
              bundle_uris->lst_value()->begin(), bundle_uris->lst_value()->end(),
              [bundler](const Uri_p &fiber_id) -> bool {
                if(!fiber_id->is_uri()) {
                  LOG_WRITE(ERROR, bundler,
                            L("fiber bundles can only store uris: {}\n", OTypes.to_chars(fiber_id->otype).c_str()));
                  return true;
                }
                const Obj_p fiber = Obj::load(fiber_id);
                if(fiber->is_noobj()) {
                  LOG_WRITE(INFO, bundler, L("!b{} !yfiber!! removed\n", fiber_id->uri_value().toString()));
                  return true;
                }
                try {
                  //  const Inst_p fiber_loop_inst = Compiler().with_derivation_tree().resolve_inst(
                  //      fiber, Obj::to_inst(Obj::to_inst_args(), id_p("loop")));
                  mmADT::delift(fiber->obj_get("loop"))->apply(fiber);
                  // mmADT::delift(fiber_loop_inst)->apply(fiber);
                  return false;
                } catch(const fError &e) {
                  LOG_WRITE(ERROR, bundler,
                            L("!b{} !yfiber !rloop error!!: {}\n", fiber->vid_or_tid()->toString(), e.what()));
                  return true;
                }
              }),
          bundle_uris->lst_value()->end());
      if(bundle_uris->lst_value()->size() != count)
        bundler->obj_set("bundle", bundle_uris);
    }
  };
} // namespace fhatos

#endif
