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
#include <util/enums.hpp>
#include FOS_PROCESS(coroutine.hpp)

namespace fhatos {
  using namespace std;

  struct Tracker {
    int8_t parens = 0;
    int8_t brackets = 0;
    int8_t angles = 0;
    int8_t braces = 0;
    int8_t within = 0;
    bool quotes = false;
    char last = '\0';

    char track(const char c) {
      if (c != '\'' && quotes) {
        // do nothing
      } else if (c == '\'') {
        quotes = !quotes;
      } else if ((c == '=' || c == '-')) {
        if (last == '<') // <- <=
          angles--;
        else if (last == '>') // >- >=
          angles++;
      } else if (c == '(')
        parens++;
      else if (c == ')')
        parens--;
      else if (c == '[')
        brackets++;
      else if (c == ']')
        brackets--;
      else if (c == '<') {
        if (last != '-' && last != '=') // -< =<
          angles++;
      } else if (c == '>') {
        if (last != '-' && last != '=') // -> =>
          angles--;
      } else if (c == '{')
        braces++;
      else if (c == '}')
        braces--;
      else if (c == '/' && last == '_') // _/
        within++;
      else if (c == '_' && last == '\\') // \_
        within--;
      //////////////////////////
      last = c;
      return c;
    }

    [[nodiscard]] bool printable() const { return last >= 32 && last < 127; }

    [[nodiscard]] bool closed() const {
      return parens == 0 && brackets == 0 && angles == 0 && braces == 0 && within == 0 && !quotes;
    }

    [[nodiscard]] bool grouping(const char grouping) const {
      if (grouping == '[')
        return parens == 0 && brackets > 0 && angles == 0 && braces == 0 && within == 0 && !quotes;
      if (grouping == '(')
        return parens > 0 && brackets == 0 && angles == 0 && braces == 0 && within == 0 && !quotes;
      if (grouping == '{')
        return parens == 0 && brackets == 0 && angles == 0 && braces > 0 && within == 0 && !quotes;
      if (grouping == ']')
        return parens == 0 && brackets < 0 && angles == 0 && braces == 0 && within == 0 && !quotes;
      if (grouping == ')')
        return parens > 0 && brackets == 0 && angles == 0 && braces == 0 && within == 0 && !quotes;
      if (grouping == '}')
        return parens == 0 && brackets == 0 && angles == 0 && braces < 0 && within == 0 && !quotes;
      throw fError("unknown grouping symbol: %c\n", grouping);
    }

    [[nodiscard]] bool closed(const char grouping) const {
      if (grouping == ']')
        return brackets == 0 && !quotes;
      if (grouping == ')')
        return parens == 0 && !quotes;
      if (grouping == '}')
        return braces == 0 && !quotes;
      throw fError("unknown closed-grouping symbol: %c\n", grouping);
    }

    [[nodiscard]] bool open(const char grouping) const {
      if (grouping == '[')
        return brackets > 0 && !quotes;
      if (grouping == '(')
        return parens > 0 && !quotes;
      if (grouping == '{')
        return braces > 0 && !quotes;
      throw fError("unknown open-grouping symbol: %c\n", grouping);
    }
  };

  enum class GROUPING {
    BRACKET, PAREN, BRACE
  };

  const static Enums<GROUPING> parse_token = Enums<GROUPING>(
          {
                  {GROUPING::BRACKET, "[]"},
                  {GROUPING::PAREN,   "()"},
                  {GROUPING::BRACE,   "{}"}
          });

  class Parser final : public Actor<Coroutine, Empty> {
    explicit Parser(const ID &id = ID("/parser/")) : Actor<Coroutine, Empty>(id) {
    }

  public:
    static ptr<Parser> singleton(const ID &id = ID("/parser/")) {
      static ptr<Parser> parser_p = ptr<Parser>(new Parser(id));
      static bool _setup = false;
      if (!_setup) {
        _setup = true;
        OBJ_PARSER = [](const string &obj_string) {
          try {
            return Parser::singleton()->tryParseObj(obj_string).value_or(Obj::to_noobj());
          } catch (std::exception &e) {
            LOG_EXCEPTION(e);
            return Obj::to_noobj();
          }
        };
        Options::singleton()->parser<Obj>(OBJ_PARSER);
      }
      return parser_p;
    }

    static bool closedExpression(const string &line) {
      auto ss = stringstream(line);
      Tracker tracker;
      while (!ss.eof()) {
        tracker.track(static_cast<char>(ss.get()));
      }
      return tracker.closed() && StringHelper::count_substring(line, "===") % 2 == 0;
    }

    static bool dotType(const string &type) {
      return !type.empty() && type[type.length() - 1] == '.'; // dot type
    }

