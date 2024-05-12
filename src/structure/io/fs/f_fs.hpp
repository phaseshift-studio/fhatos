#ifndef fhatosf_f_fs_hpp
#define fhatosf_f_fs_hpp

#include <fhatos.hpp>
//
#include <FS.h>
#include <LittleFS.h>
#include <furi.hpp>
#include <process/actor/actor.hpp>
#include <util/ansi.hpp>
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
      Actor<PROCESS, ROUTER>::~Actor();
    }

    virtual void setup() override {
      PROCESS::setup();
      const char *mount = "/littlefs";
      const bool success = LittleFS.begin(false, mount);
      LOG_TASK(success ? INFO : ERROR, this, "%s mounted at %s", STR(FOS_FILE_SYSTEM), mount);
      this->subscribe(this->id().extend("#"), [this](const Message &message) {
        if (!message.source.equals(this->id()))
          if (auto *listing = new List<Triple<String, String, String> >();
            this->listDir(listing, LittleFS, (String("/") + message.target.path()).c_str(), 1)) {
            String temp;
            Ansi ansi(new StringStream(&temp));
            for (const auto &row: *listing) {
              ansi.printf(
                FOS_TAB "!b\\_!!!r%s %s!!\n" FOS_TAB "" FOS_TAB "" FOS_TAB
                "[!gsize!!:!b%s!!]\n",
                std::get<0>(row).c_str(),
                std::get<1>(row).c_str(),
                this->prettyPrintBytes(std::get<2>(row).toFloat()).c_str());
            }
            ansi.flush();
            this->publish(message.target, temp,RETAIN_MESSAGE);
            delete listing;
          } else {
            LOG_TASK(ERROR, this, "Unable to load %s", message.target.path().c_str());
          }
      });
    }

  private:
    const String prettyPrintBytes(const float bytes) const {
      char returnSize[256];
      if (constexpr float tb = 1099511627776; bytes >= tb)
        sprintf(returnSize, "%.2f tb", static_cast<float>(bytes) / tb);
      else if (constexpr float gb = 1073741824; bytes >= gb && bytes < tb)
        sprintf(returnSize, "%.2f gb", static_cast<float>(bytes) / gb);
      else if (constexpr float mb = 1048576; bytes >= mb && bytes < gb)
        sprintf(returnSize, "%.2f mb", static_cast<float>(bytes) / mb);
      else if (constexpr float kb = 1024; bytes >= kb && bytes < mb)
        sprintf(returnSize, "%.2f kb", static_cast<float>(bytes) / kb);
      else if (bytes < kb)
        sprintf(returnSize, "%.2f bytes", bytes);
      else
        sprintf(returnSize, "%.2f bytes", bytes);
      return String(returnSize);
    }

    const bool listDir(List<Triple<String, String, String> > *result, FS &fs, const char *dirname,
                       uint8_t levels) const {
      File root = fs.open(dirname);
      if (!root || !root.isDirectory()) {
        LOG_TASK(ERROR, this, "Failed to open directory: %s", dirname);
        return false;
      }
      result->push_back({"DIR", this->id().extend(root.name()).toString(), String(root.size())});
      File file = root.openNextFile();
      while (file) {
        if (file.isDirectory()) {
          result->push_back({
            "dir", this->id().extend(root.name()).extend(file.name()).toString(), String(file.size())
          });
          if (levels) {
            if (!listDir(result, fs, file.name(), levels - 1))
              return false;
          }
        } else {
          String f = String(file.name()) + " " + file.size();
          result->push_back(
            {"FILE", this->id().extend(root.name()).extend(file.name()).toString(), String(file.size())});
        }
        file = root.openNextFile();
      }
      return true;
    }
  };
} // namespace fhatos

#endif
