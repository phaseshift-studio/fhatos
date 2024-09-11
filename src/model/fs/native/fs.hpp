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

#include <filesystem>
#include <fstream>
#include <model/fs/x_fs.hpp>

namespace fs = std::filesystem;

namespace fhatos {
  class FileSystem : public XFileSystem {
  protected:
    explicit FileSystem(const ID &id, const ID &mount_root) : XFileSystem(id, mount_root) {
    }

  public:
    static ptr<FileSystem> create(const ID &id = ID("/io/fs"), const ID &root = ID(fs::current_path())) {
      ptr<FileSystem> fs_p = ptr<FileSystem>(new FileSystem(id, ID(fs::current_path()).resolve(root)));
      return fs_p;
    }

    bool is_dir(const ID &path) const override {
      return fs::is_directory(fs::path(this->make_native_path(path).toString()));
    }

    bool is_file(const ID &path) const override {
      return fs::is_regular_file(fs::path(this->make_native_path(path).toString()));
    }

    Dir_p mkdir(const ID &path) const override {
      if (fs::is_directory(this->make_native_path(path).toString())) {
        throw fError("!g[!b%s!g]!! %s already exists\n", this->id()->toString().c_str(), path.toString().c_str());
      }
      fs::create_directory(this->make_native_path(path).toString());
      return to_dir(path);
    }

    void rm(const Uri_p &uri) const override { fs::remove(uri->uri_value().toString()); }

    Objs_p ls(const Dir_p &dir) const override {
      auto listing = share<List<Uri_p>>(List<Uri_p>());
      for (const auto &p: fs::directory_iterator(this->make_native_path(dir->uri_value()).toString())) {
        if ((fs::is_directory(p) || fs::is_regular_file(p))) {
          listing->push_back(
            this->to_fs(ID(p.path().string().substr(
              this->mount_root_->toString().length())))); // clip off local mount location
        }
      }
      return Obj::to_objs(listing);
    }

    Lst_p more(const File_p &file, uint16_t max_lines) const override {
      std::ifstream infile(fs::path(this->make_native_path(file->uri_value()).toString()));
      List_p<Str_p> lines = share(List<Str_p>());
      string line;
      uint16_t counter = 0;
      while ((0 == max_lines || counter++ < max_lines) && std::getline(infile, line)) {
        std::istringstream iss(line);
        lines->push_back(str(line));
      }
      return Obj::to_lst(lines);
    }

    File_p cat(const File_p &file, const Obj_p &content) override {
      std::ofstream outfile;
      outfile.open(fs::path(this->make_native_path(file->uri_value()).toString()), std::ios_base::app);
      const string s = content->toString();
      outfile << s.c_str();
      return file;
    }

    File_p touch(const ID &path) const override {
      const string nativePathString = this->make_native_path(path).toString();
      LOG(INFO, "HERE %s %s\n", nativePathString.c_str(), path.toString().c_str());
      if (fs::is_regular_file(nativePathString)) {
        throw fError("!g[!!%s!g]!! %s already exists\n", this->id()->toString().c_str(), path.toString().c_str());
      }
      fstream fo;
      fo.open(fs::path(nativePathString), ios::out);
      fo.close();
      return to_file(path);
    }
  };
} // namespace fhatos

#endif
