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
#ifndef fhatos_mqtt_hpp
#define fhatos_mqtt_hpp

#ifdef NATIVE

#include <mqtt/async_client.h>
#include "../base_mqtt.hpp"
#include <unistd.h>

/*
#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-common/simple-watch.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>
*/

namespace fhatos {
  using namespace mqtt;
  class Mqtt;
  static ptr<async_client> MQTT_CONNECTION = nullptr;
  static List_p<Mqtt *> MQTT_VIRTUAL_CLIENTS = nullptr;

  /////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////
  class Mqtt final : public BaseMqtt {
  protected:
    connect_options connection_options_{};

    bool exists() const override {
      return MQTT_CONNECTION && MQTT_CONNECTION->is_connected() &&
             MQTT_CONNECTION->get_server_uri() == this->rec_get("config/broker")->uri_value().toString();
    }

  public:
    explicit Mqtt(const Pattern &pattern, const ID_p &value_id = nullptr, const Rec_p &config = Obj::to_rec()) :
      BaseMqtt(pattern, value_id, config) {
      if(this->exists()) {
        LOG_STRUCTURE(INFO, this, "reusing existing connection to %s\n",
                      this->Obj::rec_get("config/broker")->toString().c_str());
        MQTT_VIRTUAL_CLIENTS->push_back(this);
      } else {
        MQTT_CONNECTION =
            std::make_shared<async_client>(this->Obj::rec_get("config/broker")->uri_value().toString(),
                                           this->Obj::rec_get("config/client")->uri_value().toString(),
                                           mqtt::create_options());
        connect_options_builder pre_connection_options = connect_options_builder()
            .properties({{property::SESSION_EXPIRY_INTERVAL, 604800}})
            .clean_start(true)
            .clean_session(true)
            //.mqtt_version(5)
            .user_name(this->Obj::rec_get("config/client")->uri_value().toString())
            .keep_alive_interval(std::chrono::seconds(20))
            .automatic_reconnect();
        if(!this->Obj::rec_get("config/will")->is_noobj()) {
          const BObj_p source_payload = this->Obj::rec_get("config/will")->rec_get("payload")->serialize();
          pre_connection_options = pre_connection_options.will(
              message(this->Obj::rec_get("config/will")->rec_get("target")->uri_value().toString(),
                      source_payload->second,
                      this->Obj::rec_get("config/will")->rec_get("retain")->bool_value()));
        }
        this->connection_options_ = pre_connection_options.finalize();
        //// MQTT MESSAGE CALLBACK
        MQTT_CONNECTION->set_message_callback([this](const const_message_ptr &mqtt_message) {
          const binary_ref ref = mqtt_message->get_payload_ref();
          const auto bobj =
              std::make_shared<BObj>(ref.length(), reinterpret_cast<fbyte *>(const_cast<char *>(ref.data())));
          const auto [payload, retained] = make_payload(bobj);
          // assert(mqtt_message->is_retained() == retained); // TODO why does this sometimes not match?
          const Message_p message = Message::create(id_p(mqtt_message->get_topic().c_str()), payload, retained);
          LOG_STRUCTURE(TRACE, this, "received message %s\n", message->toString().c_str());
          for(const auto *client: *MQTT_VIRTUAL_CLIENTS) {
            for(const List_p<Subscription_p> matches = client->get_matching_subscriptions(message->target());
                const Subscription_p &sub: *matches) {
              client->outbox_->push_back(make_shared<Mail>(std::make_pair(sub, message)));
            }
          }
        });
        /// MQTT CONNECTION ESTABLISHED CALLBACK
        MQTT_CONNECTION->set_connected_handler([this](const string &) { connection_logging(); });
        MQTT_VIRTUAL_CLIENTS->push_back(this);
      }
    }

    void native_mqtt_subscribe(const Subscription_p &subscription) override {
      MQTT_CONNECTION->subscribe(subscription->pattern()->toString(), 1)->wait();
    }

    void native_mqtt_unsubscribe(const fURI_p &pattern) override {
      MQTT_CONNECTION->unsubscribe(pattern->toString())->wait();
    }

    void native_mqtt_publish(const Message_p &message) override {
      if(message->payload()->is_noobj()) {
        MQTT_CONNECTION->publish(message->target()->toString().c_str(), const_cast<char *>(""), 0, 0, true)->wait();
      } else {
        const BObj_p source_payload = make_bobj(message->payload(), message->retain());
        MQTT_CONNECTION
            ->publish(message->target()->toString(), source_payload->second, source_payload->first, 0,
                      message->retain())
            ->wait();
      }
    }

    void native_mqtt_disconnect() override {
      //std::erase_if(*MQTT_VIRTUAL_CLIENTS, [this](const Mqtt *m) { return m == this; });
      if(MQTT_VIRTUAL_CLIENTS->empty() && MQTT_CONNECTION->is_connected())
        MQTT_CONNECTION->disconnect();
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  public:
    static void *import(const Pattern &pattern) {
      BaseMqtt::import<Mqtt>(pattern);
      if (!MQTT_VIRTUAL_CLIENTS)
        MQTT_VIRTUAL_CLIENTS = make_shared<List<Mqtt *>>();
      return nullptr;
    }

    void setup() override {
      BaseMqtt::setup();
      if(this->exists())
        return;
      try {
        int counter = 0;
        while(counter < FOS_MQTT_MAX_RETRIES) {
          if(!MQTT_CONNECTION->connect(this->connection_options_)->wait_for(1000)) {
            if(++counter > FOS_MQTT_MAX_RETRIES)
              throw mqtt::exception(1);
            LOG_STRUCTURE(WARN, this, "!b%s !yconnection!! retry\n",
                          this->rec_get("config/broker")->uri_value().toString().c_str());
            Process::current_process()->delay(FOS_MQTT_RETRY_WAIT * 1000);
          }
          if(MQTT_CONNECTION->is_connected())
            break;
        }
      } catch(const mqtt::exception &e) {
        LOG_STRUCTURE(ERROR, this, "unable to connect to !b%s!!: %s\n",
                      this->rec_get("config/broker")->uri_value().toString().c_str(), e.what());
      }
    }
  };
} // namespace fhatos
#endif
#endif
