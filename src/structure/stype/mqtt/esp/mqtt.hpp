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

#ifndef NATIVE

#include <PubSubClient.h>
#include <WiFiClient.h>
#include <fhatos.hpp>
#include <structure/stype/mqtt/base_mqtt.hpp>
#include <util/options.hpp>
#include FOS_PROCESS(scheduler.hpp)

// #ifndef MQTT_MAX_PACKET_SIZE
#define MQTT_MAX_PACKET_SIZE 512
// #endif

namespace fhatos {

  class Mqtt;
  static ptr<PubSubClient> MQTT_CONNECTION = nullptr;
  static List_p<Mqtt *> MQTT_VIRTUAL_CLIENTS = nullptr;
  /////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////
  class Mqtt : public BaseMqtt {


  protected:
    // TODO: mutiple pubsubs by broker w/ connection counter for connect/disconnect
    const Map<int8_t, string> MQTT_STATE_CODES = {{{-4, "MQTT_CONNECTION_TIMEOUT"},
                                                   {-3, "MQTT_CONNECTION_LOST"},
                                                   {-2, "MQTT_CONNECT_FAILED"},
                                                   {-1, "MQTT_DISCONNECTED"},
                                                   {0, "MQTT_CONNECTED"},
                                                   {1, "MQTT_CONNECT_BAD_PROTOCOL"},
                                                   {2, "MQTT_CONNECT_BAD_CLIENT_ID"},
                                                   {3, "MQTT_CONNECT_UNAVAILABLE"},
                                                   {4, "MQTT_CONNECT_BAD_CREDENTIALS"},
                                                   {5, "MQTT_CONNECT_UNAUTHORIZED"}}};

    bool exists() const override { return MQTT_CONNECTION && MQTT_CONNECTION->connected(); }

    explicit Mqtt(const Pattern &pattern, const Settings &settings) : BaseMqtt(pattern, settings) {

      if (this->exists()) {
        LOG_STRUCTURE(INFO, this, "reusing existing connection to %s\n", settings.broker_.c_str());
        MQTT_VIRTUAL_CLIENTS->push_back(this);
      } else if (this->settings_.broker_.empty()) {
        LOG_STRUCTURE(WARN, this, "mqtt disabled as no broker address provided\n");
      } else {
        WiFiClient *client = new WiFiClient();
        MQTT_CONNECTION = ptr<PubSubClient>(new PubSubClient(*client));
        MQTT_CONNECTION->setServer(this->settings_.broker_.c_str(), 1883); // TODO: parse port from uri
        MQTT_CONNECTION->setBufferSize(MQTT_MAX_PACKET_SIZE);
        MQTT_CONNECTION->setSocketTimeout(1000); // may be too excessive
        MQTT_CONNECTION->setKeepAlive(1000); // may be too excessive
        MQTT_CONNECTION->setCallback([this](const char *topic, const uint8_t *data, const uint32_t length) {
          ((char *) data)[length] = '\0';
          const BObj_p bobj = make_shared<BObj>(length, (fbyte *) data);
          const auto [payload, retained] = make_payload(bobj);
          // [payload,retain]
          const Message_p message = message_p(ID(topic), payload, retained);
          LOG_STRUCTURE(TRACE, this, "mqtt broker providing message %s\n", message->toString().c_str());
          for (const auto *client: *MQTT_VIRTUAL_CLIENTS) {
            const List_p<Subscription_p> matches = client->get_matching_subscriptions(furi_p(topic));
            for (const Subscription_p &sub: *matches) {
              client->outbox_->push_back(share(Mail{sub, message}));
            }
          }
        });
        MQTT_VIRTUAL_CLIENTS->push_back(this);
      }
    }

    virtual void loop() override {
      BaseMqtt::loop();
      if (!MQTT_CONNECTION->connected()) {
        LOG_STRUCTURE(WARN, this, "reconnecting to mqtt broker: !r%s!!\n",
                      MQTT_STATE_CODES.at(MQTT_CONNECTION->state()).c_str());
        if (!MQTT_CONNECTION->connect(this->settings_.client_.c_str())) {
          usleep(FOS_MQTT_RETRY_WAIT * 10);
        }
      }
      if (!MQTT_CONNECTION->loop()) {
        LOG_STRUCTURE(ERROR, this, "mqtt processing loop failure: !r%s!!\n",
                      MQTT_STATE_CODES.at(MQTT_CONNECTION->state()).c_str());
      }
    }

    void native_mqtt_subscribe(const Subscription_p &subscription) override {
      MQTT_CONNECTION->subscribe(subscription->pattern.toString().c_str(), 1);
    }

    void native_mqtt_unsubscribe(const fURI_p &pattern) override {
      MQTT_CONNECTION->unsubscribe(pattern->toString().c_str());
    }

    void native_mqtt_publish(const Message_p &message) override {
      if (message->payload->is_noobj()) {
        MQTT_CONNECTION->publish(message->target.toString().c_str(), nullptr, 0, true);
      } else {
        const Lst_p lst = Obj::to_lst({message->payload, dool(message->retain)});
        const BObj_p payload = make_bobj(message->payload, message->retain);
        MQTT_CONNECTION->publish(message->target.toString().c_str(), payload->second, payload->first, message->retain);
      }
      MQTT_CONNECTION->loop();
    }

    void native_mqtt_disconnect() override {
      std::remove_if(MQTT_VIRTUAL_CLIENTS->begin(), MQTT_VIRTUAL_CLIENTS->end(),
                     [this](const Mqtt *m) { return m == this; });
      if (MQTT_VIRTUAL_CLIENTS->empty())
        MQTT_CONNECTION->disconnect();
    }


  public:
    static ptr<Mqtt> create(const Pattern &pattern, const Settings &settings) {
      if (!MQTT_VIRTUAL_CLIENTS)
        MQTT_VIRTUAL_CLIENTS = make_shared<List<Mqtt *>>();
      const auto mqtt_p = ptr<Mqtt>(new Mqtt(pattern, settings));
      return mqtt_p;
    }

    void setup() override {
      BaseMqtt::setup();
      if (this->exists())
        return;
      try {
        int counter = 0;
        while (counter < FOS_MQTT_MAX_RETRIES) {
          const bool pass = MQTT_CONNECTION->connect(this->settings_.client_.c_str());
          if (!pass) {
            if (++counter > FOS_MQTT_MAX_RETRIES)
              throw fError("__wrapped below__");
            LOG_STRUCTURE(WARN, this, "!b%s !yconnection!! retry\n", this->settings_.broker_.c_str());
            usleep(FOS_MQTT_RETRY_WAIT * 1000);
          }
          if (MQTT_CONNECTION->connected()) {
            connection_logging();
            break;
          }
        }
      } catch (const fError &e) {
        LOG_STRUCTURE(ERROR, this, "unable to connect to !b%s!!: %s\n", this->settings_.broker_.c_str(), e.what());
      }
    }
  };

} // namespace fhatos

#endif
#endif