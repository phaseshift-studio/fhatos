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

#ifndef fhatos_x_fs_hpp
#define fhatos_x_fs_hpp

#include <fhatos.hpp>
#include <language/obj.hpp>
#include <process/actor/actor.hpp>
#include FOS_PROCESS(fiber.hpp)
#include <language/types.hpp>

#define FOS_DEFAULT_MORE_LINES 10

namespace fhatos {
  using File = Uri;
  using File_p = ptr<File>;
  using Dir = Uri;
  using Dir_p = ptr<Dir>;

  static const ID_p FILE_FURI = id_p(FOS_TYPE_PREFIX "uri/fs:file");
  static const ID_p DIR_FURI = id_p(FOS_TYPE_PREFIX "uri/fs:dir");
  static const ID_p INST_FS_FURI = id_p(FOS_TYPE_PREFIX "inst/fs:");
  static const ID_p INST_ROOT_FURI = id_p(INST_FS_FURI->resolve("root"));

  class Mount : public Structure {
  public:
    explicit Mount(const Pattern &pattern) : Structure(pattern, SType::READWRITE) {
    }
  };

  class XFileSystem : public Actor<Fiber, Mount> {
  protected:
    const ID_p mount_root_;

  public:
    explicit XFileSystem(const ID &id, const ID &mount_root) : Actor(id, id.extend("#")),
                                                               mount_root_(id_p(mount_root.extend("/"))) {
    }

    virtual void setup() override {
      Actor::setup();
      LOG_PROCESS(INFO, this, "!b%s!! !ydirectory!! mounted\n", this->mount_root_->toString().c_str());
      // define filesystem types
      Types::singleton()->saveType(FILE_FURI, Obj::to_bcode({Insts::as(uri(FOS_TYPE_PREFIX "uri/"))}));
      Types::singleton()->saveType(DIR_FURI, Obj::to_bcode({Insts::as(uri(FOS_TYPE_PREFIX "uri/"))}));
      /*this->subscribe(this->id()->extend("#"), [this](const Message_p &message) {
          if (message->retain && message->payload->is_noobj()) {
              // delete the fs resource
              this->rm(to_fs(message->target));
          } else {
              const Obj_p result = message->payload->apply(to_fs(message->target));
              // apply the fs resource to the bytecode
              if (message->retain) // if retain, then ---
                  this->write(id_p(message->target), result, id_p(message->source));
                  // write the result to the fs resource
              else // else ---
                  this->publish(message->source, result, TRANSIENT_MESSAGE);
              // publish the result to the source id
          }
      });*/
      ///////////////////////////////////////////////////////////////////
      Types::singleton()->saveType(id_p(INST_FS_FURI->resolve("root")),
                                   Obj::to_inst(
                                           "root", {}, [this](const InstArgs &) {
                                             return [this](const Obj_p &) { return this->root(); };
                                           },
                                           IType::ZERO_TO_ONE, Obj::objs_seed(),
                                           id_p(INST_FS_FURI->resolve("root"))));
      Types::singleton()->saveType(id_p(INST_FS_FURI->resolve("ls")),
                                   Obj::to_inst(
                                           "ls", {x(0)},
                                           [this](const InstArgs &args) {
                                             return [this, args](const Obj_p &lhs) {
                                               return args.empty()
                                                      ? this->ls(lhs)
                                                      : this->ls(args.at(0)->apply(lhs));
                                             };
                                           },
                                           IType::ONE_TO_MANY, Obj::objs_seed(), id_p(INST_FS_FURI->resolve("ls"))));
      Types::singleton()->saveType(id_p(INST_FS_FURI->resolve("mkdir")),
                                   Obj::to_inst(
                                           "fs:mkdir", {x(0)},
                                           [this](const InstArgs &args) {
                                             return [this, args](const Obj_p &lhs) {
                                               return this->mkdir(args.at(0)->apply(lhs)->uri_value());
                                             };
                                           },
                                           IType::ONE_TO_ONE, Obj::noobj_seed(),
                                           id_p(INST_FS_FURI->resolve("mkdir"))));
      Types::singleton()->saveType(id_p(INST_FS_FURI->resolve("more")),
                                   Obj::to_inst(
                                           "fs:more", {x(0), x(1)},
                                           [this](const InstArgs &args) {
                                             return [this, args](const Obj_p &lhs) {
                                               if (args.empty())
                                                 return this->more(lhs, FOS_DEFAULT_MORE_LINES);
                                               if (args.size() == 1) {
                                                 const Obj_p ap_arg0 = args.at(0)->apply(lhs);
                                                 if (ap_arg0->is_int())
                                                   return this->more(lhs, ap_arg0->int_value());
                                                 else
                                                   return this->more(ap_arg0, FOS_DEFAULT_MORE_LINES);
                                               } else {
                                                 const Obj_p ap_arg0 = args.at(0)->apply(lhs);
                                                 const Obj_p ap_arg1 = args.at(1)->apply(lhs);
                                                 return this->more(ap_arg0, ap_arg1->int_value());
                                               }
                                             };
                                           },
                                           IType::ONE_TO_ONE, Obj::noobj_seed(),
                                           id_p(INST_FS_FURI->resolve("more"))));
      Types::singleton()->saveType(id_p(INST_FS_FURI->resolve("append")),
                                   Obj::to_inst(
                                           "fs:append", {x(0)},
                                           [this](const InstArgs &args) {
                                             return [this, args](const Obj_p &lhs) {
                                               return this->cat(lhs, args.at(0)->apply(lhs));
                                             };
                                           },
                                           IType::ONE_TO_ONE, Obj::noobj_seed(),
                                           id_p(INST_FS_FURI->resolve("append"))));
      Types::singleton()->saveType(id_p(INST_FS_FURI->resolve("touch")),
                                   Obj::to_inst(
                                           "fs:touch", {x(0)},
                                           [this](const InstArgs &args) {
                                             return [this, args](const Obj_p &lhs) {
                                               return this->touch(args.at(0)->apply(lhs)->uri_value());
                                             };
                                           },
                                           IType::ONE_TO_ONE, Obj::noobj_seed(),
                                           id_p(INST_FS_FURI->resolve("touch"))));
      Types::singleton()->loop();
    }

