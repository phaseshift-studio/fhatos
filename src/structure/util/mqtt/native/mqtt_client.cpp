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
#include "../mqtt_client.hpp"
#include <mqtt/async_client.h>
#include <unistd.h>

/*
#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-common/simple-watch.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>
*/

#define FOS_MQTT_MAX_RETRIES 10
#define FOS_MQTT_RETRY_WAIT 2000

namespace fhatos {
  using namespace mqtt;


  void MqttClient::loop() {}

  MqttClient::MqttClient(const Rec_p &config) :
      Rec(std::move(config->rec_value()), OType::REC, REC_FURI),
      handler_(std::make_shared<async_client>(config->get<fURI>("broker").toString(),
                                              config->get<fURI>("client").toString(), mqtt::create_options())) {
    //// MQTT MESSAGE CALLBACK]
    std::any_cast<ptr<async_client>>(this->handler_)
        ->set_message_callback([this](const const_message_ptr &mqtt_message) {
          const binary_ref ref = mqtt_message->get_payload_ref();
          const auto bobj =
              ref.empty()
                  ? nullptr
                  : std::make_shared<BObj>(ref.length(), reinterpret_cast<fbyte *>(const_cast<char *>(ref.data())));
          const auto [payload, retained] =
              bobj ? make_payload(bobj) : make_pair(Obj::to_noobj(), mqtt_message->is_retained());
          // assert(mqtt_message->is_retained() == retained); // TODO: why does this sometimes not match?
          LOG_WRITE(DEBUG, this,
                    L("!b{} !ymqtt message!! received: {}\n", mqtt_message->get_topic().c_str(),
                      mqtt_message->get_payload().c_str()));
          const Message_p message = Message::create(id_p(mqtt_message->get_topic().c_str()), payload, retained);
          this->on_recv(message);
        });
    /// MQTT CONNECTION ESTABLISHED CALLBACK
    std::any_cast<ptr<async_client>>(this->handler_)->set_connected_handler([this](const string &) {
      LOG_WRITE(INFO, this, L("!b{} !ymqtt!! {} connected\n", this->broker().toString(), this->client().toString()));
      this->on_connect();
    });
  }

  void MqttClient::subscribe(const Subscription_p &subscription, const bool async) const {
    this->subscriptions_->push_back(subscription);
    const mqtt::token_ptr result =
        std::any_cast<ptr<async_client>>(this->handler_)->subscribe(subscription->pattern()->toString(), 1);
    if(!async)
      result->wait();
    LOG_WRITE(DEBUG, this, L("!b{} !ymqtt!! {} subscribe\n", this->broker().toString(), subscription->toString()));
  }

  void MqttClient::unsubscribe(const ID &source, const fURI &pattern, const bool async) const {
    this->subscriptions_->remove_if([this, &source, &pattern, async](const Subscription_p &sub) {
      const bool remove = pattern.bimatches(*sub->pattern()) && sub->source()->equals(source);
      if(remove) {
        const mqtt::token_ptr result =
            std::any_cast<ptr<async_client>>(this->handler_)->unsubscribe(sub->pattern()->toString());
        if(!async)
          result->wait();
      }
      return remove;
    });
  }

  void MqttClient::publish(const Message_p &message, const bool async) const {
    mqtt::token_ptr result;
    if(message->payload()->is_noobj()) {
      result = std::any_cast<ptr<async_client>>(this->handler_)
                   ->publish(message->target()->toString().c_str(), const_cast<char *>(""), 0, 0, message->retain());
    } else {
      const BObj_p source_payload = make_bobj(message->payload(), message->retain());
      result = std::any_cast<ptr<async_client>>(this->handler_)
                   ->publish(message->target()->toString(), source_payload->second, source_payload->first, 0,
                             message->retain());
    }
    if(!async)
      result->wait();
  }

  bool MqttClient::disconnect(const ID &source, const bool async) const {
    this->clients_->remove(source);
    this->unsubscribe(source, "#", async);
    if(this->clients_->empty() && std::any_cast<ptr<async_client>>(this->handler_)->is_connected()) {
      const token_ptr result = std::any_cast<ptr<async_client>>(this->handler_)->disconnect();
      if(!async)
        result->wait();
      CLIENTS.erase(this->broker());
    }
    LOG_WRITE(INFO, this, L("!ydisconnecting!! from !g[!y{}!g]!!\n", this->broker().toString()));
    return true;
  }

  bool MqttClient::is_connected() const {
    if(!this->handler_.has_value())
      return false;
    const auto h = std::any_cast<ptr<async_client>>(this->handler_);
    return h->is_connected() && h->get_server_uri() == this->broker().toString();
  }


  bool MqttClient::connect(const ID &source) const {
    if(this->is_connected()) {
      if(!this->clients_->exists(source))
        this->clients_->push_back(source);
      LOG_WRITE(WARN, this, L("!b{} !yconnection!! already exists\n", this->broker().toString()));
      return true;
    }
    try {
      connect_options_builder pre_connection_options = connect_options_builder()
                                                           .properties({{property::SESSION_EXPIRY_INTERVAL, 604800}})
                                                           .clean_start(true)
                                                           .clean_session(true)
                                                           //.mqtt_version(5)
                                                           .user_name(this->client().toString())
                                                           .keep_alive_interval(std::chrono::seconds(20))
                                                           .automatic_reconnect();
      if(!this->rec_get("config/will")->is_noobj()) {
        const BObj_p source_payload = this->rec_get("config/will")->rec_get("payload")->serialize();
        pre_connection_options = pre_connection_options.will(
            message(this->rec_get("config/will")->rec_get("target")->uri_value().toString(), source_payload->second,
                    this->rec_get("config/will")->rec_get("retain")->bool_value()));
      }
      const connect_options connect_options_ = pre_connection_options.finalize();
      int counter = 0;
      LOG_WRITE(INFO, this, L("!yattempting !b{} !ymqtt!! connection\n", this->broker().toString()));
      while(counter < FOS_MQTT_MAX_RETRIES) {
        if(!std::any_cast<ptr<async_client>>(this->handler_)->connect(connect_options_)->wait_for(5000)) {
          if(++counter > FOS_MQTT_MAX_RETRIES)
            throw mqtt::exception(1);
          LOG_WRITE(WARN, this, L("!b{} !yconnection!! retry\n", this->broker().toString()));
          Thread::delay(FOS_MQTT_RETRY_WAIT);
        }
        if(std::any_cast<ptr<async_client>>(this->handler_)->is_connected()) {
          this->clients_->push_back(source);
          return true;
        }
      }
    } catch(const mqtt::exception &e) {
      LOG_WRITE(ERROR, this, L("unable to connect to !b{}!!: {}\n", this->broker().toString(), e.what()));
    }
    return false;
  }
} // namespace fhatos
#endif
