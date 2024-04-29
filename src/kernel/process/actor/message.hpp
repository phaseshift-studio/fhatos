#ifndef fhatos_kernel_message_hpp
#define fhatos_kernel_message_hpp

#include <fhatos.hpp>
#include <kernel/structure/structure.hpp>
#include <utility>

namespace fhatos::kernel {

    template<typename PAYLOAD = String>
    class Message {
    public:
        const ID source;
        const ID target;
        const PAYLOAD payload;
        const bool retain;

        Message(ID source, ID target, PAYLOAD payload,
                const bool retain = false)
                : source(std::move(source)), target(std::move(target)), payload(std::move(payload)), retain(retain) {
        };

        [[nodiscard]] virtual String toString() const {
            char temp[100];
            sprintf(temp, "[%s]=%s[retain:%s]=>[%s]", source.toString().c_str(),
                    payloadString().c_str(), FP_BOOL_STR(retain),
                    target.toString().c_str());
            return temp;
        };


        template<typename = typename std::enable_if<std::is_base_of_v<String, PAYLOAD>>>
        [[nodiscard]] String payloadString() const {
            return payload;
        }

        /*template<typename = typename std::enable_if<std::is_base_of_v<int, PAYLOAD>, int>>
        [[nodiscard]]  String payloadString() const {
            return String() + payload;
        }*/

        /*template <typename = typename std::enable_if<std::is_base_of_v<float,PAYLOAD>,float>>
        [[nodiscard]]  const String payloadString() const {
            return String() + payload;
        }

        template<typename = typename std::enable_if<std::is_base_of_v<bool, PAYLOAD>,bool>>
        [[nodiscard]]  const String payloadString()  {
            return payload ? "true" : "false";
        }*/

        [[nodiscard]] virtual const Pair<byte *, uint> toBytes() const {
            String temp = payloadString();
            byte *bytes = (byte *) temp.c_str();
            return {bytes, temp.length()};
        }
    };

    using BoolMessage = Message<bool>;
    using IntMessage = Message<int>;
    using StringMessage = Message<String>;

} // namespace fhatos::kernel

#endif