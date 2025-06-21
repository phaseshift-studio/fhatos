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

#include "../fs.hpp"
#include <FS.h>
#include <LittleFS.h>
#include "../../../../../fhatos.hpp"
#include "../../../../../lang/mmadt/parser.hpp"
#define FOS_FS LittleFS
#define SPIFFS LittleFS

using namespace fs;

namespace fhatos {

  static bool FS_MOUNTED = false;
  void FS::list_files_utility(const char *path) {
    if(!FS_MOUNTED) {
      if(FS_MOUNTED = FOS_FS.begin())
        LOG_WRITE(INFO, Router::singleton().get(), L("{} mounted successfully\n", STR(FOS_FS)));
      else
        LOG_WRITE(INFO, Router::singleton().get(), L("{} failed to mount\n", STR(FOS_FS)));
    }
    File root = FOS_FS.open(path);
    if(!root) {
      Serial.println("Failed to open directory");
      return;
    }
    if(!root.isDirectory()) {
      Serial.println("Not a directory");
      return;
    }

    File file = root.openNextFile();
    while(file) {
      if(file.isDirectory()) {
        Serial.print("dir: ");
        Serial.println(file.name());
        list_files_utility(file.path());
      } else {
        Serial.print("\tfile: ");
        Serial.println(file.name());
      }
      file = root.openNextFile();
    }
  }

  ID FS::map_fos_to_fs(const ID &fos_id) const {
    const fURI fs_retracted_id = fos_id.remove_subpath(this->pattern->retract_pattern().toString());
    return this->root.extend(fs_retracted_id);
  }

  ID FS::map_fs_to_fos(const string &fs_id) const {
    const auto fos_id = ID(fs_id);
    const fURI fos_retracted_id = fos_id.remove_subpath(this->root.toString());
    const fURI retracted_pattern = this->pattern->retract_pattern();
    return retracted_pattern.extend(fos_retracted_id);
  }

  FS::FS(const Pattern &pattern, const ID_p &value_id, const Rec_p &config) :
      Structure(pattern, id_p(FS_TID), value_id, config), root(config->rec_get("root")->uri_value()) {
    if(vid && this->root.equals(ID("."))) {
      this->root = ID("data").extend(string("/").append(vid->name()));
      this->obj_set("config", vri(this->root));
    } else {
      this->root = ID("data").extend(config->rec_get("root")->or_else(vri("."))->uri_value());
    }
    if(!FS_MOUNTED) {
      if(!FOS_FS.begin()) {
        throw fError("!runable to mount!! file system at !b%s!!", this->root.toString().c_str());
        return;
      }
      FS_MOUNTED = true;
      /*const esp_partition_t* partition = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, NULL);
      if (partition) {
        ESP_LOGI("Memory Info", "Found App partition");
        ESP_LOGI("Memory Info", "Partition Label: %s", partition->label);
        ESP_LOGI("Memory Info", "Partition Type: %d", partition->type);
        ESP_LOGI("Memory Info", "Partition Subtype: %d", partition->subtype);
        ESP_LOGI("Memory Info", "Partition Size: %" PRIu32 " bytes", partition->size);
      } else {
        ESP_LOGE("Memory Info", "Failed to get the App partition");
      }*/
    }
  }

  void FS::setup() {
    LOG_WRITE(INFO, this, L("!b{} !ylocation!! mounted\n", this->root.toString()));
    Structure::setup();
  }

  Obj_p FS::load_boot_config(const fURI &boot_config) {
    try {
      if(!FS_MOUNTED) {
        if(!FOS_FS.begin())
          return Obj::to_noobj();
        FS_MOUNTED = true;
      }
      const fURI boot_config_update = boot_config.toString()[0] == '/'
                                          ? boot_config
                                          : fURI("/").extend(boot_config); // LittleFS doesn't support relative paths
      fs::File file = FOS_FS.open(boot_config_update.toString().c_str(), "r");
      LOG_WRITE(INFO, Router::singleton().get(), L("!b{} !yboot loader littlefs location!!\n", file.path()));
      if(!file)
        return Obj::to_noobj();
      const String content = file.readString();
      boot_config_obj_copy_len = content.length();
      boot_config_obj_copy = (unsigned char *) malloc((boot_config_obj_copy_len * sizeof(unsigned char)) + 1);
      for(int i = 0; i < boot_config_obj_copy_len; i++) {
        boot_config_obj_copy[i] = static_cast<unsigned char>(content.c_str()[i]);
      }
      boot_config_obj_copy[boot_config_obj_copy_len] = '\0';
      if(boot_config_obj_copy) {
        LOG_WRITE(
            INFO, Router::singleton().get(),
            L("!b{} !yboot config file!! loaded !g[!msize!!: {} bytes!g]!!\n", file.path(), boot_config_obj_copy_len));
      } else {
        file.close();
        FOS_FS.end();
        FS_MOUNTED = false;
        return Obj::to_noobj();
      }
      file.close();
      FOS_FS.end();
      FS_MOUNTED = false;
      return Memory::singleton()->use_custom_stack(InstBuilder::build("boot_config_parser")
                                                       ->inst_f([](const Obj_p &obj, const InstArgs &) {
                                                         const auto proto = make_unique<mmadt::Parser>();
                                                         const Obj_p boot_obj = proto->parse(obj->str_value().c_str());
                                                         return boot_obj;
                                                       })
                                                       ->create(),
                                                   Obj::to_str((char *) boot_config_obj_copy),
                                                   FOS_BOOT_CONFIG_MEM_USAGE);
    } catch(const fError &e) {
      LOG_WRITE(ERROR, Router::singleton().get(), L("{}\n", e.what()));
      return Obj::to_noobj();
    }
  }

  ptr<FS> FS::create(const Pattern &pattern, const ID_p &value_id, const Rec_p &config) {
    /*const Obj_p root = config->rec_get("root")->or_else(vri("."));
    string root_path_str = root->uri_value().toString();
    const auto root_path =
        fs::canonical(fs::path(root_path_str[0] == '.' ? root_path_str : root_path_str.insert(0, "./")));
    if(!fs::exists(root_path))
      fs::create_directories(root_path);
    config->rec_value()->insert_or_assign(vri("root"), vri(root_path));*/
    return Structure::create<FS>(pattern, value_id, config);
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

  IdObjPairs read_raw_pairs_dir(const FS &fs, const fURI &match, fs::File &dir) {
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
        IdObjPairs new_pairs = read_raw_pairs_dir(fs, match, file);
        pairs.insert(pairs.begin(), new_pairs.begin(), new_pairs.end());
      }
      file = dir.openNextFile();
    }
    dir.close();
    return pairs;
  }

  IdObjPairs FS::read_raw_pairs(const fURI &match) {
    fs::File root = FOS_FS.open(this->root.toString().c_str());
    return read_raw_pairs_dir(*this, match, root);
  }

  /*void stop() override {
    FOS_FS.end();
    BaseFS::stop();
  }*/
} // namespace fhatos
#endif
