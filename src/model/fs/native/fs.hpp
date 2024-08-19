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

#ifndef fhatos_filesystem_hpp
#define fhatos_filesystem_hpp
#include <filesystem>
#include <fstream>
#include <model/fs/x_fs.hpp>

namespace fs = std::filesystem;
namespace fhatos {
  class FileSystem : public XFileSystem {
  private:
    explicit FileSystem(const ID &id, const ID &mount_root) : XFileSystem(id, mount_root) {}

  public:
    static ptr<FileSystem> singleton(const ID &id = ID("/io/fs"), const ID &root = ID(fs::current_path())) {
      static ptr<FileSystem> fs_p = ptr<FileSystem>(new FileSystem(id, root));
      return fs_p;
    }
    bool is_dir(const ID &path) const override { return fs::is_directory(fs::path(make_native_path(path).toString())); }
    bool is_file(const ID &path) const override {
      return fs::is_regular_file(fs::path(make_native_path(path).toString()));
    }
    File_p to_file(const ID &path) const override {
      if (this->is_file(path))
        return Obj::to_uri(path, FILE_FURI);
      throw fError("!g[!!%s!g]!! %s does not reference a file\n", this->id()->toString().c_str(),
                   path.toString().c_str());
    }
    Dir_p to_dir(const ID &path) const override {
      if (this->is_dir(path))
        return Obj::to_uri(path, DIR_FURI);
      throw fError("!g[!b%s!g]!! %s does not reference a directory\n", this->id()->toString().c_str(),
                   path.toString().c_str());
    }
    fURI make_native_path(const ID &path) const override {
      const string tempPath = (path.toString().substr(0, this->id()->toString().length()) == this->id()->toString())
                                  ? path.toString().substr(this->id()->toString().length())
                                  : path.toString();
      const fURI localPath =
          this->root_->resolve((!tempPath.empty() && tempPath[0] == '/') ? tempPath.substr(1) : tempPath);
      LOG_STRUCTURE(TRACE, this, "created native path %s from %s relative to %s\n", localPath.toString().c_str(),
                    path.toString().c_str(), this->root_->toString().c_str());
      if (!this->root_->is_subfuri_of(this->root_->resolve(localPath.dissolve()))) { // ../ can resolve beyond the mount
        throw fError("!r[SECURITY]!! !g[!b%s!g]!! !b%s!! outside mount location !b%s!!\n",
                     this->id()->toString().c_str(), localPath.toString().c_str(), this->root_->toString().c_str());
      }
      return localPath;
    }
    Dir_p mkdir(const ID &path) const override {
      if (fs::is_directory(make_native_path(path).toString())) {
        throw fError("!g[!b%s!g]!! %s already exists\n", this->id()->toString().c_str(), path.toString().c_str());
      }
      fs::create_directory(make_native_path(path).toString());
      return to_dir(path);
    }

    void rm(const Uri_p &uri) const override { fs::remove(uri->uri_value().toString()); }

    Objs_p ls(const Dir_p &dir) const override {
      auto listing = share<List<Uri_p>>(List<Uri_p>());
      for (const auto &p: fs::directory_iterator(this->make_native_path(dir->uri_value()).toString())) {
        if ((fs::is_directory(p) || fs::is_regular_file(p))) {
          listing->push_back(
              uri(p.path().string().substr(this->root_->toString().length()))); // clip off local mount location
        }
      }
      return Obj::to_objs(listing);
    }
    Lst_p more(const File_p &file, const uint16_t &max_lines) const override {
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
      if (fs::is_regular_file(this->make_native_path(path).toString())) {
        throw fError("!g[!!%s!g]!! %s already exists\n", this->id()->toString().c_str(), path.toString().c_str());
      }
      fstream fo;
      fo.open(path.toString(), ios::out);
      fo.close();
      return to_file(path);
    }
  };
} // namespace fhatos

#endif
