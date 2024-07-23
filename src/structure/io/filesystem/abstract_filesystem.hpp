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
#include FOS_PROCESS(coroutine.hpp)
#include <language/obj.hpp>
#include <language/types.hpp>
namespace fhatos {

  using File = Uri;
  using File_p = Uri_p;
  using Dir = Uri;
  using Dir_p = Uri_p;

  static const fURI_p FILE_FURI = fURI_p(new fURI("/uri/file"));
  static const fURI_p DIR_FURI = fURI_p(new fURI("/uri/dir"));

  class AbstractFileSystem : public Actor<Coroutine> {
  public:
    const ID_p _root;
    explicit AbstractFileSystem(const ID &id, const ID &localRoot) : Actor<Coroutine>(id), _root(share(localRoot)) {}

    void setup() override {
      Actor::setup();
      LOG_TASK(INFO, this, "!b%s!! !ydirectory!! mounted\n", this->_root->toString().c_str());
      TYPE_WRITER(*FILE_FURI, Obj::to_bcode({}));
      TYPE_WRITER(*DIR_FURI, Obj::to_bcode({}));
      // TYPE_WRITER("/inst/root",
      //             Obj::to_inst("root", {}, [this](const Obj_p &lhs) { return this->root(); }, IType::ONE_TO_ONE));
      this->subscribe("#", [this](const Message_p &message) {
        if (message->retain) {
          const ID file = makeRouterPath(message->target);
          const File_p f = this->exists(file) ? this->to_file(file) : this->touch(file);
          this->append(f, message->payload);
        }
      });
    }

    virtual File_p to_file(const ID &path) const { throw fError("must be implemented"); }
    virtual Dir_p to_dir(const ID &path) const { throw fError("must be implemented"); }
    virtual ID makeLocalPath(const ID &path) const { throw fError("must be implemented"); }
    virtual ID makeFilePath(const ID &path) const {
      return ID(path.toString().substr(this->id()->toString().length() + 1));
    }
    virtual ID makeRouterPath(const ID &path) const { return ID(this->id()->toString() + "/" + path.toString()); }
    ////
    virtual Dir_p root() const { throw fError("must be implemented"); }
    virtual bool exists(const ID &path) const { throw fError("must be implemented"); }
    virtual Dir_p mkdir(const ID &path) const { throw fError("must be implemented"); }
    virtual Objs_p ls(const Dir_p &dir, const Pattern &pattern) const { throw fError("must be implemented"); }
    virtual Objs_p more(const File_p &file) const { throw fError("must be implemented"); }
    virtual void append(const File_p &file, const Obj_p &content) { throw fError("must be implemented"); }
    virtual File_p touch(const ID &path) const { throw fError("must be implemented"); }
  };
} // namespace fhatos

#endif
