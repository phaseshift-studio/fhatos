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
#include <structure/io/filesystem/abstract_filesystem.hpp>
namespace fs = std::filesystem;
namespace fhatos {
  class FileSystem : public AbstractFileSystem {
    FileSystem(const ID &id = ID("/sys/io/fs")) : AbstractFileSystem(id) {}

  public:
    static FileSystem *singleton(const ID &id = ID("/sys/io/fs")) {
      static FileSystem fs = FileSystem();
      return &fs;
    }

    File_p file(const Uri_p &fpath) const override {
      auto p = fs::path(fpath->uri_value().toString());
      if (fs::exists(p)) {
        return share(File(fpath->uri_value()));
      }
      return Obj::to_noobj();
    };

    Dir_p dir(const Uri_p &dpath) const override {
      auto p = fs::path(dpath->uri_value().toString());
      if (fs::exists(p)) {
        return share(Dir(dpath->uri_value()));
      }
      return Obj::to_noobj();
    }
  };
} // namespace fhatos

#endif
