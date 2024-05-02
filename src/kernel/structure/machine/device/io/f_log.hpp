#ifndef fhatos_kernel__f_log_hpp
#define fhatos_kernel__f_log_hpp

#include <fhatos.hpp>
#include <kernel/process/actor/actor.hpp>
#include <kernel/process/actor/message.hpp>
#include <kernel/process/actor/router/meta_router/meta_router.hpp>
#include <kernel/structure/machine/device/io/net/f_wifi.hpp>
#include <kernel/structure/structure.hpp>
#include <kernel/util/ansi.hpp>
#include <kernel/util/string_stream.hpp>
#include <sstream>
#include FOS_PROCESS(thread.hpp)

namespace fhatos::kernel {

    template<typename PROCESS = Thread, typename PAYLOAD = String,
            typename ROUTER = LocalRouter<Message<PAYLOAD>>>
    class fLog : public Actor<PROCESS, PAYLOAD, ROUTER> {
    public:
        explicit fLog(const ID &id = fWIFI::idFromIP("log"))
                : Actor<PROCESS, PAYLOAD, ROUTER>(id) {};

        void setup() override {
            Actor<PROCESS,PAYLOAD,ROUTER>::setup();
            const ID serialID = fWIFI::idFromIP("serial");
            // INFO LOGGING
            this->subscribe(this->id().extend("INFO"), [this, serialID](
                    const Message<PAYLOAD> &message) {
                this->publish(
                        serialID,
                        this->createLogMessage(INFO, message.payloadString()).c_str(), false);
            });
            // ERROR LOGGING
            this->subscribe(this->id().extend("ERROR"), [this, serialID](
                    const Message<PAYLOAD> &message) {
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