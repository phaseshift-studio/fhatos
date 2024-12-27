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
#ifndef fhatos_fs_hpp
#define fhatos_fs_hpp

#include <FS.h>
#include <LittleFS.h>
#include <model/fs/base_fs.hpp>

#define FOS_FS LittleFS

using namespace fs;
namespace fhatos {
  class FileSystem : public BaseFileSystem {
  protected:
    explicit FileSystem(const ID& id, const Pattern &pattern, const ID &mount_root) : BaseFileSystem(id, pattern, mount_root) %s

  public:
    static ptr<FileSystem> create(const ID& id, const Pattern &pattern = Pattern("/io/fs/#"), const ID &root = ID("/")) {
      static ptr<FileSystem> fs_p = ptr<FileSystem>(new FileSystem(id, pattern, root));
      return fs_p;
    }

    virtual void setup() override {
      if (!FOS_FS.begin()) {
        throw fError("Unable to mount file system at %s", this->mount_root_->toString().c_str());
        return;
      }
      BaseFileSystem::setup();
    }

    bool is_dir(const ID &path) const override {
      fs::File file = FOS_FS.open(make_native_path(path).toString().c_str(), "r", false);
      const bool dir = file.isDirectory();
      file.close();
      return dir;
    }

    bool is_file(const ID &path) const override { return !is_dir(path); }

    Dir_p mkdir(const ID &path) const override {
      if (is_dir(path))
        throw fError("!g[!b%s!g]!! %s already exists", this->pattern()->toString().c_str(), path.toString().c_str());
      if (!FOS_FS.mkdir(this->make_native_path(path).toString().c_str()))
        throw fError("!g[!b%s!g]!! %s can't be created", this->pattern()->toString().c_str(),
                     path.toString().c_str());
      return this->to_dir(path);
    }

    void rm(const Uri_p &uri) const override {
      bool result;
      if (is_dir(uri->uri_value())) {
        result = FOS_FS.rmdir(this->make_native_path(uri->uri_value()).toString().c_str());
      } else if (is_file(uri->uri_value())) {
        result = FOS_FS.remove(this->make_native_path(uri->uri_value()).toString().c_str());
      }
      if (!result)
        throw fError("!g[!b%s!g]!! %s can't be deleted", this->pattern()->toString().c_str(),
                     uri->toString().c_str());
    }

    Objs_p ls(const Dir_p &dir) const override {
      auto listing = share<List<Uri_p>>(List<Uri_p>());
      if (!is_dir(dir->uri_value())) {
        throw fError("!g[!b%s!g]!! %s can't be opened", this->pattern()->toString().c_str(), dir->toString().c_str());
      }
      fs::File root = FOS_FS.open(this->make_native_path(dir->uri_value()).toString().c_str());
      fs::File file = root.openNextFile();
      while (file) {
        listing->push_back(
            to_fs(string(file.path()).substr(this->mount_root_->toString().length()))); // clip off local mount location
        file = root.openNextFile();
      }
      return Obj::to_objs(listing);
    }

    Lst_p more(const File_p &file, uint16_t max_lines) const override {
      fs::File f = FOS_FS.open(this->make_native_path(file->uri_value()).toString().c_str(), "r", false);
      List_p<Str_p> lines = share(List<Str_p>());
      string line;
      while (f.available()) {
        lines->push_back(str(f.readStringUntil('\n').c_str()));
      }
      f.close();
      return Obj::to_lst(lines);
    }

    Objs_p to_obj(const File_p &file) const override {
      Objs_p objs = Obj::to_objs();
      fs::File f = FOS_FS.open(this->make_native_path(file->uri_value()).toString().c_str(), "r", false);
      string source;
      while (f.available()) {
        source += f.readStringUntil('\n').c_str();
        if(Parser::closed_expression(source)) {
          const Obj_p obj = OBJ_PARSER(source);
          if(!obj->is_noobj())
            objs->add_obj(obj);
          source.clear();
        }
      }
      f.close();
      return objs;
    }

    File_p cat(const File_p &file, const Obj_p &content) override {
      fs::File f = FOS_FS.open(this->make_native_path(file->uri_value()).toString().c_str(), "rw", true);
      if (!f)
        throw fError("!g[!b%s!g]!! %s can't be opened", this->pattern()->toString().c_str(),
                     file->toString().c_str());
      if (!f.print(content->toString().c_str())) {
        throw fError("!g[!b%s!g]!! %s can't be written to", this->pattern()->toString().c_str(),
                     file->toString().c_str());
      }
      f.close();
      return file;
    }

    File_p touch(const ID &path) const override {
      if (this->is_file(path)) {
        throw fError("!g[!!%s!g]!! %s already exists", this->pattern()->toString().c_str(), path.toString().c_str());
      }
      FOS_FS.open(this->make_native_path(path).toString().c_str(), "rw", true);
      return to_file(path);
    }
  };
} // namespace fhatos

#endif