    virtual File_p to_file(const ID &path) const {
      if (this->is_file(path))
        return uri(id_p(path), FILE_FURI);
      throw fError("!g[!!%s!g]!! %s does not reference a file\n", this->id()->toString().c_str(),
                   path.toString().c_str());
    }

    virtual Dir_p to_dir(const ID &path) const {
      if (this->is_dir(path))
        return uri(id_p(path), DIR_FURI);
      throw fError("!g[!b%s!g]!! %s does not reference a directory\n", this->id()->toString().c_str(),
                   path.toString().c_str());
    }

    virtual Uri_p to_fs(const ID &furi) {
      return is_fs(furi) ? is_dir(furi) ? to_dir(furi) : to_file(furi) : noobj();
    }

    virtual fURI make_native_path(const ID &path) const {
      const string tempPathString = path.toString();
      const string tempIDString = this->id()->toString();
      const string tempPath = ((tempPathString.length() >= tempIDString.length()) &&
                               (tempPathString.substr(0, tempIDString.length()) == tempIDString))
                              ? tempPathString.substr(tempIDString.length())
                              : tempPathString;
      const fURI localPath =
              this->mount_root_->resolve(
                      (!tempPath.empty() && tempPath[0] == '/') ? tempPath.substr(1) : tempPath);
      LOG_STRUCTURE(TRACE, this, "created native path %s from %s relative to %s\n", localPath.toString().c_str(),
                    path.toString().c_str(), this->mount_root_->toString().c_str());
      if (!this->mount_root_->is_subfuri_of(localPath)) {
        throw fError("!r[SECURITY]!! !g[!b%s!g]!! !b%s!! outside mount location !b%s!!\n",
                     this->id()->toString().c_str(), localPath.toString().c_str(),
                     this->mount_root_->toString().c_str());
      }
      return localPath;
    }

    virtual fURI make_fhatos_path(const ID &path) const {
      return fURI(path.toString().substr(this->mount_root_->toString().length()));
    }

    /////

    virtual Dir_p root() const { return to_dir(*this->id()); }

    virtual bool is_fs(const ID &path) const { return this->is_dir(path) || this->is_file(path); }

    virtual bool is_dir(const ID &) const = 0;

    virtual bool is_file(const ID &) const = 0;

    virtual Dir_p mkdir(const ID &) const = 0;

    virtual void rm(const Uri_p &) const = 0;

    virtual File_p touch(const ID &) const = 0;

    virtual Objs_p ls(const Dir_p &dir) const = 0;

    virtual Obj_p more(const File_p &, uint16_t) const = 0;

    virtual File_p cat(const File_p &, const Obj_p &) = 0;

    //// CORE STRUCTURE FUNCTIONS

    Obj_p read(const fURI_p &furi, const ID_p &) override {
      // TODO: source
      if (furi->is_pattern()) {
        List<Dir_p> listA = {root()};
        List<Dir_p> listB = {};
        for (int i = this->id()->path_length(); i < furi->path_length(); i++) {
          string segment = furi->path(i);
          for (const Dir_p &d: listA) {
            if (is_dir(d->uri_value())) {
              const Objs_p objs = ls(d);
              for (const Obj_p &fd: *objs->objs_value()) {
                if (segment == "+" || segment == "#" ||
                    (fd->uri_value().name() == segment)) // todo: # infinite recurssion?
                  listB.push_back(this->to_fs(fd->uri_value()));
              }
            }
          }
          listA.clear();
          listA = List<Dir_p>(listB);
          listB.clear();
        }
        return objs(listA);
      } else {
        return !is_fs(*furi) ? noobj() : to_fs(*furi);
      }
    }

    virtual void write([[maybe_unused]] const ID_p &id, [[maybe_unused]] const Obj_p &obj,
                       [[maybe_unused]] const ID_p &source) override {
    }; // TODO: implement and remove unused
  };
} // namespace fhatos

#endif
