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
#ifdef NATIVE
#include <mqtt/async_client.h>
#include "../mqtt.hpp"
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
  std::vector<Mqtt *> Mqtt::MQTT_VIRTUAL_CLIENTS = std::vector<Mqtt *>();

  /////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////

  bool Mqtt::exists() const {
    if(!this->available_.load() || !this->handler_.has_value())
      return false;
    const auto h = std::any_cast<ptr<async_client>>(this->handler_);
    return h->is_connected() && h->get_server_uri() == this->rec_get("config/broker")->uri_value().toString();
  }

  void Mqtt::loop() {
    Structure::loop();
  }

  connect_options create_connection_options(const Obj_p &mqtt_obj) {
    connect_options_builder pre_connection_options = connect_options_builder()
        .properties({{property::SESSION_EXPIRY_INTERVAL, 604800}})
        .clean_start(true)
        .clean_session(true)
        //.mqtt_version(5)
        .user_name(mqtt_obj->Obj::rec_get("config/client")->uri_value().toString())
        .keep_alive_interval(std::chrono::seconds(20))
        .automatic_reconnect();
    if(!mqtt_obj->Obj::rec_get("config/will")->is_noobj()) {
      const BObj_p source_payload = mqtt_obj->Obj::rec_get("config/will")->rec_get("payload")->serialize();
      pre_connection_options = pre_connection_options.will(
          message(mqtt_obj->Obj::rec_get("config/will")->rec_get("target")->uri_value().toString(),
                  source_payload->second,
                  mqtt_obj->Obj::rec_get("config/will")->rec_get("retain")->bool_value()));
    }
    const connect_options connection_options_ = pre_connection_options.finalize();
    return connection_options_;
  }

  Mqtt::Mqtt(const Pattern &pattern, const ID_p &value_id, const Rec_p &config) :
    Structure(pattern, id_p(MQTT_FURI), value_id, config) {
    if(this->exists()) {
      LOG_WRITE(INFO, this, L("reusing existing connection to {}\n", this->Obj::rec_get("config/broker")->toString()));
      // MQTT_VIRTUAL_CLIENTS.push_back(this);
    } else {
      MQTT_VIRTUAL_CLIENTS.push_back(this);
      this->handler_ =
          std::make_shared<async_client>(this->Obj::rec_get("config/broker")->uri_value().toString(),
                                         this->Obj::rec_get("config/client")->uri_value().toString(),
                                         mqtt::create_options());
      //// MQTT MESSAGE CALLBACK
      std::any_cast<ptr<async_client>>(this->handler_)->set_message_callback(
          [this](const const_message_ptr &mqtt_message) {
            const binary_ref ref = mqtt_message->get_payload_ref();
            const auto bobj = ref.empty()
                                ? nullptr
                                : std::make_shared<BObj>(ref.length(),
                                                         reinterpret_cast<fbyte *>(const_cast<char *>(ref.data())));
            const auto [payload, retained] = bobj
                                               ? make_payload(bobj)
                                               : make_pair(Obj::to_noobj(), mqtt_message->is_retained());
            // assert(mqtt_message->is_retained() == retained); // TODO: why does this sometimes not match?
            const Message_p message = Message::create(id_p(mqtt_message->get_topic().c_str()), payload, retained);
            LOG_WRITE(DEBUG, this, L("received message {}\n", message->toString()));
            for(auto *client: MQTT_VIRTUAL_CLIENTS) {
              if(message->target()->matches(*client->pattern) && retained) {
                client->write_cache(*message->target(), payload);
              }
              for(const Subscription_p &sub: *client->subscriptions_) {
                if(message->target()->matches(*sub->pattern())) {
                  sub->apply(message);
                }
              }
            }
          });
      /// MQTT CONNECTION ESTABLISHED CALLBACK
      std::any_cast<ptr<async_client>>(this->handler_)->set_connected_handler([this](const string &) {
        connection_logging();
      });
    }
  }

  void Mqtt::native_mqtt_subscribe(const Subscription_p &subscription) {
    std::any_cast<ptr<async_client>>(this->handler_)->subscribe(subscription->pattern()->toString(), 1)->wait();
  }

  void Mqtt::native_mqtt_unsubscribe(const fURI &pattern) {
    std::any_cast<ptr<async_client>>(this->handler_)->unsubscribe(pattern.toString())->wait();
  }

  void Mqtt::native_mqtt_publish(const Message_p &message) {
    if(message->payload()->is_noobj()) {
      std::any_cast<ptr<async_client>>(this->handler_)->publish(message->target()->toString().c_str(),
                                                                const_cast<char *>(""), 0, 0,
                                                                message->retain())->wait();
    } else {
      const BObj_p source_payload = make_bobj(message->payload(), message->retain());
      std::any_cast<ptr<async_client>>(this->handler_)
          ->publish(message->target()->toString(), source_payload->second, source_payload->first, 0,
                    message->retain())
          ->wait();
    }
  }

  void Mqtt::native_mqtt_disconnect() {
    //std::erase_if(*MQTT_VIRTUAL_CLIENTS, [this](const Mqtt *m) { return m == this; });
    if(1 == MQTT_VIRTUAL_CLIENTS.size()) {
      if(std::any_cast<ptr<async_client>>(this->handler_)->is_connected())
        std::any_cast<ptr<async_client>>(this->handler_)->disconnect();
      MQTT_VIRTUAL_CLIENTS.clear();
    } else {
      MQTT_VIRTUAL_CLIENTS.erase(std::remove_if(MQTT_VIRTUAL_CLIENTS.begin(), MQTT_VIRTUAL_CLIENTS.end(),
                                                [this](const Mqtt *mqtt) {
                                                  const bool remove = mqtt == this;
                                                  return remove;
                                                }), MQTT_VIRTUAL_CLIENTS.end());
    }
    MQTT_VIRTUAL_CLIENTS.clear();
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  void Mqtt::setup() {
    if(this->exists())
      return;
    try {
      Structure::setup();
      const connect_options connect_options_ = create_connection_options(this->shared_from_this());
      int counter = 0;
      while(counter < FOS_MQTT_MAX_RETRIES) {
        if(!std::any_cast<ptr<async_client>>(this->handler_)->connect(connect_options_)->wait_for(1000)) {
          if(++counter > FOS_MQTT_MAX_RETRIES)
            throw mqtt::exception(1);
          LOG_WRITE(WARN, this, L("!b{} !yconnection!! retry\n",
                                  this->rec_get("config/broker")->uri_value().toString()));
          Process::current_process()->delay(FOS_MQTT_RETRY_WAIT * 1000);
        }
        if(std::any_cast<ptr<async_client>>(this->handler_)->is_connected())
          break;
      }
      if(this->get<bool>("config/cache"))
        this->enable_cache();
    } catch(const mqtt::exception &e) {
      LOG_WRITE(ERROR, this, L("unable to connect to !b{}!!: {}\n",
                               this->rec_get("config/broker")->uri_value().toString(), e.what()));
    }
  }
}
#endif
