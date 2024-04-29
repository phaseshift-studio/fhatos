#ifndef fhatos_kernel_message_hpp
#define fhatos_kernel_message_hpp

#include <fhatos.hpp>
#include <kernel/structure/structure.hpp>
#include <utility>

namespace fhatos::kernel {

    template<typename PAYLOAD>
    class Message {

    public:
        const ID source;
        const ID target;
        const PAYLOAD payload;
        const bool retain;

        Message(ID source, ID target, PAYLOAD payload,
                const bool retain = false)
                : source(std::move(source)), target(std::move(target)), payload(std::move(payload)), retain(retain) {};

        [[nodiscard]] virtual String toString() const {
            char temp[100];
            sprintf(temp, "[%s]=%s[retain:%s]=>[%s]", source.toString().c_str(),
                    payloadString().c_str(), FP_BOOL_STR(retain),
                    target.toString().c_str());
            return temp;
        };

        [[nodiscard]] virtual String payloadString() const { return ""; }

        [[nodiscard]] virtual const Pair<byte *, uint> toBytes() const {
            String temp = payloadString();
            byte *bytes = (byte *) temp.c_str();
            return {bytes, temp.length()};
        }
    };

////////////////////////////////////////////////
//////////////// STRING MESSAGE ////////////////
////////////////////////////////////////////////

    class StringMessage : public Message<String> {
    public:
        StringMessage(const ID &source, const ID &target, const String &payload,
                      const bool retain = false)
                : Message<String>(source, target, payload, retain) {};

        static String fromBytes(const byte *bytes, const uint length) {
            return {(char *) bytes, length};
        }

        virtual const Pair<byte *, uint> toBytes() const override {
            return {(byte *) this->payload.c_str(),
                                  this->payload.length()};
        }

        virtual String payloadString() const { return this->payload; }
    };

} // namespace fhatos::kernel

#endif