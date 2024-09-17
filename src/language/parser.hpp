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
#include <structure/stype/key_value.hpp>
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

    [[nodiscard]] bool printable() const {
      return last >= 32 && last < 127;
    }

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
      {GROUPING::PAREN, "()"},
      {GROUPING::BRACE, "{}"}
    });

  class Parser : public Actor<Coroutine, KeyValue> {
    explicit Parser(const ID &id = ID("/parser/")): Actor(id, id.extend("#")) {
    }

  public:
    static ptr<Parser> singleton(const ID &id = ID("/parser/")) {
      static ptr<Parser> parser_p = ptr<Parser>(new Parser(id));
      static bool setup = false;
      if (!setup) {
        setup = true;
        OBJ_PARSER = [](const string &obj_string) {
          try {
            return fhatos::Parser::try_parse_obj(obj_string).value_or(Obj::to_noobj());
          } catch (std::exception &e) {
            LOG_EXCEPTION(e);
            return Obj::to_noobj();
          }
        };
        Options::singleton()->parser<Obj>(OBJ_PARSER);
      }
      return parser_p;
    }

    void setup() override {
      Actor::setup();
      this->subscribe(this->id()->extend("parse/+/in"), [this](const Message_p &message) {
        LOG_PROCESS(DEBUG, this, "Parsing %s\n", message->payload->toString().c_str());
        this->publish(message->target.resolve("./out"),
                      Options::singleton()->processor<Obj, BCode, Obj>(
                        noobj(),
                        fhatos::Parser::try_parse_obj(message->payload->str_value()).value()),
                      RETAIN_MESSAGE
        );
        //
      });
    }

    static bool closed_expression(const string &line) {
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

    static Option<Obj_p> try_parse_source(const Lst_p &lst_src) {
      Option<Obj_p> last = {};
      for (const Obj_p &obj: *lst_src->lst_value()) {
        last = try_parse_obj(obj->is_str() ? obj->str_value() : obj->toString(true, false));
      }
      return last;
    }

    static Option<Obj_p> try_parse_obj(const string &token) {
      StringHelper::trim(token);
      if (token.empty() || try_parse_comment(token).has_value())
        return {};
      const Pair<string, string> typeValue = try_parse_obj_type(token, GROUPING::BRACKET);
      const string typeToken = typeValue.first;
      const string valueToken = typeValue.second;
      const bool dot_type = dotType(typeToken); // .obj. in _bcode (apply)
      Option<Obj_p> b = {};
      ////////////////
      b = try_parse_poly_within(valueToken);
      if (b.has_value())
        return b.value();
      ////////////////
      if (!dot_type) {
        // dot type
        b = try_parse_no_obj(valueToken, typeToken, NOOBJ_FURI);
        if (b.has_value())
          return b.value();
        b = try_parse_bool(valueToken, typeToken, BOOL_FURI);
        if (b.has_value())
          return b.value();
        b = try_parse_int(valueToken, typeToken, INT_FURI);
        if (b.has_value())
          return b.value();
        b = try_parse_real(valueToken, typeToken, REAL_FURI);
        if (b.has_value())
          return b.value();
        b = try_parse_uri(valueToken, typeToken, URI_FURI);
        if (b.has_value())
          return b.value();
        b = try_parse_str(valueToken, typeToken, STR_FURI);
        if (b.has_value())
          return b.value();
        b = try_parse_lst(valueToken, typeToken, LST_FURI);
        if (b.has_value())
          return b.value();
        b = try_parse_rec(valueToken, typeToken, REC_FURI);
        if (b.has_value())
          return b.value();
        b = try_parse_objs(valueToken, typeToken, OBJS_FURI);
        if (b.has_value())
          return b.value();
      }
      b = dot_type
            ? try_parse_bcode(token, EMPTY_CHARS, BCODE_FURI)
            : try_parse_bcode(valueToken, typeToken, BCODE_FURI);
      if (b.has_value())
        return b.value();
      b = try_parse_default(valueToken, typeToken);
      if (b.has_value())
        return b.value();
      return {};
    }

    static Pair<string, string> try_parse_obj_type(
      const string &token,
      const GROUPING grouping) {
      string type_token;
      string value_token;
      const bool grouped = !token.empty() && token[token.length() - 1] == parse_token.to_chars(grouping)[1];
      auto ss = stringstream(token);
      Tracker tracker;
      if (grouped) {
        bool onType = true;
        bool unaryFound = false;
        ////////////////////////////////////////////////////
        while (!ss.eof()) {
          if (onType) {
            for (const auto &[k, v]: Insts::unary_sugars()) {
              if (StringHelper::look_ahead(k, &ss)) {
                onType = false;
                value_token.append(type_token);
                type_token.clear();
                value_token.append(k);
                unaryFound = true;
                break;
              }
            }
            if (unaryFound)
              break;
            char c = tracker.track(static_cast<char>(ss.get()));
            if (c == parse_token.to_chars(grouping)[0]) {
              onType = false;
            } else if ((!type_token.empty() && tracker.grouping(parse_token.to_chars(grouping)[0]) && (
                          // inside value definition (must be bcode)
                          c == '.' || isspace(c))) ||
                       (tracker.open('{') && tracker.closed(parse_token.to_chars(grouping)[1]))) {
              // inside an objs definition (must be sugar bcode)
              value_token.append(type_token);
              value_token += c;
              type_token.clear();
              onType = false;
            } else {
              type_token += c;
            }
          } else {
            value_token += tracker.track(static_cast<char>(ss.get()));
          }
        }
        if (type_token.empty())
          value_token = token;
        else if (!unaryFound)
          value_token = value_token.substr(0, value_token.length() - 2);
      } else {
        type_token = "";
        value_token = token;
      }
      StringHelper::trim(value_token);
      StringHelper::trim(type_token);
      // <type_furi>
      if (!type_token.empty() && type_token[0] == '<' && type_token[type_token.length() - 1] == '>')
        type_token = type_token.substr(1, type_token.length() - 2);
      // base types resolve outside of standard algorithm due to their common usage
      if (type_token == "obj")
        type_token = OBJ_FURI->toString();
      else if (type_token == "noobj")
        type_token = NOOBJ_FURI->toString();
      else if (type_token == "bool")
        type_token = BOOL_FURI->toString();
      else if (type_token == "int")
        type_token = INT_FURI->toString();
      else if (type_token == "real")
        type_token = REAL_FURI->toString();
      else if (type_token == "str")
        type_token = STR_FURI->toString();
      else if (type_token == "uri")
        type_token = URI_FURI->toString();
      else if (type_token == "lst")
        type_token = LST_FURI->toString();
      else if (type_token == "rec")
        type_token = REC_FURI->toString();
      else if (type_token == "bcode")
        type_token = BCODE_FURI->toString();
      else if (type_token == "objs")
        type_token = OBJS_FURI->toString();
      if (Options::singleton()->log_level<LOG_TYPE>() <= TRACE) {
        LOG(TRACE, "!ytype token!!: !g%s!!" FOS_TAB_3 "!yvalue token!!: !g%s!!\n", type_token.c_str(),
            value_token.c_str());
        char t = type_token[type_token.length() - 1];
        char v = value_token[value_token.length() - 1];
        LOG(TRACE, FOS_TAB_2 "!ytypeToken!! [!glast char!!]: (!rdec!!) %i (!rhex!!) 0x%x (!rchar!!) %c\n", t, t,
            t);
        LOG(TRACE, FOS_TAB_2 "!yvalueToken!! [!glast char!!]: (!rdec!!) %i (!rhex!!) 0x%x (!rchar!!) %c\n", v,
            v, v);
      }
      return {type_token, value_token};
    }

    static Option<NoObj_p> try_parse_comment(const string &value_token) {
      return value_token.substr(0, 3) == "---" ? Option<NoObj_p>{NoObj::to_noobj()} : Option<NoObj_p>();
    }

    static Option<NoObj_p> try_parse_no_obj(
      const string &value_token, const string &type_token,
      const fURI_p & = NOOBJ_FURI) {
      return (type_token == NOOBJ_FURI->toString() || (type_token.empty() && value_token == STR(FOS_NOOBJ_TOKEN)))
               ? Option<NoObj_p>{Obj::to_noobj()}
               : Option<NoObj_p>{};
    }

    static Option<Bool_p> try_parse_bool(
      const string &value_token, const string &type_token,
      const fURI_p &base_type = BOOL_FURI) {
      return ((strcmp("true", value_token.c_str()) == 0) || (strcmp("false", value_token.c_str()) == 0))
               ? Option<Bool_p>{
                 Bool::to_bool(strcmp("true", value_token.c_str()) == 0,
                               share<ID>(base_type->resolve(type_token.c_str())))
               }
               : Option<Bool_p>{};
    }

    static Option<Int_p> try_parse_int(
      const string &valueToken, const string &typeToken,
      const fURI_p &baseType = INT_FURI) {
      if ((valueToken[0] != '-' && !isdigit(valueToken[0])) || valueToken.find_first_of('.') != string::npos)
        return {};
      for (size_t i = 1; i < valueToken.length(); i++) {
        if (!isdigit(valueToken[i]))
          return {};
      }
      return Option<Int_p>{Int::to_int(stoi(valueToken), share<ID>(baseType->resolve(typeToken.c_str())))};
    }

    static Option<Real_p> try_parse_real(
      const string &valueToken, const string &type_token,
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
                 Real::to_real(stof(valueToken), share<ID>(baseType->resolve(type_token.c_str())))
               }
               : Option<Real_p>{};
    }

    static Option<Uri_p> try_parse_uri(
      const string &value_token, const string &type_token,
      const fURI_p &base_type = URI_FURI) {
      if (value_token[0] == '<' && value_token[value_token.length() - 1] == '>')
        return Option<Uri_p>{
          Uri::to_uri(value_token.substr(1, value_token.length() - 2).c_str(),
                      id_p(base_type->resolve(type_token)))
        };
      if ((value_token[0] == '.' && value_token[1] == '/') ||
          (value_token[0] == '.' && value_token[1] == '.' && value_token[2] == '/'))
        return Option<Uri_p>{Uri::to_uri(value_token, id_p(base_type->resolve(type_token)))};
      return Option<Uri_p>{};
    }

    static Option<Str_p> try_parse_str(
      const string &valueToken, const string &typeToken,
      const fURI_p &baseType = STR_FURI) {
      return (valueToken[0] == '\'' && valueToken[valueToken.length() - 1] == '\'')
               ? Option<Str_p>{
                 Str::to_str(valueToken.substr(1, valueToken.length() - 2).c_str(),
                             id_p(baseType->resolve(typeToken)))
               }
               : Option<Str_p>{};
    }

    static Option<Lst_p> try_parse_lst(const string &token, const string &type, const fURI_p &base_type = LST_FURI) {
      if (token[0] != '[' || token[token.length() - 1] != ']')
        return {};
      auto ss = stringstream(token.substr(1, token.length() - 2));
      string value;
      auto list = Obj::LstList<>();
      Tracker tracker;
      while (!ss.eof()) {
        if (tracker.closed() && (ss.peek() == ',' || ss.peek() == EOF)) {
          Option<Obj_p> element = try_parse_obj(value);
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
      return Option<Lst_p>{Lst::to_lst(share(list), id_p(base_type->resolve(type)))};
    }

    static Option<Inst_p> try_parse_poly_within(const string &token) {
      if (!(token[0] == '_' && token[1] == '/' && token[token.length() - 2] == '\\' &&
            token[token.length() - 1] == '_'))
        return {};
      LOG(TRACE, "Parsing poly within: %s\n", token.substr(2, token.length() - 4).c_str());
      const Option<BCode_p> bcode = try_parse_bcode(token.substr(2, token.length() - 4), "", BCODE_FURI);
      if (!bcode.has_value())
        return {};
      return Option<Inst_p>{Insts::within(bcode.value())};
    }

    static Option<Rec_p> try_parse_rec(const string &token, const string &type, const fURI_p &base_type = REC_FURI) {
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
            const Option<Obj_p> k = try_parse_obj(key);
            const Option<Obj_p> v = try_parse_obj(value);
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
      return Option<Rec_p>{Rec::to_rec(map2, id_p(base_type->resolve(type.c_str())))};
    }

    static Option<Objs_p> try_parse_objs(
      const string &token, const string &type,
      const fURI_p &base_type = OBJS_FURI) {
      if (token[0] != '{' || token[token.length() - 1] != '}')
        return {};
      auto ss = stringstream(token.substr(1, token.length() - 2));
      string value;
      auto list = List<Obj_p>();
      Tracker tracker;
      while (!ss.eof()) {
        if (tracker.closed() && (ss.peek() == ',' || ss.peek() == EOF)) {
          Option<Obj_p> element = try_parse_obj(value);
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
      LOG(TRACE, "parsed as objs: %s\n", Objs::to_objs(share(list))->toString().c_str());
      return Option<Objs_p>{Objs::to_objs(share(list), id_p(base_type->resolve(type)))};
    }

    static Option<Inst_p> try_parse_inst(
      const string &value_token, const string &type_token,
      const fURI_p &base_type = INST_FURI) {
      auto args = List<ptr<Obj>>();
      stringstream ss = stringstream(value_token);
      while (!ss.eof()) {
        string arg_token;
        Tracker tracker;
        while (!ss.eof()) {
          const char c = tracker.track(static_cast<char>(ss.get()));
          if (tracker.printable()) {
            arg_token += c;
            if (ss.peek() == ',' && tracker.closed()) {
              ss.get(); // drop arg separating comma
              break;
            }
          }
        }
        StringHelper::trim(arg_token);
        if (!arg_token.empty()) {
          const Option<Obj_p> arg_p = try_parse_obj(arg_token);
          if (arg_p.has_value())
            args.push_back(arg_p.value());
          else {
            LOG(WARN, "Unable to parse potential argument to !b%s!g[!!%s!g]!!: %s\n", type_token.c_str(),
                value_token.c_str(), arg_token.c_str());
          }
        }
      }
      const Inst_p inst = Insts::to_inst(base_type->resolve(type_token.c_str()), args);
      return inst->is_noobj() ? Option<Inst_p>() : Option<Inst_p>(inst);
    }

    static Option<BCode_p> try_parse_bcode(
      const string &value_token, const string &type_token,
      const fURI_p &base_type = BCODE_FURI) {
      if (value_token == "_") {
        return type_token.empty()
                 ? Option<BCode_p>(Obj::to_bcode())
                 : Option<BCode_p>(Obj::to_bcode({Insts::as(uri(type_token))}));
      } // special character for 'no instructions' (no general common parse pattern)
      //////////////////////////////////////////////////////////////////////////////////////
      //////////////// lookahead to determine if token is potentially _bcode ////////////////
      //////////////////////////////////////////////////////////////////////////////////////
      bool mayBCode = value_token.find('.') != string::npos || //
                      (value_token.find('(') != string::npos && value_token.find(')') != string::npos);
      if (!mayBCode) {
        for (const auto &[k, v]: Insts::unary_sugars()) {
          if (value_token.find(k) != string::npos || value_token.find(v + "(") != string::npos) {
            mayBCode = true;
            break;
          }
        }
        if (!mayBCode)
          return {};
      }
      //////////////////////////////////////////////////////////////////////////////////////
      List<Inst_p> insts;
      std::stringstream ss = std::stringstream(value_token);
      while (!ss.eof()) {
        // _bcode-level (tokens are insts)
        Tracker tracker;
        bool unary = false;
        bool fullbreak = false;
        string instToken;
        while (!ss.eof()) {
          // inst-level (tokens are chars)
          if (tracker.closed()) {
            for (const auto &[k, v]: Insts::unary_sugars()) {
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
          ///////////////////////////////////////////////////////////////
          /*if (unary && tracker.grouping('(') && (ss.peek() == '.' || sugar_next(&ss))) {
            if (ss.peek() == '.') {
              ss.get();
            }
            break;
          }*/
          ///////////////////////////////////////////////////////////////
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
        Option<Inst_p> within = try_parse_poly_within(instToken);
        if (!unary && within.has_value()) {
          insts.push_back(within.value());
        } else {
          Pair<string, string> typeValue = try_parse_obj_type(instToken, GROUPING::PAREN);
          if (!unary && instToken[typeValue.first.length()] != '(') {
            // OBJ AS ARGUMENT (START OR MAP)
            LOG(TRACE, "Parsing !gobj as apply!! (!ysugar!!): %s\n", instToken.c_str());
            const Option<Obj_p> obj = try_parse_obj(instToken);
            if (!obj.has_value())
              return {};
            insts.push_back(insts.empty()
                              ? Insts::start(obj.value())
                              : Insts::map(obj.value()));
          } else {
            // CLASSIC INST WITH VARIABLE LENGTH ARGUMENTS WRAPPED IN ( )
            LOG(TRACE, "Parsing !gobj as inst!!: !b%s!g[!!%s!g]!!\n", typeValue.first.c_str(),
                typeValue.second.c_str());
            const Option<Inst_p> inst = try_parse_inst(typeValue.second, typeValue.first, INST_FURI);
            if (inst.has_value()) {
              insts.push_back(inst.value());
            } else {
              return {};
            }
          }
        }
        instToken.clear();
      }
      return Option<BCode_p>{BCode::to_bcode(insts, id_p(base_type->resolve(type_token)))};
    }

    static Option<Uri_p> try_parse_default(
      const string &valueToken, const string &typeToken,
      const fURI_p &baseType = URI_FURI) {
      return Option<Uri_p>{Uri::to_uri(valueToken, id_p(baseType->resolve(typeToken)))};
    }

  private:
    static bool sugar_next(stringstream *ss) {
      if (ss->eof())
        return false;
      for (const auto &[k, v]: Insts::unary_sugars()) {
        if (StringHelper::look_ahead(k, ss, false)) {
          return true;
        }
      }
      return StringHelper::look_ahead("_/", ss, false); // within is a wrapping sugar
      // TODO: make that a universal distinction
    }
  };


  [[maybe_unused]] static Obj_p parse(const char *format, ...) {
    char message[1024];
    va_list arg;
    va_start(arg, format);
    vsnprintf(message, 1024, format, arg);
    va_end(arg);
    return Parser::try_parse_obj(string(message)).value_or(noobj());
  }

  [[maybe_unused]] static Obj_p parse(const string &source) {
    return Parser::try_parse_obj(source).value_or(noobj());
  }
} // namespace fhatos
#endif
