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
#include <language/obj.hpp>
#include <structure/furi.hpp>

namespace fhatos {
  class Exts {

  public:
    Exts() = delete;
    static List<Pair<ID_p, Type_p>> exts(const ID &extId) {
      static Map_p<ID_p, List<Pair<ID_p, Type_p>>> _exts =
          share(Map<ID_p, List<Pair<ID_p, Type_p>>>{{id_p("/ext/process"),
                                                     {{id_p("/rec/thread"), TYPE_PARSER("[setup=>_,loop=>_]")},
                                                      {id_p("/rec/fiber"), TYPE_PARSER("[setup=>_,loop=>_]")},
                                                      {id_p("/rec/coroutine"), TYPE_PARSER("[setup=>_,loop=>_]")},
                                                      {id_p("/inst/stop"), TYPE_PARSER("[map(Ø).to(*_0)]")}}},
                                                    {id_p("/ext/collection"),
                                                     {{id_p("/lst/pair"), TYPE_PARSER("[_,_]")},
                                                      {id_p("/lst/trip"), TYPE_PARSER("[_,_,_]")},
                                                      {id_p("/lst/quad"), TYPE_PARSER("[_,_,_,_]")}}}});
      return _exts->at(share(extId));
    }
  };

  class Extension : public IDed {
  protected:
    Lst_p _imports;
    Rec_p _prefixes;
    Rec_p _typedefs;

  public:
    explicit Extension(const ID &id, const initializer_list<Uri> imports,
                       const initializer_list<Pair<const Uri, Uri>> &prefixes,
                       const initializer_list<Pair<const Uri, Obj>> &typedefs) :
        IDed(share(id)), //
        _imports(Obj::to_lst(imports)), //
        _prefixes(Obj::to_rec(prefixes)), //
        _typedefs(Obj::to_rec(typedefs)) {}
    void load(const Router *router, const ID &source = FOS_DEFAULT_SOURCE_ID) const {
      for (const auto &prefix: *_prefixes->rec_value()) {
        router->write(prefix.first->uri_value(), prefix.second, source);
      }
      for (const auto &type: *_typedefs->rec_value()) {
        router->write(type.first->uri_value(), type.second, source);
      }
    }
    const Rec_p imports() const { return this->_imports; }
    const Rec_p prefixes() const { return this->_prefixes; }
    const Rec_p typedefs() const { return this->_typedefs; }
    Rec_p to_obj() {
      Obj::RecMap_p<Uri_p, Rec_p> map = share(Obj::RecMap<Uri_p, Rec_p>());
      map->insert({u_p("prefixes"), this->_prefixes});
      map->insert({u_p("types"), this->_typedefs});
      return Obj::to_rec(map);
    }
  };

  //////////////////////////////////////
  Extension to_ext(const ID &id) {
    if (id.equals("/ext/process/")) {
      return Extension(ID("/ext/process/"),
                       // imports
                       {u("math:")},
                       // prefixes
                       {{u("thread:"), u("/type/rec/thread")},
                        {u("fiber:"), u("/type/rec/fiber")},
                        {u("coroutine:"), u("/type/rec/coroutine")}},
                       // types
                       {{u("thread:"), *TYPE_PARSER("[setup=>_,loop=>_]")},
                        {u("fiber:"), *TYPE_PARSER("[setup=>_,loop=>_]")},
                        {u("coroutine:"), *TYPE_PARSER("[setup=>_,loop=>_]")}});
    }
    if (id.equals("/ext/struct/")) {
      return Extension(ID("/ext/struct/"),
                       // imports
                       {},
                       // prefixes
                       {{u("pair:"), u("/type/lst/pair")}},
                       // types
                       {{u("pair:"), *TYPE_PARSER("[_,_]")}});
    };
    throw fError("Unknown extension: %s\n", id.toString().c_str());
  }
} // namespace fhatos

#endif
