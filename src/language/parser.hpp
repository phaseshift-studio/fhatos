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
#include <sstream>
#include <util/string_helper.hpp>

namespace fhatos {
  using namespace std;

  class Parser {
  public:
    // explicit Parser(const ID &id = ID(*UUID::singleton()->mint())) : IDed(id) {}
    Parser() = delete;
    static Option<Obj_p> tryParseObj(const string &token) {
      const string cleanToken = string(token);
      StringHelper::trim(cleanToken);
      LOG(DEBUG, "!RPARSING!!: !g!_%s!!\n", cleanToken.c_str());
      if (cleanToken.empty())
        return {};
      Pair<string, string> typeValue = tryParseObjType(token);
      string type = typeValue.first;
      string value = typeValue.second;
      Option<Obj_p> b;
      b = tryParseNoObj(value, type, NOOBJ_FURI);
      if (b.has_value())
        return b.value();
      b = tryParseBool(value, type, BOOL_FURI);
      if (b.has_value())
        return b.value();
      b = tryParseInt(value, type, INT_FURI);
      if (b.has_value())
        return b.value();
      b = tryParseReal(value, type, REAL_FURI);
      if (b.has_value())
        return b.value();
      b = tryParseStr(value, type, STR_FURI);
      if (b.has_value())
        return b.value();
      b = tryParseRec(value, type, REC_FURI);
      if (b.has_value())
        return b.value();
      b = tryParseBCode(value, type, BCODE_FURI);
      if (b.has_value())
        return b.value();
      b = tryParseUri(value, type, URI_FURI);
      if (b.has_value())
        return b.value();
      return {};
    }

    static Pair<string, string> tryParseObjType(const string &token, const bool brackets = true) {
      const size_t s_index = token.find_first_of(brackets ? '[' : '(') + 1;
      const size_t e_index = s_index > 1 && token[token.length() - 1] == (brackets ? ']' : ')') ? 2 : 0;
      // LOG(DEBUG, "%i -- %i : %s\n", s_index, e_index, token.c_str());
      const string type = token.substr(0, s_index == 0 ? 0 : s_index - 1);
      const size_t len = type.empty() ? token.length() : token.length() - (type.length() + e_index);
      const string value = token.substr(s_index > 1 ? s_index : 0, len);
      return {type, value};
    }

    static Option<NoObj_p> tryParseNoObj(const string &token, const string &type, const fURI_p &baseType = NOOBJ_FURI) {
      LOG(DEBUG, "Attempting noobj parse on %s\n", token.c_str());
      return token == "Ø" ? Option<NoObj_p>{NoObj::to_noobj()} : Option<NoObj_p>{};
    }

