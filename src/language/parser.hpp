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
#include <util/enums.hpp>
#include <util/string_helper.hpp>

namespace fhatos {
  struct Tracker {
    int8_t parens = 0;
    int8_t brackets = 0;
    int8_t angles = 0;
    int8_t braces = 0;
    int8_t within = 0;
    bool quotes = false;
    char last = '\0';

    char track(const char c) {
      if(c != '\'' && quotes) {
        // do nothing
      } else if(c == '\'') {
        quotes = !quotes;
      } else if((c == '=' || c == '-')) {
        if(last == '<') // <- <=
          angles--;
        else if(last == '>') // >- >=
          angles++;
      } else if(c == '(')
        parens++;
      else if(c == ')')
        parens--;
      else if(c == '[')
        brackets++;
      else if(c == ']')
        brackets--;
      else if(c == '<') {
        if(last != '-' && last != '=') // -< =<
          angles++;
      } else if(c == '>') {
        if(last != '-' && last != '=') // -> =>
          angles--;
      } else if(c == '{')
        braces++;
      else if(c == '}')
        braces--;
      else if(c == '/' && last == '_') // _/
        within++;
      else if(c == '_' && last == '\\') // \_
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
      if(grouping == '[')
        return parens == 0 && brackets > 0 && angles == 0 && braces == 0 && within == 0 && !quotes;
      if(grouping == '(')
        return parens > 0 && brackets == 0 && angles == 0 && braces == 0 && within == 0 && !quotes;
      if(grouping == '{')
        return parens == 0 && brackets == 0 && angles == 0 && braces > 0 && within == 0 && !quotes;
      if(grouping == ']')
        return parens == 0 && brackets < 0 && angles == 0 && braces == 0 && within == 0 && !quotes;
      if(grouping == ')')
        return parens > 0 && brackets == 0 && angles == 0 && braces == 0 && within == 0 && !quotes;
      if(grouping == '}')
        return parens == 0 && brackets == 0 && angles == 0 && braces < 0 && within == 0 && !quotes;
      throw fError("unknown grouping symbol: %c", grouping);
    }

    [[nodiscard]] bool closed(const char grouping) const {
      if(grouping == ']')
        return brackets == 0 && !quotes;
      if(grouping == ')')
        return parens == 0 && !quotes;
      if(grouping == '}')
        return braces == 0 && !quotes;
      throw fError("unknown closed-grouping symbol: %c", grouping);
    }

    [[nodiscard]] bool open(const char grouping) const {
      if(grouping == '[')
        return brackets > 0 && !quotes;
      if(grouping == '(')
        return parens > 0 && !quotes;
      if(grouping == '{')
        return braces > 0 && !quotes;
      throw fError("unknown open-grouping symbol: %c", grouping);
    }
  };

  enum class GROUPING { BRACKET, PAREN, BRACE };

  const static Enums<GROUPING> parse_token =
      Enums<GROUPING>({{GROUPING::BRACKET, "[]"}, {GROUPING::PAREN, "()"}, {GROUPING::BRACE, "{}"}});

  class Parser final : public Obj {
    explicit Parser(const ID &id = ID("/parser/")) : Obj(share(RecMap<>{}), OType::REC, REC_FURI, id_p(id)) {
      OBJ_PARSER = [this](const string &obj_string) {
        try {
          const Obj_p obj = Parser::try_parse_obj(obj_string).value_or(Obj::to_noobj());
          return obj;
        } catch(std::exception &e) {
          LOG_EXCEPTION(this, e);
          return Obj::to_noobj();
        }
      };
      Options::singleton()->parser<Obj>(OBJ_PARSER);
    }

  public:
    static ptr<Parser> singleton(const ID &id = ID("/parser/")) {
      static auto parser_p = ptr<Parser>(new Parser(id));
      return parser_p;
    }

    static bool closed_expression(const string &line) {
      auto ss = stringstream(line);
      Tracker tracker;
      while(!ss.eof()) {
        tracker.track(static_cast<char>(ss.get()));
      }
      return tracker.closed() && StringHelper::count_substring(line, "===") % 2 == 0;
    }

    static bool dotType(const string &type) {
      return !type.empty() && type[type.length() - 1] == '.'; // dot type
    }

