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

#ifndef fhatos_resource_hpp
#define fhatos_resource_hpp

#include <fhatos.hpp>
#include <FS.h>
#include <bits/fs_fwd.h>
#include <util/ansi.hpp>
#include <structure/query/query.hpp>
#include <util/pretty.hpp>

namespace fhatos {
  enum FSType { FILE, DIR };

  const static Map<FSType, string> D_TYPE_STR = {{FILE, "file"}, {DIR, "dir"}};

  struct FSInfo {
    const fURI furi;
    const size_t size;
    const FSType type;

    union FileOrDir {
      const Option<string> f;
      const Option<List<FSInfo *> > d;

      ~FileOrDir() {
        /* if (d.has_value()) {
           List<FSInfo *> list = d.value();
           list.remove_if([](FSInfo *x) {
             delete x;
             return true;
           });
         }*/
      };
    } contents;

    FSInfo(const fURI &furi, const size_t size,
           const Option<List<FSInfo *> > &contents) : furi(furi), size(size), type(DIR), contents{.d = contents} {
    }

    FSInfo(const fURI &furi, const size_t size, const Option<string> &contents) : furi(furi), size(size), type(FILE),
      contents{.f = contents} {
    }

    template<typename PRINTER>
    void print(Ansi<PRINTER> &ansi) const {
      if (this->type == FILE) {
        ansi.printf(FOS_TAB_2 "!b\\__file!!:!r%s!![!gsize:" FOS_BYTES_MB_STR "!!]\n", this->furi.path().c_str(),
                    FOS_BYTES_MB(this->size));
        if (this->contents.f.has_value()) {
          ansi.printf("%s\n", this->contents.f.value().c_str());
        }
      } else {
        ansi.printf(FOS_TAB_2 "!b\\_dir!!:!r%s!![!gsize:%i!!]\n", this->furi.path().c_str(), this->size);
        if (this->contents.d.has_value()) {
          for (const FSInfo *info: contents.d.value()) {
            if (FILE == info->type) {
              ansi.printf(FOS_TAB_2 FOS_TAB_2 "!b\\_file!!:!r%s!![!gsize:" FOS_BYTES_MB_STR "!!]\n",
                          info->furi.lastSegment().c_str(),
                          FOS_BYTES_MB(info->size));
            } else {
              ansi.printf(FOS_TAB_2 FOS_TAB_2 "!b\\_dir!!:!r%s!![!gsize:%i!!]\n",
                          info->furi.lastSegment().c_str(),
                          info->size);
            }
          }
        }
      }
    }
  };

  class qFS : public Query {
  public:
    FS &fs;

    qFS(const fURI &furi, FS &fs) : Query(furi), fs(fs) {
    }

    FSInfo *structure(const bool contents = false) const {
      LOG(INFO, "Fetching file system information on %s\n", furi.toString().c_str());
      File root = fs.open(("/" + furi.path()).c_str());
      if (root.isDirectory()) {
        uint16_t counter = 0;
        List<FSInfo *> body;
        File next = root.openNextFile();
        while (next) {
          counter++;
          if (contents)
            body.push_back(qFS(furi.extend(next.name()), fs).structure(false));
          next = root.openNextFile();
        }
        FSInfo *result = new FSInfo{
          .furi = furi,
          .size = counter,
          .contents{.d = contents ? Option<List<FSInfo *> >(body) : Option<List<FSInfo *> >()}
        };
        root.close();
        return result;
      } else {
        FSInfo *result = new FSInfo{
          .furi = furi,
          .size = root.size(),
          .contents = {.f = (contents ? Option<string>(string(root.readString().c_str())) : Option<string>())}
        };
        root.close();
        return result;
      }
    }
  };
}
#endif
