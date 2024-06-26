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
#include <regex>
#include <sstream>
#include <util/string_helper.hpp>

namespace fhatos {
  using namespace std;

  class Parser {
  public:
    Parser() = delete;
    static Option<Obj_p> tryParseObj(const string &token) {
      StringHelper::trim(token);
      LOG(DEBUG_MORE, "!RPARSING!!: !g!_%s!!\n", token.c_str());
      if (token.empty())
        return {};
      const Pair<string, string> typeValue = tryParseObjType(token);
      const string typeToken = typeValue.first;
      const string valueToken = typeValue.second;
      Option<Obj_p> b;
      b = tryParseNoObj(valueToken, typeToken, NOOBJ_FURI);
      if (b.has_value())
        return b.value();
      b = tryParseBool(valueToken, typeToken, BOOL_FURI);
      if (b.has_value())
        return b.value();
      b = tryParseInt(valueToken, typeToken, INT_FURI);
      if (b.has_value())
        return b.value();
      b = tryParseReal(valueToken, typeToken, REAL_FURI);
      if (b.has_value())
        return b.value();
      b = tryParseStr(valueToken, typeToken, STR_FURI);
      if (b.has_value())
        return b.value();
      b = tryParseRec(valueToken, typeToken, REC_FURI);
      if (b.has_value())
        return b.value();
      b = tryParseBCode(valueToken, typeToken, BCODE_FURI);
      if (b.has_value())
        return b.value();
      b = tryParseUri(valueToken, typeToken, URI_FURI);
      if (b.has_value())
        return b.value();
      return {};
    }

    static Pair<string, string> tryParseObjType(const string &token, const bool brackets = true) {
      string typeToken;
      string valueToken;
      if (!token.empty() && token[token.length() - 1] == (brackets ? ']' : ')')) {
        bool onType = true;
        auto ss = stringstream(token);
        while (!ss.eof()) {
          char c = ss.get();
          if (onType) {
            if (c == (brackets ? '[' : '(')) {
              onType = false;
            } else {
              typeToken += c;
            }
          } else {
            valueToken += c;
          }
        }
        valueToken = typeToken.empty() ? token : valueToken.substr(0, valueToken.length() - 2);
      } else {
        valueToken = token;
      }
      StringHelper::trim(typeToken);
      StringHelper::trim(valueToken);
      LOG(DEBUG_MORE, "\n" FOS_TAB_2 "!rtype token!!: %s\n" FOS_TAB_2 "!rvalue token!!: %s\n", typeToken.c_str(),
          valueToken.c_str());
      return {typeToken, valueToken};
    }

    static Option<NoObj_p> tryParseNoObj(const string &valueToken, const string &typeToken,
                                         const fURI_p &baseType = NOOBJ_FURI) {
      LOG(DEBUG_MORE, "Attempting noobj parse on %s\n", valueToken.c_str());
      return valueToken == "Ø" ? Option<NoObj_p>{NoObj::to_noobj()} : Option<NoObj_p>{};
    }

