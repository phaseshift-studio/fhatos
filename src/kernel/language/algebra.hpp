#ifndef fhatos_kernel__algebra_hpp
#define fhatos_kernel__algebra_hpp

#include <fhatos.hpp>
//
#include <kernel/language/obj.hpp>

namespace fhatos::kernel {

class Algebra {

    template <typename T>
    static T plus(const T& a, const T&b) {
       Int a;
       Int b;
       a.plus(b); 
    }

};

}

#endif