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
    explicit FileSystem(const ID &id, const ID &root) : AbstractFileSystem(id, root) {
      LOG(INFO, "Original working directory: %s\n", fs::current_path().c_str());
      const fs::path p = fs::current_path().concat("/tmp");
      // int removed = fs::remove_all(p);
      // LOG(INFO, "Deleted existing working directory with %i items\n", removed);
      // assert(fs::create_directory(p));
      fs::current_path(p); //
      LOG(INFO, "Creating new working directory: %s\n", p.c_str());
      LOG(INFO, "Test working directory: %s\n", fs::current_path().c_str());
      this->_root = share(ID(fs::current_path()));
    }

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
      throw fError("!g[!b%s!g]!! %s does not reference a directory\n",
                   FileSystem::singleton()->id()->toString().c_str(), path.toString().c_str());
    }
    ID makeLocalPath(const ID &path) const override { return singleton()->_root->extend(path.toString().c_str()); }
    Dir_p root() const override { return to_dir("/"); }
    Dir_p mkdir(const ID &path) const override {
      if (fs::is_directory(makeLocalPath(path).toString())) {
        throw fError("!g[!b%s!g]!! %s already exists\n", FileSystem::singleton()->id()->toString().c_str(),
                     path.toString().c_str());
      }
      fs::create_directory(makeLocalPath(path).toString());
      return to_dir(path);
    }
    Objs_p ls(const Dir_p &dir, const Pattern &) const override {
      auto *listing = new List<Uri_p>();
      for (const auto &p: fs::directory_iterator(fs::path(makeLocalPath(dir->uri_value()).toString()))) {
        if ((fs::is_directory(p) || fs::is_regular_file(p)) /*&& ID(p.path()).matches(pattern)*/) {
          const ID pp =
              ID(p.path().string().substr(this->_root->toString().length() + 1)); // clip off local mount location
          // if (pp.matches(pattern)) {
          listing->push_back(fs::is_directory(p) ? to_dir(pp) : to_file(pp));
        }
      }
      return Obj::to_objs(ptr<List<Uri_p>>(listing));
    }
    Obj_p more(const File_p &file) const override {
      std::ifstream fstrm(fs::path(makeLocalPath(file->uri_value()).toString()));
      std::stringstream buffer;
      buffer << fstrm.rdbuf();
      return Obj::to_str(buffer.str().c_str());
    }
    File_p append(const File_p &file, const Obj_p &content) override {
      std::ofstream outfile;
      outfile.open(fs::path(makeLocalPath(file->uri_value()).toString()), std::ios_base::app);
      const string s = content->toString();
      outfile << s.c_str();
      return file;
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
