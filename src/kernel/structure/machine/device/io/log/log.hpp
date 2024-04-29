#ifndef fhatos_kernel__log_hpp
#define fhatos_kernel__log_hpp

#include <fhatos.hpp>
#include <kernel/process/actor/actor.hpp>
#include <kernel/process/actor/message.hpp>
#include <kernel/process/actor/router/meta_router/meta_router.hpp>
#include <kernel/structure/machine/device/io/net/wifi/wifi.hpp>
#include <kernel/structure/structure.hpp>
#include <kernel/util/ansi.hpp>
#include <kernel/util/string_stream.hpp>
#include <sstream>
#include FOS_PROCESS(thread.hpp)

namespace fhatos::kernel {

    template<typename PROCESS = Thread, typename MESSAGE = String,
            typename ROUTER = LocalRouter<Message<MESSAGE>>>
    class Log : public Actor<PROCESS, MESSAGE, ROUTER> {
    public:
        explicit Log(const ID &id = WIFI::idFromIP("log"))
                : Actor<PROCESS, MESSAGE, ROUTER>(id) {};

        void setup() override {
            Actor<PROCESS,MESSAGE,ROUTER>::setup();
            const ID serialID = WIFI::idFromIP("serial");
            // INFO LOGGING
            this->subscribe(this->id().extend("INFO"), [this, serialID](
                    const Message<MESSAGE> &message) {
                this->publish(
                        serialID,
                        this->createLogMessage(INFO, message.payloadString()).c_str(), false);
            });
            // ERROR LOGGING
            this->subscribe(this->id().extend("ERROR"), [this, serialID](
                    const Message<MESSAGE> &message) {
                this->publish(
                        serialID,
                        this->createLogMessage(INFO, message.payloadString()).c_str(), false);
            });
        }

    protected:
        String createLogMessage(LOG_TYPE type, const String message) {
            if (message.startsWith("\t"))
                type = LOG_TYPE::NONE;
            String output;
            StringStream stream = StringStream(&output);
            auto ansi = Ansi<StringStream>(&stream);
            if (type != LOG_TYPE::NONE) {
                if (type == ERROR)
                    ansi.print("!r[ERROR]!!  ");
                else if (type == INFO)
                    ansi.print("!g[INFO]!!  ");
                else
                    ansi.print("!y[DEBUG]!!  ");
            }
            ansi.print(message.c_str());
            return output;
        }
    };
} // namespace fhatos::kernel

#endif