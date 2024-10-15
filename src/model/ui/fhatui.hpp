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
#ifndef fhatui_hpp
#define fhatui_hpp

#include <fhatos.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/captured_mouse.hpp>  // for ftxui
#include <ftxui/component/component.hpp>  // for Collapsible, Renderer, Vertical
#include <ftxui/component/loop.hpp>  // for Collapsible, Renderer, Vertical
#include <ftxui/component/component_base.hpp>  // for ComponentBase
#include <ftxui/component/screen_interactive.hpp>  // for Component, ScreenInteractive
#include <ftxui/dom/elements.hpp>          // for text, hbox, Element

#include "fhatui.hpp"

namespace fhatos {
  using namespace ftxui;

  inline Component Inner(std::vector<Component> children) {
    Component vlist = Container::Vertical(std::move(children));
    return Renderer(vlist, [vlist] {
      return hbox({
        text("==>"),
        vlist->Render(),
      });
    });
  }

  inline Component Empty() {
    return std::make_shared<ComponentBase>();
  }

  class StructureTree : public Coroutine {
  protected:
    fURI root_;

    StructureTree(const ID &id, const fURI &root) : Coroutine(id), root_(root) {
    }

  public:
    static ptr<StructureTree> create(const ID &id, const fURI &root) {
      const auto tree = ptr<StructureTree>(new StructureTree(id, root));
      tree->display_root();
      return tree;
    }


    void display_root() {
      const Objs_p objs = router()->read(furi_p(this->root_.extend("+")));
      List<Component> children;
      auto container = Container::Vertical({});
      for (const Obj_p &obj: *objs->objs_value()) {
        auto c = Collapsible(obj->uri_value().toString().c_str(), Empty());
        auto container2 = Container::Vertical({});
        container2->Add(Inner({Collapsible(obj->toString(true,false).c_str(), Empty())}));
        container->Add(Inner({container2}));
        children.push_back(c);
      }
      container->Add(Inner(children));
      auto screen = ScreenInteractive::Fullscreen();
      screen.TrackMouse(true);
      screen.Active();
      screen.Loop(container);
      printer<Ansi<>>()->print(screen.ToString().c_str());
    }
  };
} // namespace fhatos
#endif
