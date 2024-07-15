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
#pragma once
#ifndef fhatos_parser_hpp
#define fhatos_parser_hpp

#include <fhatos.hpp>
#include <language/insts.hpp>
#include <language/obj.hpp>
#include <sstream>
#include <util/string_helper.hpp>

namespace fhatos {
  using namespace std;

  class Parser final : public Coroutine {
  private:
    explicit Parser(const ID &id = ID("/parser/")) : Coroutine(id) {}

  public:
    static Parser *singleton(const ID &id = ID("/parser/")) {
      static Parser parser =  Parser(id);
      return &parser;
    }

    void setup() override {
      Coroutine::setup();
      TYPE_PARSER = [](const string &bcode) {
        return Parser::singleton()->tryParseObj(bcode).value_or(Obj::to_noobj());
      };
    }

    static bool closedExpression(const string &line) {
      auto ss = stringstream(line);
      uint8_t parens = 0;
      uint8_t brackets = 0;
      bool quotes = false;
      while (!ss.eof()) {
        char c = ss.get();
        if (c == '(' && !quotes)
          parens++;
        else if (c == ')' && !quotes)
          parens--;
        else if (c == '[' && !quotes)
          brackets++;
        else if (c == ']' && !quotes)
          brackets--;
        else if (c == '\'')
          quotes = !quotes;
      }
      return parens == 0 && brackets == 0 && !quotes;
    }
    static bool dotType(const string &type) {
      return !type.empty() && type[type.length() - 1] == '.'; // dot type
    }
    Option<Obj_p> tryParseObj(const string &token) {
      //
      StringHelper::trim(token);
      // LOG(TRACE, "!RPARSING!!: !g!_%s!!\n", token.c_str());
      if (token.empty())
        return {};
      const Pair<string, string> typeValue = tryParseObjType(token);
      const string typeToken = typeValue.first;
      const string valueToken = typeValue.second;
      const bool dot_type = dotType(typeToken);
      Option<Obj_p> b;
      if (!dot_type) { // dot type
        b = tryParseNoObj(valueToken);
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
        b = tryParseUri(valueToken, typeToken, URI_FURI);
        if (b.has_value())
          return b.value();
        b = tryParseStr(valueToken, typeToken, STR_FURI);
        if (b.has_value())
          return b.value();
        b = tryParseLst(valueToken, typeToken, LST_FURI);
        if (b.has_value())
          return b.value();
        b = tryParseRec(valueToken, typeToken, REC_FURI);
        if (b.has_value())
          return b.value();
      }
      b = dot_type ? tryParseBCode(token, "", BCODE_FURI) : tryParseBCode(valueToken, typeToken, BCODE_FURI);
      if (b.has_value())
        return b.value();
      b = tryParseDEFAULT(valueToken, typeToken);
      if (b.has_value())
        return b.value();
      return {};
    }

    Pair<string, string> tryParseObjType(const string &token, const bool brackets = true) {
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
        typeToken = "";
        valueToken = token;
      }
      StringHelper::trim(typeToken);
      StringHelper::trim(valueToken);
      LOG(TRACE, "\n" FOS_TAB_2 "!rtype token!!: %s\n" FOS_TAB_2 "!rvalue token!!: %s\n", typeToken.c_str(),
          valueToken.c_str());
      return {typeToken, valueToken};
    }

    Option<NoObj_p> tryParseNoObj(const string &valueToken) {
      // LOG(TRACE, "Attempting noobj parse on %s\n", valueToken.c_str());
      return valueToken == "Ø" ? Option<NoObj_p>{NoObj::to_noobj()} : Option<NoObj_p>{};
    }

