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
      stringstream *ss = new stringstream(*line);
      Bytecode<S, E> *bcode = (Bytecode<S, E> *) this->parseBytecode<S, E>(ss);
      LOG(INFO, "BYTECODE: %s\n", OTYPE_STR.at(bcode->Obj::type()).c_str());
      return bcode;
    }

    template<typename S, typename E>
    Bytecode<S, E> *parseBytecode(stringstream *ss) {
      Bytecode<S, E> *bcode = new Bytecode<S, E>();
      while (!ss->eof()) {
        string *opcode = this->parseOpcode(ss);
        List<Obj *> *args = this->parseArgs(ss);
        Inst<Int, Int> *inst = this->parseInst(*opcode, args);
        bcode = (Bytecode<S, E> *) bcode->template addInst<Int>(inst);
        LOG(INFO, "BYTECODE: %s\n", bcode->toString().c_str());

        LOG(INFO, "PEEK: %s\n", (string() +ss->get()).c_str());
      }
      return bcode;
    }

    Inst<Int, Int> *parseInst(const string &opcode, const List<Obj *> *args) {
      if (opcode == "plus")
        return new PlusInst<Int>((Int *) (args->at(0)));
      else
        return nullptr;
    }

    string *parseOpcode(stringstream *ss) {
      string *opcode = new string();
      while (std::isspace(ss->peek())) {
        ss->get();
      }
      while (ss->peek() != '(' && !ss->eof()) {
        opcode->append({(char) ss->get()});
      }
      trim(*opcode);
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
        Obj *x = this->parseArg<Obj>(ss);
        args->push_back(x);
        LOG(INFO, "\t\tARG %s [%s]\n", x->toString().c_str(), OTYPE_STR.at(x->type()).c_str());
        LOG(INFO, "PEEK2: %s\n", (string() + (char)ss->peek()).c_str());
        if (ss->peek() == ',')
          ss->get();
        if (ss->peek() == ')') {
          ss->get();
          break;
        }
        // LOG(NONE, "\tARG: %s (type: %s)\n", arg.first->c_str(), OTYPE_STR.at(arg.second->type()).c_str());
      }
      return args;
    }

    template<typename S>
    S *parseArg(stringstream *ss) {
      while (std::isspace(ss->peek())) {
        ss->get();
      }
      string argS = string();
      int paren = 0;
      while (!ss->eof()) {
        if (ss->peek() == '(')
          paren++;
        if (ss->peek() == ')')
          paren--;
        argS += (char) ss->get();
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
        return (S *) new Bool((bool) (strcmp("true", token.c_str()) == 0));
      } else if (token.length() > 3 && token[0] == '_' && token[1] == '_' && token[2] == '.') {
        stringstream *ss = new stringstream(token.substr(3, token.length() - 1));
        return (S *) this->parseBytecode<Int, Int>(ss);
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
