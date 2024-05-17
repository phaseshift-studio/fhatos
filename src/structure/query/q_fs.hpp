#ifndef fhatos_resource_hpp
#define fhatos_resource_hpp

#include <fhatos.hpp>
#include <FS.h>
#include <util/ansi.hpp>
#include <structure/query/query.hpp>
#include <util/pretty.hpp>

namespace fhatos {
  enum FSType { FILE, DIR };

  const static Map<FSType, string> D_TYPE_STR = {{FILE, "file"}, {DIR, "dir"}};

  struct FSInfo {
    const FSType type;
    const fURI *furi;
    const uint16_t size;


    FSInfo(const FSType type, const fURI *furi, const uint16_t size) : type(type), furi(furi), size(size) {
    }
  };

  struct FileInfo : public FSInfo {
    const Option<string> contents;

    FileInfo(const fURI *furi, const uint16_t size, const Option<string> contents) : FSInfo{
        FILE, furi, size
      }, contents{contents} {
    }

    void print(Ansi<StringPrinter> &ansi) const {
      ansi.printf("!b==>FILE!![!gsize:%s!!]!r%s!!\n", Pretty::prettyBytes(this->size).c_str(),
                  this->furi->path().c_str());
      if (this->contents.has_value()) {
        ansi.printf("%s\n", this->contents.value().c_str());
      }
    }
  };

  struct DirInfo : public FSInfo {
    const Option<List<const FSInfo *> > contents;

    DirInfo(const fURI *furi, const uint16_t size, const Option<List<const FSInfo *> > contents) : FSInfo{
        DIR, furi, size
      }, contents{contents} {
    }

    void print(Ansi<StringPrinter> &ansi) const {
      ansi.printf("!b==>DIR!![!gsize:%i!!]!r%s!!\n", this->size, this->furi->path().c_str());
      if (this->contents.has_value()) {
        for (const FSInfo *info: contents.value()) {
          ansi.printf(FOS_TAB_3 "!b\\_%s!![!gsize:%s!!]!r%s!!\n", info->type == FILE ? "FILE" : "DIR",
                      info->type == FILE ? Pretty::prettyBytes(this->size).c_str() : Int(this->size).toString().c_str(),
                      info->furi->path().c_str());
        }
      }
    }
  };

  class qFS : public Query {
  public:
    FS &fs;

    qFS(const fURI *furi, FS &fs) : Query(furi), fs(fs) {
    }

    const FSInfo *structure(const bool contents = false) {
      LOG(INFO, "Fetching file system information on %s\n", furi->toString().c_str());
      File root = fs.open(("/" + furi->path()).c_str());
      if (root.isDirectory()) {
        uint16_t counter = 0;
        List<const FSInfo *> body;
        File next = root.openNextFile();
        while (next) {
          counter++;
          if (contents)
            body.push_back(qFS(new fURI(furi->extend(next.name())), this->fs).structure(false));
          next = root.openNextFile();
        }
        const DirInfo *result = new DirInfo{
          .furi = furi,
          .size = counter,
          .contents = (contents ? Option<List<const FSInfo *> >(body) : Option<List<const FSInfo *> >())
        };
        root.close();
        return result;
      } else {
        const FileInfo *result = new FileInfo{
          .furi = furi,
          .size = (uint16_t) root.size(),
          .contents = (contents ? Option<string>(string(root.readString().c_str())) : Option<string>())
        };
        root.close();
        return result;
      }
    }
  };
}
#endif
