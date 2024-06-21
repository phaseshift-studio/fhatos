#ifndef fhatos_serializer
#define fhatos_serializer

#include <fhatos.hpp>
#include <language/obj.hpp>

namespace fhatos {

  ////////////////////////////
  //////// SERIALIZER ////////
  ////////////////////////////

  class PtrSerializer {
    static PtrSerializer *singleton() {
      static PtrSerializer serializer = PtrSerializer();
      return &serializer;
    }

    static const ptr<BObj> serialize(const Obj *obj) {
      auto *bytes = static_cast<fbyte *>(malloc(sizeof(*obj)));
      memcpy(bytes, reinterpret_cast<const fbyte *>(obj), sizeof(*obj));
      LOG(DEBUG, "[serialization] %i bytes allocated for %s\n", sizeof(*obj), obj->toString().c_str());
      return share<BObj>({obj->otype(), bytes, sizeof(*obj)});
    }

    template<typename OBJ>
    ptr<OBJ> deserialize(BObj bobj) {
      const fbyte *bytes = std::get<1>(bobj);
      LOG(DEBUG, "[deserialization] %i bytes retrieved for %s\n", std::get<2>(bobj), OTYPE_STR.at(std::get<0>(bobj)));
      return ptr<OBJ>((OBJ *) bytes);
    }
  };


} // namespace fhatos

#endif
