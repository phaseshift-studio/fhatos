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
#include <process/actor/actor.hpp>
#include <sstream>
#include <structure/stype/empty.hpp>
#include <util/string_helper.hpp>

namespace fhatos {
  using namespace std;

  struct Tracker {
    uint8_t parens = 0;
    uint8_t brackets = 0;
    uint8_t angles = 0;
    uint8_t braces = 0;
    bool quotes = false;
    char last = '\0';

    char track(const char c) {
      if (c != '\'' && quotes) {
        // do nothing
      } else if (c == '\'') {
        quotes = !quotes;
      } else if ((c == '=' || c == '-')) {
        if (last == '<')
          angles--;
      } else if (c == '(')
        parens++;
      else if (c == ')')
        parens--;
      else if (c == '[')
        brackets++;
      else if (c == ']')
        brackets--;
      else if (c == '<')
        angles++;
      else if (c == '>') {
        if (last != '-' && last != '=')
          angles--;
      } else if (c == '{')
        braces++;
      else if (c == '}')
        braces--;
      //////////////////////////
      last = c;
      return c;
    }
    [[nodiscard]] bool printable() const { return last >= 32 && last < 127; }
    [[nodiscard]] bool closed() const { return parens == 0 && brackets == 0 && angles == 0 && braces == 0 && !quotes; }
  };

  class Parser final : public Actor<Coroutine, Empty> {
  private:
    explicit Parser(const ID &id = ID("/parser/")) : Actor(id) {}
    enum class PARSE_TOKENS { BRACKET, PAREN };

  public:
    static Parser *singleton(const ID &id = ID("/parser/")) {
      static Parser parser = Parser(id);
      static bool _setup = false;
      if (!_setup) {
        TYPE_PARSER = [](const string &bcode) {
          try {
            return Parser::singleton()->tryParseObj(bcode).value_or(Obj::to_noobj());
          } catch (std::exception &e) {
            LOG_EXCEPTION(e);
            return Obj::to_noobj();
          }
        };
        Options::singleton()->parser<Obj>(TYPE_PARSER);
        _setup = true;
      }
      return &parser;
    }

