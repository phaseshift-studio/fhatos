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
#include <util/string_helper.hpp>

namespace fhatos {
  using namespace std;

  class Parser : public IDed {
  public:
    explicit Parser(const ID &id = ID(*UUID::singleton()->mint())) : IDed(id) {
    }

    const ptr<Bytecode> parse(const char *line) {
      string cleanLine = string(line);
      StringHelper::trim(cleanLine);
      LOG(DEBUG, "!RPARSING!!: !g!_%s!!\n", cleanLine.c_str());
      if (cleanLine.empty()) {
        return share<Bytecode>(Bytecode(this->id()));
      }
      const auto ss = new stringstream(cleanLine);
      ptr<Bytecode> bcode = this->parseBytecode(ss);
      LOG(DEBUG, "!rBYTECODE!!: %s [%s]\n", bcode->toString().c_str(), OTYPE_STR.at(bcode->type()));
      return bcode;
    }

    const ptr<Fluent<> > parseToFluent(const char *line) {
      return share<Fluent<> >(Fluent<>(this->parse(line)));
    }

    static const ptr<Bytecode> parseBytecode(stringstream *ss) {
      Fluent<> *fluent = new Fluent<>();
      while (!ss->eof()) {
        const ptr<string> opcode = Parser::parseOpcode(ss);
        const ptr<List<ptr<OBJ_OR_BYTECODE> > > args = Parser::parseArgs(ss);

        if (*opcode == "count") {
          fluent = new Fluent(fluent->count());
        } else if (*opcode == "is") {
          fluent = new Fluent(fluent->is(*args->at(0)));
        } else if (*opcode == "ref") {
          fluent = new Fluent(fluent->ref(URI_OR_BYTECODE(*args->at(0))));
        } else if (*opcode == "dref") {
          fluent = new Fluent(fluent->dref(URI_OR_BYTECODE(*args->at(0))));
        } else if (*opcode == "switch") {
          fluent = new Fluent(fluent->bswitch(OBJ_OR_BYTECODE(*args->at(0))));
        } else if (*opcode == "eq") {
          fluent = new Fluent(fluent->eq(OBJ_OR_BYTECODE(*args->at(0))));
        } else if (*opcode == "neq") {
          fluent = new Fluent(fluent->neq(OBJ_OR_BYTECODE(*args->at(0))));
        } else if (*opcode == "gt") {
          fluent = new Fluent(fluent->gt(OBJ_OR_BYTECODE(*args->at(0))));
        } else if (*opcode == "gte") {
          fluent = new Fluent(fluent->gte(OBJ_OR_BYTECODE(*args->at(0))));
        } else if (*opcode == "lt") {
          fluent = new Fluent(fluent->lt(OBJ_OR_BYTECODE(*args->at(0))));
        } else if (*opcode == "lte") {
          fluent = new Fluent(fluent->lte(OBJ_OR_BYTECODE(*args->at(0))));
        } else if (*opcode == "plus") {
          fluent = new Fluent(fluent->plus(OBJ_OR_BYTECODE(*args->at(0))));
        } else if (*opcode == "mult") {
          fluent = new Fluent(fluent->mult(OBJ_OR_BYTECODE(*args->at(0))));
        } else if (*opcode == "start") {
          fluent = new Fluent(fluent->start(*args));
        } else if (*opcode == "explain") {
          fluent = new Fluent(fluent->explain());
        } else if (*opcode == "<=") {
          fluent = new Fluent(fluent->publish(URI_OR_BYTECODE(*args->at(0)), OBJ_OR_BYTECODE(*args->at(1))));
        } else if (*opcode == "=>") {
          fluent = new Fluent(fluent->subscribe(URI_OR_BYTECODE(*args->at(0)), OBJ_OR_BYTECODE(*args->at(1))));
        } else {
          const fError error = fError("Unknown instruction opcode: !b%s!!\n", opcode->c_str());
          LOG(ERROR, error.what());
          throw error;
        }
        LOG(DEBUG, FOS_TAB "INST: %s\n", fluent->bcode->value()->back()->toString().c_str());
      }

      //delete fluent;
      return fluent->bcode;
    }

    static const ptr<string> parseOpcode(stringstream *ss) {
      auto opcode = share(string());
      while (std::isspace(ss->peek())) {
        ss->get();
      }
      while (ss->peek() != '.' && ss->peek() != '(' && !ss->eof()) {
        opcode->append({static_cast<char>(ss->get())});
      }
      StringHelper::trim(*opcode);
      if ((*opcode)[0] == '_' && (*opcode)[1] == '_') {
        opcode->clear();
        opcode->append("start");
      }
      LOG(DEBUG, FOS_TAB_2 "!rOPCODE!!: %s\n", opcode->c_str());
      return opcode;
    }

    static const ptr<List<ptr<OBJ_OR_BYTECODE> > > parseArgs(stringstream *ss) {
      const auto args = share(List<ptr<OBJ_OR_BYTECODE> >());
      while (std::isspace(ss->peek())) {
        ss->get();
      }
      if (ss->peek() == '(') {
        ss->get(); // (
        if (ss->peek() == ')') {
          ss->get();
          if (ss->peek() == '.')
            ss->get();
          return args;
        }
      } else if (ss->peek() == '.') {
        ss->get();
        return args;
      }
      while (!ss->eof()) {
        ptr<OBJ_OR_BYTECODE> argObj = Parser::parseArg(ss);
        args->push_back(argObj);
        LOG(NONE, FOS_TAB_8 FOS_TAB_6 "!g==>!!%s [!y%s!!]\n", argObj->toString().c_str(),
            OTYPE_STR.at(argObj->type()));
        if (ss->peek() == ',' || ss->peek() == '.' || ss->peek() == ')') {
          ss->get();
          break;
        }
      }
      return args;
    }