    static Option<Obj_p> tryParseObj(const string &token) {
      StringHelper::trim(token);
      if (token.empty() || tryParseComment(token).has_value())
        return {};
      const Pair<string, string> typeValue = tryParseObjType(token, GROUPING::BRACKET);
      const string typeToken = typeValue.first;
      const string valueToken = typeValue.second;
      const bool dot_type = dotType(typeToken); // .obj. in _bcode (apply)
      Option<Obj_p> b = {};
      ////////////////
      b = tryParsePolyWithin(valueToken);
      if (b.has_value())
        return b.value();
      ////////////////
      if (!dot_type) {
        // dot type
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
        b = tryParseObjs(valueToken, typeToken, OBJS_FURI);
        if (b.has_value())
          return b.value();
      }
      b = dot_type
          ? tryParseBCode(token, EMPTY_CHARS, BCODE_FURI)
          : tryParseBCode(valueToken, typeToken, BCODE_FURI);
      if (b.has_value())
        return b.value();
      b = tryParseDEFAULT(valueToken, typeToken);
      if (b.has_value())
        return b.value();
      return {};
    }

    static Pair<string, string> tryParseObjType(const string &token,
                                                const GROUPING grouping) {
      string typeToken;
      string valueToken;
      const bool grouped = !token.empty() && token[token.length() - 1] == parse_token.toChars(grouping)[1];
      auto ss = stringstream(token);
      Tracker tracker;
      if (grouped) {
        bool onType = true;
        bool unaryFound = false;
        ////////////////////////////////////////////////////
        while (!ss.eof()) {
          if (onType) {
            for (const auto &[k, v]: Insts::unarySugars()) {
              if (StringHelper::look_ahead(k, &ss)) {
                onType = false;
                valueToken.append(typeToken);
                typeToken.clear();
                valueToken.append(k);
                unaryFound = true;
                break;
              }
            }
            if (unaryFound)
              break;
            char c = tracker.track(static_cast<char>(ss.get()));
            if (c == parse_token.toChars(grouping)[0]) {
              onType = false;
            } else if ((!typeToken.empty() && tracker.grouping(parse_token.toChars(grouping)[0]) && (
                    // inside value definition (must be bcode)
                    c == '.' || isspace(c))) ||
                       (tracker.open('{') && tracker.closed(parse_token.toChars(grouping)[1]))) {
              // inside an objs definition (must be sugar bcode)
              valueToken.append(typeToken);
              valueToken += c;
              typeToken.clear();
              onType = false;
            } else {
              typeToken += c;
            }
          } else {
            valueToken += tracker.track(static_cast<char>(ss.get()));
          }
        }
        if (typeToken.empty())
          valueToken = token;
        else if (!unaryFound)
          valueToken = valueToken.substr(0, valueToken.length() - 2);
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
        LOG(TRACE, FOS_TAB_2 "!ytypeToken!! [!glast char!!]: (!rdec!!) %i (!rhex!!) 0x%x (!rchar!!) %c\n", t, t,
            t);
        LOG(TRACE, FOS_TAB_2 "!yvalueToken!! [!glast char!!]: (!rdec!!) %i (!rhex!!) 0x%x (!rchar!!) %c\n", v,
            v, v);
      }
      return {typeToken, valueToken};
    }

    static Option<NoObj_p> tryParseComment(const string &valueToken) {
      return valueToken.substr(0, 3) == "---" ? Option<NoObj_p>{NoObj::to_noobj()} : Option<NoObj_p>();
    }

    static Option<NoObj_p> tryParseNoObj(const string &valueToken, const string &typeToken,
                                         const fURI_p & = NOOBJ_FURI) {
      return (typeToken == NOOBJ_FURI->toString() || (typeToken.empty() && valueToken == STR(FOS_NOOBJ_TOKEN)))
             ? Option<NoObj_p>{Obj::to_noobj()}
             : Option<NoObj_p>{};
    }

    static Option<Bool_p> tryParseBool(const string &valueToken, const string &typeToken,
                                       const fURI_p &baseType = BOOL_FURI) {
      return ((strcmp("true", valueToken.c_str()) == 0) || (strcmp("false", valueToken.c_str()) == 0))
             ? Option<Bool_p>{
                      Bool::to_bool(strcmp("true", valueToken.c_str()) == 0,
                                    share<ID>(baseType->resolve(typeToken.c_str())))
              }
             : Option<Bool_p>{};
    }

    static Option<Int_p> tryParseInt(const string &valueToken, const string &typeToken,
                                     const fURI_p &baseType = INT_FURI) {
      if ((valueToken[0] != '-' && !isdigit(valueToken[0])) || valueToken.find_first_of('.') != string::npos)
        return {};
      for (size_t i = 1; i < valueToken.length(); i++) {
        if (!isdigit(valueToken[i]))
          return {};
      }
      return Option<Int_p>{Int::to_int(stoi(valueToken), share<ID>(baseType->resolve(typeToken.c_str())))};
    }