    static Option<Obj_p> try_parse_source(const Lst_p &lst_src) {
      Option<Obj_p> last = {};
      for(const Obj_p &obj: *lst_src->lst_value()) {
        last = try_parse_obj(obj->is_str() ? obj->str_value() : obj->toString()); // todo: no ansi?
      }
      return last;
    }

    static Option<Obj_p> try_parse_obj(const string &token) {
      StringHelper::trim(token);
      if(token.empty() || try_parse_comment(token).has_value())
        return {};
      const auto [type_id, value_token] = try_parse_obj_type(token, GROUPING::BRACKET);
      if(value_token.empty() && !type_id.empty()) {
        return Obj::create(Any(), OType::OBJ, id_p(ID(*ROUTER_RESOLVE(fURI(type_id)))));
      }
      const bool dot_type = dotType(type_id.toString()); // .obj. in _bcode (apply)
      Option<Obj_p> b = {};
      ////////////////
      b = try_parse_poly_within(value_token);
      if(b.has_value())
        return b.value();
      ////////////////
      if(!dot_type) {
        // dot type
        b = try_parse_no_obj(type_id, value_token);
        if(b.has_value())
          return b.value();
        b = try_parse_bool(type_id, value_token);
        if(b.has_value())
          return b.value();
        b = try_parse_int(type_id, value_token);
        if(b.has_value())
          return b.value();
        b = try_parse_real(type_id, value_token);
        if(b.has_value())
          return b.value();
        // b = try_parse_error(type_id,value_token);
        // if (b.has_value())
        //   return b.value();
        b = try_parse_uri(type_id, value_token);
        if(b.has_value())
          return b.value();
        b = try_parse_str(type_id, value_token);
        if(b.has_value())
          return b.value();
        b = try_parse_lst(type_id, value_token);
        if(b.has_value())
          return b.value();
        b = try_parse_rec(type_id, value_token);
        if(b.has_value())
          return b.value();
        b = try_parse_objs(type_id, value_token);
        if(b.has_value())
          return b.value();
      }
      b = dot_type ? try_parse_bcode(ID(""), token) : try_parse_bcode(type_id, value_token);
      if(b.has_value())
        return b.value();
      b = try_parse_default(type_id, value_token);
      if(b.has_value())
        return b.value();
      return {};
    }

    // type_id?range_id<=domain_id
    static ID parse_type_range_domain(string &token) {
      if(token.length() > 1 && token[0] == '<' && token[token.length() - 1] == '>')
        token = token.substr(1, token.length() - 2);
      StringHelper::trim(token);
      if(token.empty()) return {""};
      string type_token;
      string domain_token;
      string range_token;
      if(const size_t index = token.find('?'); index == string::npos) {
        type_token = token;
        return ID(*ROUTER_RESOLVE(type_token));
      } else {
        type_token = token.substr(0, index);
        if(const size_t index2 = token.find("<="); index2 == string::npos) {
          range_token = token.substr(index + 1, token.length() - index - 1);
          domain_token = string(range_token);
        } else {
          range_token = token.substr(index + 1, index2 - index - 1);
          domain_token = token.substr(index2 + 2, token.length() + index2 - 2);
        }
      }
      return ROUTER_RESOLVE(type_token)->query({
        {"domain", ROUTER_RESOLVE(domain_token)->toString()},
        {"range", ROUTER_RESOLVE(range_token)->toString()}});
    }

    static List<Pair<string, string>> unary_sugars() {
      static List<Pair<string, string>> map = {
        {"-->", "via_inv"}, {"@", "at"},
        {"??", "optional"}, {"-<", "split"},
        {">-", "merge"}, {"~", "match"},
        {"<-", "to"}, {"->", "to_inv"},
        {"|", "block"}, {"^", "lift"},
        {"V", "drop"}, {"*", "from"},
        {"==", "each"}, {";", "end"},
        {"\\", "from_get"}/*, {" +", "plus"},
        {" *", "mult"}*/};
      return map;
    }

