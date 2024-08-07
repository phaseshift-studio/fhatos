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
#include FOS_PROCESS(coroutine.hpp)
#include <language/obj.hpp>
#include <structure/io/filesystem/fs.hpp>
namespace fhatos {

  using File = Uri;
  using File_p = Uri_p;
  using Dir = Uri;
  using Dir_p = Uri_p;

  static const ID_p FILE_FURI = share(ID("/uri/fs:file"));
  static const ID_p DIR_FURI = share(ID("/uri/fs:dir"));
  static const ID_p INST_FS_FURI = share(ID("/inst/fs:"));

  class XFileSystem : public Actor<Coroutine> {

  protected:
    const ID_p _root;
    ID_p _current;

  public:
    explicit XFileSystem(const ID &id, const ID &localRoot) :
        Actor(id), _root(id_p(localRoot.extend("/"))), _current(id_p(localRoot.extend("/"))) {}


    /* Obj_p find(const ID &id) const  {
       if (!this->in_range(id))
         throw ID_NOT_IN_RANGE(id, *this->range_);
       if (this->exists(id))
         return this->is_dir(id) ? this->to_dir(id) : this->to_file(id);
       throw ID_DOES_NOT_EXIST(id, *this->range_);
     };*/

    void setup() override {
      Actor::setup();
      LOG_PROCESS(INFO, this, "!b%s!! !ydirectory!! mounted\n", this->_root->toString().c_str());
      this->publish(*FILE_FURI, Obj::to_bcode(), true);
      this->publish(*DIR_FURI, Obj::to_bcode(), true);
      this->publish(*this->id(), this->root()->apply(Obj::to_noobj()), RETAIN_MESSAGE);
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
            [this, args](const Obj_p &lhs) {
              return args.empty() ? this->ls(lhs, "#") : this->ls(args.at(0)->apply(lhs), "#");
            },
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
            [this, args](const Objs_p &lhs) { return this->cd(args.at(0)->apply(lhs)->uri_value()); },
            IType::MANY_TO_ONE, Obj::to_objs(), id_p(INST_FS_FURI->resolve("cd")));
      });
    }

    virtual File_p to_file(const ID &) const = 0;
    virtual Dir_p to_dir(const ID &) const = 0;
    virtual ID makeNativePath(const ID &) const = 0;
    virtual ID makeFhatPath(const ID &path) const {
      return ID(path.toString().substr(this->_root->toString().length()));
    }
    /////

    virtual Dir_p root() const = 0;
    virtual bool exists(const ID &) const = 0;
    virtual bool is_dir(const ID &) const = 0;
    virtual bool is_file(const ID &) const = 0;
    virtual Dir_p mkdir(const ID &) const = 0;
    virtual Objs_p ls(const Dir_p &, const Pattern &) const = 0;
    virtual Obj_p more(const File_p &) const = 0;
    virtual File_p append(const File_p &, const Obj_p &) = 0;
    virtual File_p touch(const ID &) const = 0;
    virtual Dir_p cd(const ID &) = 0;
  };
} // namespace fhatos

#endif
