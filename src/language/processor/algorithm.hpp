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
#ifndef fhatos_algorithm_hpp
#define fhatos_algorithm_hpp

#include <fhatos.hpp>
#include <language/obj.hpp>
#include <furi.hpp>

namespace fhatos {
  class Algorithm final {
  public:
    Algorithm() = delete;

    static void embed(const Obj_p &obj, const fURI_p &root, const ID_p &source) {
      if (obj->is_noobj()) {
        // nobj
        router()->remove(id_p(*root));
      } else if (obj->is_rec()) {
        // rec
        const auto remaining = share(Obj::RecMap<>());
        for (const auto &[key,value]: *obj->rec_value()) {
          if (key->is_uri()) {
            // uri key
            if (key->uri_value().is_pattern()) // pattern key // TODO: should insert {key,value} ?
              remaining->insert({key, value});
            else
              Algorithm::embed(value, id_p(key->uri_value()), source);
          } else // non-uri key
            remaining->insert({key, value});
        }
        if (!remaining->empty())
          router()->write(id_p(root->is_branch() ? root->extend("0") : *root), Obj::to_rec(remaining), source);
      } else if (obj->is_lst()) {
        const List_p<Obj_p> list = obj->lst_value();
        for (size_t i = 0; i < list->size(); i++) {
          Algorithm::embed(list->at(i), id_p(root->extend(to_string(i))), source);
        }
      } else {
        router()->write(id_p(root->is_branch() ? root->extend("0") : *root), obj, source);
      }
    }
  };
}
#endif
