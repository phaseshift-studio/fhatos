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
#include <language/fluent.hpp>
#include <language/obj.hpp>
#include <language/otype/mono.hpp>
#include <sstream>
#include <util/string_helper.hpp>

namespace fhatos {
  using namespace std;

  class Parser : public IDed {
  public:
    explicit Parser(const ID &id = ID(*UUID::singleton()->mint())) : IDed(id) {}

    const ptr<Bytecode> parse(const char *line) {
      string cleanLine = string(line);
      StringHelper::trim(cleanLine);
      LOG(DEBUG, "!RPARSING!!: !g!_%s!!\n", cleanLine.c_str());
      if (cleanLine.empty()) {
        return ptr<Bytecode>(new Bytecode(ptr<List<ptr<Inst>>>(new List<ptr<Inst>>({}))));
      }
      stringstream ss = stringstream(cleanLine);
      ptr<Bytecode> bcode = this->parseBytecode(&ss);
      LOG(DEBUG, "!rBYTECODE!!: %s [%s]\n", bcode->toString().c_str(), OTYPE_STR.at(bcode->otype()));
      return bcode;
    }

    const ptr<Fluent<>> parseToFluent(const char *line) { return share<Fluent<>>(Fluent<>(this->parse(line))); }

    static const ptr<Bytecode> parseBytecode(stringstream *ss) {
      Fluent<> *fluent = new Fluent<>();
      while (!ss->eof()) {
        const ptr<string> opcode = Parser::parseOpcode(ss);
        const ptr<List<ptr<Obj>>> args = Parser::parseArgs(ss);

        /*if (*opcode == "count") {
          fluent = new Fluent(fluent->count());
        } else*/
        if (*opcode == "is") {
          fluent = new Fluent(fluent->is(*(Bool *) args->at(0).get()));
        } else if (*opcode == "print") {
          fluent = new Fluent(fluent->print(*args->at(0)));
        } /* else if (*opcode == "ref") {
           fluent = new Fluent(fluent->ref(URI_OR_BYTECODE(*args->at(0))));
         } else if (*opcode == "dref") {
           fluent = new Fluent(fluent->dref(URI_OR_BYTECODE(*args->at(0))));
         } */
        else if (*opcode == "switch") {
          fluent = new Fluent(fluent->bswitch(*(Rec *) args->at(0).get()));
        } else if (*opcode == "eq") {
          fluent = new Fluent(fluent->eq(*args->at(0)));
        } else if (*opcode == "neq") {
          fluent = new Fluent(fluent->neq(*args->at(0)));
        } else if (*opcode == "gt") {
          fluent = new Fluent(fluent->gt(*args->at(0)));
        } else if (*opcode == "gte") {
          fluent = new Fluent(fluent->gte(*args->at(0)));
        } else if (*opcode == "lt") {
          fluent = new Fluent(fluent->lt(*args->at(0)));
        } else if (*opcode == "lte") {
          fluent = new Fluent(fluent->lte(*args->at(0)));
        } else if (*opcode == "plus") {
          fluent = new Fluent(fluent->plus(*args->at(0)));
        } else if (*opcode == "mult") {
          fluent = new Fluent(fluent->mult(*args->at(0)));
        } else if (*opcode == "mod") {
          fluent = new Fluent(fluent->mod(*args->at(0)));
        } else if (*opcode == "start") {
          fluent = new Fluent(fluent->start(args));
        } /*else if (*opcode == "select") {
          fluent = new Fluent(args->at(0)->type() == OType::REC ? fluent->select(args->at(0)->cast<Rec>())
                                                                : fluent->select(*args));
        } else if (*opcode == "as") {
          fluent = new Fluent(args->empty() ? fluent->as() : fluent->as(*args->at(0)));
        } else if (*opcode == "define") {
          fluent = new Fluent(fluent->define(URI_OR_BYTECODE(*args->at(0)), *args->at(1)));
        } else if (*opcode == "explain") {
          fluent = new Fluent(fluent->explain());
        } else if (*opcode == "<=") {
          fluent = new Fluent(fluent->publish(URI_OR_BYTECODE(*args->at(0)), *args->at(1)));
        } else if (*opcode == "=>") {
          fluent = new Fluent(fluent->subscribe(URI_OR_BYTECODE(*args->at(0)), *args->at(1)));
        } */
        else {
          throw fError("Unknown instruction opcode: !b%s!!\n", opcode->c_str());
        }
        LOG(DEBUG, FOS_TAB "INST: %s\n", fluent->bcode->v_insts()->back()->toString().c_str());
      }
      // delete fluent;
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

    static const ptr<List<ptr<Obj>>> parseArgs(stringstream *ss) {
      const auto args = share(List<ptr<Obj>>());
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
        ptr<Obj> argObj = Parser::parseArg(ss);
        args->push_back(argObj);
        LOG(NONE, FOS_TAB_8 FOS_TAB_6 "!g==>!!%s [!y%s!!]\n", argObj->toString().c_str(),
            OTYPE_STR.at(argObj->otype()));
        if (ss->peek() == ',' || ss->peek() == '.' || ss->peek() == ')') {
          ss->get();
          break;
        }
      }
      return args;
    }

