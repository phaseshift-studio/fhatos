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
#ifdef NATIVE

#include "../fs.hpp"
#include <filesystem>
#include <fstream>
#include "../../../../../lang/mmadt/parser.hpp"
#define FOS_FS NTFS

namespace fs = std::filesystem;

namespace fhatos {
  // using namespace fs;

  void FS::list_files_utility(const char *path) {
    LOG_WRITE(ERROR, Obj::to_noobj().get(), L("FS::list_files_utility not implemented"));
  }

  FS::FS(const Pattern &pattern, const ID_p &value_id, const Rec_p &config) :
      Structure(pattern, id_p(FS_TID), value_id, config),
      root(fURI("data").extend(config->rec_get("root")->uri_value())) {
    string root_path_str = this->root.toString();
    const auto root_path =
        fs::canonical(fs::path(root_path_str[0] == '.' ? root_path_str : root_path_str.insert(0, "./")));
    if(!fs::exists(root_path))
      fs::create_directories(root_path);
    //this->rec_value()->insert_or_assign(vri("config/root"), vri(root_path));
  }

  void FS::setup() {
    Structure::setup();
    LOG_WRITE(INFO, this, L("!b{} !ylocation!! mounted\n", this->root.toString()));
  }

  ptr<FS> FS::create(const Pattern &pattern, const ID_p &value_id, const Rec_p &config) {
    const Obj_p root = config->rec_get("root")->or_else(vri("."));
    string root_path_str = root->uri_value().toString();
    const auto root_path =
        fs::canonical(fs::path(root_path_str[0] == '.' ? root_path_str : root_path_str.insert(0, "./")));
    if(!fs::exists(root_path))
      fs::create_directories(root_path);
    config->rec_value()->insert_or_assign(vri("root"), vri(root_path));
    return Structure::create<FS>(pattern, value_id, config);
  }

  Obj_p FS::load_boot_config(const fURI &boot_config) {
    const fs::path data_extend_path = fs::path(string("data").append(boot_config.toString()));
    try {
      const fs::path boot_path = fs::canonical(data_extend_path);
      LOG_WRITE(INFO, Router::singleton().get(), L("!b{} !yboot loader native location!!\n", boot_path.c_str()));
      if(fs::is_regular_file(boot_path)) {
        auto infile = std::ifstream(boot_path, ios::in);
        if(!infile.is_open())
          throw fError("unable to read from boot config from !b%s!!", boot_path.c_str());
        const auto content = string(std::istreambuf_iterator<char>(infile), std::istreambuf_iterator<char>());
        infile.close();
        const auto proto = make_unique<mmadt::Parser>();
        const Obj_p boot_obj = proto->parse(content.c_str());
        return boot_obj;
      }
    } catch(std::exception &) {
      LOG_WRITE(ERROR, Obj::to_noobj().get(),
                L("!yboot config!! file !rnot found!!: !b!-{}!!\n", data_extend_path.c_str()));
    }
    return Obj::to_noobj();
  }

  void FS::write_raw_pairs(const ID &id, const Obj_p &obj, const bool retain) {
    // TODO: retain is overwrite and transient is append
    const fs::path file_path = map_fos_to_fs(id).toString();
    if(obj->is_noobj()) {
      if(fs::is_regular_file(file_path))
        fs::remove(file_path);
      // this->distribute_to_subscribers(Message::create(id_p(id), obj, retain));
      return;
    }
    if(id.is_node()) {
      if(const fs::path parent_path = file_path.parent_path(); !fs::exists(parent_path))
        fs::create_directories(parent_path);
      const BObj_p bobj = obj->serialize();
      auto outfile = std::ofstream(file_path, retain ? ios::trunc : ios::app);
      if(!outfile.is_open())
        LOG_WRITE(WARN, this, L("unable to write to !b%s!! via !b%s!!\n", id.toString().c_str(), file_path.c_str()));
      outfile << bobj->second;
      outfile.flush();
      outfile.close();
    } else {
      throw fError("unimplemented dir writer\n");
    }
    // this->distribute_to_subscribers(Message::create(id_p(id), obj, retain));
  }


  void read_raw_pairs_dir(const FS &fs, const fURI &match, const fs::path &fs_path, IdObjPairs *pairs) {
    const ID fos_path = fs.map_fs_to_fos(fs_path);
    // LOG(INFO, "matching %s with %s via %s\n", match->toString().c_str(), fos_path.toString().c_str(),
    // fs_path.c_str());
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
        read_raw_pairs_dir(fs, match, p, pairs);
      }
    }
  }

  IdObjPairs FS::read_raw_pairs(const fURI &match) {
    auto pairs = IdObjPairs();
    // LOG(INFO, "trying to read %s starting at %s\n",
    // match->toString().c_str(),fs::path(this->root.toString()).c_str());
    read_raw_pairs_dir(*this, match, fs::path(this->root.toString()), &pairs);
    return pairs;
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

} // namespace fhatos
#endif
