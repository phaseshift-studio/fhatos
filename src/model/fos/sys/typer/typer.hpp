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
#ifndef fhatos_typer_hpp
#define fhatos_typer_hpp

#include "../../../../fhatos.hpp"
#include "../../../../furi.hpp"
#include "../../../../lang/mmadt/compiler.hpp"
#include "../../../../lang/obj.hpp"

namespace fhatos {
  using std::const_pointer_cast;
  inline thread_local ptr<ProgressBar> type_progress_bar_;

  inline auto REGISTERED_MODULES = new Map<ID, Inst_p>();

  class Typer final : public Obj {
  protected:
    std::vector<fURI> *filters = nullptr;

  public:
    explicit Typer(const ID &value_id);

    static ptr<Typer> &singleton(const ID &id = "/boot/typer");

    void set_filters(std::vector<fURI> *filters);

    void clear_filters();

    void start_progress_bar(uint16_t size);

    void end_progress_bar(const string &message);

    static void import();

    void save_type(const ID &type_id, const Obj_p &type_def) const;

    void register_module(const ID &module_id, const Inst_p &module) const;
    void import_module(const Pattern &module_furi);
  };

} // namespace fhatos
#endif
