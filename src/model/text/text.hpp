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
#ifndef fhatos_text_hpp
#define fhatos_text_hpp
#ifdef NATIVE
#include "../../fhatos.hpp"
#include "../../furi.hpp"
#include "../../util/string_helper.hpp"
#include "../../lang/mmadt/parser.hpp"
#include "../../lang/obj.hpp"
#include "../terminal.hpp"
#include "../../util/print_helper.hpp"
#include "ext/kilo.hpp"

#include  STR(../../process/ptype/HARDWARE/thread.hpp)

namespace fhatos {


  class Text final : public Thread {

  public:
	 ext::Kilo* kilo;

  explicit Text(const ID &value_id) :  Thread(Obj::to_rec({
{":setup", InstBuilder::build(id_p(value_id.extend(":setup")))
   ->domain_range(NOOBJ_FURI, {0, 0}, NOOBJ_FURI, {0, 0})
                             ->inst_f([this](const Obj_p &, const InstArgs &) -> Obj_p {
                               this->setup();
                               return Obj::to_noobj();
                             })
                             ->create()},
    {":loop", InstBuilder::build(id_p(value_id.extend(":loop")))
                             ->domain_range(NOOBJ_FURI, {0, 0}, NOOBJ_FURI, {0, 0})
                             ->inst_f([this](const Obj_p &, const InstArgs &) -> Obj_p {
                               this->loop();
                               return Obj::to_noobj();
                             })
                             ->create()}}, REC_FURI, id_p(value_id))), kilo(new ext::Kilo()) {
    }

void setup() const {
   kilo->initEditor();
  //  kilo.editorSelectSyntaxHighlight(argv[1]);
 //   kilo.editorOpen(argv[1]);
    kilo->enableRawMode(STDIN_FILENO);
    kilo->editorSetStatusMessage(
        "HELP: Ctrl-S = save | Ctrl-Q = quit | Ctrl-F = find");
}

void loop() const {
       kilo->editorRefreshScreen();
        kilo->editorProcessKeypress(STDIN_FILENO);
}




    static ptr<Text> create(const ID &id) {
      const static auto text = ptr<Text>(new Text(id));
      return text;
    }

    static void *import(const ID &lib_id = "/io/lib/text") {
      // Type::singleton()->save_type(id_p("/io/console/"),rec({{}}));
      InstBuilder::build(ID(lib_id.extend(":create")))
          ->domain_range(OBJ_FURI, {0, 1}, REC_FURI, {1, 1})
          ->inst_args(Obj::to_rec({{"id",Obj::to_type(URI_FURI)}}))
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            ptr<Text> text = Text::create(ID(args->arg("id")->uri_value()));
            return text;
          })
          ->save();
      return nullptr;
    }
    };
   }
   #endif
#endif