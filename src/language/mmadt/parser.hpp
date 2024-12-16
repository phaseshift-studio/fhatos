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

#include "../../fhatos.hpp"
#include "../../language/obj.hpp"
#include "../util/peglib.h"

using namespace peg;
using namespace std;

namespace mmadt {
  class Parser final : public Obj {
  public:
    static ptr<Parser> singleton(const ID &id = ID("/parser/")) {
      static auto parser_p = ptr<Parser>(new Parser(id));
      return parser_p;
    }

  private:
    Definition WS, ROOT, COMMENT, FURI, FURI_INLINE, FURI_NO_Q, NOOBJ, BOOL, INT, REAL, STR, LST, REC, URI, INST, INST_P
        , INST_SUGAR
        ,
        INST_ARG_OBJ, OBJS, OBJ, TYPE, TYPE_ID, NO_CODE_OBJ, NO_CODE_PROTO, INST_ARG_PROTO, BCODE, BCODE_P, PROTO,
        EMPTY_BCODE,
        DOM_RNG;
#ifndef FOS_SUGARLESS_MMADT
    Definition AT, REPEAT, FROM, REF, PASS, MULT, PLUS, BLOCK, WITHIN, MERGE, SPLIT, EACH;
#endif
    QuadConsumer<const size_t, const size_t, const string, const string> PARSER_LOGGER =
        [](const size_t line, const size_t column, const string &message, const string &rule) {
      throw fError("!^r%i^!y^--!r%s!! at line !y%i!!:!y%i!! !g[!r%s!g]!!",
                   column - 1, message.c_str(), line, column, rule.c_str());
    };

  protected:
    Obj_p parse(const string &mmadt) const {
      Obj_p result;
      LOG_OBJ(TRACE, this, "!yparsing!! %s\n", mmadt.c_str());
      if(Definition::Result ret = OBJ.parse_and_get_value<Obj_p>(mmadt.c_str(), result, nullptr, PARSER_LOGGER);
        ret.ret) {
        LOG_OBJ(TRACE, this, "!gsuccessful!! parse of %s\n", mmadt.c_str());
      } else {
        ret.error_info.output_log(PARSER_LOGGER, mmadt.c_str(), mmadt.length());
      }
      return result;
    }

