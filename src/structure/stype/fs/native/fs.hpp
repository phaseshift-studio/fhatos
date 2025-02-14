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
#define FOS_FS NTFS

namespace fs = std::filesystem;

namespace fhatos {
  class FSx : public BaseFS {
  public:
    explicit FSx(
        const Pattern &pattern,
        const ID_p &value_id = nullptr,
        const Rec_p &config = Obj::to_rec({{"root", vri(".")}})) :
      BaseFS(
          pattern,
          value_id,
          config->rec_merge(Obj::to_rec({{
              vri("root"),
              vri(ID(fs::canonical(fs::path(config->rec_get("root")->uri_value().toString()))))
          }})->rec_value())) {
      LOG_OBJ(INFO, this, "!b%s !yfile system location!! mounted\n", this->root.toString().c_str());
    }

    static void *import(const Pattern &pattern) {
      BaseFS::import<FSx>(pattern);
      return nullptr;
    }

    static ptr<FSx> create(const Pattern &pattern, const ID_p &value_id = nullptr,
                           const Rec_p &config = Obj::to_rec()) {
      Obj_p root = config->rec_get("root");
      if(root->is_noobj())
        root = vri(".");
      const auto root_path = fs::path(root->uri_value().toString());
      if(!fs::exists(root_path))
        fs::create_directories(root_path);
      config->rec_value()->insert_or_assign(vri("root"), vri(fs::canonical(root_path)));
      return Structure::create<FSx>(pattern, value_id, config);
    }


    static void load_boot_config(const fURI &boot_config = FOS_BOOT_CONFIG_FS_URI) {
      const fs::path boot_path = fs::canonical(fs::path(boot_config.toString()));
      if(fs::is_regular_file(boot_path)) {
        auto infile = std::ifstream(boot_path, ios::in);
        if(!infile.is_open())
          throw fError("unable to read from boot config from !b%s!!", boot_path.c_str());
        const auto content = string((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
        infile.close();
        boot_config_obj_copy_len = content.length();
        boot_config_obj_copy = (unsigned char *) malloc((boot_config_obj_copy_len * sizeof(unsigned char)) + 1);
        for(int i = 0; i < boot_config_obj_copy_len; i++) {
          boot_config_obj_copy[i] = static_cast<unsigned char>(content[i]);
        }
        boot_config_obj_copy[boot_config_obj_copy_len] = '\0';

      } else {
        throw fError("could not locate boot_config !b%s!! at !b%s!!", boot_config.toString().c_str());
      }
    }

    void write_raw_pairs(const ID &id, const Obj_p &obj, const bool retain) override {
      // TODO: retain is overwrite and transient is append
      const fs::path file_path = map_fos_to_fs(id).toString();
      //LOG(INFO, "trying to write to %s\n", file_path.c_str());
      if(obj->is_noobj()) {
        if(is_regular_file(file_path))
          fs::remove(file_path);
        this->distribute_to_subscribers(Message::create(id_p(id), obj, retain));
        return;
      }
      if(id.is_node()) {
        const fs::path parent_path = file_path.parent_path();
        if(!fs::exists(parent_path))
          fs::create_directories(parent_path);
        //LOG(INFO, "writing to %s -> %s\n", id->toString().c_str(), file_path.c_str());
        const BObj_p bobj = obj->serialize();
        auto outfile = std::ofstream(file_path, retain ? ios::trunc : ios::app);
        if(!outfile.is_open())
          throw fError("unable to write to !b%s!! via !b%s!!", id.toString().c_str(), file_path.c_str());
        outfile << bobj->second;
        outfile.flush();
        outfile.close();
      } else {
        throw fError("unimplemented dir writer\n");
      }
      this->distribute_to_subscribers(Message::create(id_p(id), obj, retain));
    }

    void read_raw_pairs_dir(const fURI &match, const fs::path &fs_path, IdObjPairs *pairs) {
      const ID fos_path = map_fs_to_fos(fs_path);
      //LOG(INFO, "matching %s with %s via %s\n", match->toString().c_str(), fos_path.toString().c_str(), fs_path.c_str());
      if(fos_path.is_node() && fs::is_regular_file(fs_path) && fos_path.matches(match)) {
        auto infile = std::ifstream(fs_path, ios::in);
        if(!infile.is_open())
          throw fError("unable to read from !b%s!! via !b%s!!", fos_path.toString().c_str(), fs_path.c_str());
        const auto content = string((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
        infile.close();
        const Obj_p obj = Obj::deserialize(make_shared<BObj>(content.length(), (fbyte *) content.c_str()));
        pairs->push_back(Pair<ID, Obj_p>(fos_path, static_cast<Obj_p>(obj)));
      } else if(fs::is_directory(fs_path)) {
        for(const auto &p: fs::directory_iterator(fs_path)) {
          this->read_raw_pairs_dir(match, p, pairs);
        }
      }
    }

    IdObjPairs read_raw_pairs(const fURI &match) override {
      auto pairs = IdObjPairs();
      // LOG(INFO, "trying to read %s starting at %s\n", match->toString().c_str(),fs::path(this->root.toString()).c_str());
      read_raw_pairs_dir(match, fs::path(this->root.toString()), &pairs);
      return pairs;
    }
  };
}
#endif
#endif