    static const ptr<Obj> parseArg(stringstream *ss) {
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

    template<typename OBJ = Obj>
    static const ptr<OBJ> parseObj(const string &token2) {
      char *array; //[token2.length()];
      array = strdup(token2.c_str());
      string token = string(array);
      ptr<Obj> obj_or_bcode;
      LOG(DEBUG, FOS_TAB_4 "!rTOKEN!!: %s\n", token.c_str());
      StringHelper::trim(token);
      int index = token.find('[');
      ptr<fURI> utype = ptr<fURI>((fURI *) nullptr);
      if (index != string::npos && index != 0 && token.back() == ']') {
        string typeToken = token.substr(0, index);
        // bool hasAuthority = typeToken.find('@') != std::string::npos;
        // bool hasSlash = typeToken.starts_with("/");
        utype = share(fURI(typeToken));
        token.pop_back();
        token = token.substr(index + 1);
        LOG(DEBUG, "\n" FOS_TAB_5 "!rtype token!!: %s\n" FOS_TAB_5 "!rvalue token!!: %s\n", utype->toString().c_str(),
            token.c_str());
      }
      if (strcmp(token.c_str(), "Ø") == 0) {
        utype = Type::obj_t(OType::NOOBJ, utype);
        obj_or_bcode = NoObj::self_ptr<NoObj>();
      } else if (token[0] == '\'' && token[token.length() - 1] == '\'') {
        utype = Type::obj_t(OType::STR, utype);
        obj_or_bcode = ptr<Str>(new Str(token.substr(1, token.length() - 2), utype));
        // might be wrong indices
      } else if (strcmp("true", token.c_str()) == 0 || strcmp("false", token.c_str()) == 0) {
        utype = Type::obj_t(OType::BOOL, utype);
        obj_or_bcode = ptr<Bool>(new Bool(strcmp("true", token.c_str()) == 0, utype));
      } else if (token[0] == '_' && token[1] == '_') {
        stringstream ss = stringstream(token);
        // utype = utype.get() ? utype : URI_BCODE;
        obj_or_bcode = ptr<Bytecode>(parseBytecode(&ss).get());
      } else if (token[token.length() - 1] == ')' && token.find('(')) {
        stringstream ss = stringstream(token);
        obj_or_bcode = ptr<Bytecode>(parseBytecode(&ss).get());
      } else if (token.find("=>") != string::npos /*token[0] == '[' && token[token.length() - 1] == ']'*/) {
        stringstream x = stringstream(token[0] == '[' ? token.substr(1, token.length() - 2) : token);
        stringstream *ss = &x;
        string first;
        OType type = OType::LST;
        while (!ss->eof()) {
          if (StringHelper::lookAhead("=>", ss)) {
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
        if (type == OType::REC) {
          RecMap<> map = RecMap<>();
          bool onKey = false;
          string key = first;
          string value;
          int bracketCounter = 0;
          int parenCounter = 0;
          while (!ss->eof()) {
            if (onKey) {
              if (bracketCounter == 0 && StringHelper::lookAhead("=>", ss)) {
                onKey = false;
              } else {
                if (ss->peek() == '[')
                  bracketCounter++;
                if (ss->peek() == ']')
                  bracketCounter--;
                if (ss->peek() == '(')
                  parenCounter++;
                if (ss->peek() == ')')
                  parenCounter--;
                if (!ss->eof())
                  key += ss->get();
              }
            } else {
              ///////
              if (bracketCounter == 0 && parenCounter == 0 && ss->peek() == ',') {
                ss->get(); // drop k/v separating comma
                onKey = true;
                map.insert({parseObj(key), parseObj(value)});
                key.clear();
                value.clear();
              } else {
                if (ss->peek() == '[')
                  bracketCounter++;
                if (ss->peek() == ']')
                  bracketCounter--;
                if (ss->peek() == '(')
                  parenCounter++;
                if (ss->peek() == ')')
                  parenCounter--;
                if (!ss->eof())
                  value += ss->get();
              }
            }
          }
          map.insert({parseObj(key), parseObj(value)});
          RecMap<> map2 = RecMap<>(); // necessary to revese entries
          for (const auto &pair: map) {
            map2.insert(pair);
          }
          utype = Type::obj_t(OType::REC, utype);
          obj_or_bcode = ptr<Rec>(new Rec(map2, utype));
        }
      } else if (((token[0] == '-' && isdigit(token[1])) || isdigit(token[0])) && token.find('.') != string::npos) {
        utype = Type::obj_t(OType::REAL, utype);
        obj_or_bcode = ptr<Real>(new Real(stof(token), utype));
      } else if ((token[0] == '-' && isdigit(token[1])) || isdigit(token[0])) {
        utype = Type::obj_t(OType::INT, utype);
        obj_or_bcode = ptr<Int>(new Int(stoi(token), utype));
      } else {
        utype = Type::obj_t(OType::URI, utype);
        obj_or_bcode = ptr<Uri>(new Uri(fURI(token), utype));
      }
      delete array;
      ptr<OBJ> temp = std::dynamic_pointer_cast<OBJ>(obj_or_bcode);
      LOG(DEBUG, FOS_TAB_5 "%s: !yvar!![%s] !yotype!![%s] !ystype!![%s] !yutype!![%s]\n", temp->toString().c_str(),
          temp->type()->variable().c_str(), OTYPE_STR.at(temp->otype()), temp->type()->stype().c_str(),
          temp->type()->name().c_str());
      return temp;
    }
  };
} // namespace fhatos
#endif
