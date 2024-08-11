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
#include <structure/io/filesystem/x_filesystem.hpp>

namespace fs = std::filesystem;
namespace fhatos {
  class FileSystem : public XFileSystem {
  private:
    explicit FileSystem(const ID &id, const ID &root) : XFileSystem(id, root) {}

  public:
    static FileSystem *singleton(const ID &id = ID("/sys/io/fs"), const ID &root = ID(fs::current_path())) {
      static FileSystem fs = FileSystem(id, root);
      return &fs;
    }
    bool is_dir(const ID &path) const override { return fs::is_directory(fs::path(makeNativePath(path).toString())); }
    bool is_file(const ID &path) const override {
      return fs::is_regular_file(fs::path(makeNativePath(path).toString()));
    }
    bool exists(const ID &path) const override {
      return fs::is_regular_file(fs::path(makeNativePath(path).toString())) ||
             fs::is_directory(fs::path(makeNativePath(path).toString()));
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
    fURI makeNativePath(const ID &path) const override {
      const fURI localPath =
          (path.toString().substr(0, this->id()->toString().length()) == this->id()->toString())
              ? this->_current->uri_value().resolve(path.toString().substr(this->id()->toString().length()))
              : this->_current->uri_value().resolve(path.toString().c_str());
      LOG_STRUCTURE(TRACE, this, "created native path %s from %s relative to %s\n", localPath.toString().c_str(),
                    path.toString().c_str(), this->_current->toString().c_str());
      if (!this->_root->is_subfuri_of(this->_root->resolve(localPath.dissolve()))) { // ../ can resolve beyond the mount
        throw fError("!r[SECURITY]!! !g[!b%s!g]!! !b%s!! outside mount location !b%s!!\n",
                     this->id()->toString().c_str(), localPath.toString().c_str(), this->_root->toString().c_str());
      }
      return localPath;
    }
    Dir_p root() const override { return to_dir(*this->id()); }
    Dir_p mkdir(const ID &path) const override {
      if (fs::is_directory(makeNativePath(path).toString())) {
        throw fError("!g[!b%s!g]!! %s already exists\n", this->id()->toString().c_str(), path.toString().c_str());
      }
      fs::create_directory(makeNativePath(path).toString());
      return to_dir(path);
    }
    Objs_p ls(const Dir_p &dir) const override {
      auto listing = share<List<Uri_p>>(List<Uri_p>());
      for (const auto &p: fs::directory_iterator(this->makeNativePath(dir->uri_value()).toString())) {
        if ((fs::is_directory(p) || fs::is_regular_file(p))) {
          listing->push_back(
              uri(p.path().string().substr(this->_root->toString().length()))); // clip off local mount location
        }
      }
      return Obj::to_objs(listing);
    }
    Obj_p more(const File_p &file) const override {
      std::ifstream fstrm(fs::path(this->makeNativePath(file->uri_value()).toString()));
      std::stringstream buffer;
      buffer << fstrm.rdbuf();
      return Obj::to_str(buffer.str().c_str());
    }
    File_p append(const File_p &file, const Obj_p &content) override {
      std::ofstream outfile;
      outfile.open(fs::path(this->makeNativePath(file->uri_value()).toString()), std::ios_base::app);
      const string s = content->toString();
      outfile << s.c_str();
      return file;
    }
    File_p touch(const ID &path) const override {
      if (fs::is_regular_file(this->makeNativePath(path).toString())) {
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
