#ifndef fhatos_parser_hpp
#define fhatos_parser_hpp

#include <fhatos.hpp>
#include <sstream>
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
      Bytecode<S, E> *bcode = this->parseBytecode<S, E>(ss);
      LOG(DEBUG, "BYTECODE: %s [%s]\n", bcode->toString().c_str(), OTYPE_STR.at(bcode->type()).c_str());
      return bcode;
    }

    template<typename S, typename E>
    Bytecode<S, E> *parseBytecode(stringstream *ss) {
      Fluent<S, E> fluent = Fluent<S, E>();
      while (!ss->eof()) {
        const string *opcode = this->parseOpcode(ss);
        const List<S_E *> *args = this->parseArgs<S, E>(ss);
        if (*opcode == "plus") {
          range = domain;
          fluent = fluent.plus(*args->at(0));
        } else if (*opcode == "mult") {
          range = domain;
          fluent = fluent.mult(*args->at(0));
        } else if (*opcode == "start") {
          range = args->at(0)->type;
          fluent = fluent.start(*args);
        }
        LOG(DEBUG, FOS_TAB_2 "domain(%s) => range(%s)\n", OTYPE_STR.at(domain).c_str(), OTYPE_STR.at(range).c_str());
        LOG(DEBUG, FOS_TAB "INST: %s\n", fluent.bcode->value()->back()->toString().c_str());
        domain = range;
        range = OBJ;
        delete args;
      }
      return new Bytecode<S, E>(fluent.bcode->value());
    }

    const string *parseOpcode(stringstream *ss) {
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

    template<typename S, typename E>
    List<S_E *> *parseArgs(stringstream *ss) {
      List<S_E *> *args = new List<S_E *>();
      while (std::isspace(ss->peek())) {
        ss->get();
      }
      if (ss->peek() == '(')
        ss->get(); // (
      while (!ss->eof()) {
        S_E *argObj = this->parseArg<S, E>(ss);
        args->push_back(argObj);
        LOG(DEBUG, FOS_TAB_3 "ARG: %s [%s]\n", argObj->obj->toString().c_str(),
            OTYPE_STR.at(argObj->obj->type()).c_str());
        if (ss->peek() == ',' || ss->peek() == '.' || ss->peek() == ')') {
          ss->get();
          break;
        }
      }
      return args;
    }

    template<typename S, typename E>
    S_E *parseArg(stringstream *ss) {
      string argS;
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
      return parseObj<S, E>(argS);
    }

    template<typename S, typename E>
    S_E *parseObj(const string &token) {
      LOG(DEBUG, FOS_TAB_4 "PARSING TOKEN: %s\n", token.c_str());
      if (token[0] == '\'' && token[token.length() - 1] == '\'') {
        return new S_E(token.substr(1, token.length() - 2)); // might be wrong indices
      } else if (strcmp("true", token.c_str()) == 0 || strcmp("false", token.c_str()) == 0) {
        return new S_E((strcmp("true", token.c_str()) == 0));
      } else if (token.length() > 3 && token[0] == '_' && token[1] == '_' && token[2] == '.') {
        stringstream *ss = new stringstream(token.substr(3));
        OType tdomain = domain;
        OType trange = range;
        //domain = range;
        S_E *b = new S_E(reinterpret_cast<Bytecode<Obj, Obj> *>(this->parseBytecode<S, E>(ss)));
        domain = tdomain;
        range = trange;
        return b;
      } else if (token.find('.') != string::npos) {
        return new S_E(stof(token));
      } else {
        return new S_E(stoi(token));
      }
    }

    static void trim(string &s) {
      s.erase(0, s.find_first_not_of(" "));
      s.erase(s.find_last_not_of(" ") + 1);
    }
  };
}
#endif
