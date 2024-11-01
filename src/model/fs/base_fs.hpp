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
#ifndef fhatos_x_fs_hpp
#define fhatos_x_fs_hpp

#include <fhatos.hpp>
#include <language/obj.hpp>
#include <structure/stype/computed.hpp>
#include <language/type.hpp>

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

  class BaseFileSystem : public Computed {
  protected:
    const ID_p mount_root_;
    const ID clean_root_;

  public:
    // Map<fURI_p, Function<fURI_p, List<Pair<ID_p, Obj_p>>>, furi_p_less>
    explicit BaseFileSystem(const Pattern &root, const ID &mount_root) :
      Computed(root),
      mount_root_(id_p(mount_root.extend("/"))),
      clean_root_(root.retract_pattern()) {
    }

    void setup() override {
      Type::start_progress_bar(9);
      Type::singleton()->save_type(FILE_FURI, Obj::to_bcode({Insts::as(vri(URI_FURI))}));
      Type::singleton()->save_type(DIR_FURI, Obj::to_bcode({Insts::as(vri(URI_FURI))}));
      ///////////////////////////////////////////////////////////////////
      Type::singleton()->save_type(
          id_p(INST_FS_FURI->resolve("root")),
          Obj::to_inst(
              "root", {}, [this](const InstArgs &) { return [this](const Obj_p &) { return this->root(); }; },
              IType::ZERO_TO_ONE, Obj::objs_seed(), id_p(INST_FS_FURI->resolve("root"))));
      Type::singleton()->save_type(id_p(INST_FS_FURI->resolve("ls")),
                                   Obj::to_inst(
                                       "ls", {x(0, bcode())},
                                       [this](const InstArgs &args) {
                                         return [this, args](const Obj_p &lhs) {
                                           return args.empty() ? this->ls(lhs) : this->ls(args.at(0)->apply(lhs));
                                         };
                                       },
                                       IType::ONE_TO_MANY, Obj::objs_seed(), id_p(INST_FS_FURI->resolve("ls"))));
      Type::singleton()->save_type(id_p(INST_FS_FURI->resolve("mkdir")),
                                   Obj::to_inst(
                                       "fs:mkdir", {x(0, bcode())},
                                       [this](const InstArgs &args) {
                                         return [this, args](const Obj_p &lhs) {
                                           return this->mkdir(args.at(0)->apply(lhs)->uri_value());
                                         };
                                       },
                                       IType::ONE_TO_ONE, Obj::noobj_seed(), id_p(INST_FS_FURI->resolve("mkdir"))));
      Type::singleton()->save_type(id_p(INST_FS_FURI->resolve("more")),
                                   Obj::to_inst(
                                       "fs:more", {x(0, Obj::to_bcode()), x(1, jnt(10))},
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
                                       IType::ONE_TO_ONE, Obj::noobj_seed(), id_p(INST_FS_FURI->resolve("more"))));
      Type::singleton()->save_type(id_p(INST_FS_FURI->resolve("append")),
                                   Obj::to_inst(
                                       "fs:append", {x(0)},
                                       [this](const InstArgs &args) {
                                         Insts::arg_check(id_p(INST_FS_FURI->resolve("append")), args, 1);
                                         return [this, args](const Obj_p &lhs) {
                                           return this->cat(lhs, args.at(0)->apply(lhs));
                                         };
                                       },
                                       IType::ONE_TO_ONE, Obj::noobj_seed(), id_p(INST_FS_FURI->resolve("append"))));
      Type::singleton()->save_type(id_p(INST_FS_FURI->resolve("touch")),
                                   Obj::to_inst(
                                       "fs:touch", {x(0, bcode())},
                                       [this](const InstArgs &args) {
                                         return [this, args](const Obj_p &lhs) {
                                           return this->touch(args.at(0)->apply(lhs)->uri_value());
                                         };
                                       },
                                       IType::ONE_TO_ONE, Obj::noobj_seed(), id_p(INST_FS_FURI->resolve("touch"))));
      Type::end_progress_bar(StringHelper::format("!b%s !yobjs!! loaded\n", pattern()->toString().c_str()));
      to_dir(this->clean_root_); // test to ensure mount root is a valid local directory
      LOG_STRUCTURE(INFO, this, "!b%s!! !ydirectory!! mounted\n", this->mount_root_->toString().c_str());
      Computed::setup();
    }

    virtual File_p to_file(const ID &path) const {
      if (this->is_file(path)) {
        return vri(this->clean_root_.is_subfuri_of(path) ? path : ID(this->clean_root_.extend(path)));
      }
      throw fError("!g[!!%s!g]!! %s does not reference a file", this->pattern()->toString().c_str(),
                   path.toString().c_str());
    }

    virtual Dir_p to_dir(const ID &path) const {
      if (this->is_dir(path))
        return vri((this->clean_root_.is_subfuri_of(path) ? fURI(path) : this->clean_root_.extend(path)).to_branch());
      throw fError("!g[!b%s!g]!! %s does not reference a directory", this->pattern()->toString().c_str(),
                   path.toString().c_str());
    }

    virtual Uri_p to_fs(const ID &furi) const {
      return is_fs(furi) ? is_dir(furi) ? to_dir(furi) : to_file(furi) : noobj();
    }

    virtual fURI make_native_path(const ID &path) const {
      const string temp_path_string = path.toString();
      const string temp_id_string = this->pattern_->retract_pattern().toString();
      const string temp_path = ((temp_path_string.length() >= temp_id_string.length()) &&
                                (temp_path_string.substr(0, temp_id_string.length()) == temp_id_string))
                                 ? temp_path_string.substr(temp_id_string.length())
                                 : temp_path_string;
      const fURI local_path =
          this->mount_root_->resolve((!temp_path.empty() && temp_path[0] == '/') ? temp_path.substr(1) : temp_path);
      LOG_STRUCTURE(TRACE, this, "created native path %s from %s relative to %s\n", local_path.toString().c_str(),
                    path.toString().c_str(), this->mount_root_->toString().c_str());
      if (!this->mount_root_->is_subfuri_of(local_path)) {
        throw fError("!y[!r!*SECURITY!!!y]!! !g[!b%s!g]!! !b%s!! outside mount location !b%s!!",
                     this->pattern()->toString().c_str(), local_path.toString().c_str(),
                     this->mount_root_->toString().c_str());
      }
      return local_path;
    }

    virtual fURI make_fhatos_path(const ID &path) const {
      return fURI(path.toString().substr(this->mount_root_->toString().length()));
    }

    /////

    virtual Dir_p root() const { return to_dir(this->pattern()->retract_pattern()); }

    virtual bool is_fs(const ID &path) const { return this->is_dir(path) || this->is_file(path); }

    virtual bool is_dir(const ID &) const = 0;

    virtual bool is_file(const ID &) const = 0;

    virtual Dir_p mkdir(const ID &) const = 0;

    virtual void rm(const Uri_p &) const = 0;

    virtual File_p touch(const ID &) const = 0;

    virtual Objs_p ls(const Dir_p &dir) const = 0;

    virtual Obj_p more(const File_p &, uint16_t) const = 0;

    virtual File_p cat(const File_p &, const Obj_p &) = 0;

    virtual Obj_p to_obj(const File_p &file) const = 0;

    //// CORE STRUCTURE FUNCTIONS

    Obj_p read(const fURI_p &furi) override {
      if (furi->is_pattern()) {
        List<fURI> list_a = {fURI(this->clean_root_)};
        List<fURI> list_b = {};
        for (int i = this->clean_root_.path_length(); i < furi->path_length(); i++) {
          const string segment = furi->path(i);
          for (const fURI &d: list_a) {
            if (is_dir(d)) {
              const Objs_p objs = this->ls(to_dir(d));
              for (const Obj_p &fd: *objs->objs_value()) {
                if (segment == "+" || segment == "#" || fd->uri_value().name() == segment) // todo: # recurssion?
                  list_b.push_back(fd->uri_value());
              }
            } else if (i == furi->path_length() - 1 && (segment == "+" || segment == "#" || d.name() == segment)) {
              list_b.push_back(d);
            }
          }
          list_a.clear();
          list_a = List<fURI>(list_b);
          list_b.clear();
        }
        //////////////////////////////
        List<Uri_p> ret;
        for (const auto &f: list_a) {
          if (f.matches(*furi))
            ret.push_back(to_fs(f));
        }
        return objs(ret);
      } else {
        if (furi->is_branch()) {
          // ls
          if (is_dir(*furi)) {
            return this->ls(to_dir(*furi));
          }
        } else {
          if (is_file(*furi)) {
            // more
            return this->to_obj(to_file(*furi));
          } else if (is_dir(*furi)) {
            Rec_p rec = Obj::to_rec();
            const Objs_p objs = this->ls(to_dir(*furi));
            for (const Obj_p &obj: *objs->objs_value()) {
              if (is_file(obj->uri_value())) {
                rec->rec_set(obj, to_obj(to_file(obj->uri_value())));
              } else {
                const Objs_p objs_dir = this->ls(to_dir(obj->uri_value()));
                rec->rec_set(obj, objs_dir);
              }
            }
            return rec;
          }
        }
      }
      return noobj();
    }

    void publish_retained(const Subscription_p &) override {
      // TODO:
    }

    virtual void write([[maybe_unused]] const fURI_p &furi, [[maybe_unused]] const Obj_p &obj,
                       [[maybe_unused]] const bool retain) override {
    }; // TODO: implement and remove unused
  };
} // namespace fhatos

#endif