    // type_id?range_id<=domain_id[value]
    static Pair<ID, string> try_parse_obj_type(const string &token, const GROUPING grouping) {
      string type_token;
      string value_token;
      const bool grouped = !token.empty() && token[token.length() - 1] == parse_token.to_chars(grouping)[1];
      auto ss = stringstream(token);
      Tracker tracker;
      if(grouped) {
        bool on_type = true;
        bool unary_found = false;
        ////////////////////////////////////////////////////
        while(!ss.eof()) {
          if(on_type) {
            if(StringHelper::look_ahead("<=", &ss, true)) // range<=domain token found
              type_token.append("<=");
            for(const auto &[k, v]: Parser::unary_sugars()) {
              if(StringHelper::look_ahead(k, &ss) || ss.peek() == '.') {
                if(ss.peek()) ss.get(); // drop .
                on_type = false;
                value_token.append(type_token);
                type_token.clear();
                value_token.append(k);
                unary_found = true;
                break;
              }
            }
            if(unary_found)
              break;
            const char c = tracker.track(static_cast<char>(ss.get()));
            if(c == parse_token.to_chars(grouping)[0]) {
              on_type = false;
            } else if((!type_token.empty() && tracker.grouping(parse_token.to_chars(grouping)[0]) &&
                       (
                         // inside value definition (must be bcode)
                         c == '.' || isspace(c))) ||
                      (tracker.open('{') && tracker.closed(parse_token.to_chars(grouping)[1]))) {
              // inside an objs definition (must be sugar bcode)
              value_token.append(type_token);
              value_token += c;
              type_token.clear();
              on_type = false;
            } else {
              type_token += c;
            }
          } else {
            value_token += tracker.track(static_cast<char>(ss.get()));
          }
        }
        if(type_token.empty())
          value_token = token;
        else if(!unary_found)
          value_token = value_token.substr(0, value_token.length() - 2);
      } else {
        type_token = "";
        value_token = token;
      }
      StringHelper::trim(value_token);
      StringHelper::trim(type_token);
      //////////////////////////////////////////////////////////////////////////////
      ID type_id = parse_type_range_domain(type_token);
      LOG(DEBUG, "!gtype id and value!!:!b%s!g?!b%s!g<=!b%s!g[!y%s!g]!!\n",
          type_id.query("").toString().c_str(),
          type_id.query_value("range").value_or("").c_str(),
          type_id.query_value("domain").value_or("").c_str(),
          value_token.c_str());
      return {type_id, value_token};
    }

    static Option<NoObj_p> try_parse_comment(const string &value_token) {
      return value_token.substr(0, 3) == "---" || value_token.substr(0, 3) == "==="
               ? Option<NoObj_p>{noobj()}
               : Option<NoObj_p>();
    }

    static Option<NoObj_p> try_parse_no_obj(const ID &type_id, const string &value_token) {
      return (NOOBJ_FURI->equals(type_id) || (type_id.empty() && value_token == STR(FOS_NOOBJ_TOKEN)))
               ? Option<NoObj_p>{_noobj_}
               : Option<NoObj_p>{};
    }

    static Option<Bool_p> try_parse_bool(const ID &type_id, const string &value_token) {
      return ((strcmp("true", value_token.c_str()) == 0) || (strcmp("false", value_token.c_str()) == 0))
               ? Option<Bool_p>{Bool::to_bool(strcmp("true", value_token.c_str()) == 0,
                                              or_type(type_id, BOOL_FURI))}
               : Option<Bool_p>{};
    }

    static Option<Int_p> try_parse_int(const ID &type_id, const string &value_token) {
      if((value_token[0] != '-' && !isdigit(value_token[0])) || value_token.find_first_of('.') != string::npos)
        return {};
      for(size_t i = 1; i < value_token.length(); i++) {
        if(!isdigit(value_token[i]))
          return {};
      }
      return Option<Int_p>{Int::to_int(stoi(value_token), or_type(type_id, INT_FURI))};
    }

    static Option<Real_p> try_parse_real(const ID &type_id, const string &value_token) {
      if(value_token[0] != '-' && !isdigit(value_token[0]))
        return {};
      bool dotFound = false;
      for(size_t i = 1; i < value_token.length(); i++) {
        if(value_token[i] == '.') {
          if(dotFound)
            return {};
          dotFound = true;
        }
        if(value_token[i] != '.' && !isdigit(value_token[i]))
          return {};
      }
      return dotFound
               ? Option<Real_p>{Real::to_real(stof(value_token), or_type(type_id, REAL_FURI))}
               : Option<Real_p>{};
    }

    static Option<Uri_p> try_parse_uri(const ID &type_id, const string &value_token) {
      if(value_token[0] == '<' && value_token[value_token.length() - 1] == '>')
        return Option<Uri_p>{
          Uri::to_uri(value_token.substr(1, value_token.length() - 2).c_str(), or_type(type_id, URI_FURI))};
      if((value_token[0] == '.' && value_token[1] == '/') ||
         (value_token[0] == '.' && value_token[1] == '.' && value_token[2] == '/'))
        return Option<Uri_p>{Uri::to_uri(value_token, or_type(type_id, URI_FURI))};
      return Option<Uri_p>{};
    }

