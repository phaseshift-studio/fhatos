#ifndef fhatosf_f_fs_hpp
#define fhatosf_f_fs_hpp

#include <fhatos.hpp>
//
#include <FS.h>
#include <LittleFS.h>
#include <process/actor/actor.hpp>
#include <util/ansi.hpp>
#include <structure/query/q_fs.hpp>
#include FOS_PROCESS(thread.hpp)

namespace fhatos {
  /////////////////////////////////////////////////////////////////////
#ifndef FOS_FILE_SYSTEM
#define FOS_FILE_SYSTEM LittleFS
#endif
  /////////////////////////////////////////////////////////////////////

  template<typename PROCESS = Fiber, typename ROUTER = LocalRouter<> >
  class fFS final : public Actor<PROCESS, ROUTER> {
  public:
    static fFS *singleton() {
      static fFS singleton = fFS();
      return &singleton;
    }

    explicit fFS(const ID &id = fWIFI::idFromIP("kernel", "fs"))
      : Actor<PROCESS, ROUTER>(id) {
    }

    virtual ~fFS() override {
      FOS_FILE_SYSTEM.end();
      // Actor<PROCESS,ROUTER>::~Actor();
    }

    virtual void setup() override {
      PROCESS::setup();
      const char *mount = "/littlefs";
      const bool success = LittleFS.begin(false, mount);
      LOG_TASK(success ? INFO : ERROR, this, "%s mounted at %s", STR(FOS_FILE_SYSTEM), mount);
      this->subscribe(this->id().extend("#"),
                      [this](const Message &message) {
                        if (!message.source.equals(this->id()) && message.isQuery()) {
                          const FSInfo *info = qFS(&message.target, LittleFS).structure(true);
                          String temp;
                          Ansi<Stream> ansi(new StringStream(&temp));
                          if (info->type == FILE)
                            ((FileInfo *) info)->print(ansi);
                          else
                            ((DirInfo *) info)->print(ansi);
                          ansi.flush();
                          delete info;
                          this->publish(message.target, temp,RETAIN_MESSAGE);
                        }
                      });
    }
  };
} // namespace fhatos

#endif
