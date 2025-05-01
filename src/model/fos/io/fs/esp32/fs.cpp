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
#ifdef ESP_PLATFORM

#include <FS.h>
#include <LittleFS.h>
#include "../fs.hpp"
#include "../../../../../fhatos.hpp"
#define FOS_FS LittleFS
#define SPIFFS LittleFS

using namespace fs;

namespace fhatos {

  FS::FS(
      const Pattern &pattern,
      const ID_p &value_id,
      const Rec_p &config) :
    Structure(pattern, id_p(FS_FURI), value_id, config), root(config->rec_get("root")->uri_value()) {
    if(!FOS_FS.begin()) {
      throw fError("!runable to mount!! file system at !b%s!!", this->root.toString().c_str());
      return;
    }
    LOG_WRITE(INFO, this, L("!b{} !yfile system location!! mounted\n", this->root.toString()));
  }

  ptr<FS> FS::create(const Pattern &pattern, const ID_p &value_id, const Rec_p &config) {
    return Structure::create<FS>(pattern, value_id, config);
  }

  void FS::load_boot_config(const fURI &boot_config) {
    try {
      if(!FOS_FS.begin())
        return;
      fURI boot_config_update = fURI("/").extend(boot_config); // LittleFS doesn't support relative paths
      fs::File file = FOS_FS.open(boot_config_update.toString().c_str(), "r");
      if(!file)
        return;
      const String content = file.readString();
      const char *c = content.c_str();
      boot_config_obj_copy_len = content.length();
      boot_config_obj_copy = (unsigned char *) malloc((boot_config_obj_copy_len * sizeof(unsigned char)) + 1);
      for(int i = 0; i < boot_config_obj_copy_len; i++) {
        boot_config_obj_copy[i] = static_cast<unsigned char>(c[i]);
      }
      boot_config_obj_copy[boot_config_obj_copy_len] = '\0';
      file.close();
      FOS_FS.end();
    } catch(std::exception &ex) {
      LOG_WRITE(ERROR, Router::singleton().get(),L("{}", ex.what()));
    }
  }

  void FS::write_raw_pairs(const ID &id, const Obj_p &obj, const bool retain) {
    // TODO: retain is overwrite and transient is append
    const char *file_name = map_fos_to_fs(id).toString().c_str();
    if(obj->is_noobj()) {
      if(FOS_FS.exists(file_name))
        FOS_FS.remove(file_name);
      return;
    }

    const char *dir_name = map_fos_to_fs(id.retract()).toString().c_str();
    if(!FOS_FS.exists(dir_name))
      FOS_FS.mkdir(dir_name);
    fs::File file = FOS_FS.open(file_name, retain ? "w" : "a", true);
    const BObj_p bobj = obj->serialize();
    for(unsigned int i = 0; i < bobj->first; i++) {
      file.write(bobj->second[i]);
    }
    file.flush();
    file.close();
  }

  IdObjPairs read_raw_pairs_dir(const FS& fs,  const fURI &match, fs::File &dir) {
    IdObjPairs pairs = List<Pair<ID, Obj_p>>();
    fs::File file = dir.openNextFile();
    while(file) {
      if(!file.isDirectory()) {
        const ID path = fs.map_fs_to_fos(file.path());
        if(path.matches(match)) {
          const String contents = file.readString();
          file.close();
          const BObj_p bobj = make_shared<BObj>(contents.length(), (fbyte *) contents.c_str());
          pairs.emplace_back(Pair<ID, Obj_p>(path, Obj::deserialize(bobj)));
        }
      } else {
        IdObjPairs new_pairs = read_raw_pairs_dir(fs,match, file);
        pairs.insert(pairs.begin(), new_pairs.begin(), new_pairs.end());
      }
      file = dir.openNextFile();
    }
    dir.close();
    return pairs;
  }

  IdObjPairs FS::read_raw_pairs(const fURI &match)  {
    fs::File root = FOS_FS.open(this->root.toString().c_str());
    return read_raw_pairs_dir(*this,match, root);
  }

  /*void stop() override {
    FOS_FS.end();
    BaseFS::stop();
  }*/
}
#endif