    static Option<Real_p> tryParseReal(const string &valueToken, const string &typeToken,
                                       const fURI_p &baseType = REAL_FURI) {
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
      return dotFound
             ? Option<Real_p>{
                      Real::to_real(stof(valueToken), share<ID>(baseType->resolve(typeToken.c_str())))
              }
             : Option<Real_p>{};
    }

    static Option<Uri_p> tryParseUri(const string &valueToken, const string &typeToken,
                                     const fURI_p &baseType = URI_FURI) {
      if (valueToken[0] == '<' && valueToken[valueToken.length() - 1] == '>')
        return Option<Uri_p>{
                Uri::to_uri(strdup(valueToken.substr(1, valueToken.length() - 2).c_str()),
                            id_p(baseType->resolve(typeToken)))
        };
      if ((valueToken[0] == '.' && valueToken[1] == '/') ||
          (valueToken[0] == '.' && valueToken[1] == '.' && valueToken[2] == '/'))
        return Option<Uri_p>{Uri::to_uri(valueToken, id_p(baseType->resolve(typeToken)))};
      return Option<Uri_p>{};
    }

    static Option<Str_p> tryParseStr(const string &valueToken, const string &typeToken,
                                     const fURI_p &baseType = STR_FURI) {
      return (valueToken[0] == '\'' && valueToken[valueToken.length() - 1] == '\'')
             ? Option<Str_p>{
                      Str::to_str(strdup(valueToken.substr(1, valueToken.length() - 2).c_str()),
                                  id_p(baseType->resolve(typeToken)))
              }
             : Option<Str_p>{};
    }

    static Option<Lst_p> tryParseLst(const string &token, const string &type, const fURI_p &baseType = LST_FURI) {
      if (token[0] != '[' || token[token.length() - 1] != ']')
        return {};
      auto ss = stringstream(token.substr(1, token.length() - 2));
      string value;
      auto list = Obj::LstList<>();
      Tracker tracker;
      while (!ss.eof()) {
        if (tracker.closed() && (ss.peek() == ',' || ss.peek() == EOF)) {
          Option<Obj_p> element = tryParseObj(value);
          if (element.has_value())
            list.push_back(element.value());
          if (ss.peek() == ',')
            ss.get(); // consume comma
          value.clear();
        } else if (tracker.closed() && StringHelper::look_ahead("=>", &ss)) {
          return {};
        } else {
          const char c = tracker.track(static_cast<char>(ss.get()));
          if (tracker.printable())
            value += c;
        }
      }
      return Option<Lst_p>{Lst::to_lst(share(list), id_p(baseType->resolve(type)))};
    }

    static Option<Inst_p> tryParsePolyWithin(const string &token) {
      if (!(token[0] == '_' && token[1] == '/' && token[token.length() - 2] == '\\' &&
            token[token.length() - 1] == '_'))
        return {};
      LOG(TRACE, "Parsing poly within: %s\n", token.substr(2, token.length() - 4).c_str());
      const Option<BCode_p> bcode = tryParseBCode(token.substr(2, token.length() - 4), "", BCODE_FURI);
      if (!bcode.has_value())
        return {};
      return Option<Inst_p>{Insts::within(bcode.value())};
    }

    static Option<Rec_p> tryParseRec(const string &token, const string &type, const fURI_p &baseType = REC_FURI) {
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
          if (tracker.closed() && StringHelper::look_ahead("=>", &ss)) {
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
            const Option<Obj_p> k = tryParseObj(key);
            const Option<Obj_p> v = tryParseObj(value);
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

    static Option<Objs_p> tryParseObjs(const string &token, const string &type,
                                       const fURI_p &baseType = OBJS_FURI) {
      if (token[0] != '{' || token[token.length() - 1] != '}')
        return {};
      auto ss = stringstream(token.substr(1, token.length() - 2));
      string value;
      auto list = List<Obj_p>();
      Tracker tracker;
      while (!ss.eof()) {
        if (tracker.closed() && (ss.peek() == ',' || ss.peek() == EOF)) {
          Option<Obj_p> element = tryParseObj(value);
          if (element.has_value())
            list.push_back(element.value());
          if (ss.peek() == ',')
            ss.get(); // consume comma
          value.clear();
        } else {
          const char c = tracker.track(static_cast<char>(ss.get()));
          if (tracker.printable())
            value += c;
        }
      }
      return Option<Objs_p>{Objs::to_objs(share(list), id_p(baseType->resolve(type)))};
    }

    static Option<Inst_p> tryParseInst(const string &valueToken, const string &typeToken,
                                       const fURI_p &baseType = INST_FURI) {
      auto args = List<ptr<Obj> >();
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
          const Option<Obj_p> arg_p = tryParseObj(argToken);
          if (arg_p.has_value())
            args.push_back(arg_p.value());
          else {
            LOG(WARN, "Unable to parse potential argument to !b%s!g[!!%s!g]!!: %s\n", typeToken.c_str(),
                valueToken.c_str(), argToken.c_str());
          }
        }
      }
      const Inst_p inst = Insts::to_inst(baseType->resolve(typeToken.c_str()), args);
      return inst->is_noobj() ? Option<Inst_p>() : Option<Inst_p>(inst);
    }

