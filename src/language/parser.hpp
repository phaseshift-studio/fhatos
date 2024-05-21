#ifndef fhatos_parser_hpp
#define fhatos_parser_hpp

#include <fhatos.hpp>
#include <sstream>
#include <cctype>
#include <functional>
#include <algorithm>
#include <language/obj.hpp>
#include <language/fluent.hpp>

using namespace std;

namespace fhatos {
  class Parser {
  protected:
    //const string input;
    //stringstream ss;

  public:
    Parser() {
    }

    template<typename S, typename E>
    Bytecode<S, E> *parse(string *line) {
      LOG(INFO, "PARSING: %s\n", line->c_str());
      const auto ss = new stringstream(*line);
      List<Inst<Obj, Obj> *> *insts = this->parseBytecode(ss);
      auto bcode = new Bytecode<S, E>(insts);
      LOG(INFO, "BYTECODE: %s []\n",OTYPE_STR.at(bcode->type()).c_str());
      return bcode;
    }


    List<Inst<Obj, Obj> *> *parseBytecode(stringstream *ss) {
      List<Inst<Obj, Obj> *> *list = new List<Inst<Obj, Obj> *>();
      while (!ss->eof()) {
        const string *opcode = this->parseOpcode(ss);
        const List<Obj *> *args = this->parseArgs(ss);
        Inst<Obj, Obj> *inst = this->parseInst(*opcode, args);
        list->push_back(inst);
        LOG(INFO, "INST: %s\n", list->back()->toString().c_str());
        //LOG(INFO, "PEEK: %s\n", (string() + (char)ss->peek()).c_str());
      }
      return list;
    }

    Inst<Obj, Obj> *parseInst(const string &opcode, const List<Obj *> *args) {
      if (opcode == "plus")
        return reinterpret_cast<Inst<Obj, Obj> *>(new PlusInst<Int>(static_cast<Int *>(args->at(0))));
      if (opcode == "start")
        return (Inst<Obj,Obj>*) new StartInst<Int>((List<Int*>*)args);
      else
        return nullptr;
    }

    string *parseOpcode(stringstream *ss) {
      string *opcode = new string();
      while (std::isspace(ss->peek())) {
        ss->get();
      }
      while (ss->peek() != '(' && !ss->eof()) {
        opcode->append({static_cast<char>(ss->get())});
      }
      trim(*opcode);
      if ((*opcode)[0] == '_' && (*opcode)[1] == '_') {
        opcode->clear();
        opcode->append("start");
      }
      LOG(INFO, "\tPARSING OPCODE: %s\n", opcode->c_str());
      return opcode;
    }

    List<Obj *> *parseArgs(stringstream *ss) {
      List<Obj *> *args = new List<Obj *>();
      while (std::isspace(ss->peek())) {
        ss->get();
      }
      ss->get(); // (
      while (!ss->eof()) {
        Obj *argObj = this->parseArg<Obj>(ss);
        args->push_back(argObj);
        LOG(INFO, "\t\tARG %s [%s]\n", argObj->toString().c_str(), OTYPE_STR.at(argObj->type()).c_str());
        //LOG(INFO, "PEEK2: %s\n", (string() + (char)ss->peek()).c_str());
        if (ss->peek() == 'x')
          break;
        if (ss->peek() == ',' || ss->peek() == '.' || ss->peek() == ')') {
          ss->get();
          break;
        }
      }
      return args;
    }

    template<typename S>
    S *parseArg(stringstream *ss) {
      string argS = string();
      int paren = 0;
      while (!ss->eof()) {
        if (ss->peek() == '(')
          paren++;
        if (ss->peek() == ')')
          paren--;
        argS += static_cast<char>(ss->get());
        if (ss->peek() == ')' && paren == 0) {
          ss->get();
          break;
        }
        if (ss->peek() == ',' && paren == 0) {
          ss->get();
          break;
        }
      }
      trim(argS);

      return parseObj<S>(argS);
    }

    template<typename S>
    S *parseObj(string &token) {
      LOG(INFO, "\t\tPARSING TOKEN: %s\n", token.c_str());
      if (token[0] == '\'' && token[token.length() - 1] == '\'') {
        return (S *) new Str(token.substr(1, token.length() - 2)); // might be wrong indices
      } else if (strcmp("true", token.c_str()) == 0 || strcmp("false", token.c_str()) == 0) {
        return (S *) new Bool((strcmp("true", token.c_str()) == 0));
      } else if (token.length() > 3 && token[0] == '_' && token[1] == '_' && token[2] == '.') {
        stringstream *ss = new stringstream(token.substr(3, token.length() - 1));
        return (S *) this->parseBytecode(ss);
      } else if (token.find('.') != string::npos) {
        return (S *) new Real(stof(token));
      } else {
        return (S *) new Int(stoi(token));
      }
    }

    static void trim(string &s) {
      s.erase(0, s.find_first_not_of(" "));
      s.erase(s.find_last_not_of(" ") + 1);
    }
  };
}
#endif
