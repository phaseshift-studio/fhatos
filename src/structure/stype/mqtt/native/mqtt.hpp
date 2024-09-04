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

#include <fhatos.hpp>
#include <mqtt/async_client.h>
#include <structure/structure.hpp>

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
    async_client *xmqtt;
    connect_options connection_options;

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
      connect_options_builder pre_connection_options = connect_options_builder()
          .properties({{property::SESSION_EXPIRY_INTERVAL, 604800}})
          .clean_start(true)
          .clean_session(true)
          .user_name(string("client_" + to_string(rand())))
          .keep_alive_interval(std::chrono::seconds(20))
          .automatic_reconnect();
      if (will_message.get()) {
        const BObj_p source_payload = Message::wrapSource(id_p(will_message->source), will_message->payload);
        pre_connection_options = pre_connection_options.will(
          message(this->will_message->target.toString(), source_payload->second, this->will_message->retain));
      }
      this->connection_options = pre_connection_options.finalize();
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
        const List_p<Subscription_p> matches = this->get_matching_subscriptions(furi_p(message->target));
        for (const Subscription_p &sub: *matches) {
          this->outbox_->push_back(share(Mail{sub, message}));
        }
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
    }

  public:
    static ptr<Mqtt> create(const Pattern &pattern, const char *server_addr = FOS_MQTT_BROKER_ADDR,
                            const Message_p &will_message = ptr<Message>(nullptr)) {
      const auto mqtt_p = ptr<Mqtt>(new Mqtt(pattern, server_addr, will_message));
      return mqtt_p;
    }

    ~Mqtt() override {
      delete this->xmqtt;
    }

    void stop() override {
      LOG_STRUCTURE(INFO, this, "Disconnecting from mqtt broker !g[!y%s!g]!!\n", this->server_addr);
      this->xmqtt->disconnect();
      Structure::stop();
      //if (!this->xmqtt->is_connected())
      //  LOG_STRUCTURE(ERROR, this, "Unable to disconnect from !b%s!!\n", this->server_addr);
    }

    void setup() override {
      Structure::setup();
      try {
        int counter = 0;
        while (counter < FOS_MQTT_MAX_RETRIES) {
          if (!this->xmqtt->connect(this->connection_options)->wait_for(1000)) {
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

    RESPONSE_CODE recv_message(const Message_p &message) override {
      LOG_STRUCTURE(DEBUG, this, "!yreceived!! %s\n", message->toString().c_str());
      this->write(id_p(message->target), message->payload, id_p(message->source), message->retain);
      RESPONSE_CODE rc = OK;
      LOG_PUBLISH(rc, *message);
      return rc;
    }

    void recv_subscription(const Subscription_p &subscription) override {
      const bool mqtt_sub = !this->has_equal_subscription_pattern(furi_p(subscription->pattern));
      Structure::recv_subscription(subscription);
      if (mqtt_sub) {
        LOG_STRUCTURE(DEBUG, this, "Subscribing as no existing subscription found: %s\n",
                      subscription->toString().c_str());
        this->xmqtt->subscribe(subscription->pattern.toString(), static_cast<int>(subscription->qos))->wait();
      }
    }

    void recv_unsubscribe(const ID_p &source, const fURI_p &target) override {
      const bool mqtt_sub = this->has_equal_subscription_pattern(target);
      Structure::recv_unsubscribe(source, target);
      if (mqtt_sub && !this->has_equal_subscription_pattern(target)) {
        LOG_STRUCTURE(DEBUG, this, "Unsubscribing from mqtt broker as no existing subscription pattern found: %s\n",
                      target->toString().c_str());
        this->xmqtt->unsubscribe(target->toString())->wait();
      }
    }

    Obj_p read(const fURI_p &furi, const ID_p &source) override {
      auto thing = new std::atomic<Obj *>(nullptr);
      if (furi->is_pattern())
        thing->store(new Objs(share(List<Obj_p>()), OBJS_FURI));
      this->recv_subscription(share(Subscription{
        .source = static_cast<fURI>(*source), .pattern = *furi,
        .onRecv = [this, furi, thing](const Message_p &message) {
          // TODO: try to not copy obj while still not accessing heap after delete
          LOG_STRUCTURE(DEBUG, this, "subscription pattern %s matched: %s\n", furi->toString().c_str(),
                        message->toString().c_str());
          if (furi->is_pattern()) {
            const Obj_p obj = ptr<Obj>(new Uri(fURI(message->target), URI_FURI));
            thing->load()->add_obj(obj);
          } else {
            thing->store(new Obj(Any(message->payload->_value), id_p(*message->payload->id())));
          }
        }
      }));
      this->loop();
      const time_t startTimestamp = time(nullptr);
      if (furi->is_pattern()) {
        while (time(nullptr) - startTimestamp < 2) {
          this->loop();
        }
      } else {
        while (!thing->load()) {
          if (time(nullptr) - startTimestamp > 1)
            break;
          this->loop();
        }
      }
      this->loop();
      this->recv_unsubscribe(source, furi);
      this->loop();
      if (furi->is_pattern()) {
        auto objs = ptr<Objs>(thing->load());
        delete thing;
        return objs;
      }
      if (nullptr == thing->load()) {
        delete thing;
        return Obj::to_noobj();
      }
      const auto ret = ptr<Obj>(thing->load());
      delete thing;
      return ret;
    }

    void write(const ID_p &target, const Obj_p &obj, const ID_p &source, const bool retain) override {
      const BObj_p source_payload = Message::wrapSource(source, obj);
      LOG_STRUCTURE(DEBUG, this, "writing to xmpp broker: %s\n", source_payload->second);
      this->xmqtt->publish(
        string(target->toString().c_str()),
        source_payload->second,
        source_payload->first,
        1 /*qos*/,
        retain)->wait();
    }

    void distributed_retainined(const Subscription_p &subscription) override {
      // handled by mqtt broker
    }
  };
} // namespace fhatos
#endif
#endif
