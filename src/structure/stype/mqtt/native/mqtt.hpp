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

#ifdef NATIVE

#include "fhatos.hpp"
#include <mqtt/async_client.h>
#include "structure/structure.hpp"

#ifndef FOS_MQTT_BROKER_ADDR
#define FOS_MQTT_BROKER_ADDR "localhost:1883"
#endif
#define FOS_MQTT_MAX_RETRIES 10
#define FOS_MQTT_RETRY_WAIT 5000

namespace fhatos {
  using namespace mqtt;

  class Mqtt : public Structure {
  protected:
    Message_p will_message;
    const char *server_addr;
    async_client* xmqtt;


    //                                     +[scheme]//+[authority]/#[path]
    explicit Mqtt(const Pattern &pattern = Pattern("//+/#"), const char *server_addr = FOS_MQTT_BROKER_ADDR,
                  const Message_p &will_message = ptr<Message>(nullptr)) : Structure(pattern, SType::READWRITE) {
      this->remote_retains = true;
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
        const Message_p message = share(Message{
                .source = *source,
                .target = ID(mqtt_message->get_topic()),
                .payload = payload,
                .retain = mqtt_message->is_retained()
        });
        LOG_STRUCTURE(TRACE, this, "mqtt broker providing message %s\n", message->toString().c_str());
        mutex.read<RESPONSE_CODE>([this, message]() {
          RESPONSE_CODE rc2 = NO_SUBSCRIPTION;
          for (const auto &subscription: *this->subscriptions) {
            if (message->target.matches(subscription->pattern)) {
              rc2 = OK;
              Subscription_p sub = share(Subscription(*subscription));
              this->outbox_->push_back(share(Mail{sub, message}));
            }
          }
          return rc2;
        });
        MESSAGE_INTERCEPT(message->source, message->target, message->payload, message->retain);
      });
      /// MQTT CONNECTION ESTABLISHED CALLBACK
      this->xmqtt->set_connected_handler([this](const string &) {
        LOG_STRUCTURE(INFO, this,
                      "\n" FOS_TAB_4 "!ybroker address!!: !b%s!!\n" FOS_TAB_4 "!yclient name!!   : !b%s!!\n"
                              FOS_TAB_4
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
        int counter = 0;
        const connect_options finalized = connection_options.finalize();
        while (counter < FOS_MQTT_MAX_RETRIES) {
          if (!this->xmqtt->connect(finalized)->wait_for(1000)) {
            if (++counter > FOS_MQTT_MAX_RETRIES)
              throw mqtt::exception(1);
            LOG_STRUCTURE(WARN, this, "!bmqtt://%s !yconnection!! retry\n", this->server_addr);
            sleep(FOS_MQTT_RETRY_WAIT / 1000);
          }
          if (this->xmqtt->is_connected())
            break;
        }
      } catch (const mqtt::exception &e) {
        LOG_STRUCTURE(ERROR, this, "Unable to connect to !b%s!!: %s\n", this->server_addr, e.what());
      }
    }

  public:
    ~Mqtt() override {
      delete this->xmqtt;
    }

    void stop() override {
      LOG_STRUCTURE(INFO, this, "Disconnecting from mqtt broker !g[!y%s!g]!!\n", this->server_addr);
      this->xmqtt->disconnect();
      //if (!this->xmqtt->is_connected())
      //  LOG_STRUCTURE(ERROR, this, "Unable to disconnect from !b%s!!\n", this->server_addr);
    }

    static ptr<Mqtt> create(const Pattern &pattern, const char *server_addr = FOS_MQTT_BROKER_ADDR,
                            const Message_p &will_message = ptr<Message>(nullptr)) {
      ptr<Mqtt> mqtt_p = ptr<Mqtt>(new Mqtt(pattern, server_addr, will_message));
      return mqtt_p;
    }

    void recv_message(const Message_p &message) override {
      LOG_STRUCTURE(DEBUG, this, "!yreceived!! %s\n", message->toString().c_str());
      this->write(id_p(message->target), message->payload, id_p(message->source));
      RESPONSE_CODE rc = OK;
      LOG_PUBLISH(rc, *message);
    }

    void recv_subscription(const Subscription_p &subscription) override {
      this->mutex.read<void *>([this, subscription]() {
        bool found = false;
        for (const auto &sub: *this->subscriptions) {
          if (subscription->pattern.equals(sub->pattern)) {
            found = true;
            break;
          }
        }
        if (!found) {
          LOG_STRUCTURE(DEBUG, this, "Subscribing as no existing subscription found: %s\n",
                        subscription->toString().c_str());
          this->xmqtt->subscribe(subscription->pattern.toString(), (int) subscription->qos);
        }
        return nullptr;
      });
      Structure::recv_subscription(subscription);
    }

    void recv_unsubscribe(const ID_p &source, const fURI_p &target) override {
      Structure::recv_unsubscribe(source, target);
      this->mutex.read<void *>([this, target]() {
        bool found = false;
        for (const auto &sub: *this->subscriptions) {
          if (target->equals(sub->pattern)) {
            found = true;
            break;
          }
        }
        if (!found) {
          LOG_STRUCTURE(DEBUG, this, "Unsubscribing as no existing subscription pattern found: %s\n",
                        target->toString().c_str());
          this->xmqtt->unsubscribe(target->toString());
        }
        return nullptr;
      });
    }

    Objs_p read(const fURI_p &furi, const ID_p &source) override {
      if (furi->is_pattern()) {
        return Obj::to_objs(); // TODO
      } else {
        auto *thing = new std::atomic<const Obj *>(nullptr);
        this->recv_subscription(share(Subscription{
                .source = ID(*source), .pattern = *furi, .onRecv = [this, furi, thing](const Message_p &message) {
                  // TODO: try to not copy obj while still not accessing heap after delete
                  LOG_STRUCTURE(TRACE, this, "subscription pattern %s matched: %s\n", furi->toString().c_str(),
                                message->toString().c_str());
                  const Obj *obj = new Obj(Any(message->payload->_value), id_p(*message->payload->id()));
                  thing->store(obj);
                }
        }));
        const time_t startTimestamp = time(nullptr);
        while (!thing->load()) {
          if ((time(nullptr) - startTimestamp) > 2) {
            break;
          }
        }
        this->recv_unsubscribe(source, furi);
        if (nullptr == thing->load()) {
          delete thing;
          return Obj::to_noobj();
        } else {
          const Obj_p ret = ptr<Obj>((Obj *) thing->load());
          delete thing;
          return ret;
        }
      }
    }

    void write(const ID_p &target, const Obj_p &obj, const ID_p &source) override {
      BObj_p source_payload = Message::wrapSource(source, obj);
      LOG_STRUCTURE(TRACE, this, "writing to xmpp broker: %s\n", source_payload->second);
      this->xmqtt->publish(target->toString(), source_payload->second, source_payload->first, 1 /*qos*/,
                           RETAIN_MESSAGE);
    }
  };
} // namespace fhatos
#endif
#endif