    static Option<Bool_p> tryParseBool(const string &token, const string &type, const fURI_p &baseType = BOOL_FURI) {
      LOG(DEBUG, "Attempting bool parse on %s\n", token.c_str());
      return ((strcmp("true", token.c_str()) == 0) || (strcmp("false", token.c_str()) == 0))
                 ? Option<Bool_p>{Bool::to_bool(strcmp("true", token.c_str()) == 0,
                                                share(baseType->resolve(type.c_str())))}
                 : Option<Bool_p>{};
    }
    static Option<Int_p> tryParseInt(const string &token, const string &type, const fURI_p &baseType = INT_FURI) {
      LOG(DEBUG, "Attempting int parse on %s\n", token.c_str());
      if ((token[0] != '-' && !isdigit(token[0])) || token.find('.') != string::npos)
        return {};
      for (int i = 1; i < token.length(); i++) {
        if (!isdigit(token[i]))
          return {};
      }
      return Option<Int_p>{Int::to_int(stoi(token), share(baseType->resolve(type.c_str())))};
    }
    static Option<Real_p> tryParseReal(const string &token, const string &type, const fURI_p &baseType = REAL_FURI) {
      LOG(DEBUG, "Attempting real parse on %s\n", token.c_str());
      if ((token[0] != '-' && !isdigit(token[0])) || token.find('.') == string::npos)
        return {};
      for (int i = 1; i < token.length(); i++) {
        if (token[i] != '.' && !isdigit(token[i]))
          return {};
      }
      return Option<Real_p>{Real::to_real(stof(token), share(baseType->resolve(type.c_str())))};
    }
    static Option<Uri_p> tryParseUri(const string &token, const string &type, const fURI_p &baseType = URI_FURI) {
      LOG(DEBUG, "Attempting uri parse on %s\n", token.c_str());
      return Option<Uri_p>{Uri::to_uri(token, share(baseType->resolve(type.c_str())))};
    }
    static Option<Str_p> tryParseStr(const string &token, const string &type, const fURI_p &baseType = STR_FURI) {
      LOG(DEBUG, "Attempting str parse on %s\n", token.c_str());
      return (token[0] == '\'' && token[token.length() - 1] == '\'')
                 ? Option<Uri_p>{Str::to_str(token.substr(1, token.length() - 2),
                                             share(baseType->resolve(type.c_str())))}
                 : Option<Uri_p>{};
    }
    static Option<Rec_p> tryParseRec(const string &token, const string &type, const fURI_p &baseType = REC_FURI) {
      LOG(DEBUG, "Attempting rec parse on %s\n", token.c_str());
      if (token[0] != '[' || token[token.length() - 1] != ']' || token.find("=>") == string::npos)
        return {};
      auto *ss = new stringstream(token.substr(1, token.length() - 2));
      string first;
      while (!ss->eof()) {
        if (StringHelper::lookAhead("=>", ss)) {
          break;
        } else if (ss->peek() == ',') {
          ss->get();
          break;
        }
        first += ss->get();
      }
      StringHelper::trim(first);
      //////////////////////////////////// {
      Obj::RecMap<> map = Obj::RecMap<>();
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
              key += (char) ss->get();
          }
        } else {
          ///////
          if (bracketCounter == 0 && parenCounter == 0 && ss->peek() == ',') {
            ss->get(); // drop k/v separating comma
            onKey = true;
            StringHelper::trim(key);
            StringHelper::trim(value);
            map.insert({Parser::tryParseObj(key).value(), Parser::tryParseObj(value).value()});
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
              value += (char) ss->get();
          }
        }
      }
      StringHelper::trim(key);
      StringHelper::trim(value);
      map.insert({Parser::tryParseObj(key).value(), Parser::tryParseObj(value).value()});
      Obj::RecMap_p<> map2 = share(Obj::RecMap<>()); // necessary to reverse entries
      for (const auto &pair: map) {
        map2->insert(pair);
      }
      delete ss;
      return Option<Rec_p>{Rec::to_rec(map2, share(baseType->resolve(type.c_str())))};
    }
    static Option<Inst_p> tryParseInst(const string &token, const string &type, const fURI_p baseType = INST_FURI) {
      LOG(DEBUG, "Attempting inst parse on %s\n", token.c_str());
      string ttype = type;
      if (type.length() > 1 && type[0] == '_' && type[1] == '_') {
        ttype = "start";
      }
      LOG(DEBUG, FOS_TAB_2 "!rOPCODE!!: %s\n", ttype.c_str());
      auto args = List<ptr<Obj>>();
      stringstream *ss = new stringstream(token);
      while (std::isspace(ss->peek())) {
        ss->get();
      }
      bool done = false;
      if (ss->peek() == '(') {
        ss->get(); // (
        if (ss->peek() == ')') {
          ss->get();
          if (ss->peek() == '.')
            ss->get();
          done = true;
        }
      } else if (ss->peek() == '.') {
        ss->get();
        done = true;
      }
      if (!done) {
        while (!ss->eof()) {
          Obj_p argObj;
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
          Option<Obj_p> arg_p = Parser::tryParseObj(arg);
          if (arg_p.has_value())
            args.push_back(arg_p.value());
          if (ss->peek() == ',' || ss->peek() == '.' || ss->peek() == ')') {
            ss->get();
            break;
          }
        }
      }
      return Option<Inst_p>(Insts::to_inst(baseType->resolve(ttype.c_str()), args));
    }

    static Option<BCode_p> tryParseBCode(const string &token, const string &type, const fURI_p &baseType = BCODE_FURI) {
      LOG(DEBUG, "Attempting bcode parse on %s\n", token.c_str());
      if ((token[0] == '_' && token[1] == '_') || (token[token.length() - 1] == ')' && token.find('('))) {
        List<Inst_p> insts;
        auto ss = stringstream(token);
        char **instTokens = new char *[150];
        int length = private_fhatos::split(token.c_str(), ".", instTokens);
        for (int i = 0; i < length; i++) {
          const Pair<string, string> typeValue = tryParseObjType(instTokens[i], false);
          Option<Inst_p> inst = tryParseInst(typeValue.second, typeValue.first, INST_FURI);
          if (inst.has_value()) {
            insts.push_back(inst.value());
          }
        }
        return Option<BCode_p>{BCode::to_bcode(insts, share(baseType->resolve(type.c_str())))};
      } else {
        return {};
      }
    }
  };
} // namespace fhatos
#endif
