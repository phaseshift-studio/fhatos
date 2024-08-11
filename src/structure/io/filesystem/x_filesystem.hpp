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

#ifndef fhatos_x_filesystem_hpp
#define fhatos_x_filesystem_hpp
#include <process/actor/actor.hpp>
#include FOS_PROCESS(fiber.hpp)
#include <language/obj.hpp>
#include <structure/stype/empty.hpp>
namespace fhatos {

  using File = Uri;
  using File_p = ptr<File>;
  using Dir = Uri;
  using Dir_p = ptr<Dir>;

  static const ID_p FILE_FURI = id_p(FOS_TYPE_PREFIX "uri/fs:file");
  static const ID_p DIR_FURI = id_p(FOS_TYPE_PREFIX "uri/fs:dir");
  static const ID_p INST_FS_FURI = id_p(FOS_TYPE_PREFIX "inst/fs:");

  class Mount : public Structure {
  public:
    explicit Mount(const Pattern &pattern) : Structure(pattern, SType::READWRITE) {}
  };

  class XFileSystem : public Actor<Fiber, Mount> {

  protected:
    const ID_p _root;
    Dir_p _current;

  public:
    explicit XFileSystem(const ID &id, const ID &localRoot) :
        Actor(id, id.extend("#")), _root(id_p(localRoot.extend("/"))) {}

    void setup() override {
      Actor::setup();
      LOG_PROCESS(INFO, this, "!b%s!! !ydirectory!! mounted\n", this->_root->toString().c_str());
      this->publish(*FILE_FURI, Obj::to_bcode(), true);
      this->publish(*DIR_FURI, Obj::to_bcode(), true);
      this->_current = uri(this->_root, DIR_FURI);
      /*this->subscribe("#", [this](const Message_p &message) {
        if (message->retain) {
          const ID file = makeRouterPath(message->target);
          const File_p f = this->exists(file) ? this->to_file(file) : this->touch(file);
          this->append(f, message->payload);
        }
      });*/
      ///////////////////////////////////////////////////////////////////
      Insts::register_inst(INST_FS_FURI->resolve("root"), [this](const List<Obj_p> &) {
        return Obj::to_inst(
            "root", {}, [this](const Obj_p &) { return this->root(); }, IType::ZERO_TO_ONE, Obj::to_noobj(),
            share<ID>(INST_FS_FURI->resolve("root")));
      });
      Insts::register_inst(INST_FS_FURI->resolve("ls"), [this](const List<Obj_p> &args) {
        return Obj::to_inst(
            "ls", args,
            [this, args](const Obj_p &lhs) { return args.empty() ? this->ls(lhs) : this->ls(args.at(0)->apply(lhs)); },
            IType::ONE_TO_MANY, Obj::to_objs(), share<ID>(INST_FS_FURI->resolve("ls")));
      });
      Insts::register_inst(INST_FS_FURI->resolve("mkdir"), [this](const List<Obj_p> &args) {
        return Obj::to_inst(
            "mkdir", {args.at(0)},
            [this, args](const Obj_p &lhs) { return this->mkdir(args.at(0)->apply(lhs)->uri_value()); },
            IType::ONE_TO_MANY, Obj::to_objs(), share<ID>(INST_FS_FURI->resolve("mkdir")));
      });
      Insts::register_inst(INST_FS_FURI->resolve("more"), [this](const List<Obj_p> &args) {
        return Obj::to_inst(
            "more", args,
            [this, args](const Obj_p &lhs) {
              return args.empty() ? this->more(lhs) : this->more(args.at(0)->apply(lhs));
            },
            IType::ONE_TO_ONE, Obj::to_noobj(), share<ID>(INST_FS_FURI->resolve("more")));
      });
      Insts::register_inst(INST_FS_FURI->resolve("append"), [this](const List<Obj_p> &args) {
        return Obj::to_inst(
            "append", args, [this, args](const Obj_p &lhs) { return this->append(lhs, args.at(0)->apply(lhs)); },
            IType::ONE_TO_ONE, Obj::to_noobj(), share<ID>(INST_FS_FURI->resolve("append")));
      });
      Insts::register_inst(INST_FS_FURI->resolve("touch"), [this](const List<Obj_p> &args) {
        return Obj::to_inst(
            "touch", args, [this, args](const Obj_p &lhs) { return this->touch(args.at(0)->apply(lhs)->uri_value()); },
            IType::ONE_TO_ONE, Obj::to_noobj(), share<ID>(INST_FS_FURI->resolve("touch")));
      });
      Insts::register_inst(INST_FS_FURI->resolve("cd"), [this](const List<Obj_p> &args) {
        return Obj::to_inst(
            "cd", {args.at(0)},
            [this, args](const Objs_p &lhs) { return this->cd(to_dir(args.at(0)->apply(lhs)->uri_value())); },
            IType::MANY_TO_ONE, Obj::to_objs(), id_p(INST_FS_FURI->resolve("cd")));
      });
    }

    virtual File_p to_file(const ID &) const = 0;
    virtual Dir_p to_dir(const ID &) const = 0;
    virtual Uri_p to_fs(const ID &furi) { return exists(furi) ? is_dir(furi) ? to_dir(furi) : to_file(furi) : noobj(); }
    virtual fURI makeNativePath(const ID &) const = 0;
    virtual fURI makeFhatPath(const ID &path) const {
      return fURI(path.toString().substr(this->_root->toString().length()));
    }
    /////

    virtual Dir_p root() const = 0;
    virtual bool exists(const ID &) const = 0;
    virtual bool is_dir(const ID &) const = 0;
    virtual bool is_file(const ID &) const = 0;
    virtual Dir_p mkdir(const ID &) const = 0;
    virtual Objs_p ls(const Dir_p &dir) const = 0;
    virtual Obj_p more(const File_p &) const = 0;
    virtual File_p append(const File_p &, const Obj_p &) = 0;
    virtual File_p touch(const ID &) const = 0;
    virtual Dir_p cd(const Dir_p &dir) { return this->_current = dir; };

    //// CORE STRUCTURE FUNCTIONS

    virtual Obj_p read(const ID_p &id, const ID_p &) override { // TODO: source
      return exists(*id) ? is_dir(*id) ? to_dir(*id) : to_file(*id) : noobj();
    }

    virtual Objs_p read(const fURI_p &furi, const ID_p &) override { // TODO: source
      List<Dir_p> listA = {root()};
      List<Dir_p> listB = {};
      for (int i = this->id()->path_length(); i < furi->path_length(); i++) {
        string segment = furi->path(i);
        for (const Dir_p &d: listA) {
          if (is_dir(d->uri_value())) {
            const Objs_p objs = ls(d);
            for (const Obj_p &fd: *objs->objs_value()) {
              if (segment == "+" || segment == "#" || (fd->uri_value().name() == segment))
                listB.push_back(this->to_fs(fd->uri_value()));
            }
          }
        }
        listA.clear();
        listA = List<Dir_p>(listB);
        listB.clear();
      }
      return Obj::to_objs(listA);
    }

    virtual void write(const ID_p &id, const Obj_p &obj, const ID_p &source) override {}
  };
} // namespace fhatos

#endif
