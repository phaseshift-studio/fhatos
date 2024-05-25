#ifndef fhatos_parser_hpp
#define fhatos_parser_hpp

#include <fhatos.hpp>
#include <sstream>
#include <language/obj.hpp>
#include <language/fluent.hpp>


namespace fhatos {
  using namespace std;

  class Parser {
  protected:
    OType domain = OType::OBJ;
    OType range = OType::OBJ;
    const ID context;

  public:
    Parser(const ID context = ID("anonymous")) : context((context)) {
    }

    template<typename S, typename E>
    ptr<Bytecode> parse(const char *line) {
      LOG(DEBUG, "!RPARSING!!: !g!_%s!!\n", line);
      const auto ss = new stringstream(string(line));
      ptr<Bytecode> bcode = this->parseBytecode<S, E>(ss);
      LOG(DEBUG, "!rBYTECODE!!: %s [%s]\n", bcode->toString().c_str(), OTYPE_STR.at(bcode->type()).c_str());
      return bcode;
    }

    template<typename S, typename E>
    Fluent<S, E> *parseToFluent(const char *line) {
      return new Fluent<S, E>(this->parse<S, E>(line));
    }

    template<typename S, typename E>
    ptr<Bytecode> parseBytecode(stringstream *ss) {
      Fluent<S, E> *fluent = new Fluent<S, E>(this->context);
      while (!ss->eof()) {
        const ptr<string> opcode = this->parseOpcode(ss);
        const ptr<List<ptr<S_E> > > args = this->parseArgs<S, E>(ss);

        if (*opcode == "plus") {
          range = domain;
          fluent = new Fluent<S, E>(fluent->plus(*args->at(0)).bcode);
        } else if (*opcode == "mult") {
          range = domain;
          fluent = new Fluent<S, E>(fluent->mult(*args->at(0)).bcode);
        } else if (*opcode == "start") {
          range = args->size() > 0 ? args->at(0)->type : OType::OBJ;
          fluent = new Fluent<S, E>(fluent->start(*args).bcode);
        } else if (*opcode == "<=") {
          range = OType::URI;
          fluent = new Fluent<S, E>(fluent->template publish<Obj>(*args->at(0), *args->at(1)).bcode);
        } else if (*opcode == "=>") {
          range = OType::URI;
          fluent = new Fluent<S, E>(fluent->template subscribe<Obj>(*args->at(0), *args->at(1)).bcode);
        } else {
          fError *error = new fError("Unknown instruction opcode: %s", opcode->c_str());
          LOG(ERROR, error->what());
          throw error;
        }
        //delete fluent;
        LOG(DEBUG, FOS_TAB_2 "!gdomain!!(!y%s!!) => !grange!!(!y%s!!)\n", OTYPE_STR.at(domain).c_str(),
            OTYPE_STR.at(range).c_str());
        LOG(DEBUG, FOS_TAB "INST: %s\n", fluent->bcode->value()->back()->toString().c_str());
        domain = range;
        range = OType::OBJ;
      }

      //delete fluent;
      return fluent->bcode;
    }

    const ptr<string> parseOpcode(stringstream *ss) {
      auto opcode = share(string());
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
      LOG(DEBUG, FOS_TAB_2 "!rOPCODE!!: %s\n", opcode->c_str());
      return opcode;
    }

    template<typename S, typename E>
    ptr<List<ptr<S_E> > > parseArgs(stringstream *ss) {
      auto args = share(List<ptr<S_E> >());
      while (std::isspace(ss->peek())) {
        ss->get();
      }
      if (ss->peek() == '(')
        ss->get(); // (
      while (!ss->eof()) {
        ptr<S_E> argObj = this->parseArg<S, E>(ss);
        args->push_back(argObj);
        LOG(NONE, FOS_TAB_8 FOS_TAB_6 "!g==>!!%s [!y%s!!]\n", argObj->toString().c_str(),
            OTYPE_STR.at(argObj->type).c_str());
        if (ss->peek() == ',' || ss->peek() == '.' || ss->peek() == ')') {
          ss->get();
          break;
        }
      }
      return args;
    }

    template<typename S, typename E>
    ptr<S_E> parseArg(stringstream *ss) {
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

    template<typename S, typename E = S>
    ptr<S_E> parseObj(const string &token) {
      ptr<S_E> se;
      LOG(DEBUG, FOS_TAB_4 "!rTOKEN!!: %s\n", token.c_str());
      if (token == "Ø") {
        se = share<S_E>(S_E(NoObj::singleton()));
      } else if (token[0] == '\'' && token[token.length() - 1] == '\'') {
        se = share<S_E>(S_E(token.substr(1, token.length() - 2))); // might be wrong indices
      } else if (strcmp("true", token.c_str()) == 0 || strcmp("false", token.c_str()) == 0) {
        se = share<S_E>(S_E(strcmp("true", token.c_str()) == 0));
      } else if (token[0] == '_' && token[1] == '_') {
        stringstream *ss = new stringstream(token.length() > 2 ? token.substr(3) : token);
        OType tdomain = domain;
        OType trange = range;
        //domain = range;
        se = share<S_E>(S_E(this->parseBytecode<S, E>(ss).get()));
        domain = tdomain;
        range = trange;
        // delete ss;
      } else if (token[0] == '[' && token[token.length() - 1] == ']') {
        RecMap<Obj *, Obj *> map = RecMap<Obj *, Obj *>();
        stringstream *ss = new stringstream(token.substr(1, token.length() - 2));
        bool onKey = true;
        string key;
        string value;
        int bracketCounter = 0;
        while (!ss->eof()) {
          if (onKey) {
            if (bracketCounter == 0 && ss->peek() == '=') {
              ss->get();
              if (ss->peek() == '>') {
                ss->get();
                onKey = false;
              } else {
                key += '=';
              }
            } else {
              if (ss->peek() == '[')
                bracketCounter++;
              if (ss->peek() == ']')
                bracketCounter--;
              if (!ss->eof())
                key += ss->get();
            }
          } else {
            ///////
            if (bracketCounter == 0 && ss->peek() == ',') {
              ss->get();
              onKey = true;
              map.emplace(parseObj<Obj>(key)->cast<Obj>(), parseObj<Obj>(value)->cast<Obj>());
              key.clear();
              value.clear();
            } else {
              if (ss->peek() == '[')
                bracketCounter++;
              if (ss->peek() == ']')
                bracketCounter--;
              if (!ss->eof())
                value += ss->get();
            }
          }
        }
        map.emplace(parseObj<Obj>(key)->cast<Obj>(), parseObj<Obj>(value)->cast<Obj>());
        RecMap<Obj *, Obj *> *map2 = new RecMap<Obj *, Obj *>(); // necessary to revese entries
        for (const auto &pair: map) {
          map2->insert(pair);
        }
        delete ss;
        se = share<S_E>(S_E(map2));
      } else if (isdigit(token[0]) && token.find('.') != string::npos) {
        se = share<S_E>(S_E(stof(token)));
      } else if (isdigit(token[0])) {
        se = share<S_E>(S_E(stoi(token)));
      } else {
        se = share<S_E>(S_E(fURI(token)));
      }
      return se;
    }

    static void trim(string &s) {
      s.erase(0, s.find_first_not_of(" "));
      s.erase(s.find_last_not_of(" ") + 1);
    }
  };
}
#endif
