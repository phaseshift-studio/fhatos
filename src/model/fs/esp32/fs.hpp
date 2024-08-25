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

#ifndef fhatos_fs_hpp
#define fhatos_fs_hpp

#include <LittleFS.h>
#include <model/fs/x_fs.hpp>
#include "FS.h"

#define FOS_FS LittleFS

using namespace fs;
namespace fhatos {
  class FileSystem : public XFileSystem {
  private:
    explicit FileSystem(const ID &id, const ID &mount_root) : XFileSystem(id, mount_root) {}

  public:
    static ptr<FileSystem> singleton(const ID &id = ID("/io/fs"), const ID &root = ID("/")) {
      static ptr<FileSystem> fs_p = ptr<FileSystem>(new FileSystem(id, root));
      return fs_p;
    }

    virtual void setup() override {
      if (!FOS_FS.begin()) {
        LOG(ERROR, "Mount Failed\n");
        return;
      }
      XFileSystem::setup();
    }

    bool is_dir(const ID &path) const override {
      return FOS_FS.open(make_native_path(path).toString().c_str()).isDirectory();
    }

    bool is_file(const ID &path) const override { return !is_dir(path); }

    Dir_p mkdir(const ID &path) const override {
      if (is_dir(path))
        throw fError("!g[!b%s!g]!! %s already exists\n", this->id()->toString().c_str(), path.toString().c_str());
      if (!FOS_FS.mkdir(this->make_native_path(path).toString().c_str()))
        throw fError("!g[!b%s!g]!! %s can't be created\n", this->id()->toString().c_str(), path.toString().c_str());
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
        throw fError("!g[!b%s!g]!! %s can't be deleted\n", this->id()->toString().c_str(), uri->toString().c_str());
    }

    Objs_p ls(const Dir_p &dir) const override {
      auto listing = share<List<Uri_p>>(List<Uri_p>());
      if (!is_dir(dir->uri_value())) {
        throw fError("!g[!b%s!g]!! %s can't be opened\n", this->id()->toString().c_str(), dir->toString().c_str());
      }
      fs::File root = FOS_FS.open(this->make_native_path(dir->uri_value()).toString().c_str());
      fs::File file = root.openNextFile();
      while (file) {
        listing->push_back(
            uri(string(file.path()).substr(this->mount_root_->toString().length()))); // clip off local mount location
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

    File_p cat(const File_p &file, const Obj_p &content) override {
      fs::File f = FOS_FS.open(this->make_native_path(file->uri_value()).toString().c_str(), "rw", true);
      if (!f)
        throw fError("!g[!b%s!g]!! %s can't be opened\n", this->id()->toString().c_str(), file->toString().c_str());
      if (!f.print(content->toString().c_str())) {
        throw fError("!g[!b%s!g]!! %s can't be written to\n", this->id()->toString().c_str(), file->toString().c_str());
      }
      f.close();
      return file;
    }

    File_p touch(const ID &path) const override {
      if (this->is_file(path)) {
        throw fError("!g[!!%s!g]!! %s already exists\n", this->id()->toString().c_str(), path.toString().c_str());
      }
      FOS_FS.open(this->make_native_path(path).toString().c_str(), "rw", true);
      return to_file(path);
    }
  };
} // namespace fhatos

#endif
