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
    OType domain = OBJ;
    OType range = OBJ;

  public:
    Parser() {
    }

    template<typename S, typename E>
    Bytecode<S, E> *parse(string *line) {
      LOG(DEBUG, "PARSING: %s\n", line->c_str());
      const auto ss = new stringstream(*line);
      List<Inst<Obj, Obj> *> *insts = this->parseBytecode(ss);
      auto bcode = new Bytecode<S, E>(insts);
      LOG(DEBUG, "BYTECODE: %s [%s]\n\n", bcode->toString().c_str(), OTYPE_STR.at(bcode->type()).c_str());
      return bcode;
    }


    List<Inst<Obj, Obj> *> *parseBytecode(stringstream *ss) {
      List<Inst<Obj, Obj> *> *list = new List<Inst<Obj, Obj> *>();
      while (!ss->eof()) {
        const string *opcode = this->parseOpcode(ss);
        const List<Obj *> *args = this->parseArgs(ss);
        //LOG(ERROR, "PE444E! %s\n", (string() + (char)ss->peek()).c_str());

        /*if (*opcode == "start" && args->empty()) {
          LOG(ERROR,"PE444E! %s\n",(string() + (char)ss->peek()).c_str());
          continue;
        }*/

        if (*opcode == "start") {
          range = args->empty() ? domain : args->front()->type();
        } else {
          range = domain;
        }

        LOG(DEBUG, FOS_TAB_2 "domain(%s) => range(%s)\n", OTYPE_STR.at(domain).c_str(), OTYPE_STR.at(range).c_str());
        Inst<Obj, Obj> *inst;
        switch (range) {
          case INT: inst = this->parseInst<Int>(*opcode, args);
            break;
          case BYTECODE: inst = this->parseInst<Int>(*opcode, args);
            break;
          default: inst = nullptr;
        }
        //domain = range;
        list->push_back(inst);
        LOG(DEBUG, FOS_TAB "INST: %s\n", list->back()->toString().c_str());
        //LOG(INFO, "PEEK: %s\n", (string() + (char)ss->peek()).c_str());
      }
      return list;
    }

    template<typename S, typename E =S>
    Inst<Obj, Obj> *parseInst(const string &opcode, const List<Obj *> *args) {
      domain = range;
      range = OBJ;
      if (opcode == "plus")
        return reinterpret_cast<Inst<Obj, Obj> *>(new PlusInst<E>(static_cast<E *>(args->at(0))));
      if (opcode == "mult")
        return reinterpret_cast<Inst<Obj, Obj> *>(new MultInst<E>(static_cast<E *>(args->at(0))));
      if (opcode == "start")
        return reinterpret_cast<Inst<Obj, Obj> *>(new StartInst<E>((List<E *> *) args));
      else
        return nullptr;
    }

    string *parseOpcode(stringstream *ss) {
      string *opcode = new string();
      while (std::isspace(ss->peek())) {
        ss->get();
      }
      while (ss->peek() != '.' && ss->peek() != '(' && !ss->eof()) {
        opcode->append({static_cast<char>(ss->get())});
      }
      trim(*opcode);
      if ((*opcode)[0] == '_' && (*opcode)[1] == '_') {
        opcode->clear();
        opcode->append("start");
      }
      LOG(DEBUG, FOS_TAB_2 "OPCODE: %s\n", opcode->c_str());
      return opcode;
    }

    List<Obj *> *parseArgs(stringstream *ss) {
      List<Obj *> *args = new List<Obj *>();
      //LOG(ERROR, "PEE! %s\n", (string() + (char)ss->peek()).c_str());
      while (std::isspace(ss->peek())) {
        ss->get();
      }
      if (ss->peek() == '(')
        ss->get(); // (
      while (!ss->eof()) {
        Obj *argObj = this->parseArg<Obj>(ss);
        args->push_back(argObj);
        LOG(DEBUG, FOS_TAB_3 "ARG: %s [%s]\n", argObj->toString().c_str(), OTYPE_STR.at(argObj->type()).c_str());
        //LOG(INFO, "PEEK2: %s\n", (string() + (char)ss->peek()).c_str());
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
      LOG(DEBUG, FOS_TAB_4 "PARSING TOKEN: %s\n", token.c_str());
      if (token[0] == '\'' && token[token.length() - 1] == '\'') {
        return static_cast<S *>(new Str(token.substr(1, token.length() - 2))); // might be wrong indices
      } else if (strcmp("true", token.c_str()) == 0 || strcmp("false", token.c_str()) == 0) {
        return (S *) new Bool((strcmp("true", token.c_str()) == 0));
      } else if (token.length() > 3 && token[0] == '_' && token[1] == '_' && token[2] == '.') {
        stringstream *ss = new stringstream(token.substr(3));
        OType tdomain = domain;
        OType trange = range;
        domain = range;
        S *b = (S *) this->parseBytecode(ss);
        domain = tdomain;
        range = trange;
        return b;
      } else if (token.find('.') != string::npos) {
        return (S *) new Real(stof(token));
      } else {
        return static_cast<S *>(new Int(stoi(token)));
      }
    }

    static void trim(string &s) {
      s.erase(0, s.find_first_not_of(" "));
      s.erase(s.find_last_not_of(" ") + 1);
    }
  };
}
#endif
