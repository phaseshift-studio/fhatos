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

#ifndef fhatos_abstract_filesystem_hpp
#define fhatos_abstract_filesystem_hpp
#include <process/actor/actor.hpp>
#include FOS_PROCESS(coroutine.hpp)
#include <language/obj.hpp>
namespace fhatos {

  using File = Uri;
  using File_p = Uri_p;
  using Dir = Uri;
  using Dir_p = Uri_p;

  static const ID_p FILE_FURI = share(ID("/uri/fs:file"));
  static const ID_p DIR_FURI = share(ID("/uri/fs:dir"));
  static const ID_p INST_FS_FURI = share(ID("/inst/fs:"));

  class AbstractFileSystem : public Actor<Coroutine> {
  public:
    ID_p _root;
    explicit AbstractFileSystem(const ID &id, const ID &localRoot) : Actor(id), _root(share(localRoot)) {}

    void setup() override {
      Actor::setup();
      LOG_TASK(INFO, this, "!b%s!! !ydirectory!! mounted\n", this->_root->toString().c_str());
      TYPE_WRITER(*FILE_FURI, Obj::to_bcode({}));
      TYPE_WRITER(*DIR_FURI, Obj::to_bcode({}));
      this->publish(*this->id(), this->root()->apply(Obj::to_noobj()), RETAIN_MESSAGE);
      this->subscribe("#", [this](const Message_p &message) {
        if (message->retain) {
          const ID file = makeRouterPath(message->target);
          const File_p f = this->exists(file) ? this->to_file(file) : this->touch(file);
          this->append(f, message->payload);
        }
      });
      ///////////////////////////////////////////////////////////////////
      Insts::register_inst(INST_FS_FURI->resolve("root"), [this](List<Obj_p>) {
        return Obj::to_inst(
            "root", {}, [this](const Obj_p &) { return this->root(); }, IType::ZERO_TO_ONE, Obj::to_noobj(),
            share(INST_FS_FURI->resolve("root")));
      });
      Insts::register_inst(INST_FS_FURI->resolve("ls"), [this](List<Obj_p> args) {
        return Obj::to_inst(
            "ls", args,
            [this, args](const Obj_p &lhs) {
              return args.empty() ? this->ls(lhs, "#") : this->ls(args.at(0)->apply(lhs), "#");
            },
            IType::ONE_TO_MANY, Obj::to_objs(), share(INST_FS_FURI->resolve("ls")));
      });
      Insts::register_inst(INST_FS_FURI->resolve("mkdir"), [this](List<Obj_p> args) {
        return Obj::to_inst(
            "mkdir", {args.at(0)},
            [this, args](const Obj_p &lhs) { return this->mkdir(args.at(0)->apply(lhs)->uri_value()); },
            IType::ONE_TO_MANY, Obj::to_objs(), share(INST_FS_FURI->resolve("mkdir")));
      });
      Insts::register_inst(INST_FS_FURI->resolve("more"), [this](List<Obj_p> args) {
        return Obj::to_inst(
            "more", args,
            [this, args](const Obj_p &lhs) {
              return args.empty() ? this->more(lhs) : this->more(args.at(0)->apply(lhs));
            },
            IType::ONE_TO_ONE, Obj::to_noobj(), share(INST_FS_FURI->resolve("more")));
      });
      Insts::register_inst(INST_FS_FURI->resolve("append"), [this](List<Obj_p> args) {
        return Obj::to_inst(
            "append", args, [this, args](const Obj_p &lhs) { return this->append(lhs, args.at(0)->apply(lhs)); },
            IType::ONE_TO_ONE, Obj::to_noobj(), share(INST_FS_FURI->resolve("append")));
      });
    }

    virtual File_p to_file(const ID &) const { throw fError("must be implemented"); }
    virtual Dir_p to_dir(const ID &) const { throw fError("must be implemented"); }
    virtual ID makeLocalPath(const ID &) const { throw fError("must be implemented"); }
    virtual ID makeFilePath(const ID &path) const {
      return ID(path.toString().substr(this->id()->toString().length() + 1));
    }
    virtual ID makeFhatPath(const ID &path) const { return this->id()->extend(path.toString().c_str()); }
    virtual ID makeRouterPath(const ID &path) const { return ID(this->id()->toString() + "/" + path.toString()); }
    ////
    virtual Dir_p root() const { throw fError("must be implemented"); }
    virtual bool exists(const ID &) const { throw fError("must be implemented"); }
    virtual Dir_p mkdir(const ID &) const { throw fError("must be implemented"); }
    virtual Objs_p ls(const Dir_p &, const Pattern &) const { throw fError("must be implemented"); }
    virtual Obj_p more(const File_p &) const { throw fError("must be implemented"); }
    virtual File_p append(const File_p &, const Obj_p &) { throw fError("must be implemented"); }
    virtual File_p touch(const ID &) const { throw fError("must be implemented"); }
  };
} // namespace fhatos

#endif
