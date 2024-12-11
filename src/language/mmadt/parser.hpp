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
#ifndef mmadt_parser_hpp
#define mmadt_parser_hpp

#include <fhatos.hpp>
#include <language/obj.hpp>
#include <peglib.h>

using namespace peg;
using namespace std;

namespace mmadt {
  class Parser final : public Obj {
    constexpr static auto MMADT_GRAMMAR = R"(
    ROOT           <- OBJ / COMMENT
    COMMENT        <- '---' (!'\n' .)*
    TYPE           <- TYPE_ID '[]'
    NOOBJ          <- < 'noobj' >
    BOOL           <- < 'true' | 'false' >
    INT            <- < [-]?[0-9]+ >
    REAL           <- < [-]?[0-9]+ '.' [0-9]+ >
    STR            <- '\'' < (('\\\'') / (!'\'' .))* > '\''
    FURI           <- < [a-zA-Z:/?#+]+([a-zA-Z0-9:/?=&@#+])* >
    FURI_NO_Q      <- < [a-zA-Z:/]+([a-zA-Z0-9:/=&@#+])* >
    URI            <- '<' FURI '>' / FURI
    REC            <- '[' (OBJ '=>' OBJ)? (',' OBJ '=>' OBJ)* ']'
    LST            <- '[' (OBJ)? (',' OBJ)* ']'
    OBJS           <- '{' (OBJ)? (',' OBJ)* '}'
    INST           <- (FURI '(' (INST_ARG_OBJ)? (',' INST_ARG_OBJ )* ')')
    INST_P         <- INST_SUGAR / INST / NO_CODE_OBJ
    INST_SUGAR     <- WITHIN / FROM / REF / BLOCK / EACH / MERGE
    EMPTY_BCODE    <- '\\_'
    BCODE          <- EMPTY_BCODE / (INST_P ('.' INST_P)*)
    DOM_RNG        <- FURI_NO_Q '?' FURI_NO_Q '<=' FURI_NO_Q
    TYPE_ID        <- DOM_RNG / FURI
    NO_CODE_PROTO  <- NOOBJ / BOOL / INT / REAL / STR / LST / REC / OBJS / URI
    INST_ARG_PROTO <- NOOBJ / BOOL / INT / REAL / STR / LST / REC / OBJS / BCODE / URI
    PROTO          <- BCODE / NO_CODE_PROTO
    NO_CODE_OBJ    <- TYPE / (TYPE_ID '[' NO_CODE_PROTO  ']' ('@' FURI)?) / (NO_CODE_PROTO  ('@' FURI)?)
    INST_ARG_OBJ   <- TYPE / (TYPE_ID '[' INST_ARG_PROTO ']' ('@' FURI)?) / (INST_ARG_PROTO ('@' FURI)?)
    OBJ            <- TYPE / (TYPE_ID '[' PROTO          ']' ('@' FURI)?) / (PROTO          ('@' FURI)?)
    %whitespace    <- [ \t]*
    # ############# INST SUGARS ############## #
    FROM           <- '*' (('(' (URI / BCODE) ')') / (URI / BCODE))
    REF            <- '->' INST_ARG_OBJ
    BLOCK          <- '|' OBJ
    # PASS         <- '-->' INST_ARG_OBJ
    MERGE          <- '>' < [0-9]* > '-'
    EACH           <- '==' INST_ARG_OBJ
    WITHIN         <- '_/' OBJ '\\_'
  )";

  public:
    static ptr<Parser> singleton(const ID &id = ID("/parser/")) {
      static auto parser_p = ptr<Parser>(new Parser(id));
      return parser_p;
    }

  protected:
    peg::parser parser_{};

    explicit Parser(const ID &id = ID("/parser/")) : Obj(share(RecMap<>{}), OType::REC, REC_FURI, id_p(id)) {
      this->parser_ = peg::parser();
      this->parser_.set_logger(
        [this](const size_t line, const size_t column, const string &message, const string &rule) {
          throw fError("!^r%i^!y^--!r%s!! at line !y%i!!:!y%i!! !g[!r%s!g]!!",
                       column - 1, message.c_str(), line, column, rule.c_str());
        });
      this->parser_.enable_packrat_parsing();
      this->parser_.load_grammar(MMADT_GRAMMAR);
      LOG_OBJ(INFO, this, "!ymmADT grammar!! loaded\n");
      /////////////////////////////////////////////////////////////////////////////////
      auto y = [this](const SemanticValues &vs) -> Obj_p {
        LOG_OBJ(TRACE, this, "rule obj with choice %i [%s]\n", vs.choice(), vs.choice() == 0 ? "a[b]@c" : "b@c");
        switch(vs.choice()) {
          case 0: {
            return any_cast<Obj_p>(vs[0]);
          }
          case 1: { // x[a]@xyz
            LOG_OBJ(TRACE, this, "rule obj with choice 0 has value for v[0]: %i \n", vs[0].has_value());
            const ID_p type_id = id_p(*ROUTER_RESOLVE(*any_cast<fURI_p>(vs[0])));
            LOG_OBJ(TRACE, this, "rule obj with type %s\n", type_id->toString().c_str());
            const auto &[v,o] = *any_cast<Pair_p<Any, OType>>(vs[1]);
            return Obj::create(v, o, type_id, vs.size() == 3 ? id_p(*std::any_cast<fURI_p>(vs[2])) : nullptr);
          }
          case 2: { // a@xyz
            const auto &[v,o] = *any_cast<Pair_p<Any, OType>>(vs[0]);
            return Obj::create(v, o, OTYPE_FURI.at(o),
                               vs.size() == 2 ? id_p(*std::any_cast<fURI_p>(vs[1])) : nullptr);
          }
          default: {
            throw fError("unknown state");
          }
        }
      };
      this->parser_["OBJ"] = y;
      this->parser_["OBJ"].enter = [](const Context &c, const char *s, size_t n, any &dt) {
        LOG_OBJ(TRACE, Parser::singleton(), "entering rule !bobj!! with token !y%s!!\n", s);
      };
      this->parser_["NO_CODE_OBJ"] = y;
      this->parser_["INST_ARG_OBJ"] = y;

      auto x = [](const SemanticValues &vs) -> Pair_p<Any, OType> {
        return any_cast<Pair_p<Any, OType>>(vs[0]);
      };
      this->parser_["PROTO"] = x;
      this->parser_["NO_CODE_PROTO"] = x;

      this->parser_["FURI"] = [](const SemanticValues &vs) -> fURI_p {
        return furi_p(vs.token_to_string());
      };
      this->parser_["FURI"].enter = [](const Context &c, const char *s, size_t n, any &dt) {
        LOG_OBJ(TRACE, Parser::singleton(), "entering rule !bfuri!! with token !y%s!!\n", s);
      };

      this->parser_["FURI"].predicate = [](const SemanticValues &vs, const std::any &, std::string &msg) {
        const char last = vs.token_to_string()[vs.token_to_string().length() - 1];
        msg = "furis can not end with . nor _";
        return last != '.' && last != '_';
      };

      this->parser_["COMMENT"] = [](const SemanticValues &) -> Obj_p {
        return Obj::to_objs();
      };

      ///////////////////////////////////////////////////

      this->parser_["OBJS"] = [](const SemanticValues &vs) -> Pair_p<Any, OType> {
        const auto list = make_shared<List<Obj_p>>();
        for(const auto &v: vs) {
          list->push_back(any_cast<Obj_p>(v));
        }
        return make_shared<Pair<Any, OType>>(list, OType::OBJS);
      };

      this->parser_["NOOBJ"] = [](const SemanticValues &) -> Pair_p<Any, OType> {
        return make_shared<Pair<Any, OType>>(nullptr, OType::NOOBJ);
      };

      this->parser_["TYPE"] = [](const SemanticValues &vs) -> Obj_p {
        return Obj::create(Any(), OType::OBJ, id_p(*ROUTER_RESOLVE(*any_cast<fURI_p>(vs[0]))));
      };


      this->parser_["BOOL"] = [](const SemanticValues &vs) -> Pair_p<Any, OType> {
        return make_shared<Pair<Any, OType>>(vs.token_to_string() == "true", OType::BOOL);
      };

      this->parser_["INT"] = [](const SemanticValues &vs) -> Pair_p<Any, OType> {
        return make_shared<Pair<Any, OType>>(vs.token_to_number<FOS_INT_TYPE>(), OType::INT);
      };
      this->parser_["INT"].enter = [](const Context &c, const char *s, size_t n, any &dt) {
        LOG_OBJ(TRACE, Parser::singleton(), "entering rule !bint!! with token !y%s!!\n", s);
      };

      this->parser_["REAL"] = [](const SemanticValues &vs) -> Pair_p<Any, OType> {
        return make_shared<Pair<Any, OType>>(vs.token_to_number<FOS_REAL_TYPE>(), OType::REAL);
      };

      this->parser_["STR"] = [](const SemanticValues &vs) -> Pair_p<Any, OType> {
        return make_shared<Pair<Any, OType>>(vs.token_to_string(), OType::STR);
      };

      this->parser_["URI"] = [](const SemanticValues &vs) -> Pair_p<Any, OType> {
        return make_shared<Pair<Any, OType>>(*any_cast<fURI_p>(vs[0]), OType::URI);
      };

      this->parser_["LST"] = [](const SemanticValues &vs) -> Pair_p<Any, OType> {
        const auto list = make_shared<List<Obj_p>>();
        for(const auto &v: vs) {
          list->push_back(any_cast<Obj_p>(v));
        }
        return make_shared<Pair<Any, OType>>(list, OType::LST);
      };

      this->parser_["REC"] = [](const SemanticValues &vs) -> Pair_p<Any, OType> {
        const auto map = Obj::RecMap_p<>();
        for(int i = 0; i < vs.size(); i = i + 2) {
          map->emplace(any_cast<Obj_p>(vs[i]), any_cast<Obj_p>(vs[i + 1]));
        }
        return make_shared<Pair<Any, OType>>(map, OType::REC);
      };

      ///////////////////////////////////////////////////

      this->parser_["INST"] = [](const SemanticValues &vs) -> Inst_p {
        InstArgs list;
        const ID_p op = id_p(*any_cast<fURI_p>(vs[0]));
        for(int i = 1; i < vs.size(); i++) {
          list.push_back(any_cast<Obj_p>(vs[i]));
        }
        return Obj::to_inst(list, id_p(*ROUTER_RESOLVE(fURI(*op))));
      };
      this->parser_["INST"].enter = [](const Context &c, const char *s, size_t n, any &dt) {
        LOG_OBJ(TRACE, Parser::singleton(), "entering rule !binst!! with token !y%s!!\n", s);
      };

      this->parser_["DOM_RNG"] = [](const SemanticValues &vs) -> fURI_p {
        return furi_p(any_cast<fURI_p>(vs[0])->query({
          {"domain", any_cast<fURI_p>(vs[2])->toString()},
          {"range", any_cast<fURI_p>(vs[1])->toString()}}));
      };

      this->parser_["FURI_NO_Q"] = [](const SemanticValues &vs) -> fURI_p {
        return furi_p(vs.token_to_string());
      };

      this->parser_["BCODE"] = [](const SemanticValues &vs) -> Pair_p<Any, OType> {
        const auto list = make_shared<InstList>();
        switch(vs.choice()) {
          case 0: return make_shared<Pair<Any, OType>>(list, OType::BCODE);
          case 1: {
            for(uint16_t i = 0; i < vs.size(); i++) {
              const Obj_p obj_or_inst = any_cast<Obj_p>(vs[i]);
              list->push_back(obj_or_inst->otype_ == OType::INST
                                ? obj_or_inst
                                : Obj::to_inst({obj_or_inst}, id_p(*ROUTER_RESOLVE("map"))));
            }
            return make_shared<Pair<Any, OType>>(list, OType::BCODE);
          }
          default: throw fError("unknown choice");
        }
      };
      this->parser_["BCODE"].enter = [](const Context &c, const char *s, size_t n, any &dt) {
        LOG_OBJ(TRACE, Parser::singleton(), "entering rule !bbcode!! with token !y%s!! and choice %i\n", s,
                c.value_stack.back()->choice());
      };
      /////////////////////////////////////////////////////////////////////////////////////
      this->parser_["FROM"] = [](const SemanticValues &vs) -> Inst_p {
        const auto &[v,o] = *any_cast<Pair_p<Any, OType>>(vs[0]);
        return Obj::to_inst({Obj::create(v, o, OTYPE_FURI.at(o))}, id_p(*ROUTER_RESOLVE("from")));
      };
      this->parser_["BLOCK"] = [](const SemanticValues &vs) -> Inst_p {
        return Obj::to_inst({any_cast<Obj_p>(vs[0])}, id_p(*ROUTER_RESOLVE("block")));
      };
      this->parser_["REF"] = [](const SemanticValues &vs) -> Inst_p {
        return Obj::to_inst({any_cast<Obj_p>(vs[0])}, id_p(*ROUTER_RESOLVE("to_inv")));
      };
      this->parser_["WITHIN"] = [](const SemanticValues &vs) -> Inst_p {
        return Obj::to_inst({any_cast<Obj_p>(vs[0])}, id_p(*ROUTER_RESOLVE("within")));
      };
      this->parser_["EACH"] = [](const SemanticValues &vs) -> Inst_p {
        return Obj::to_inst({any_cast<Obj_p>(vs[0])}, id_p(*ROUTER_RESOLVE("each")));
      };
      this->parser_["MERGE"] = [](const SemanticValues &vs) -> Inst_p {
        return Obj::to_inst({jnt(!vs.token_to_string().empty() ? vs.token_to_number<FOS_INT_TYPE>() : INT32_MAX)},
                            id_p(*ROUTER_RESOLVE("merge")));
      };
      /////////////////////////////////////////////////////////////////////////////////////
      OBJ_PARSER = [](const string &obj_string) {
        Obj_p obj;
        Parser::singleton()->parser_.parse(std::string_view(obj_string), obj);
        return obj;
      };
      Options::singleton()->parser<Obj>(OBJ_PARSER);
    }
  };
}

#endif
