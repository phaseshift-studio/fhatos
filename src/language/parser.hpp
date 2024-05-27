/*******************************************************************************
 FhatOS: A Distributed Operating System
 Copyright (c) 2024 PhaseShift Studio, LLC

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Affero General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Affero General Public License for more details.

 You should have received a copy of the GNU Affero General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

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
    explicit Parser(const ID &context = ID("anonymous")) : context((context)) {
    }

    const ptr<Bytecode> parse(const char *line) {
      LOG(DEBUG, "!RPARSING!!: !g!_%s!!\n", line);
      const auto ss = new stringstream(string(line));
      ptr<Bytecode> bcode = this->parseBytecode(ss);
      LOG(DEBUG, "!rBYTECODE!!: %s [%s]\n", bcode->toString().c_str(), OTYPE_STR.at(bcode->type()).c_str());
      return bcode;
    }

    const ptr<Fluent<> > parseToFluent(const char *line) {
      return share<Fluent<> >(Fluent<>(this->parse(line)));
    }

    const ptr<Bytecode> parseBytecode(stringstream *ss) {
      Fluent<> *fluent = new Fluent<>(this->context);
      while (!ss->eof()) {
        const ptr<string> opcode = this->parseOpcode(ss);
        const ptr<List<ptr<OBJ_OR_BYTECODE> > > args = this->parseArgs(ss);

        if (*opcode == "is") {
          range = domain;
          fluent = new Fluent(fluent->is(*args->at(0)));
        } else if (*opcode == "branch") {
          fluent = new Fluent(fluent->branch(OBJ_OR_BYTECODE(*args->at(0))));
        } else if (*opcode == "eq") {
          fluent = new Fluent(fluent->eq(OBJ_OR_BYTECODE(*args->at(0))));
        } else if (*opcode == "plus") {
          range = domain;
          fluent = new Fluent(fluent->plus(OBJ_OR_BYTECODE(*args->at(0))));
        } else if (*opcode == "mult") {
          range = domain;
          fluent = new Fluent(fluent->mult(OBJ_OR_BYTECODE(*args->at(0))));
        } else if (*opcode == "start") {
          range = args->size() > 0 ? args->at(0)->type() : OType::OBJ;
          fluent = new Fluent(fluent->start(*args).bcode);
        } else if (*opcode == "<=") {
          range = OType::URI;
          fluent = new Fluent(fluent->publish<Obj>(*args->at(0), *args->at(1)).bcode);
        } else if (*opcode == "=>") {
          range = OType::URI;
          fluent = new Fluent(fluent->subscribe<Obj>(*args->at(0), *args->at(1)).bcode);
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

    const ptr<List<ptr<OBJ_OR_BYTECODE> > > parseArgs(stringstream *ss) {
      const auto args = share(List<ptr<OBJ_OR_BYTECODE> >());
      while (std::isspace(ss->peek())) {
        ss->get();
      }
      if (ss->peek() == '(')
        ss->get(); // (
      while (!ss->eof()) {
        ptr<OBJ_OR_BYTECODE> argObj = this->parseArg(ss);
        args->push_back(argObj);
        LOG(NONE, FOS_TAB_8 FOS_TAB_6 "!g==>!!%s [!y%s!!]\n", argObj->toString().c_str(),
            OTYPE_STR.at(argObj->type()).c_str());
        if (ss->peek() == ',' || ss->peek() == '.' || ss->peek() == ')') {
          ss->get();
          break;
        }
      }
      return args;
    }

    const ptr<OBJ_OR_BYTECODE> parseArg(stringstream *ss) {
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
      return parseObj(argS);
    }

    const ptr<OBJ_OR_BYTECODE> parseObj(const string &token) {
      ptr<OBJ_OR_BYTECODE> se;
      LOG(DEBUG, FOS_TAB_4 "!rTOKEN!!: %s\n", token.c_str());
      if (token == "Ø") {
        se = share<OBJ_OR_BYTECODE>(OBJ_OR_BYTECODE(NoObj::singleton()));
      } else if (token[0] == '\'' && token[token.length() - 1] == '\'') {
        se = share<OBJ_OR_BYTECODE>(OBJ_OR_BYTECODE(new Str(token.substr(1, token.length() - 2))));
        // might be wrong indices
      } else if (strcmp("true", token.c_str()) == 0 || strcmp("false", token.c_str()) == 0) {
        se = share<OBJ_OR_BYTECODE>(OBJ_OR_BYTECODE(new Bool(strcmp("true", token.c_str()) == 0)));
      } else if (token[0] == '_' && token[1] == '_') {
        stringstream *ss = new stringstream(token.length() > 2 ? token.substr(3) : token);
        OType tdomain = domain;
        OType trange = range;
        //domain = range;
        se = share<OBJ_OR_BYTECODE>(OBJ_OR_BYTECODE(this->parseBytecode(ss).get()));
        domain = tdomain;
        range = trange;
        delete ss;
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
              map.emplace(parseObj(key)->cast<Obj>(), parseObj(value)->cast<Obj>());
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
        map.emplace(parseObj(key)->cast<Obj>(), parseObj(value)->cast<Obj>());
        RecMap<Obj *, Obj *> *map2 = new RecMap<Obj *, Obj *>(); // necessary to revese entries
        for (const auto &pair: map) {
          map2->insert(pair);
        }
        delete ss;
        se = share<OBJ_OR_BYTECODE>(OBJ_OR_BYTECODE(new Rec(map2)));
      } else if (isdigit(token[0]) && token.find('.') != string::npos) {
        se = share<OBJ_OR_BYTECODE>(OBJ_OR_BYTECODE(new Real(stof(token))));
      } else if (isdigit(token[0])) {
        se = share<OBJ_OR_BYTECODE>(OBJ_OR_BYTECODE(new Int(stoi(token))));
      } else {
        se = share<OBJ_OR_BYTECODE>(OBJ_OR_BYTECODE(new Uri(fURI(token))));
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