    Option<Bool_p> tryParseBool(const string &valueToken, const string &typeToken, const fURI_p &baseType = BOOL_FURI) {
      // LOG(TRACE, "Attempting bool parse on %s\n", valueToken.c_str());
      return ((strcmp("true", valueToken.c_str()) == 0) || (strcmp("false", valueToken.c_str()) == 0))
                 ? Option<Bool_p>{Bool::to_bool(strcmp("true", valueToken.c_str()) == 0,
                                                share(baseType->resolve(typeToken.c_str())))}
                 : Option<Bool_p>{};
    }
    Option<Int_p> tryParseInt(const string &valueToken, const string &typeToken, const fURI_p &baseType = INT_FURI) {
      // LOG(TRACE, "Attempting int parse on %s\n", valueToken.c_str());
      if ((valueToken[0] != '-' && !isdigit(valueToken[0])) ||
          1 == std::count(valueToken.begin(), valueToken.end(), '.'))
        return {};
      for (int i = 1; i < valueToken.length(); i++) {
        if (!isdigit(valueToken[i]))
          return {};
      }
      return Option<Int_p>{Int::to_int(stoi(valueToken), share(baseType->resolve(typeToken.c_str())))};
    }
    Option<Real_p> tryParseReal(const string &valueToken, const string &typeToken, const fURI_p &baseType = REAL_FURI) {
      // LOG(TRACE, "Attempting real parse on %s\n", valueToken.c_str());
      if (valueToken[0] != '-' && !isdigit(valueToken[0]))
        return {};
      bool dotFound = false;
      for (int i = 1; i < valueToken.length(); i++) {
        if (valueToken[i] == '.') {
          if (dotFound)
            return {};
          dotFound = true;
        }
        if (valueToken[i] != '.' && !isdigit(valueToken[i]))
          return {};
      }
      return dotFound ? Option<Real_p>{Real::to_real(stof(valueToken), share(baseType->resolve(typeToken.c_str())))}
                      : Option<Real_p>{};
    }
    Option<Uri_p> tryParseUri(const string &valueToken, const string &typeToken, const fURI_p &baseType = URI_FURI) {
      // LOG(TRACE, "Attempting uri parse on %s\n", valueToken.c_str());
      return (valueToken[0] == '<' && valueToken[valueToken.length() - 1] == '>')
                 ? Option<Uri_p>{Uri::to_uri(valueToken.substr(1, valueToken.length() - 2),
                                             share(baseType->resolve(typeToken.c_str())))}
                 : Option<Uri_p>{};
    }