    static Option<BCode_p> tryParseBCode(const string &valueToken, const string &typeToken,
                                         const fURI_p &baseType = BCODE_FURI) {
      if (valueToken == "_") {
        return typeToken.empty()
               ? Option<BCode_p>(Obj::to_bcode())
               : Option<BCode_p>(Obj::to_bcode({Insts::as(uri(typeToken))}));
      } // special character for 'no instructions' (no general common parse pattern)
      //////////////////////////////////////////////////////////////////////////////////////
      //////////////// lookahead to determine if token is potentially _bcode ////////////////
      //////////////////////////////////////////////////////////////////////////////////////
      bool mayBCode = valueToken.find('.') != string::npos || //
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
      while (!ss.eof()) {
        // _bcode-level (tokens are insts)
        Tracker tracker;
        bool unary = false;
        bool fullbreak = false;
        string instToken;
        while (!ss.eof()) {
          // inst-level (tokens are chars)
          if (tracker.closed()) {
            for (const auto &[k, v]: Insts::unarySugars()) {
              if (StringHelper::look_ahead(k, &ss, instToken.empty())) {
                if (!instToken.empty()) {
                  LOG(TRACE, "Found beginning of unary %s after parsing %s\n", k.c_str(), instToken.c_str());
                  fullbreak = true;
                  break;
                }
                instToken += v;
                StringHelper::eat_space(&ss);
                if (ss.peek() != '(') {
                  // enable paren-use of unary operators
                  instToken += '(';
                  tracker.parens++;
                  unary = true;
                }
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
          if (((unary && tracker.parens == 1) || tracker.parens == 0) && tracker.brackets == 0 && tracker.angles == 0 &&
              tracker.braces == 0 && tracker.within == 0 && !tracker.quotes &&
              (ss.peek() == '.' || sugar_next(&ss))) {
            if (ss.peek() == '.') {
              ss.get();
            }
            break;
          }
        }
        if (unary)
          instToken += ')';
        if (instToken.empty())
          continue;
        LOG(TRACE, "Parsing !ginst token!!: !y%s!!\n", instToken.c_str());
        // TODO: end inst with ; sugar
        Option<Inst_p> within = tryParsePolyWithin(instToken);
        if (!unary && within.has_value()) {
          insts.push_back(within.value());
        } else {
          Pair<string, string> typeValue = tryParseObjType(instToken, GROUPING::PAREN);
          if (!unary && instToken[typeValue.first.length()] != '(') {
            // OBJ AS ARGUMENT (START OR MAP)
            LOG(TRACE, "Parsing !gobj as apply!! (!ysugar!!): %s\n", instToken.c_str());
            const Option<Obj_p> obj = tryParseObj(instToken);
            if (!obj.has_value())
              return {};
            insts.push_back(insts.empty()
                            ? Insts::start(obj.value())
                            : Insts::map(obj.value()));
          } else {
            // CLASSIC INST WITH VARIABLE LENGTH ARGUMENTS WRAPPED IN ( )
            LOG(TRACE, "Parsing !gobj as inst!!: !b%s!g[!!%s!g]!!\n", typeValue.first.c_str(),
                typeValue.second.c_str());
            const Option<Inst_p> inst = tryParseInst(typeValue.second, typeValue.first, INST_FURI);
            if (inst.has_value()) {
              insts.push_back(inst.value());
            } else {
              return {};
            }
          }
        }
        instToken.clear();
      }
      return Option<BCode_p>{BCode::to_bcode(insts, id_p(baseType->resolve(typeToken)))};
    }

    static Option<Uri_p> tryParseDEFAULT(const string &valueToken, const string &typeToken,
                                          const fURI_p &baseType = URI_FURI) {
      return Option<Uri_p>{Uri::to_uri(valueToken, id_p(baseType->resolve(typeToken)))};
    }

  private:
    static bool sugar_next(stringstream *ss) {
      if (ss->eof())
        return false;
      for (const auto &[k, v]: Insts::unarySugars()) {
        if (StringHelper::look_ahead(k, ss, false)) {
          return true;
        }
      }
      return StringHelper::look_ahead("_/", ss, false); // within is a wrapping sugar
      // TODO: make that a universal distinction
    }
  };

  [[maybe_unused]] static Obj_p parse(const string &source) { return Parser::tryParseObj(source).value_or(noobj()); }
} // namespace fhatos
#endif