    static Option<Str_p> try_parse_str(const ID &type_id, const string &value_token) {
      return (value_token[0] == '\'' && value_token[value_token.length() - 1] == '\'')
               ? Option<Str_p>{Str::to_str(value_token.substr(1, value_token.length() - 2).c_str(),
                                           or_type(type_id, STR_FURI))}
               : Option<Str_p>{};
    }

    static Option<Lst_p> try_parse_lst(const ID &type_id, const string &value_token) {
      if(value_token[0] != '[' || value_token[value_token.length() - 1] != ']')
        return {};
      auto ss = stringstream(value_token.substr(1, value_token.length() - 2));
      string value;
      const auto list = make_shared<Obj::LstList>();
      Tracker tracker;
      while(!ss.eof()) {
        if(tracker.closed() && (ss.peek() == ',' || ss.peek() == EOF)) {
          if(Option<Obj_p> element = try_parse_obj(value); element.has_value())
            list->push_back(element.value());
          if(ss.peek() == ',')
            ss.get(); // consume comma
          value.clear();
        } else if(tracker.closed() && StringHelper::look_ahead("=>", &ss)) {
          return {};
        } else {
          const char c = tracker.track(static_cast<char>(ss.get()));
          if(tracker.printable())
            value += c;
        }
      }
      return Option<Lst_p>{Lst::to_lst(list, or_type(type_id, LST_FURI))};
    }

    static Option<Inst_p> try_parse_poly_within(const string &token) {
      if(!(token[0] == '_' && token[1] == '/' && token[token.length() - 2] == '\\' &&
           token[token.length() - 1] == '_'))
        return {};
      LOG(TRACE, "Parsing poly within: %s\n", token.substr(2, token.length() - 4).c_str());
      if(const Option<BCode_p> bcode = try_parse_bcode(*BCODE_FURI, token.substr(2, token.length() - 4)); !bcode.
        has_value())
        return {};
      return {};
      //return Option<Inst_p>{Insts::within(bcode.value())};
    }

    static Option<Rec_p> try_parse_rec(const ID &type_id, const string &value_token) {
      if(value_token[0] != '[' || value_token[value_token.length() - 1] != ']' || value_token.find("=>") ==
         string::npos)
        return {};
      auto ss = stringstream(value_token.substr(1, value_token.length() - 2));
      auto map = Obj::RecMap<>();
      string key;
      string value;
      bool on_key = true;
      Tracker tracker;
      while(!ss.eof() || (!on_key && !value.empty())) {
        //// KEY PARSE
        if(on_key) {
          if(tracker.closed() && StringHelper::look_ahead("=>", &ss)) {
            on_key = false;
          } else if(!ss.eof()) {
            char c = tracker.track(static_cast<char>(ss.get()));
            if(tracker.printable())
              key += c;
          }
        } else {
          //// VALUE PARSE
          if(tracker.closed() && (ss.eof() || ss.peek() == ',' || ss.peek() == EOF)) {
            if(ss.peek() == ',')
              ss.get(); // drop k/v separating comma
            StringHelper::trim(key);
            StringHelper::trim(value);
            const Option<Obj_p> k = try_parse_obj(key);
            const Option<Obj_p> v = try_parse_obj(value);
            if(!k.has_value() || !v.has_value())
              return {};
            map.insert({k.value(), v.value()});
            key.clear();
            value.clear();
            on_key = true;
          } else {
            char c = tracker.track(static_cast<char>(ss.get()));
            if(tracker.printable())
              value += c;
          }
        }
      }
      ////
      auto map2 = make_shared<Obj::RecMap<>>(); // necessary to reverse entries
      map2->insert(map.begin(), map.end());
      ////
      return Option<Rec_p>{Rec::to_rec(map2, or_type(type_id, REC_FURI))};
    }

