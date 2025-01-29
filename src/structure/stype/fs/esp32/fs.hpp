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

#ifndef NATIVE

#include <FS.h>
#include <LittleFS.h>
#include "../base_fs.hpp"

#define FOS_FS LittleFS

using namespace fs;
namespace fhatos {
  class FSx : public BaseFS {
  protected:
    explicit FSx(const Pattern &pattern,const Rec_p& config) : BaseFS(pattern, config) {

    }

  public:
    static ptr<FSx> create(const Pattern &pattern = Pattern("/io/fs/#"), const Rec_p& config = Obj::to_rec({{"root",vri("/")}})) {
      static ptr<FSx> fs = ptr<FSx>(new FSx(pattern, config));
      return fs;
    }

    virtual void setup() override {
      if (!FOS_FS.begin()) {
        throw fError("unable to mount file system at %s", this->root.toString().c_str());
        return;
      }
      // TODO: add fs statistics 
      BaseFS::setup();
    }

    void write_raw_pairs(const ID_p &id, const Obj_p &obj, const bool retain) override {
     // TODO: retain is overwrite and transient is append
     const char* file_name = map_fos_to_fs(id).toString().c_str();
    if(obj->is_noobj()) {
      if(FOS_FS.exists(file_name))
        FOS_FS.remove(file_name);
      return;
    } 
     const char* dir_name = map_fos_to_fs(id_p(id->retract())).toString().c_str();
      if (!FOS_FS.exists(dir_name))
        FOS_FS.mkdir(dir_name);
      fs::File file = FOS_FS.open(file_name,retain ? "w" : "a", true);
      const BObj_p bobj = obj->serialize();
      for(unsigned int i = 0; i < bobj->first; i++) {
        file.write(bobj->second[i]);
      }
      file.flush();
      file.close();
    }

    IdObjPairs read_raw_pairs_dir(const fURI_p &match, fs::File& dir) {
      IdObjPairs pairs =  List<Pair<ID_p, Obj_p>>();
      fs::File file = dir.openNextFile();
      while (file) {
       if(!file.isDirectory()) {
        const ID_p path = id_p(map_fs_to_fos(file.path()));
        if(path->matches(*match)) {
          const String contents =  file.readString();
          file.close();
          const BObj_p bobj = make_shared<BObj>(contents.length(), (fbyte *) contents.c_str());
          //LOG_OBJ(INFO,this,"reading %s: %s\n",file.path(),contents.c_str());
          pairs.emplace_back(std::make_pair<ID_p,Obj_p>(id_p(*path),Obj::deserialize(bobj)));
        }
       } else {
          IdObjPairs new_pairs = this->read_raw_pairs_dir(match,file);
          pairs.insert(pairs.begin(),new_pairs.begin(),new_pairs.end());
        } 
        file = dir.openNextFile();
      }
      dir.close();
      return pairs;
    }

    IdObjPairs read_raw_pairs(const fURI_p &match) {
      fs::File root = FOS_FS.open(this->root.toString().c_str());
      return read_raw_pairs_dir(match, root);
    }

    void stop() override {
        FOS_FS.end();
        Structure::stop();
    }
  };


}

#endif
#endif