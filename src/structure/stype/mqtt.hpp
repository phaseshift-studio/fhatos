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
#ifndef fhatos_mqtt_hpp
#define fhatos_mqtt_hpp

#include <fhatos.hpp>
#include <mqtt/async_client.h>
#include <structure/structure.hpp>
#ifndef FOS_MQTT_BROKER_ADDR
#define FOS_MQTT_BROKER_ADDR "localhost:1883"
#endif
#define FOS_MQTT_MAX_RETRIES 10
#define FOS_MQTT_RETRY_WAIT 5000

namespace fhatos {
  class Mqtt : public Structure {
  protected:
    Message_p will_message;
    const char *server_addr;
    async_client *xmqtt;

  public:
    //                                     +[scheme]//+[authority]/#[path]
    explicit Mqtt(const Pattern &pattern = Pattern("//+/#"), const char *server_addr = MQTT_BROKER_ADDR,
                  const Message_p &will_message = ptr<Message>(nullptr)) : Structure(pattern, SType::READWRITE) {

      this->server_addr = string(server_addr).find_first_of("mqtt://") == string::npos
                              ? string("mqtt://").append(string(server_addr)).c_str()
                              : server_addr;
      this->xmqtt = new async_client(this->server_addr, "", mqtt::create_options(MQTTVERSION_5));
      this->will_message = will_message;
      srand(time(nullptr));
      auto connection_options = connect_options_builder()
                                    .properties({{property::SESSION_EXPIRY_INTERVAL, 604800}})
                                    .clean_start(true)
                                    .clean_session(true)
                                    .user_name(string("client_" + to_string(rand())))
                                    .keep_alive_interval(std::chrono::seconds(20))
                                    .automatic_reconnect();
      if (will_message.get()) {
        const BObj_p source_payload = Message::wrapSource(id_p(will_message->source), will_message->payload);
        connection_options.will(
            message(this->will_message->target.toString(), source_payload->second, this->will_message->retain));
      }
      //// MQTT MESSAGE CALLBACK
      this->xmqtt->set_message_callback([this](const const_message_ptr &mqtt_message) {
        const binary_ref ref = mqtt_message->get_payload_ref();
        const BObj_p bobj = share(BObj(ref.length(), (fbyte *) ref.data()));
        const auto &[source, payload] = Message::unwrapSource(bobj);
        LOG(TRACE, "Incoming MQTT message from %s to %s\n", source->toString().c_str(),
            mqtt_message->get_topic().c_str());
        const Message_p message = share(Message{.source = *source,
                                                .target = ID(mqtt_message->get_topic()),
                                                .payload = payload,
                                                .retain = mqtt_message->is_retained()});
        this->recv_message(message);
      });
      /// MQTT CONNECTION ESTABLISHED CALLBACK
      this->xmqtt->set_connected_handler([this](const string &) {
        LOG_STRUCTURE(INFO, this,
                      "\n" FOS_TAB_4 "!ybroker address!!: !b%s!!\n" FOS_TAB_4 "!yclient name!!   : !b%s!!\n" FOS_TAB_4
                      "!ywill topic!!    : !m%s!!\n" FOS_TAB_4 "!ywill message!!  : !m%s!!\n" FOS_TAB_4
                      "!ywill qos!!      : !m%s!!\n" FOS_TAB_4 "!ywill retain!!   : !m%s!!\n",
                      this->server_addr, this->xmqtt->get_client_id().c_str(),
                      this->will_message.get() ? this->will_message->target.toString().c_str() : "<none>",
                      this->will_message.get() ? this->will_message->payload->toString().c_str() : "<none>",
                      this->will_message.get() ? "1" : "<none>",
                      this->will_message.get() ? FOS_BOOL_STR(this->will_message->retain) : "<none>");
      });
      /// MQTT CONNECTION
      try {
        this->xmqtt->connect(connection_options.finalize());
        int counter = 0;
        while (!this->xmqtt->is_connected()) {
          if (counter++ > FOS_MQTT_MAX_RETRIES)
            throw mqtt::exception(1);
          sleep(FOS_MQTT_RETRY_WAIT / 1000);
          LOG_STRUCTURE(WARN, this, "!bmqtt://%s !yconnection!! retry\n", this->server_addr);
        }
      } catch (const mqtt::exception &e) {
        LOG_STRUCTURE(ERROR, this, "Unable to connect to !b%s!!: %s\n", this->server_addr, e.what());
      }
    }
    Objs_p read(const fURI_p &furi, const ID_p &source) override { return fhatos::Objs_p(); }
    void write(const ID_p &target, const Obj_p &obj, const ID_p &source) override {
      BObj_p source_payload = Message::wrapSource(source, obj);
      this->xmqtt->publish(target->toString(), source_payload->second, source_payload->first, 1 /*qos*/,
                           RETAIN_MESSAGE);
    }
    Obj_p read(const ID_p &id, const ID_p &source) override {
      auto *thing = new std::atomic<const Obj *>(nullptr);
      this->recv_subscription(
          share(Subscription{.source = ID(*source), .pattern = ID(*id), .onRecv = [thing](const Message_p &message) {
                               // TODO: try to not copy obj while still not accessing heap after delete
                               const Obj *obj = new Obj(Any(message->payload->_value), id_p(*message->payload->id()));
                               thing->store(obj);
                             }}));
      const time_t startTimestamp = time(nullptr);
      while (!thing->load()) {
        if ((time(nullptr) - startTimestamp) > 2) {
          break;
        }
      }
      this->recv_unsubscribe(source, id);
      if (nullptr == thing->load()) {
        delete thing;
        return Obj::to_noobj();
      } else {
        const Obj_p ret = ptr<Obj>((Obj *) thing->load());
        delete thing;
        return ret;
      }
    }
  };
} // namespace fhatos

#endif