    static Option<Objs_p> try_parse_error(const ID &type_id, const string &value_token) {
      if(value_token[0] != '<' || value_token[1] != '<' || value_token[value_token.length() - 2] != '>' || value_token[
           value_token.length() - 1] != '>' ||
         value_token.find("@") == string::npos)
        return {};
      const size_t split = value_token.find("@");
      const string obj_token = value_token.substr(2, split - 2);
      const string inst_token = value_token.substr(split + 1, value_token.length() - split - 3);
      const auto [t,v] = try_parse_obj_type(inst_token, GROUPING::PAREN);
      return Option<Error_p>{Obj::to_error(try_parse_obj(obj_token).value(), try_parse_inst(t, v).value(),
                                           or_type(type_id, ERROR_FURI))};
    }

    static Option<Objs_p> try_parse_objs(const ID &type_id, const string &value_token) {
      if(value_token[0] != '{' || value_token[value_token.length() - 1] != '}')
        return {};
      auto ss = stringstream(value_token.substr(1, value_token.length() - 2));
      string value;
      const auto list = make_shared<List<Obj_p>>();
      Tracker tracker;
      while(!ss.eof()) {
        if(tracker.closed() && (ss.peek() == ',' || ss.peek() == EOF)) {
          Option<Obj_p> element = try_parse_obj(value);
          if(element.has_value())
            list->push_back(element.value());
          if(ss.peek() == ',')
            ss.get(); // consume comma
          value.clear();
        } else {
          const char c = tracker.track(static_cast<char>(ss.get()));
          if(tracker.printable())
            value += c;
        }
      }
      return Option<Objs_p>{Objs::to_objs(list, or_type(type_id, OBJS_FURI))};
    }

    static Option<Inst_p> try_parse_inst(const ID &type_id, const string &value_token) {
      auto args = List<ptr<Obj>>();
      auto ss = stringstream(value_token);
      while(!ss.eof()) {
        string arg_token;
        Tracker tracker;
        while(!ss.eof()) {
          const char c = tracker.track(static_cast<char>(ss.get()));
          if(tracker.printable()) {
            arg_token += c;
            if(ss.peek() == ',' && tracker.closed()) {
              ss.get(); // drop arg separating comma
              break;
            }
          }
        }
        StringHelper::trim(arg_token);
        if(!arg_token.empty()) {
          if(const Option<Obj_p> arg_p = try_parse_obj(arg_token); arg_p.has_value())
            args.push_back(arg_p.value());
          else {
            LOG(WARN, "Unable to parse potential argument to !b%s!g[!!%s!g]!!: %s\n", type_id.toString().c_str(),
                value_token.c_str(), arg_token.c_str());
          }
        }
      }
      // resolve relative to obj given no other contextual information
      // at runtime, if not compiled, dynamic dispatch to determine most specific inst implementation
      const Inst_p inst = TYPE_INST_RESOLVER(Obj::to_obj(), Obj::to_inst(args, id_p(type_id)));
      return inst->is_noobj() ? Option<Inst_p>() : Option<Inst_p>(inst);
    }

