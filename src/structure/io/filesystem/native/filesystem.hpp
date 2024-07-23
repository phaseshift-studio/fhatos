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
#include <structure/io/filesystem/abstract_filesystem.hpp>

namespace fs = std::filesystem;
namespace fhatos {
  class FileSystem : public AbstractFileSystem {
  private:
    explicit FileSystem(const ID &id, const ID &root) :
        AbstractFileSystem(id, root) {}

  public:
    static FileSystem *singleton(const ID &id = ID("/sys/io/fs"), const ID &root = ID(fs::current_path())) {
      static FileSystem fs = FileSystem(id, root);
      return &fs;
    }
    bool exists(const ID &path) const override {
      return fs::is_regular_file(fs::path(makeLocalPath(path).toString())) ||
             fs::is_directory(fs::path(makeLocalPath(path).toString()));
    }
    File_p to_file(const ID &path) const override {
      if (is_regular_file(fs::path(makeLocalPath(path).toString())))
        return Obj::to_uri(path, FILE_FURI);
      throw fError("!g[!!%s!g]!! %s does not reference a file\n", FileSystem::singleton()->id()->toString().c_str(),
                   path.toString().c_str());
    }
    Dir_p to_dir(const ID &path) const override {
      if (is_directory(fs::path(makeLocalPath(path).toString())))
        return Obj::to_uri(path, DIR_FURI);
      throw fError("!g[!!%s!g]!! %s does not reference a directory\n",
                   FileSystem::singleton()->id()->toString().c_str(), path.toString().c_str());
    }
    ID makeLocalPath(const ID &path) const override {
      return ID(singleton()->_root->toString() + "/" + path.toString());
    }
    Dir_p root() const override { return to_dir(makeLocalPath("")); }
    Dir_p mkdir(const ID &path) const override {
      if (fs::is_directory(makeLocalPath(path).toString())) {
        throw fError("!g[!!%s!g]!! %s already exists\n", FileSystem::singleton()->id()->toString().c_str(),
                     path.toString().c_str());
      }
      fs::create_directory(makeLocalPath(path).toString());
      return to_dir(path);
    }
    Objs_p ls(const Dir_p &dir, const Pattern &pattern) const override {
      auto *listing = new List<Uri_p>();
      for (const auto &p: fs::directory_iterator(fs::path(makeLocalPath(dir->uri_value()).toString()))) {
        if ((p.is_directory() || p.is_regular_file()) && ID(p.path()).matches(pattern)) {
          const ID pp =
              ID(p.path().string().substr(this->_root->toString().length() + 1)); // clip off local mount location
          if (pp.matches(pattern)) {
            listing->push_back(p.is_directory() ? to_dir(pp) : to_file(pp));
          }
        }
      }
      return Obj::to_objs(ptr<List<Uri_p>>(listing));
    }
    Objs_p more(const File_p &file) const override {
      std::ifstream fstrm(makeLocalPath(file->uri_value()).toString());
      return Obj::to_str(std::string((std::istreambuf_iterator<char>(fstrm)), std::istreambuf_iterator<char>()));
    }
    void append(const File_p &file, const Obj_p &content) override {
      std::ofstream outfile;
      outfile.open(this->id()->toString(), std::ios_base::app);
      const string s = content->toString();
      outfile.write(s.c_str(), s.length());
    }
    File_p touch(const ID &path) const override {
      if (fs::is_regular_file(makeLocalPath(path).toString())) {
        throw fError("!g[!!%s!g]!! %s already exists\n", FileSystem::singleton()->id()->toString().c_str(),
                     path.toString().c_str());
      }
      fstream fo;
      fo.open(path.toString(), ios::out);
      fo.close();
      return to_file(path);
    }
  };
} // namespace fhatos

#endif
