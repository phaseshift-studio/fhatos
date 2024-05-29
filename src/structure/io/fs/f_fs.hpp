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

#ifndef fhatosf_f_fs_hpp
#define fhatosf_f_fs_hpp

#include <fhatos.hpp>
//
#include <esp_flash_partitions.h>
#include <esp_littlefs.h>
#include <FS.h>
#include <LittleFS.h>
#include <process/actor/actor.hpp>
#include <util/ansi.hpp>
#include <structure/query/q_fs.hpp>
#include FOS_PROCESS(thread.hpp)

namespace fhatos {
  /////////////////////////////////////////////////////////////////////
#ifndef FOS_DEFAULT_FILE_SYSTEM
#define FOS_DEFAULT_FILE_SYSTEM LittleFS
#endif
  /////////////////////////////////////////////////////////////////////

  template<typename PROCESS = Fiber, typename ROUTER = FOS_DEFAULT_ROUTER >
  class fFS final : public Actor<PROCESS, ROUTER> {
  public:
    static fFS *singleton() {
      static fFS singleton = fFS();
      return &singleton;
    }

    explicit fFS(const ID &id = FOS_DEFAULT_ROUTER::mintID("kernel", "fs"))
      : Actor<PROCESS, ROUTER>(id) {
    }


    virtual void stop() override {
      FOS_DEFAULT_FILE_SYSTEM.end();
      Actor<PROCESS, ROUTER>::stop();
    }

    virtual void setup() override {
      PROCESS::setup();
      const char *mount = "/littlefs";
      const bool success = FOS_DEFAULT_FILE_SYSTEM.begin(false, mount);
      LOG_TASK(success ? INFO : ERROR, this, "%s mounted at %s", STR(FOS_DEFAULT_FILE_SYSTEM), mount);
      const BiConsumer<SourceID, TargetID> handler = [this](const SourceID &, const TargetID &target) {
        const FSInfo *info = qFS(target.query(""), FOS_DEFAULT_FILE_SYSTEM).structure(true);
        string temp;
        StringPrinter printer(&temp);
        Ansi<StringPrinter> ansi(&printer);
        info->print(ansi);
        this->publish(target, string(temp.c_str()),RETAIN_MESSAGE);
        ansi.flush();
        delete info;
      };
      this->onQuery(this->id().query("?"), handler);
      this->onQuery(this->id().extend("partitions").query("?"),
                    [this](const SourceID &source, const TargetID &target) {
                      string temp;
                      StringPrinter printer(&temp);

                      Ansi<StringPrinter> ansi(&printer);
                      esp_partition_iterator_t itty = esp_partition_find(
                        ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY,NULL);
                      for (; itty != NULL; itty = esp_partition_next(itty)) {
                        const esp_partition_t *part = esp_partition_get(itty);
                        ansi.printf(
                          FOS_TAB_2 "!b\\_%s!!:!r%s!![!gused:" FOS_BYTES_MB_STR"!!][!gaddress:0x%" PRIx32
                          "!!][!gencrypted:%s!!]\n",
                          part->type == ESP_PARTITION_TYPE_APP ? "inst" : "data",
                          part->label,
                          FOS_BYTES_MB(part->size), part->address, FOS_BOOL_STR(part->encrypted));
                      }
                      esp_partition_iterator_release(itty);
                      this->publish(target, string(temp.c_str()),RETAIN_MESSAGE);
                      ansi.flush();
                    });
      this->onQuery(this->id().extend("files").extend("#"), {{"?", handler}});
    }
  };
} // namespace fhatos

#endif
