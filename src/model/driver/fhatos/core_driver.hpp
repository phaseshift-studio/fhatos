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
#ifndef fhatos_core_driver_hpp
#define fhatos_core_driver_hpp

#include <fhatos.hpp>
#include <language/obj.hpp>

#include FOS_MQTT(mqtt.hpp)

#define TOTAL_INSTRUCTIONS 75

namespace fhatos {
  class FhatOSCoreDriver final {
    explicit FhatOSCoreDriver() = delete;

  public:
    static void *import() {
      Typer::singleton()->start_progress_bar(6);
      Typer::singleton()->save_type(MESSAGE_FURI, Obj::to_rec({
                                      {"target", Obj::to_type(URI_FURI)},
                                      {"payload", Obj::to_bcode()},
                                      {"retain", Obj::to_type(BOOL_FURI)}}));
      Typer::singleton()->save_type(SUBSCRIPTION_FURI, Obj::to_rec({
                                      {"source", Obj::to_type(URI_FURI)},
                                      {"pattern", Obj::to_type(URI_FURI)},
                                      {":on_recv", Obj::to_bcode()}}));
      //this->save_type(THREAD_FURI, Obj::to_rec({{":loop", Obj::to_bcode()}}, id_p("/sys/scheduler/lib/process")));
      Typer::singleton()->save_type(HEAP_FURI, Obj::to_rec({{"pattern", Obj::to_type(URI_FURI)}}));
      InstBuilder::build("/fos/lib/heap/create")
          ->type_args(x(0, "pattern"))
          ->domain_range(OBJ_FURI, HEAP_FURI)
          ->coefficients({0, 1}, {1, 1})
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            const Pattern pattern = args->arg(0)->uri_value();
            const ptr<Heap<>> heap = Heap<>::create(pattern);
            router()->attach(heap);
            return heap;
          })->save();

      Typer::singleton()->save_type(MQTT_FURI, Obj::to_rec({
                                      {"pattern", Obj::to_type(URI_FURI)},
                                      {"broker", Obj::to_type(URI_FURI)},
                                      {"client", Obj::to_type(URI_FURI)}}));
      Typer::singleton()->end_progress_bar(
        StringHelper::format("\n\t\t!^u1^ " FURI_WRAP " !yfhatos objs!! loaded \n",
                             OBJ_FURI->extend("+").toString().c_str()));
      Typer::singleton()->end_progress_bar("!bfhatos !yobjs!! loaded\n");
      return nullptr;
    }
  };
} // namespace fhatos
#endif