    Option<Str_p> tryParseStr(const string &token, const string &type, const fURI_p &baseType = STR_FURI) {
      // LOG(TRACE, "Attempting str parse on %s\n", token.c_str());
      return (token[0] == '\'' && token[token.length() - 1] == '\'')
                 ? Option<Uri_p>{Str::to_str(token.substr(1, token.length() - 2),
                                             share(baseType->resolve(type.c_str())))}
                 : Option<Uri_p>{};
    }
    Option<Lst_p> tryParseLst(const string &token, const string &type, const fURI_p &baseType = LST_FURI) {
      // LOG(TRACE, "Attempting lst parse on %s\n", token.c_str());
      if (token[0] != '[' || token[token.length() - 1] != ']')
        return {};
      auto ss = stringstream(token.substr(1, token.length() - 2));
      string value;
      Obj::LstList<> list = Obj::LstList<>();
      int bracketCounter = 0;
      int parenCounter = 0;
      while (!ss.eof()) {
        if (bracketCounter == 0 && ss.peek() == ',') {
          Option<Obj_p> element = Parser::tryParseObj(value);
          if(!element.has_value())
            return {};
          list.push_back(element.value());
          ss.get();
          value.clear();
        } else if (bracketCounter == 0 && StringHelper::lookAhead("=>", &ss)) {
          return {};
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
      StringHelper::trim(value);
      Option<Obj_p> element = Parser::tryParseObj(value);
      if(!element.has_value())
        return {};
      list.push_back(element.value());
      return Option<Lst_p>{Lst::to_lst(share(list), share(baseType->resolve(type.c_str())))};
    }
    Option<Rec_p> tryParseRec(const string &token, const string &type, const fURI_p &baseType = REC_FURI) {
      // LOG(TRACE, "Attempting rec parse on %s\n", token.c_str());
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
    Option<Inst_p> tryParseInst(const string &valueToken, const string &typeToken, const fURI_p &baseType = INST_FURI) {
      // LOG(TRACE, "Attempting inst parse on %s\n", valueToken.c_str());
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
          const char temp = ss.get();
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
          const Option<Obj_p> arg_p = Parser::tryParseObj(argToken);
          if (arg_p.has_value())
            args.push_back(arg_p.value());
          else {
            LOG(ERROR, "Unable to parse %s inst argument: %s\n", typeToken.c_str(), argToken.c_str());
          }
        }
      }
      try {
        const Inst_p inst = Insts::to_inst(baseType->resolve(typeToken.c_str()), args);
        return inst->isNoObj() ? Option<Inst_p>() : Option<Inst_p>(inst);
      } catch (const fError &) {
        return {};
      }
    }

    Option<BCode_p> tryParseBCode(const string &valueToken, const string &typeToken,
                                  const fURI_p &baseType = BCODE_FURI) {
      // LOG(TRACE, "Attempting bcode parse on %s\n", valueToken.c_str());
      if (typeToken.empty() && valueToken == "_")
        return {Obj::to_bcode({})}; // special character for 'no instructions' (no common parse pattern)
      if ((valueToken[0] == '_' && valueToken[1] == '_') || //
          valueToken.find('.') != string::npos || //
          valueToken.find('*') != string::npos || //
          (valueToken.find('(') != string::npos && valueToken.find(')') != string::npos)) {
        List<Inst_p> insts;
        auto ss = stringstream(valueToken);
        while (!ss.eof()) {
          int paren = 0;
          int bracket = 0;
          bool quote = false;
          string instToken;
          while (!ss.eof()) {
            char c = ss.get();
            instToken += c;
            if (c == '(')
              paren++;
            else if (c == ')')
              paren--;
            else if (c == '[')
              bracket++;
            else if (c == ']')
              bracket--;
            else if (c == '\'')
              quote = !quote;
            ///////////////////////////////////////////////////////////////
            if (paren == 0 && bracket == 0 && !quote) {
              // if (ss.peek() == '+') {
              //   break;
              // }
              if (ss.peek() == '.') {
                ss.get();
                break;
              }
            }
          }
          if (instToken.empty())
            continue;
          if (instToken[instToken.length() - 1] == '\0')
            instToken = instToken.substr(0, instToken.length() - 1);
          /// parse a * dereference and wrap in a from() ??
          Pair<string, string> typeValue;
          if (instToken.length() > 1 && instToken[0] == '*' && instToken[1] != '(')
            typeValue = {"*", instToken.substr(1)};
          else if (instToken.length() > 1 && instToken[0] == '+' && instToken[1] != '(')
            typeValue = {"+", instToken.substr(1)};
          else if (typeToken.empty() || typeToken[typeToken.length() - 1] != '.')
            typeValue = tryParseObjType(instToken, false);
          ///////////// parse an obj and wrap in a start() ??
          if (instToken[0] != '*' && instToken[typeValue.first.length()] != '(') {
            insts.push_back(insts.empty() ? Insts::start(Obj::to_objs({tryParseObj(instToken).value()}))
                                          : Insts::map(tryParseObj(instToken).value()));
          } else {
            /////////// parse an instruction
            const Option<Inst_p> inst = tryParseInst(typeValue.second, typeValue.first, INST_FURI);
            if (inst.has_value()) {
              insts.push_back(inst.value());
            } else {
              return {};
            }
          }
        }
        return Option<BCode_p>{BCode::to_bcode(insts, share(baseType->resolve(typeToken.c_str())))};
      } else {
        return {};
      }
    }

    Option<Uri_p> tryParseDEFAULT(const string &valueToken, const string &typeToken,
                                  const fURI_p &baseType = URI_FURI) {
      // LOG(TRACE, "Attempting uri parse on %s\n", valueToken.c_str());
      return Option<Uri_p>{Uri::to_uri(valueToken, share(baseType->resolve(typeToken.c_str())))};
    }
  };
} // namespace fhatos
#endif
