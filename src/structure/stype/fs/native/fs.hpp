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

#ifdef NATIVE

#include <filesystem>
#include <fstream>
#include "../base_fs.hpp"
#include "../../../../fhatos.hpp"
#include "../../../router.hpp"
#define FOS_FS NTFS

namespace fs = std::filesystem;

namespace fhatos {
  class FSx : public BaseFS {
  public:
    explicit FSx(
      const Pattern &pattern,
      const ID_p &value_id = nullptr,
      const Rec_p &config = Obj::to_rec({{"root", vri(".")}})) : BaseFS(pattern, value_id, config) {
      LOG_OBJ(INFO,this,"file system mounted at %s\n",fs::path(config->rec_get("root")->uri_value().toString().c_str()));
    }

    static void *import(const Pattern &pattern) {
      BaseFS::import<FSx>(pattern);
      return nullptr;
    }

    static void load_boot_config(const fURI& boot_config = FOS_BOOT_CONFIG_FS_URI) {
        if(fs::is_regular_file(fs::path(boot_config.toString()))) {
          auto infile = std::ifstream(fs::path(boot_config.toString()));
          const auto content = string((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
          boot_config_obj_copy_len = content.length();
          boot_config_obj_copy = (unsigned char *) malloc((boot_config_obj_copy_len * sizeof(unsigned char)) + 1);
          for(int i = 0; i < boot_config_obj_copy_len; i++) {
            boot_config_obj_copy[i] = static_cast<unsigned char>(content[i]);
          }
          boot_config_obj_copy[boot_config_obj_copy_len] = '\0';
          infile.close();
        } else {
          throw fError("could not locate boot_config !b%s!!",boot_config.toString().c_str());
        }
    }

    void write_raw_pairs(const ID_p &id, const Obj_p &obj, const bool retain) override {
      // TODO: retain is overwrite and transient is append
      const char *file_name = map_fos_to_fs(id).toString().c_str();
      if(obj->is_noobj()) {
        if(fs::is_regular_file(fs::path(file_name)))
          fs::remove(fs::path(file_name));
        return;
      }

      const char *dir_name = map_fos_to_fs(id_p(id->retract())).toString().c_str();
      if(!fs::is_directory(fs::path(dir_name)))
        fs::create_directory(dir_name);
      std::ofstream outfile;
      outfile.open(fs::path(file_name), retain ? std::ios_base::out : std::ios_base::app);
      const BObj_p bobj = obj->serialize();
      outfile << bobj->second;
      outfile.flush();
      outfile.close();
    }

    IdObjPairs read_raw_pairs_dir(const fURI_p &match, const char *dir) {
      IdObjPairs pairs = List<Pair<ID_p, Obj_p>>();
      if(fs::is_directory(dir)) {
        for(const auto &p: fs::directory_iterator(dir)) {
          if(fs::is_directory(p)) {
            const ID_p path = id_p(map_fs_to_fos(dir));
            if(path->matches(*match)) {
              const IdObjPairs new_pairs = read_raw_pairs_dir(match, p.path().c_str());
              pairs.insert(pairs.end(), new_pairs.begin(), new_pairs.end());
            }
          } else if(fs::is_regular_file(p)) {
            auto infile = std::ifstream(fs::path(p));
            const auto content = string((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
            const Obj_p obj = Obj::deserialize(make_shared<BObj>(content.length(), (fbyte *) content.c_str()));
            infile.close();
            pairs.push_back(make_pair<ID_p, Obj_p>(make_shared<ID>(this->map_fs_to_fos(p.path().c_str())),
                                                   make_shared<Obj>(*obj)));
          }
        }
      } else if(fs::is_regular_file(dir)) {
        auto infile = std::ifstream(fs::path(dir));
        const auto content = string((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
        const Obj_p obj = Obj::deserialize(make_shared<BObj>(content.length(), (fbyte *) content.c_str()));
        infile.close();
        pairs.push_back(make_pair<ID_p, Obj_p>(make_shared<ID>(this->map_fs_to_fos(dir)),
                                               make_shared<Obj>(*obj)));
      }

        return pairs;
      }

    IdObjPairs read_raw_pairs(const fURI_p &match) {
      return read_raw_pairs_dir(match,this->root.toString().c_str());
    }
  };
}
#endif
#endif
