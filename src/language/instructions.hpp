#ifndef fhatos_instructions_hpp
#define fhatos_instructions_hpp

#include <fhatos.hpp>
//
#include <any>
#include <language/algebra.hpp>
#include <language/obj.hpp>
#ifndef NATIVE
#include <process/router/local_router.hpp>
#endif

namespace fhatos {
  class S_E;

  template<typename S>
  static List<Obj *> *cast(const List<S *> *list) {
    List<Obj *> *newList = new List<Obj *>();
    for (const auto s: *list) {
      newList->push_back((Obj *) s);
    }
    return newList;
  }

  template<typename S>
  class StartInst final : public Inst<S, S> {
  public:
    explicit StartInst(const List<S *> *starts)
      : Inst<S, S>({
        "start", *cast(starts),
        [starts](S *b) -> S *{
          return new S(((S *) starts->front())->value());
        }
      }) {
    }
  };

  template<typename E, typename ALGEBRA = Algebra>
  class PlusInst final : public Inst<E, E> {
  public:
    explicit PlusInst(const E *a)
      : Inst<E, E>({
        "plus", List<Obj *>({(Obj *) a->obj()}),
        [this](const E *b) {
          return (E *) ALGEBRA::singleton()->plus(this->template arg<E>(0)->apply(b), b);
        }
      }) {
    }
  };

  template<typename E, typename ALGEBRA=Algebra>
  class MultInst final : public Inst<E, E> {
  public:
    explicit MultInst(const E *a)
      : Inst<E, E>({
        "mult", List<Obj *>({(Obj *) a->obj()}),
        [this](const E *b) {
          return (E *) ALGEBRA::singleton()->mult(this->template arg<E>(0)->apply(b), b);
        }
      }) {
    }
  };

  template<typename E, typename ALGEBRA=Algebra>
  class PublishInst final : public Inst<Uri, Uri> {
  public:
    explicit PublishInst(const E *a) : Inst<Uri, Uri>({
      "<=", List<Obj *>({(Obj *) a->obj()}),
      [this](const Uri *uri) {
#ifndef NATIVE
        FOS_DEFAULT_ROUTER::singleton()->publish(Message{
          .source = ID("anoymous"),
          .target = uri,
          .payload = this->template arg<E>(0)->apply(uri),
          .retain = TRANSIENT_MESSAGE
        });
        return uri;
#else
        LOG(INFO, "Pbulished %s\n", this->template arg<E>(0)->toString().c_str());
        return (Uri*) uri;
#endif

      }
    }) {
    }
  };
} // namespace fhatos

#endif
