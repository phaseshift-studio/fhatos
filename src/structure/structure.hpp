//  FhatOS: A Distributed Operating System
//  Copyright (c) 2024 PhaseShift Studio, LLC
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Affero General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Affero General Public License for more details.
//
//  You should have received a copy of the GNU Affero General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#pragma once
#ifndef fhatos_structure_hpp
#define fhatos_structure_hpp

#include <fhatos.hpp>
#include <language/obj.hpp>
#include <util/enums.hpp>

namespace fhatos {
  enum class SType { READ, WRITE, READWRITE };
  static const Enums<SType> StructureTypes =
      Enums<SType>({{SType::READ, "read"}, {SType::WRITE, "write"}, {SType::READWRITE, "readwrite"}});

  class Structure : public Patterned {
  protected:
    std::atomic_bool _available = std::atomic_bool(false);

  public:
    const SType _stype;

    explicit Structure(const Pattern_p &type, const SType stype) : Patterned(type), _stype(stype) {}

    virtual ~Structure() = default;

    virtual void open() {
      if (this->_available.load()) {
        LOG_STRUCTURE(WARN, this, "!ystructure!! already open");
        this->_available.store(true);
      }
    }
    virtual void maintain() {}
    virtual void close() {
      if (this->_available.load()) {
        LOG_STRUCTURE(WARN, this, "!ystructure!! already closed");
        this->_available.store(false);
      }
    }
    virtual void publish(const Message_p &message) = 0;
    virtual void subscribe(const Subscription_p &subscription) = 0;
    virtual void unsubscribe(const ID &source, const Pattern pattern) {
      this->subscribe(share(Subscription{.source = source, .pattern = pattern, .onRecv = nullptr}));
    }
    virtual Obj_p read(const ID &id, const ID &source = FOS_DEFAULT_SOURCE_ID) {
      auto *thing = new std::atomic<const Obj *>(nullptr);
      this->subscribe(share(Subscription{.source = source, .pattern = id, .onRecv = [thing](const Message_p &message) {
                                           // TODO: try to not copy obj while still not accessing heap after delete
                                           const Obj *obj = new Obj(message->payload->_value, message->payload->id());
                                           thing->store(obj);
                                         }}));
      const time_t startTimestamp = time(nullptr);
      while (!thing->load()) {
        if ((time(nullptr) - startTimestamp) > 2) {
          break;
        }
      }
      this->subscribe(share(Subscription{.source = FOS_DEFAULT_SOURCE_ID, .pattern = id, .onRecv = nullptr}));
      if (nullptr == thing->load()) {
        delete thing;
        return Obj::to_noobj();
      } else {
        const Obj_p ret = ptr<Obj>((Obj *) thing->load());
        delete thing;
        return ret;
      }
    }
    Obj_p read(const ID &id) { return this->read(id, FOS_DEFAULT_SOURCE_ID); }

    virtual void write(const fURI &furi, const Obj_p &obj, const ID &source) {
      this->publish(share(Message{.source = source, .target = furi, .payload = obj, .retain = true}));
    }
    void write(const fURI &furi, const Obj_p &obj) { this->write(furi, obj, FOS_DEFAULT_SOURCE_ID); }

    bool available() { return this->_available; }

    static fError ID_NOT_IN_RANGE(const ID &id, const Pattern &pattern) {
      return fError("!g[!b%s!g]!! is not within the boundaries of space !g[!!%s!g]!!\n", id.toString().c_str(),
                    pattern.toString().c_str());
    }

    static fError ID_DOES_NOT_EXIST(const ID &id, const Pattern &pattern) {
      return fError("!g[!b%s!g]!! does not reference an obj in space !g[!!%s!g]!!\n", id.toString().c_str(),
                    pattern.toString().c_str());
    }
  };

} // namespace fhatos

#endif