    explicit Parser(const ID &id = ID("/parser/")) : Obj(share(RecMap<>{}), OType::REC, REC_FURI, id_p(id)) {
      static auto noobj_action = [](const SemanticValues &) -> Pair_p<Any, OType> {
        return make_shared<Pair<Any, OType>>(nullptr, OType::NOOBJ);
      };
      static auto bool_action = [](const SemanticValues &vs) -> Pair_p<Any, OType> {
        return make_shared<Pair<Any, OType>>(vs.choice() == 0, OType::BOOL);
      };
      static auto int_action = [](const SemanticValues &vs) -> Pair_p<Any, OType> {
        return make_shared<Pair<Any, OType>>(vs.token_to_number<FOS_INT_TYPE>(), OType::INT);
      };
      static auto real_action = [](const SemanticValues &vs) -> Pair_p<Any, OType> {
        return make_shared<Pair<Any, OType>>(vs.token_to_number<FOS_REAL_TYPE>(), OType::REAL);
      };
      static auto str_action = [](const SemanticValues &vs) -> Pair_p<Any, OType> {
        return make_shared<Pair<Any, OType>>(vs.token_to_string(), OType::STR);
      };
      static auto uri_action = [](const SemanticValues &vs) -> Pair_p<Any, OType> {
        return make_shared<Pair<Any, OType>>(vs.choice() == 0 ? fURI("") : *any_cast<fURI_p>(vs[0]), OType::URI);
      };
      static auto lst_action = [](const SemanticValues &vs) -> Pair_p<Any, OType> {
        const auto list = make_shared<List<Obj_p>>();
        for(const auto &v: vs) {
          list->push_back(any_cast<Obj_p>(v));
        }
        return make_shared<Pair<Any, OType>>(list, OType::LST);
      };
      static auto rec_action = [](const SemanticValues &vs) -> Pair_p<Any, OType> {
        const auto map = make_shared<Obj::RecMap<>>();
        for(int i = 0; i < vs.size(); i = i + 2) {
          map->insert(make_pair<Obj_p, Obj_p>(any_cast<Obj_p>(vs[i]), any_cast<Obj_p>(vs[i + 1])));
        }
        return make_shared<Pair<Any, OType>>(map, OType::REC);
      };
      static auto objs_action = [](const SemanticValues &vs) -> Pair_p<Any, OType> {
        const auto list = make_shared<List<Obj_p>>();
        for(const auto &v: vs) {
          list->push_back(any_cast<Obj_p>(v));
        }
        return make_shared<Pair<Any, OType>>(list, OType::OBJS);
      };
      static auto inst_action = [](const SemanticValues &vs) -> Inst_p {
        InstArgs list;
        const ID_p op = id_p(*any_cast<fURI_p>(vs[0]));
        for(int i = 1; i < vs.size(); i++) {
          list.push_back(any_cast<Obj_p>(vs[i]));
        }
        return Obj::to_inst(list, id_p(*ROUTER_RESOLVE(fURI(*op))));
      };

      static auto furi_action = [](const SemanticValues &vs) {
        return furi_p(vs.token_to_string());
      };

      static auto dom_rng_action = [](const SemanticValues &vs) -> fURI_p {
        return furi_p(any_cast<fURI_p>(vs[0])->query({
          {FOS_DOMAIN, any_cast<fURI_p>(vs[2])->toString()},
          {FOS_RANGE, any_cast<fURI_p>(vs[1])->toString()}}));
      };

      static auto empty_bcode_action = [](const SemanticValues &) -> BCode_p {
        return Obj::to_bcode();
      };

      static auto bcode_action = [](const SemanticValues &vs) -> Pair_p<Any, OType> {
        const auto list = make_shared<InstList>();
        switch(vs.choice()) {
          case 0: return make_shared<Pair<Any, OType>>(list, OType::BCODE);
          case 1: {
            for(uint16_t i = 0; i < vs.size(); i++) {
              const Obj_p obj_or_inst = any_cast<Obj_p>(vs[i]);
              list->push_back(obj_or_inst->otype_ == OType::INST
                                ? obj_or_inst
                                : Obj::to_inst({obj_or_inst}, id_p(*ROUTER_RESOLVE(0 == i ? "start" : "map"))));
            }
            return make_shared<Pair<Any, OType>>(list, OType::BCODE);
          }
          default: throw fError("unknown choice");
        }
      };

      static auto type_action = [](const SemanticValues &vs) -> Obj_p {
        return Obj::create(Any(), OType::OBJ, id_p(*ROUTER_RESOLVE(*any_cast<fURI_p>(vs[0]))));
      };

      static auto comment_action = [](const SemanticValues &vs) -> Obj_p {
        return Obj::to_objs();
      };

      static auto obj_action = [this](const SemanticValues &vs) -> Obj_p {
        LOG_OBJ(TRACE, Parser::singleton(), "obj_action: %i\n", vs.choice());
        enum class OBJ_STATE { OBJ, TYPE_VALUE, VALUE };
        switch(static_cast<OBJ_STATE>(vs.choice())) {
          case OBJ_STATE::OBJ: {
            return any_cast<Obj_p>(vs[0]);
          }
          case OBJ_STATE::TYPE_VALUE: { // x[a]@xyz
            const ID_p type_id = id_p(*ROUTER_RESOLVE(*any_cast<fURI_p>(vs[0])));
            const auto &[v,o] = *any_cast<Pair_p<Any, OType>>(vs[1]);
            return Obj::create(v, o, type_id, vs.size() == 3 ? id_p(*std::any_cast<fURI_p>(vs[2])) : nullptr);
          }
          case OBJ_STATE::VALUE: { // a@xyz
            const auto &[v,o] = *any_cast<Pair_p<Any, OType>>(vs[0]);
            return Obj::create(v, o, OTYPE_FURI.at(o),
                               vs.size() == 2 ? id_p(*std::any_cast<fURI_p>(vs[1])) : nullptr);
          }
        }
      };
      //////////////////////////////////////////////////////////////////////////
      static auto enter_y = [](const string &rule) {
        return [rule](const Context &c, const char *s, size_t n, any &dt) {
          LOG_OBJ(TRACE, Parser::singleton(), "entering rule !b%s!! with token !y%s!!\n", rule.c_str(), s);
        };
      };
      //////////////////////////////////////////////////////////////////////////
#ifndef FOS_SUGARLESS_MMADT
      static auto at_action = [](const SemanticValues &vs) -> Inst_p {
        const auto &[v,o] = *any_cast<Pair_p<Any, OType>>(vs[0]);
        return Obj::to_inst({Obj::create(v, o, OTYPE_FURI.at(o))}, id_p(*ROUTER_RESOLVE("at")));
      };
      static auto from_action = [](const SemanticValues &vs) -> Inst_p {
        const auto &[v,o] = *any_cast<Pair_p<Any, OType>>(vs[0]);
        return Obj::to_inst({Obj::create(v, o, OTYPE_FURI.at(o))}, id_p(*ROUTER_RESOLVE("from")));
      };
      static auto repeat_action = [](const SemanticValues &vs) -> Inst_p {
        const Obj_p bcode = any_cast<BCode_p>(vs);
        return Obj::to_bcode({Obj::to_inst({bcode}, id_p("repeat"))});
      };
      static auto block_action = [](const SemanticValues &vs) -> Inst_p {
        return Obj::to_inst({any_cast<Obj_p>(vs[0])}, id_p(*ROUTER_RESOLVE("block")));
      };
      static auto ref_action = [](const SemanticValues &vs) -> Inst_p {
        return Obj::to_inst({any_cast<Obj_p>(vs[0])}, id_p(*ROUTER_RESOLVE("to_inv")));
      };
      static auto within_action = [](const SemanticValues &vs) -> Inst_p {
        return Obj::to_inst({any_cast<Obj_p>(vs[0])}, id_p(*ROUTER_RESOLVE("within")));
      };
      static auto each_action = [](const SemanticValues &vs) -> Inst_p {
        return Obj::to_inst({any_cast<Obj_p>(vs[0])}, id_p(*ROUTER_RESOLVE("each")));
      };
      static auto merge_action = [](const SemanticValues &vs) -> Inst_p {
        return Obj::to_inst(vs.empty() ? InstArgs() : InstArgs{any_cast<Obj_p>(vs[0])}, id_p(*ROUTER_RESOLVE("merge")));
      };
      static auto split_action = [](const SemanticValues &vs) -> Inst_p {
        return Obj::to_inst({any_cast<Obj_p>(vs[0])}, id_p(*ROUTER_RESOLVE("split")));
      };

      static auto pass_action = [](const SemanticValues &vs) -> Inst_p {
        return Obj::to_inst({any_cast<Obj_p>(vs[0]), dool(false)}, id_p(*ROUTER_RESOLVE("to_inv")));
      };

      static auto plus_action = [](const SemanticValues &vs) -> Inst_p {
        return Obj::to_inst({any_cast<Obj_p>(vs[0])}, id_p(*ROUTER_RESOLVE("plus")));
      };

      static auto mult_action = [](const SemanticValues &vs) -> Inst_p {
        return Obj::to_inst({any_cast<Obj_p>(vs[0])}, id_p(*ROUTER_RESOLVE("mult")));
      };
#endif
      /*
        ROOT           <- OBJ / COMMENT
          COMMENT        <- '---' (!'\n' .)*
          TYPE           <- TYPE_ID '[]'
          NOOBJ          <-  'noobj'
          BOOL           <-  'true' / 'false'
          INT            <- < [-]?[0-9]+ >
          REAL           <- < [-]?[0-9]+ '.' [0-9]+ >
          STR            <- '\'' < (('\\\'') / (!'\'' .))* > '\''
          FURI           <- < [a-zA-Z:/?_.#+]+([a-zA-Z0-9:/?_=&@.#+])* >
          FURI_NO_Q      <- < [a-zA-Z:/_.#+]+([a-zA-Z0-9:/_=&@.#+])* >
          URI            <- '<' FURI '>' / FURI
          REC            <- '[' (OBJ '=>' OBJ)? (',' OBJ '=>' OBJ)* ']'
          LST            <- '[' (OBJ)? (',' OBJ)* ']'
          OBJS           <- '{' (OBJ)? (',' OBJ)* '}'
          INST           <- (FURI '(' (INST_ARG_OBJ)? (',' INST_ARG_OBJ )* ')')
          INST_P         <- INST_SUGAR / INST / NO_CODE_OBJ
          INST_SUGAR     <- PLUS / MULT / WITHIN / FROM / PASS / REF / BLOCK / EACH / MERGE / SPLIT
          EMPTY_BCODE    <- '_'
          BCODE          <- EMPTY_BCODE / (INST_P ('.'? INST_P)*)
          DOM_RNG        <- FURI_NO_Q '?' FURI_NO_Q '<=' FURI_NO_Q
          TYPE_ID        <- DOM_RNG / FURI
          # POLY         <- LST / REC
          NO_CODE_PROTO  <- NOOBJ / BOOL / REAL / INT / STR / LST / REC / OBJS / URI
          INST_ARG_PROTO <- NOOBJ / BOOL / INT / REAL / STR / LST / REC / OBJS / BCODE / URI
          PROTO          <- REAL / BCODE / NO_CODE_PROTO
          NO_CODE_OBJ    <- TYPE / (TYPE_ID '[' NO_CODE_PROTO  ']' ('@' FURI)?) / (NO_CODE_PROTO  ('@' FURI)?)
          INST_ARG_OBJ   <- TYPE / (TYPE_ID '[' INST_ARG_PROTO ']' ('@' FURI)?) / (INST_ARG_PROTO ('@' FURI)?)
          OBJ            <- TYPE / (TYPE_ID '[' PROTO          ']' ('@' FURI)?) / (PROTO          ('@' FURI)?)
          %whitespace    <- [ \t]*
          ###############################################################
          ########################  INST SUGARS  ########################
          ###############################################################
          FROM           <- ('*' (URI / BCODE)) / ('*(' (URI / BCODE) ')')
          REF            <- ('->' INST_ARG_OBJ) / ('->(' INST_ARG_OBJ ')')
          BLOCK          <- ('|' OBJ) / ('|(' OBJ ')')
          PASS           <- ('-->' INST_ARG_OBJ) / ('-->(' INST_ARG_OBJ ')')
          MERGE          <- '>' < [0-9]* > '-'
          SPLIT          <- ('-<' INST_ARG_OBJ) / ('-<(' INST_ARG_OBJ ')')
          EACH           <- ('==' INST_ARG_OBJ) / ('==(' INST_ARG_OBJ ')')
          WITHIN         <- '_/' OBJ '\\_'
          PLUS           <- ('+' OBJ) / ('+(' OBJ ')')
          MULT           <- ('x' OBJ) / ('x(' OBJ ')')
       */
      WS <= zom(cls(" \t"));
      ROOT <= cho(OBJ, COMMENT);
      COMMENT <= seq(lit("---"), zom(ncls("\n"))), comment_action;
      TYPE <= seq(~WS, TYPE_ID, lit("[]"), ~WS), type_action;
      NOOBJ <= lit("noobj"), noobj_action;
      BOOL <= cho(lit("true"), lit("false")), bool_action;
      INT <= seq(opt(chr('-')), oom(cls("0-9"))), int_action;
      REAL <= seq(opt(chr('-')), oom(cls("0-9")), chr('.'), oom(cls("0-9"))), real_action;
      STR <= seq(chr('\''), tok(zom(cho(lit("\\'"), ncls("\'")))), chr('\'')), str_action;
      FURI <= tok(seq(oom(cls("a-zA-Z:/?_.#+")), zom(seq(npd(lit("=>")), cls("a-zA-Z0-9:/?_=&@.#+"))))), furi_action;
      FURI_INLINE <= tok(seq(oom(cls("a-zA-Z:/?_#+")), zom(seq(npd(lit("=>")), cls("a-zA-Z0-9:/?_=&#+"))))),
          furi_action;
      FURI_NO_Q <= tok(seq(oom(cls("a-zA-Z:/_.#+")), zom(seq(npd(lit("=>")), cls("a-zA-Z0-9:/_=&@.#+"))))), furi_action;
      URI <= cho(lit("<>"), seq(chr('<'), FURI, chr('>')), FURI_INLINE, FURI), uri_action;
      REC <= seq(chr('['), opt(seq(OBJ, lit("=>"), OBJ)),
                 zom(seq(chr(','), OBJ, lit("=>"), OBJ)), chr(']')),
          rec_action;
      LST <= seq(chr('['), opt(OBJ), zom(seq(chr(','), OBJ)), chr(']')), lst_action;
      OBJS <= seq(chr('{'), opt(OBJ), zom(seq(chr(','), OBJ)), chr('}')), objs_action;
      INST <= seq(FURI, chr('('), opt(INST_ARG_OBJ), zom(seq(chr(','), INST_ARG_OBJ)), chr(')')), inst_action;
      INST_P <= cho(INST_SUGAR, INST, NO_CODE_OBJ);
      EMPTY_BCODE <= lit("_"), empty_bcode_action;
      BCODE <= cho(EMPTY_BCODE, seq(INST_P, zom(cho(INST_SUGAR, seq(lit("."), INST_P))))), bcode_action;
      BCODE_P <= cho(seq(chr('('), BCODE, chr(')')), BCODE);
      NO_CODE_PROTO <= cho(NOOBJ, BOOL, REAL, INT, STR, LST, REC, OBJS, URI);
      INST_ARG_PROTO <= cho(NOOBJ, BOOL, REAL, INT, STR, LST, REC, OBJS, BCODE_P, URI);
      PROTO <= cho(REAL, BCODE_P, NO_CODE_PROTO);
      DOM_RNG <= seq(FURI_NO_Q, chr('?'), FURI_NO_Q, lit("<="), FURI_NO_Q), dom_rng_action;
      TYPE_ID <= cho(DOM_RNG, FURI), furi_action;
      NO_CODE_OBJ <= cho(TYPE, seq(~WS, TYPE_ID, chr('['), NO_CODE_PROTO, chr(']'), opt(seq(chr('@'), FURI)), ~WS),
                         seq(~WS, NO_CODE_PROTO, opt(seq(chr('@'), FURI)), ~WS)), obj_action;
      INST_ARG_OBJ <= cho(TYPE, seq(~WS, TYPE_ID, chr('['), INST_ARG_PROTO, chr(']'), opt(seq(chr('@'), FURI)), ~WS),
                          seq(~WS, INST_ARG_PROTO, opt(seq(chr('@'), FURI)), ~WS)), obj_action;
      OBJ <= cho(TYPE, seq(~WS, TYPE_ID, chr('['), PROTO, chr(']'), opt(seq(chr('@'), FURI)), ~WS),
                 seq(~WS, PROTO, opt(seq(chr('@'), FURI)), ~WS)), obj_action;
      /////////////////////////////////////////////////////////////////
      ///////////////////////  INST SUGARS ////////////////////////////
      /////////////////////////////////////////////////////////////////
#ifndef FOS_SUGARLESS_MMADT
      INST_SUGAR <= cho(AT, REPEAT, PLUS, MULT, WITHIN, FROM, PASS, REF, BLOCK, EACH, MERGE, SPLIT);
      AT <= cho(seq(chr('@'), cho(URI, BCODE_P)), seq(lit("@("), INST_ARG_OBJ, chr(')'))), at_action;
      REPEAT <= seq(chr('('), INST_ARG_OBJ, lit(")^*")), repeat_action; // )^*(until,emit)
      FROM <= cho(seq(chr('*'), cho(URI, BCODE_P)), seq(lit("*("), cho(URI, BCODE_P), chr(')'))),
          from_action;
      REF <= cho(seq(lit("->"), INST_ARG_OBJ), seq(lit("->("), INST_ARG_OBJ, chr(')'))), ref_action;
      BLOCK <= seq(lit("|"), INST_ARG_OBJ), block_action;
      PASS <= seq(lit("-->"), INST_ARG_OBJ), pass_action;
      MERGE <= seq(~WS, chr('>'), opt(INST_ARG_OBJ), chr('-'), ~WS), merge_action;
      SPLIT <= seq(lit("-<"), INST_ARG_OBJ), split_action;
      EACH <= seq(lit("=="), INST_ARG_OBJ), each_action;
      WITHIN <= seq(lit("_/"), OBJ, lit("\\_")), within_action;
      PLUS <= seq(chr('+'), chr(' '), INST_ARG_OBJ), plus_action;
      MULT <= seq(chr('x'), chr(' '), INST_ARG_OBJ), mult_action;
      ///////////////////////// DEBUG UTILITIES //////////////////////////////////////////
      REPEAT.enter = enter_y("repeat");
#endif
      ///////////////////////////////// PREDICATES ///////////////////////////////////////
      BCODE.predicate = [](const SemanticValues &vs, const std::any &dt, std::string &msg) -> bool {
        return vs.size() != 1 || any_cast<Obj_p>(vs[0])->is_code();
      };
      FURI.predicate = [](const SemanticValues &vs, const std::any &, std::string &msg) {
        const char last = vs.token_to_string()[vs.token_to_string().length() - 1];
        msg = "furis can not end with . nor _ nor =";
        return last != '.' && last != '_' && last != '=';
      };
      ///////////////////////// DEBUG UTILITIES //////////////////////////////////////////
      OBJ.enter = enter_y("obj");
      TYPE.enter = enter_y("type");
      PROTO.enter = enter_y("proto");
      //////////////////////// WHITESPACE IGNORING ///////////////////////////////////////
      ROOT.whitespaceOpe = make_shared<Whitespace>(Whitespace(zom(cls(" \t"))));
      ROOT.enablePackratParsing = true;
      ROOT.eoi_check = true;
      /////////////////////////////////////////////////////////////////////////////////////
      OBJ_PARSER = [](const string &obj_string) {
        return Parser::singleton()->parse(obj_string);
      };
      Options::singleton()->parser<Obj>(OBJ_PARSER);
    }
  };
}

#endif
