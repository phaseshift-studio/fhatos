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
#ifndef fhatos_exts_hpp
#define fhatos_exts_hpp


#include <fhatos.hpp>
#include <furi.hpp>
#include <language/obj.hpp>

namespace fhatos {
  class Exts {
  public:
    Exts() = delete;

    static List<Pair<ID, Type_p>> exts(const ID &extId) {
      static Map_p<ID, List<Pair<ID, Type_p>>> _exts =
          ptr<Map<ID, List<Pair<ID, Type_p>>>>(new Map<ID, List<Pair<ID, Type_p>>>{
            {"/model/sys", {
             // {"/type/rec/process", OBJ_PARSER("[id=>uri[_],setup=>_,loop=>_]")},
              {"/type/rec/thread", OBJ_PARSER("[:setup=>_,:loop=>_,:stop=>_]")},
             // {"/type/rec/fiber", OBJ_PARSER("process[_]")},
             // {"/type/rec/coroutine", OBJ_PARSER("process[_]")},
             ///////
             // {"/type/rec/structure", OBJ_PARSER("[pattern=>uri[_],setup=>_,loop=>_]")},
            //  {"/type/rec/database", OBJ_PARSER("structure[_]")},
            //  {"/type/rec/ephemeral", OBJ_PARSER("structure[_]")},
            // {"/type/rec/hardware", OBJ_PARSER("structure[_]")},
             // {"/type/rec/networked", OBJ_PARSER("structure[_]")},
             // {"/type/rec/variables", OBJ_PARSER("structure[_]")},
              //////
            //  {"/type/rec/actor", OBJ_PARSER("~[process=>lst,structure=>lst]")},
              {"/type/inst/stop", OBJ_PARSER("map(noobj).to(*_0)")},
           // {"/model/pubsub", {
             //   {"/type/rec/sub",
           //       OBJ_PARSER("[:source=>uri[_],:pattern=>uri[_],:qos=>is(gt(0)).is(lt(4)),:on_recv=>_]")},
             //   {"/type/rec/pub", OBJ_PARSER("[:source=>uri[_],:target=>uri[_],:payload=>_,:retain=>bool[_]]")}}
            }
          }});
      /* {"/ext/collection",
        {{"/lst/pair", TYPE_PARSER("[_,_]")},
         {"/lst/trip", TYPE_PARSER("[_,_,_]")},
         {"/lst/quad", TYPE_PARSER("[_,_,_,_]")}}}*/
      return _exts->at(extId);
    }
  };
} // namespace fhatos

#endif
