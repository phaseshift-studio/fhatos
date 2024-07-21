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

#ifndef fhatos_abstract_filesystem_hpp
#define fhatos_abstract_filesystem_hpp
#include <process/actor/actor.hpp>
#include FOS_PROCESS(thread.hpp)
#include <language/obj.hpp>
#include <language/types.hpp>
#include FOS_PROCESS(scheduler.hpp)
namespace fhatos {


  class AbstractFileSystem : public Actor<Thread> {
  public:
    using Dir = Rec;
    using File = Rec;
    using File_p = ptr<File>;
    using Dir_p = ptr<Dir>;

    ///////

    explicit AbstractFileSystem(const ID &id) : Actor<Thread>(id) {}
    const Extension ext() {
      return Extension(ID("/ext/fs"), //
                       {"math:"}, // imports
                       {{u("fs:"), u("/ext/fs/")}, // prefixes
                        {u("file:"), u("fs:type/rec/file")}, //
                        {u("dir:"), u("fs:type/rec/dir")}}, //
                       {{u("file:"), *TYPE_PARSER("[name=>uri:," // typedefs
                                                  "size=>nat:,"
                                                  "data=>objs:]")},
                        {u("dir:"), *TYPE_PARSER("[name=>uri:,"
                                                 "size=>nat:,"
                                                 "files=>lst:?file:,"
                                                 "dirs=>lst:?dir:]")}});
    }

    virtual File_p file(const Uri_p &path) const { return nullptr; }
    virtual Dir_p dir(const Uri_p &path) const { return nullptr; }
  };


} // namespace fhatos

#endif