    static Option<BCode_p> try_parse_bcode(const ID &type_id, const string &value_token) {
      if(value_token == "_") {
        return type_id.empty()
                 ? Option<BCode_p>(Obj::to_bcode())
                 : Option<BCode_p>(Obj::to_bcode({from(vri(type_id))}));
      } // special character for 'no instructions' (no general common parse pattern)
      ///////////////////////////////////////////////////////////////////////////////////////
      //////////////// lookahead to determine if token is potentially _bcode ////////////////
      ///////////////////////////////////////////////////////////////////////////////////////
      bool may_b_code = value_token.find('.') != string::npos || //
                        (value_token.find('(') != string::npos && value_token.find(')') != string::npos);
      if(!may_b_code) {
        for(const auto &[k, v]: Parser::unary_sugars()) {
          if(value_token.find(k) != string::npos || value_token.find(v + "(") != string::npos) {
            may_b_code = true;
            break;
          }
        }
        if(!may_b_code)
          return {};
      }
      //////////////////////////////////////////////////////////////////////////////////////
      List<Inst_p> insts;
      auto ss = std::stringstream(value_token);
      while(!ss.eof()) {
        // _bcode-level (tokens are insts)
        Tracker tracker;
        bool unary = false;
        bool fullbreak = false;
        string inst_token;
        while(!ss.eof()) {
          // inst-level (tokens are chars)
          if(tracker.closed()) {
            for(const auto &[k, v]: Parser::unary_sugars()) {
              if(StringHelper::look_ahead(k, &ss, inst_token.empty())) {
                if(!inst_token.empty()) {
                  LOG(TRACE, "Found beginning of unary %s after parsing %s\n", k.c_str(), inst_token.c_str());
                  fullbreak = true;
                  break;
                }
                inst_token += v;
                StringHelper::eat_space(&ss);
                if(ss.peek() != '(') {
                  // enable paren-use of unary operators
                  inst_token += '(';
                  tracker.parens++;
                  unary = true;
                }
                break;
              }
            }
          }
          if(fullbreak || ss.eof())
            break;
          if(char c = tracker.track(static_cast<char>(ss.get()));
            (tracker.quotes || !isspace(c)) && tracker.printable())
            inst_token += c;
          ///////////////////////////////////////////////////////////////
          if(((unary && tracker.parens == 1) || tracker.parens == 0) && tracker.brackets == 0 && tracker.angles == 0 &&
             tracker.braces == 0 && tracker.within == 0 && !tracker.quotes && (ss.peek() == '.' || sugar_next(&ss))) {
            if(ss.peek() == '.') {
              ss.get();
            }
            break;
          }
        }
        if(unary)
          inst_token += ')';
        if(inst_token.empty())
          continue;
        LOG(TRACE, "Parsing !ginst token!!: !y%s!! with type !y%s!!\n", inst_token.c_str(), type_id.toString().c_str());
        // TODO: end inst with ; sugar
        // Option<Inst_p> within = try_parse_poly_within(inst_token);
        // if(!unary && within.has_value()) {
        //   insts.push_back(within.value());
        // } else {
        auto [inst_op_id, inst_args] = try_parse_obj_type(inst_token, GROUPING::PAREN);
        if(!unary && inst_token.find('(') == string::npos) { //[inst_op_id.toString().length()] != '(') { TODO
          // OBJ AS ARGUMENT (START OR MAP)
          LOG(TRACE, "Parsing !gobj as apply!! (!ysugar!!): %s\n", inst_token.c_str());
          const Option<Obj_p> obj = try_parse_obj(inst_args);
          if(!obj.has_value())
            return {};
          insts.push_back(Obj::to_inst(Obj::to_rec(rmap{{"lhs", obj.value()}}), "map")); // Insts::map(obj.value()));
        } else {
          // CLASSIC INST WITH VARIABLE LENGTH ARGUMENTS WRAPPED IN ( )
          LOG(TRACE, "Parsing !gobj as inst!!: !b%s!g[!!%s!g]!!\n", inst_op_id.toString().c_str(), inst_args.c_str());
          if(const Option<Inst_p> inst = try_parse_inst(inst_op_id, inst_args); inst.has_value()) {
            const Inst_p i = inst.value();
            insts.push_back(Obj::create(i->inst_value(), i->o_type(), i->tid(), i->vid()));
          } else {
            return {};
          }
          // }
        }
        inst_token.clear();
      }
      return Option<BCode_p>{BCode::to_bcode(insts, or_type(type_id, BCODE_FURI))};
    }

    static Option<Uri_p> try_parse_default(const ID &type_id, const string &value_token) {
      return Option<Uri_p>{Uri::to_uri(value_token, or_type(type_id, URI_FURI))};
    }

  private:
    static ID_p or_type(const ID &type_id, const ID_p &default_type) {
      return type_id.empty() ? default_type : id_p(type_id);
    }

    static bool sugar_next(stringstream *ss) {
      if(ss->eof())
        return false;
      for(const auto &[k, v]: Parser::unary_sugars()) {
        if(StringHelper::look_ahead(k, ss, false)) {
          return true;
        }
      }
      return StringHelper::look_ahead("_/", ss, false); // within is a wrapping sugar
      // TODO: make that a universal distinction
    }
  };

  [[maybe_unused]] static Obj_p parse(const string &source) {
    const Obj_p obj = Parser::singleton()->try_parse_obj(source).value_or(noobj());
    return obj;
  }

  [[maybe_unused]] static Obj_p parse(const char *format, ...) {
    va_list arg;
    va_start(arg, format);
    char *source;
    const size_t length = vasprintf(&source, format, arg);
    va_end(arg);
    if(format[strlen(format) - 1] == '\n')
      source[length - 1] = '\n';
    source[length] = '\0';
    const Obj_p obj = Parser::singleton()->try_parse_obj(string(source)).value_or(noobj());
    free(source);
    return obj;
  }
} // namespace fhatos
#endif