    static Option<Bool_p> tryParseBool(const string &valueToken, const string &typeToken,
                                       const fURI_p &baseType = BOOL_FURI) {
      LOG(DEBUG_MORE, "Attempting bool parse on %s\n", valueToken.c_str());
      return ((strcmp("true", valueToken.c_str()) == 0) || (strcmp("false", valueToken.c_str()) == 0))
                 ? Option<Bool_p>{Bool::to_bool(strcmp("true", valueToken.c_str()) == 0,
                                                share(baseType->resolve(typeToken.c_str())))}
                 : Option<Bool_p>{};
    }
    static Option<Int_p> tryParseInt(const string &valueToken, const string &typeToken,
                                     const fURI_p &baseType = INT_FURI) {
      LOG(DEBUG_MORE, "Attempting int parse on %s\n", valueToken.c_str());
      if ((valueToken[0] != '-' && !isdigit(valueToken[0])) || valueToken.find('.') != string::npos)
        return {};
      for (int i = 1; i < valueToken.length(); i++) {
        if (!isdigit(valueToken[i]))
          return {};
      }
      return Option<Int_p>{Int::to_int(stoi(valueToken), share(baseType->resolve(typeToken.c_str())))};
    }
    static Option<Real_p> tryParseReal(const string &valueToken, const string &typeToken,
                                       const fURI_p &baseType = REAL_FURI) {
      LOG(DEBUG_MORE, "Attempting real parse on %s\n", valueToken.c_str());
      if ((valueToken[0] != '-' && !isdigit(valueToken[0])) || valueToken.find('.') == string::npos)
        return {};
      for (int i = 1; i < valueToken.length(); i++) {
        if (valueToken[i] != '.' && !isdigit(valueToken[i]))
          return {};
      }
      return Option<Real_p>{Real::to_real(stof(valueToken), share(baseType->resolve(typeToken.c_str())))};
    }
    static Option<Uri_p> tryParseUri(const string &valueToken, const string &typeToken,
                                     const fURI_p &baseType = URI_FURI) {
      LOG(DEBUG_MORE, "Attempting uri parse on %s\n", valueToken.c_str());
      return Option<Uri_p>{Uri::to_uri(valueToken, share(baseType->resolve(typeToken.c_str())))};
    }
    static Option<Str_p> tryParseStr(const string &token, const string &type, const fURI_p &baseType = STR_FURI) {
      LOG(DEBUG_MORE, "Attempting str parse on %s\n", token.c_str());
      return (token[0] == '\'' && token[token.length() - 1] == '\'')
                 ? Option<Uri_p>{Str::to_str(token.substr(1, token.length() - 2),
                                             share(baseType->resolve(type.c_str())))}
                 : Option<Uri_p>{};
    }
    static Option<Rec_p> tryParseRec(const string &token, const string &type, const fURI_p &baseType = REC_FURI) {
      LOG(DEBUG_MORE, "Attempting rec parse on %s\n", token.c_str());
      if (token[0] != '[' || token[token.length() - 1] != ']' || token.find("=>") == string::npos)
        return {};
      auto ss = stringstream(token.substr(1, token.length() - 2));
      string key;
      string value;
      while (!ss.eof()) {
        if (StringHelper::lookAhead("=>", &ss)) {
          break;
        } else if (ss.peek() == ',') {
          ss.get();
          break;
        }
        key += ss.get();
      }
      StringHelper::trim(key);
      //////////////////////////////////// {
      Obj::RecMap<> map = Obj::RecMap<>();
      bool onKey = false;

      int bracketCounter = 0;
      int parenCounter = 0;
      while (!ss.eof()) {
        if (onKey) {
          if (bracketCounter == 0 && StringHelper::lookAhead("=>", &ss)) {
            onKey = false;
          } else {
            if (ss.peek() == '[')
              bracketCounter++;
            if (ss.peek() == ']')
              bracketCounter--;
            if (ss.peek() == '(')
              parenCounter++;
            if (ss.peek() == ')')
              parenCounter--;
            if (!ss.eof())
              key += (char) ss.get();
          }
        } else {
          ///////
          if (bracketCounter == 0 && parenCounter == 0 && ss.peek() == ',') {
            ss.get(); // drop k/v separating comma
            onKey = true;
            StringHelper::trim(key);
            StringHelper::trim(value);
            map.insert({Parser::tryParseObj(key).value(), Parser::tryParseObj(value).value()});
            key.clear();
            value.clear();
          } else {
            if (ss.peek() == '[')
              bracketCounter++;
            if (ss.peek() == ']')
              bracketCounter--;
            if (ss.peek() == '(')
              parenCounter++;
            if (ss.peek() == ')')
              parenCounter--;
            if (!ss.eof())
              value += (char) ss.get();
          }
        }
      }
      StringHelper::trim(key);
      StringHelper::trim(value);
      map.insert({Parser::tryParseObj(key).value(), Parser::tryParseObj(value).value()});
      ////
      Obj::RecMap_p<> map2 = share(Obj::RecMap<>()); // necessary to reverse entries
      for (const auto &pair: map) {
        map2->insert(pair);
      }
      ////
      return Option<Rec_p>{Rec::to_rec(map2, share(baseType->resolve(type.c_str())))};
    }
    static Option<Inst_p> tryParseInst(const string &valueToken, const string &typeToken,
                                       const fURI_p &baseType = INST_FURI) {
      LOG(DEBUG_MORE, "Attempting inst parse on %s\n", valueToken.c_str());
      auto args = List<ptr<Obj>>();
      stringstream ss = stringstream(valueToken);
      while (!ss.eof()) {
        string argToken;
        int paren = 0;
        int bracket = 0;
        while (!ss.eof()) {
          if (ss.peek() == '(')
            paren++;
          else if (ss.peek() == ')')
            paren--;
          else if (ss.peek() == '[') {
            bracket++;
          } else if (ss.peek() == ']') {
            bracket--;
          }
          char temp = ss.get();
          if (ss.eof())
            argToken = argToken.substr(0, argToken.length() - 2);
          else {
            argToken += temp;
            if (ss.peek() == ',' && paren == 0 && bracket == 0) {
              ss.get(); // drop arg separating comma
              break;
            }
          }
        }
        if (!argToken.empty()) {
          Option<Obj_p> arg_p = Parser::tryParseObj(argToken);
          if (arg_p.has_value())
            args.push_back(arg_p.value());
          else {
            LOG(ERROR, "Unable to parse %s inst argument: %s\n", typeToken.c_str(), argToken.c_str());
          }
        }
      }
      return Option<Inst_p>(Insts::to_inst(baseType->resolve(typeToken.c_str()), args));
    }

    static Option<BCode_p> tryParseBCode(const string &valueToken, const string &typeToken,
                                         const fURI_p &baseType = BCODE_FURI) {
      LOG(DEBUG_MORE, "Attempting bcode parse on %s\n", valueToken.c_str());
      if ((valueToken[0] == '_' && valueToken[1] == '_') ||
          (valueToken[valueToken.length() - 1] == ')' && valueToken.find('('))) {
        List<Inst_p> insts;
        auto ss = stringstream(valueToken);
        while (!ss.eof()) {
          int paren = 0;
          int bracket = 0;
          string instToken;
          while (!ss.eof()) {
            char c = ss.get();
            instToken += c;
            if (c == '(') {
              paren++;
            } else if (c == ')') {
              paren--;
              if (paren == 0 && bracket == 0) {
                break;
              }
            } else if (c == '[') {
              bracket++;
            } else if (c == ']') {
              bracket--;
              if (paren == 0 && bracket == 0) {
                break;
              }
            }
          }
          const Pair<string, string> typeValue = tryParseObjType(instToken, false);
          Option<Inst_p> inst = tryParseInst(typeValue.second, typeValue.first, INST_FURI);
          if (inst.has_value()) {
            insts.push_back(inst.value());
          }
          if (ss.peek() == '.' || ss.peek() == '*') {
            ss.get(); // drop instruction mult
          }
        }
        return Option<BCode_p>{BCode::to_bcode(insts, share(baseType->resolve(typeToken.c_str())))};
      } else {
        return {};
      }
    }
  };
} // namespace fhatos
#endif
