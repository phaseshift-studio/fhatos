#ifndef fhatos_kernel__processor_hpp
#define fhatos_kernel__processor_hpp

#include <fhatos.hpp>
//
#include <kernel/language/instructions.hpp>
#include <kernel/language/obj.hpp>

namespace fhatos::kernel {

    template<typename A>
    class Monad {
    protected:
        A *value;
        const Inst<ObjY, A> *inst = nullptr;
        const long _bulk = 1;

    public:
        Monad(A *value) : value(value) {}

        template<typename B>
        const Monad<B> *split(const Inst<A, B> *next) const {
            return new Monad<B>(next->apply(this->get()));
        }

        A *get() const { return this->value; }

        const long bulk() const { return this->_bulk; }

        const Inst<ObjX, A> *at() const { return this->inst; }
        // const bool equals(const Monad<ObjX> &other) const {
        //   return this->value.equals(other.get());
        // }
    };

    template<typename S, typename E, typename MONAD = Monad<E>>
    class Processor {

    protected:
        const Bytecode<S, E> bcode;
        List<E *> ends;


    public:
        Processor(const Bytecode<S, E> &bcode) : bcode(bcode) {}

        const void forEach(const Consumer<const E *> consumer)  {
            for (const auto *end: this->toList()) {
                consumer(end);
            }
        }

         List<E *> toList()  {
            static bool done = false;
            if (done)
                return this->ends;
            else
                done = true;
            List<void *> starts = List<void *>(this->bcode.get().front().args());
            for (const auto *start: starts) {
                MONAD *end = new MONAD((E *) start);
                int counter = 0;
                for (const auto &inst: this->bcode.get()) {
                    if (counter++ != 0)
                        end = (MONAD *) (void *) end->split(&inst);
                }
                this->ends.push_back(end->get());
            }
            return this->ends;
        }

        const bool hasNext()  {
            return !this->toList().empty();
        }

        const E *next()  {

            if (!this->hasNext())
                throw fError("No more obj results");
            else {
                const E *e = this->toList().front();
                this->toList().pop_front();
                return e;
            }
        }
    };

} // namespace fhatos::kernel

#endif