    static bool closedExpression(const string &line) {
      auto ss = stringstream(line);
      Tracker tracker;
      while (!ss.eof()) {
        tracker.track(static_cast<char>(ss.get()));
      }
      return tracker.closed() && StringHelper::countSubstring(line, "===") % 2 == 0;
    }
    static bool dotType(const string &type) {
      return !type.empty() && type[type.length() - 1] == '.'; // dot type
    }
    Option<Obj_p> tryParseObj(const string &token, const string &prev = "NONE") {
      if (token == prev)
        throw fError("Unable to parse !y%s!!\n", token.c_str());
      StringHelper::trim(token);
      if (token.empty() || tryParseComment(token).has_value())
        return {};
      const Pair<string, string> typeValue = tryParseObjType(token);
      const string typeToken = typeValue.first;
      const string valueToken = typeValue.second;
      const bool dot_type = dotType(typeToken); // .obj. in _bcode (apply)
      Option<Obj_p> b = {};
      if (!dot_type) { // dot type
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
      b = dot_type ? tryParseBCode(token, EMPTY_CHARS, BCODE_FURI) : tryParseBCode(valueToken, typeToken, BCODE_FURI);
      if (b.has_value())
        return b.value();
      b = tryParseDEFAULT(valueToken, typeToken);
      if (b.has_value())
        return b.value();
      return {};
    }

    Pair<string, string> tryParseObjType(const string &token, const PARSE_TOKENS grouping = PARSE_TOKENS::BRACKET) {
      string typeToken;
      string valueToken;
      if (!token.empty() && token[token.length() - 1] == (PARSE_TOKENS::BRACKET == grouping ? ']' : ')')) {
        bool onType = true;
        auto ss = stringstream(token);
        /////////////////////////////////////////////////////
        // LOOK FOR SYNTACTIC SUGARS ON UNARY INSTRUCTIONS //
        for (const auto &[k, v]: Insts::unarySugars()) {
          if (StringHelper::lookAhead(k, &ss)) {
            valueToken.append(k);
            onType = false;
            break;
          }
        }
        ////////////////////////////////////////////////////
        while (!ss.eof()) {
          char c = static_cast<char>(ss.get());
          if (onType) {
            if (c == (PARSE_TOKENS::BRACKET == grouping ? '[' : '(')) {
              onType = false;
            } else if (c == '.') {
              valueToken.append(typeToken);
              valueToken += c;
              typeToken.clear();
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
      if (Options::singleton()->log_level<LOG_TYPE>() <= TRACE) {
        LOG(TRACE, "!ytype token!!: !g%s!!" FOS_TAB_3 "!yvalue token!!: !g%s!!\n", typeToken.c_str(),
            valueToken.c_str());
        char t = typeToken[typeToken.length() - 1];
        char v = valueToken[valueToken.length() - 1];
        LOG(TRACE, FOS_TAB_2 "!ytypeToken!! [!glast char!!]: (!rdec!!) %i (!rhex!!) 0x%x (!rchar!!) %c\n", t, t, t);
        LOG(TRACE, FOS_TAB_2 "!yvalueToken!! [!glast char!!]: (!rdec!!) %i (!rhex!!) 0x%x (!rchar!!) %c\n", v, v, v);
      }
      return {typeToken, valueToken};
    }

    Option<NoObj_p> tryParseComment(const string &valueToken) {
      return valueToken.substr(0, 3) == "---" ? Option<NoObj_p>{NoObj::to_noobj()} : Option<NoObj_p>();
    }

    Option<NoObj_p> tryParseNoObj(const string &valueToken, const string &typeToken, const fURI_p & = NOOBJ_FURI) {
      return (typeToken == NOOBJ_FURI->toString() ||
              (typeToken.empty() && valueToken == "Ø")) // second is deprecated (remove -- not ascii)
                 ? Option<NoObj_p>{Obj::to_noobj()}
                 : Option<NoObj_p>{};
    }

    Option<Bool_p> tryParseBool(const string &valueToken, const string &typeToken, const fURI_p &baseType = BOOL_FURI) {
      return ((strcmp("true", valueToken.c_str()) == 0) || (strcmp("false", valueToken.c_str()) == 0))
                 ? Option<Bool_p>{Bool::to_bool(strcmp("true", valueToken.c_str()) == 0,
                                                share<ID>(baseType->resolve(typeToken.c_str())))}
                 : Option<Bool_p>{};
    }
    Option<Int_p> tryParseInt(const string &valueToken, const string &typeToken, const fURI_p &baseType = INT_FURI) {
      if ((valueToken[0] != '-' && !isdigit(valueToken[0])) || valueToken.find_first_of('.') != string::npos)
        return {};
      for (size_t i = 1; i < valueToken.length(); i++) {
        if (!isdigit(valueToken[i]))
          return {};
      }
      return Option<Int_p>{Int::to_int(stoi(valueToken), share<ID>(baseType->resolve(typeToken.c_str())))};
    }
    Option<Real_p> tryParseReal(const string &valueToken, const string &typeToken, const fURI_p &baseType = REAL_FURI) {
      if (valueToken[0] != '-' && !isdigit(valueToken[0]))
        return {};
      bool dotFound = false;
      for (size_t i = 1; i < valueToken.length(); i++) {
        if (valueToken[i] == '.') {
          if (dotFound)
            return {};
          dotFound = true;
        }
        if (valueToken[i] != '.' && !isdigit(valueToken[i]))
          return {};
      }
      return dotFound ? Option<Real_p>{Real::to_real(stof(valueToken), share<ID>(baseType->resolve(typeToken.c_str())))}
                      : Option<Real_p>{};
    }
    Option<Uri_p> tryParseUri(const string &valueToken, const string &typeToken, const fURI_p &baseType = URI_FURI) {
      return (valueToken[0] == '<' && valueToken[valueToken.length() - 1] == '>')
                 ? Option<Uri_p>{Uri::to_uri(strdup(valueToken.substr(1, valueToken.length() - 2).c_str()),
                                             id_p(baseType->resolve(typeToken)))}
                 : Option<Uri_p>{};
    }

    Option<Str_p> tryParseStr(const string &valueToken, const string &typeToken, const fURI_p &baseType = STR_FURI) {
      return (valueToken[0] == '\'' && valueToken[valueToken.length() - 1] == '\'')
                 ? Option<Str_p>{Str::to_str(strdup(valueToken.substr(1, valueToken.length() - 2).c_str()),
                                             id_p(baseType->resolve(typeToken)))}
                 : Option<Str_p>{};
    }
    Option<Lst_p> tryParseLst(const string &token, const string &type, const fURI_p &baseType = LST_FURI) {
      if (token[0] != '[' || token[token.length() - 1] != ']')
        return {};
      auto ss = stringstream(token.substr(1, token.length() - 2));
      string value;
      Obj::LstList<> list = Obj::LstList<>();
      Tracker tracker;
      while (!ss.eof()) {
        if (tracker.closed() && (ss.peek() == ',' || ss.peek() == EOF)) {
          Option<Obj_p> element = this->tryParseObj(value);
          if (element.has_value())
            list.push_back(element.value());
          if (ss.peek() == ',')
            ss.get(); // consume comma
          value.clear();
        } else if (tracker.closed() && StringHelper::lookAhead("=>", &ss)) {
          return {};
        } else {
          char c = tracker.track(static_cast<char>(ss.get()));
          if (tracker.printable())
            value += c;
        }
      }
      return Option<Lst_p>{Lst::to_lst(share(list), id_p(baseType->resolve(type)))};
    }
    Option<Rec_p> tryParseRec(const string &token, const string &type, const fURI_p &baseType = REC_FURI) {
      if (token[0] != '[' || token[token.length() - 1] != ']' || token.find("=>") == string::npos)
        return {};
      auto ss = stringstream(token.substr(1, token.length() - 2));
      Obj::RecMap<> map = Obj::RecMap<>();
      string key;
      string value;
      bool onKey = true;
      Tracker tracker;
      while (!ss.eof() || (!onKey && !value.empty())) {
        //// KEY PARSE
        if (onKey) {
          if (tracker.closed() && StringHelper::lookAhead("=>", &ss)) {
            onKey = false;
          } else if (!ss.eof()) {
            char c = tracker.track(static_cast<char>(ss.get()));
            if (tracker.printable())
              key += c;
          }
        } else {
          //// VALUE PARSE
          if (tracker.closed() && (ss.eof() || ss.peek() == ',' || ss.peek() == EOF)) {
            if (ss.peek() == ',')
              ss.get(); // drop k/v separating comma
            StringHelper::trim(key);
            StringHelper::trim(value);
            const Option<Obj_p> k = this->tryParseObj(key);
            const Option<Obj_p> v = this->tryParseObj(value);
            if (!k.has_value() || !v.has_value())
              return {};
            map.insert({k.value(), v.value()});
            key.clear();
            value.clear();
            onKey = true;
          } else {
            char c = tracker.track(static_cast<char>(ss.get()));
            if (tracker.printable())
              value += c;
          }
        }
      }
      ////
      Obj::RecMap_p<> map2 = share(Obj::RecMap<>()); // necessary to reverse entries
      for (const auto &[kk, vv]: map) {
        map2->insert({PtrHelper::clone(kk), PtrHelper::clone(vv)});
      }
      ////
      return Option<Rec_p>{Rec::to_rec(map2, id_p(baseType->resolve(type.c_str())))};
    }
    Option<Inst_p> tryParseInst(const string &valueToken, const string &typeToken, const fURI_p &baseType = INST_FURI) {
      auto args = List<ptr<Obj>>();
      stringstream ss = stringstream(valueToken);
      while (!ss.eof()) {
        string argToken;
        Tracker tracker;
        while (!ss.eof()) {
          const char c = tracker.track(static_cast<char>(ss.get()));
          if (tracker.printable()) {
            argToken += c;
            if (ss.peek() == ',' && tracker.closed()) {
              ss.get(); // drop arg separating comma
              break;
            }
          }
        }
        StringHelper::trim(argToken);
        if (!argToken.empty()) {
          const Option<Obj_p> arg_p = this->tryParseObj(argToken);
          if (arg_p.has_value())
            args.push_back(arg_p.value());
          else {
            LOG(WARN, "Unable to parse potential argument to !b%s!g[!!%s!g]!!: %s\n", typeToken.c_str(),
                valueToken.c_str(), argToken.c_str());
          }
        }
      }
      const Inst_p inst = Insts::to_inst(baseType->resolve(typeToken.c_str()), args);
      return inst->isNoObj() ? Option<Inst_p>() : Option<Inst_p>(inst);
    }

    Option<BCode_p> tryParseBCode(const string &valueToken, const string &typeToken,
                                  const fURI_p &baseType = BCODE_FURI) {
      if (typeToken.empty() && valueToken == "_")
        return {Obj::to_bcode()}; // special character for 'no instructions' (no general common parse pattern)
      //////////////////////////////////////////////////////////////////////////////////////
      //////////////// lookahead to determine if token is potentially _bcode ////////////////
      //////////////////////////////////////////////////////////////////////////////////////
      bool mayBCode = (valueToken[0] == '_' && valueToken[1] == '_') || //
                      valueToken.find('.') != string::npos || //
                      (valueToken.find('(') != string::npos && valueToken.find(')') != string::npos);
      if (!mayBCode) {
        for (const auto &[k, v]: Insts::unarySugars()) {
          if (valueToken.find(k) != string::npos || valueToken.find(v + "(") != string::npos) {
            mayBCode = true;
            break;
          }
        }
        if (!mayBCode)
          return {};
      }
      //////////////////////////////////////////////////////////////////////////////////////
      List<Inst_p> insts;
      std::stringstream ss = std::stringstream(valueToken);
      while (!ss.eof()) { // _bcode-level (tokens are insts)
        Tracker tracker;
        bool unary = false;
        bool fullbreak = false;
        string instToken;
        while (!ss.eof()) { // inst-level (tokens are chars)
          if (tracker.closed()) {
            for (const auto &[k, v]: Insts::unarySugars()) {
              if (StringHelper::lookAhead(k, &ss, instToken.empty())) {
                if (!instToken.empty()) {
                  LOG(TRACE, "Found beginning of unary %s after parsing %s\n", k.c_str(), instToken.c_str());
                  fullbreak = true;
                  break;
                }
                instToken += v;
                instToken += '(';
                tracker.parens++;
                unary = true;
                break;
              }
            }
          }
          if (fullbreak || ss.eof())
            break;
          char c = tracker.track(static_cast<char>(ss.get()));
          if ((tracker.quotes || !isspace(c)) && tracker.printable())
            instToken += c;
          ///////////////////////////////////////////////////////////////
          if ((unary || tracker.parens == 0) && tracker.brackets == 0 && tracker.angles == 0 && tracker.braces == 0 &&
              !tracker.quotes && ss.peek() == '.') {
            ss.get();
            break;
          }
        }
        if (instToken.empty())
          continue;
        if (unary)
          instToken += ')';
        LOG(TRACE, "Parsing !ginst token!!: !y%s!!\n", instToken.c_str());
        // TODO: end inst with ; sugar
        Pair<string, string> typeValue = tryParseObjType(instToken, PARSE_TOKENS::PAREN);
        if (!unary && instToken[typeValue.first.length()] != '(') { // OBJ AS ARGUMENT (START OR MAP)
          LOG(TRACE, "Parsing !gobj as apply!! (!ysugar!!): %s\n", instToken.c_str());
          const Option<Obj_p> obj = tryParseObj(instToken);
          if (!obj.has_value())
            return {};
          insts.push_back(insts.empty() ? Insts::start(Obj::to_objs({obj.value()})) : Insts::map(obj.value()));
        } else { // CLASSIC INST WITH VARIABLE LENGTH ARGUMENTS WRAPPED IN ( )
          LOG(TRACE, "Parsing !gobj as inst!!: !b%s!g[!!%s!g]!!\n", typeValue.first.c_str(), typeValue.second.c_str());
          const Option<Inst_p> inst = tryParseInst(typeValue.second, typeValue.first, INST_FURI);
          if (inst.has_value()) {
            insts.push_back(inst.value());
          } else {
            return {};
          }
        }
        instToken.clear();
      }
      return Option<BCode_p>{BCode::to_bcode(insts, id_p(baseType->resolve(typeToken)))};
    }

    Option<Uri_p> tryParseDEFAULT(const string &valueToken, const string &typeToken,
                                  const fURI_p &baseType = URI_FURI) {
      return Option<Uri_p>{Uri::to_uri(valueToken, id_p(baseType->resolve(typeToken)))};
    }
  };
} // namespace fhatos
#endif
