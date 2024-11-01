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
#ifndef fhatos_sys_hpp
#define fhatos_sys_hpp

#include <fhatos.hpp>
#include <furi.hpp>
#include <structure/stype/computed.hpp>
#include <util/string_helper.hpp>

namespace fhatos {
  class Sys : public Computed {
    explicit Sys(const Pattern &pattern = "/sys/#") :
      Computed(pattern) {
      ////////////////////////////////////////////////////////////////////////
      /////////////////////////////// SCHEDULER //////////////////////////////
      ////////////////////////////////////////////////////////////////////////
      this->read_functions_->insert(
      {furi_p(pattern.resolve("./scheduler")), [this](const fURI_p &furi) {
        return make_id_objs({{id_p(this->pattern()->resolve("./scheduler")), Obj::to_rec({{vri(":spawm"),
                                Insts::to_bcode([](const Uri_p &uri) {
                                  return uri;
                                })}})}});
      }});
      this->read_functions_->insert(
      {furi_p(pattern.resolve("./scheduler/process/#")), [this](const fURI_p &furi) {
        IdObjPairs_p pairs = make_id_objs();
        auto c = new int(0);
        scheduler()->processes_->forEach([this,pairs,c,furi](const Process_p &process) {
          const ID_p pid = id_p(this->pattern()->resolve(string("./scheduler/process/") + to_string((*c)++)));
          if (pid->bimatches(*furi))
            pairs->push_back({pid, vri(process->id())});
        });
        delete c;
        return pairs;
      }});
      ///////////// SCHEDULER BARRIER /////////////
      this->read_functions_->insert(
      {furi_p(pattern.resolve("./scheduler/barrier/#")), [this](const fURI_p &) {
        return make_id_objs({scheduler()->barrier_});
      }});
      this->write_functions_->insert(
      {furi_p(pattern.resolve("./scheduler/barrier/#")), [this](const fURI_p &furi, const Obj_p &obj) -> IdObjPairs {
        if (obj->is_noobj() && scheduler()->barrier_.first->bimatches(*furi))
          scheduler()->recv_mail(mail_p(
              subscription_p(*scheduler()->id(), *furi, Insts::to_bcode([](const Obj_p &obj) {
                scheduler()->stop();
                printer<Ansi<>>()->flush();
                return noobj();
              })),
              message_p(*scheduler()->id(), noobj(), true)));
        return *make_id_objs();
      }});
      ////////////////////////////////////////////////////////////////////////
      //////////////////////////////// ROUTER ////////////////////////////////
      ////////////////////////////////////////////////////////////////////////
      this->read_functions_->insert(
      {furi_p(pattern.resolve("./router")), [this](const fURI_p &) {
        return make_id_objs({{id_p(this->pattern()->resolve("./router")), Obj::to_rec({{vri(":mount"),
                                Insts::to_bcode([](const Uri_p &uri) {
                                  return uri;
                                })}})}});
      }});
      this->read_functions_->insert(
      {furi_p(pattern.resolve("./router/structure/#")), [this](const fURI_p &furi) {
        IdObjPairs_p pairs = make_id_objs();
        int counter = 0;
        int *c = &counter;
        router()->structures_.forEach([this,pairs,c,furi](const Structure_p &structure) {
          const ID_p pid = id_p(this->pattern()->resolve(string("./router/structure/") + to_string((*c)++)));
          if (pid->bimatches(*furi))
            pairs->push_back({pid, vri(structure->pattern())});
        });
        return pairs;
      }});
      ////////////////////////////////////////////////////////////////////////
      /////////////////////////////// HARDWARE ///////////////////////////////
      ////////////////////////////////////////////////////////////////////////
#ifdef NATIVE
      this->read_functions_->insert(
      {furi_p(pattern.resolve("./hardware/#")), [pattern](const fURI_p &) {
         std::ifstream cpuInfo("/proc/cpuinfo");
         std::string line;
         const IdObjPairs_p result = make_id_objs();
         int proc = -1;
         int cores = -1;
         string cpu;
         while (std::getline(cpuInfo, line)) {
           const std::string value = StringHelper::substring(':', line);
           StringHelper::trim(value);
           if (line.find("processor") != std::string::npos) {
             if (proc != -1) {
               const Rec_p r = rec();
               if (cores != -1)
                 r->rec_set(vri(":cores"), jnt(cores));
               if (!cpu.empty())
                 r->rec_set(vri(":cpu"), str(cpu));
               result->push_back({id_p(pattern.resolve("./hardware/processor/" + to_string(proc))), r});
             }
             proc = stoi(value);
             cores = -1;
             cpu.clear();
           } else if (line.find("model name") != std::string::npos) {
             cpu = value;
           } else if (line.find("cpu cores") != std::string::npos) {
             cores = stoi(value);
           }
         }
         cpuInfo.close();
         ///////////////////////////////
         std::ifstream version("/proc/version");
         string total = "";
         while (std::getline(version, line)) {
           total = total + line;
         }
         version.close();
         StringHelper::trim(total);
         result->push_back({id_p(pattern.resolve("./hardware/version")), str(total)});
         return result;
       }
      });
#endif
    }

  public:
    static ptr<Sys> singleton(const Pattern &pattern) {
      static auto sysp = ptr<Sys>(new Sys(pattern));
      return sysp;
    }
  };
}

#endif