    static const ptr<OBJ_OR_BYTECODE> parseArg(stringstream *ss) {
      string arg;
      int paren = 0;
      int bracket = 0;
      while (!ss->eof()) {
        if (ss->peek() == '(')
          paren++;
        else if (ss->peek() == ')')
          paren--;
        else if (ss->peek() == '[') {
          bracket++;
        } else if (ss->peek() == ']') {
          bracket--;
        }
        arg += ss->get();
        if (ss->peek() == ')' && paren == 0) {
          ss->get();
          break;
        } else if (ss->peek() == ',' && paren == 0 && bracket == 0) {
          ss->get();
          break;
        }
      }
      StringHelper::trim(arg);
      return parseObj(arg);
    }

    static const ptr<OBJ_OR_BYTECODE> parseObj(const string &token2) {
      char *array; //[token2.length()];
      array = strdup(token2.c_str());
      string token = string(array);
      ptr<OBJ_OR_BYTECODE> se;
      LOG(DEBUG, FOS_TAB_4 "!rTOKEN!!: %s\n", token.c_str());
      StringHelper::trim(token);
      int index = token.find("::");
      fURI *utype = nullptr;
      if (index != string::npos) {
        utype = new fURI(token.substr(0, index));
        token = token.substr(index + 2);
        LOG(DEBUG, FOS_TAB_5 "!yutype!!: %s\n", utype->toString().c_str());
      }
      if (strcmp(token.c_str(), "Ø") == 0) {
        se = share<OBJ_OR_BYTECODE>(OBJ_OR_BYTECODE(NoObj::singleton()));
      } else if (token[0] == '\'' && token[token.length() - 1] == '\'') {
        se = share<OBJ_OR_BYTECODE>(OBJ_OR_BYTECODE(new Str(token.substr(1, token.length() - 2))));
        // might be wrong indices
      } else if (strcmp("true", token.c_str()) == 0 || strcmp("false", token.c_str()) == 0) {
        se = share<OBJ_OR_BYTECODE>(OBJ_OR_BYTECODE(new Bool(strcmp("true", token.c_str()) == 0)));
      } else if (token[0] == '_' && token[1] == '_') {
        stringstream *ss = new stringstream(token.length() > 2 ? token.substr(3) : token);
        se = share<OBJ_OR_BYTECODE>(OBJ_OR_BYTECODE(Parser::parseBytecode(ss).get()));
        delete ss;
      } else if (token[token.length() - 1] == ')' && token.find('(')) {
        stringstream *ss = new stringstream(token);
        se = share<OBJ_OR_BYTECODE>(OBJ_OR_BYTECODE(Parser::parseBytecode(ss).get()));
        delete ss;
      } else if (token[0] == '[' && token[token.length() - 1] == ']') {
        stringstream *ss = new stringstream(token.substr(1, token.length() - 2));
        string first;
        OType type = OType::LST;
        while (!ss->eof()) {
          if (ss->peek() == '|') {
            type = OType::INST;
            ss->get();
            break;
          } else if (StringHelper::lookAhead("=>", ss)) {
            type = OType::REC;
            break;
          } else if (ss->peek() == ',') {
            type = OType::LST;
            ss->get();
            break;
          }
          first += ss->get();
        }
        StringHelper::trim(first);
        ////////////////////////////////////
        if (type == OType::INST) {
          string opcode = first;
          List<ptr<OBJ_OR_BYTECODE> > args = List<ptr<OBJ_OR_BYTECODE> >();
          string arg;
          int bracketCounter = 0;
          while (!ss->eof()) {
            if (ss->peek() == ']') {
              ss->get();
              break;
            } else if (ss->peek() == ',' && bracketCounter == 0) {
              ss->get();
              args.push_back(parseObj(arg));
              arg.clear();
            }
            if (ss->peek() == '[') {
              bracketCounter++;
            } else if (ss->peek() == ']') {
              bracketCounter--;
            }
            arg += ss->get();
          }
          //
        } else if (type == OType::REC) {
          RecMap<Obj *, Obj *> map = RecMap<Obj *, Obj *>();
          bool onKey = false;
          string key = first;
          string value;
          int bracketCounter = 0;
          while (!ss->eof()) {
            if (onKey) {
              if (bracketCounter == 0 && StringHelper::lookAhead("=>", ss)) {
                onKey = false;
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
                ss->get(); // drop k/v separating comma
                onKey = true;
                map.insert({parseObj(key)->cast<Obj>(), parseObj(value)->cast<Obj>()});
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
          map.insert({parseObj(key)->cast<Obj>(), parseObj(value)->cast<Obj>()});
          RecMap<Obj *, Obj *> *map2 = new RecMap<Obj *, Obj *>(); // necessary to revese entries
          for (const auto &pair: map) {
            map2->insert(pair);
          }
          delete ss;
          se = share<OBJ_OR_BYTECODE>(OBJ_OR_BYTECODE(new Rec(map2)));
        }
      } else if
      (((token[0] == '-' && isdigit(token[1])) || isdigit(token[0])) && token.find('.') != string::npos) {
        se = share<OBJ_OR_BYTECODE>(OBJ_OR_BYTECODE(new Real(stof(token))));
      } else if ((token[0] == '-' && isdigit(token[1])) || isdigit(token[0])) {
        se = share<OBJ_OR_BYTECODE>(
          OBJ_OR_BYTECODE(new Int(stoi(token), utype ? *utype : fURI(OTYPE_STR.at(OType::INT)))));
      } else {
        se = share<OBJ_OR_BYTECODE>(OBJ_OR_BYTECODE(new Uri(fURI(token))));
      }
      delete array;
      if (utype)
        delete utype;
      return se;
    }
  };
}
#endif
