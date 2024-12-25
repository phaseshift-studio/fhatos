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

#define WRAP(LEFT,DEFINITION,RIGHT) cho(seq(ign(lit((LEFT))), (DEFINITION), ign(lit((RIGHT)))),(DEFINITION))

namespace mmadt {
  class Tracker {
    int8_t parens = 0;
    int8_t brackets = 0;
    int8_t angles = 0;
    int8_t braces = 0;
    int8_t within = 0;
    int8_t multi_comment = 0;
    bool quotes = false;
    char last[3] = {'\0', '\0', '\0'};

  public:
    Tracker() = default;

    void track(const string &line) {
      for(const auto &c: line) {
        track(c);
      }
    }


    [[nodiscard]] bool closed() const {
      return multi_comment == 0 && parens == 0 && brackets == 0 && angles == 0 && braces == 0 && within == 0 && !quotes;
    }

    void clear() {
      multi_comment = 0;
      parens = 0;
      brackets = 0;
      angles = 0;
      braces = 0;
      within = 0;
      quotes = false;
      last[0] = last[1] = last[2] = '\0';
    }


    char track(const char c) {
      if(c != '\'' && quotes) {
        // do nothing
      } else if(c == '\'') {
        quotes = !quotes;
      } else if((c == '=' || c == '-')) {
        if(last[0] == '<') // <- <=
          angles--;
        else if(last[0] == '>') // >- >=
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
        if(last[0] != '-' && last[0] != '=') // -< =<
          angles++;
      } else if(c == '>') {
        if(last[0] != '-' && last[0] != '=') // -> =>
          angles--;
      } else if(c == '{')
        braces++;
      else if(c == '}')
        braces--;
      else if(c == '/' && last[0] == '_') // _/
        within++;
      else if(c == '_' && last[0] == '\\') // \_
        within--;
      else if(c == '#' && last[0] == '#' && last[1] == '#' && last[2] != '#') {
        if(multi_comment == -1)
          multi_comment++;
        else
          multi_comment = -1;
      }
      //////////////////////////
      last[2] = last[1];
      last[1] = last[0];
      last[0] = c;
      return c;
    }
  };

  class Parser final : public Obj {
  public:
    static ptr<Parser> singleton(const ID &id = ID("/parser/")) {
      static auto parser_p = ptr<Parser>(new Parser(id));
      return parser_p;
    }

  private:
    Definition
        WS, ROOT, ARGS, ARGS_LST, ARGS_REC, CODE, COMMENT, SINGLE_COMMENT, MULTI_COMMENT, FURI, FURI_INLINE, FURI_NO_Q,
        NOOBJ, BOOL, INT, REAL, STR, LST, REC, URI, INST, INST_P, SET,
        INST_ARG_OBJ, OBJS, OBJ, TYPE, TYPE_ID, NO_CODE_OBJ, CARDINALITY, RANGE, COEF, VALUE_ID,
        NO_CODE_PROTO, INST_ARG_PROTO, BCODE, PROTO, START, EMPTY,
        EMPTY_BCODE, DOM_RNG, INST_SUGAR;
#ifndef FOS_SUGARLESS_MMADT
    Definition
        AT, REPEAT, END, FROM, REF, PASS,
        MULT, PLUS, BLOCK, WITHIN, MERGE,
        SPLIT, EACH;
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
      Definition::Result ret = START.parse_and_get_value<Obj_p>(mmadt.c_str(), result, nullptr, PARSER_LOGGER);
      LOG(DEBUG, "parsing complete: %s\n", mmadt.c_str());
      if(ret.ret) {
        LOG_OBJ(DEBUG, this, "!gsuccessful!! parse of %s\n", mmadt.c_str());
        return result;
      } else {
        ret.error_info.output_log(PARSER_LOGGER, mmadt.c_str(), mmadt.length());
        throw fError("parse failed: %s\n", mmadt.c_str());
      }
    }

    explicit Parser(const ID &id = ID("/parser/")) : Obj(make_shared<RecMap<>>(),
                                                         OType::REC,
                                                         REC_FURI,
                                                         id_p(id)) {
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
        if(1 == vs.choice()) {
          for(int i = 0; i < vs.size(); i = i + 2) {
            map->insert(make_pair<Obj_p, Obj_p>(any_cast<Obj_p>(vs[i]), any_cast<Obj_p>(vs[i + 1])));
          }
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

      static auto code_action = [](const SemanticValues &vs) -> BCode_p {
        //  CODE <= seq(DOM_RNG,chr('|'),opt(ARGS),chr('['),PROTO,chr(')')), code_action;
        const ID_p type_id = id_p(*any_cast<fURI_p>(vs[0]));
        const InstArgs args = any_cast<InstArgs>(vs[1]);
        const Pair_p<Any, OType> proto = any_cast<Pair_p<Any, OType>>(vs[2]);
        // Quad<InstArgs, InstF, IType, Obj_p>;
        const Obj_p body = Obj::create(proto->first, proto->second, OTYPE_FURI.at(proto->second));
        const Obj_p code = Inst::to_inst(
          InstValue(args,
                    make_shared<InstF>([body](const Obj_p &a, const InstArgs &b) { return body->apply(a, b); }),
                    IType::ONE_TO_ONE, noobj()), type_id, vs.size() == 4 ? id_p(*any_cast<fURI_p>(vs[3])) : nullptr);
        LOG(INFO, "parsed code: %s\n", code->toString().c_str());
        return code;
      };

      static auto args_action = [](const SemanticValues &vs) -> InstArgs {
        const Pair_p<Any, OType> proto = any_cast<Pair_p<Any, OType>>(vs[0]);
        const Obj_p args_struct = Obj::create(proto->first, proto->second, OTYPE_FURI.at(proto->second));
        if(0 == vs.choice())
          return args_struct;
        int counter = 0;
        const auto args = Obj::to_inst_args();
        for(const auto &kv: *args_struct->lst_value()) {
          args->rec_value()->insert({vri(to_string(counter++).insert(0, "_")), kv});
        }
        return args;
      };

      static auto inst_action = [](const SemanticValues &vs) -> Inst_p {
        const ID_p op = id_p(*any_cast<fURI_p>(vs[0]));
        const InstArgs args = any_cast<InstArgs>(vs[1]);
        return Obj::to_inst(args, id_p(*ROUTER_RESOLVE(fURI(*op))),
                            vs.size() == 3 ? id_p(*any_cast<fURI_p>(vs[2])) : nullptr);
      };

      static auto furi_action = [](const SemanticValues &vs) {
        string s = vs.token_to_string();
        StringHelper::replace(&s, "`.", ".");
        return furi_p((s[0] == '<' && s[s.length() - 1] == '>') ? s.substr(1, s.length() - 2) : s);
      };

      static auto coeff_action = [](const SemanticValues &vs) -> string {
        return vs.token_to_string();
      };

      static auto dom_rng_action = [](const SemanticValues &vs) -> fURI_p {
        const auto [rf, rc] = any_cast<Pair<fURI_p, Cardinality>>(vs[1]);
        const auto [df, dc] = any_cast<Pair<fURI_p, Cardinality>>(vs[2]);
        return furi_p(any_cast<fURI_p>(vs[0])->query({
          {FOS_DOMAIN, df->toString()},
          {"ftype",
            to_string(static_cast<char>(rc))
            .append("-")
            .append(to_string(static_cast<char>(dc)))},
          {FOS_RANGE, rf->toString()}}));
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
              const IType prev_itype = list->empty() ? IType::ZERO_TO_ZERO : list->back()->itype();
              list->push_back(obj_or_inst->otype_ == OType::INST
                                ? obj_or_inst
                                : Obj::to_inst(Obj::to_inst_args({obj_or_inst}),
                                               id_p(*ROUTER_RESOLVE(is_terminal(prev_itype) ? "start" : "map"))));
            }
            return make_shared<Pair<Any, OType>>(list, OType::BCODE);
          }
          default: throw fError("unknown choice");
        }
      };

      static auto type_action = [](const SemanticValues &vs) -> Obj_p {
        return Obj::create(Any(), OType::OBJ, id_p(*ROUTER_RESOLVE(*any_cast<fURI_p>(vs[0]))));
      };

      static auto cardinality_action = [](const SemanticValues &vs) -> Cardinality {
        return static_cast<Cardinality>(vs.token_to_string()[0]);
      };

      static auto obj_action = [this](const SemanticValues &vs) -> Obj_p {
        LOG_OBJ(TRACE, Parser::singleton(), "obj_action: %i\n", vs.choice());
        switch(vs.choice()) {
          case 0: { // a[b]@xyz
            const ID_p type_id = id_p(*ROUTER_RESOLVE(*any_cast<fURI_p>(vs[0])));
            const auto [v,o] = *any_cast<Pair_p<Any, OType>>(vs[1]);
            return Obj::create(v, o, type_id,
                               vs.size() == 3 ? id_p(*std::any_cast<fURI_p>(vs[2])) : nullptr);
          }
          case 1: { // b@xyz
            const auto &[v,o] = *any_cast<Pair_p<Any, OType>>(vs[0]);
            return Obj::create(v, o, OTYPE_FURI.at(o), vs.size() == 2 ? id_p(*std::any_cast<fURI_p>(vs[1])) : nullptr);
          }
          default: throw fError("unknown obj parse branch");
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
        List<Obj_p> args;
        for(const auto &v: vs) {
          const Pair_p<Any, OType> proto = any_cast<Pair_p<Any, OType>>(v);
          args.push_back(Obj::create(proto->first, proto->second, OTYPE_FURI.at(proto->second)));
        }
        return Obj::to_inst(Obj::to_inst_args(args), id_p(*ROUTER_RESOLVE("from")));
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
      static auto end_action = [](const SemanticValues &) -> Inst_p {
        return Obj::to_inst(Obj::to_inst_args(), id_p(*ROUTER_RESOLVE("end")));
      };
      static auto merge_action = [](const SemanticValues &vs) -> Inst_p {
        return Obj::to_inst(vs.empty() ? Obj::to_inst_args() : Obj::to_inst_args({any_cast<Obj_p>(vs[0])}),
                            id_p(*ROUTER_RESOLVE("merge")));
      };
      static auto split_action = [](const SemanticValues &vs) -> Inst_p {
        return Obj::to_inst({any_cast<Obj_p>(vs[0])}, id_p(*ROUTER_RESOLVE("split")));
      };

      static auto pass_action = [](const SemanticValues &vs) -> Inst_p {
        return Obj::to_inst(Obj::to_inst_args({any_cast<Obj_p>(vs[0]), dool(false)}),
                            id_p(*ROUTER_RESOLVE("to_inv")));
      };

      static auto plus_action = [](const SemanticValues &vs) -> Inst_p {
        return Obj::to_inst({any_cast<Obj_p>(vs[0])}, id_p(*ROUTER_RESOLVE("plus")));
      };

      static auto mult_action = [](const SemanticValues &vs) -> Inst_p {
        return Obj::to_inst({any_cast<Obj_p>(vs[0])}, id_p(*ROUTER_RESOLVE("mult")));
      };

#endif
      static auto OBJ_PEG_GENERATOR = [this](const ptr<Ope> &obj_proto) -> ptr<Ope> {
        return cho(
          seq(~WS, /*zom(~COMMENT), */
              TYPE_ID, lit("["), cho(obj_proto, EMPTY), lit("]"), opt(VALUE_ID),
              ~WS/*, zom(~COMMENT)*/),
          seq(~WS, /*zom(~COMMENT), */
              obj_proto, opt(VALUE_ID), ~WS
              /*, zom(~COMMENT)*/)
        );
      };

      static auto INST_ARGS_PEG_GENERATOR = [this](const std::initializer_list<ptr<Ope>> &arg_slots,
                                                   const bool varargs = true) -> ptr<Ope> {
        List<ptr<Ope>> slots;
        auto v_slots = List<ptr<Ope>>(arg_slots);
        if(v_slots.size() > 1) {
          for(const ptr<Ope> arg_slot: arg_slots) {
            slots.push_back(seq(~WS, lit("("), ~WS, arg_slot, ~WS,
                                zom(seq(~WS, lit(","), ~WS, arg_slot, ~WS, lit(")"), ~WS))));
          }
        }
        if(v_slots.size() == 1 || varargs) {
          slots.push_back(seq(~WS, lit("("), ~WS, v_slots.at(0), ~WS, lit(")"), ~WS));
          slots.push_back(v_slots.at(0));
        }
        if(v_slots.empty() || varargs)
          slots.push_back(seq(~WS, lit("("), ~WS, lit(")"), ~WS));
        return make_shared<PrioritizedChoice>(slots);
      };

      WS <= zom(cls(" \t"));
      ////////////////////// COMMENTS ///////////////////////////
      COMMENT <= cho(SINGLE_COMMENT, MULTI_COMMENT);
      SINGLE_COMMENT <= seq(~WS, lit("_oO"), zom(seq(npd(lit("Oo_")), dot())), lit("Oo_"), ~WS);
      MULTI_COMMENT <= seq(~WS, lit("###"), zom(seq(ncls("#"), dot())), lit("###"), ~WS);
      /////////////////// BASE CONCEPTS ///////////////////////////
      TYPE <= seq(TYPE_ID, lit("[]")), type_action;
      NOOBJ <= lit("noobj"), noobj_action;
      BOOL <= cho(lit("true"), lit("false")), bool_action;
      INT <= seq(~WS, opt(chr('-')), oom(cls("0-9")), ~WS), int_action;
      REAL <= seq(~WS, opt(chr('-')), oom(cls("0-9")), chr('.'), oom(cls("0-9")), ~WS), real_action;
      STR <= seq(~WS, chr('\''), tok(zom(cho(lit("\\'"), ncls("\'")))), chr('\''), ~WS), str_action;
      FURI <= seq(~WS, WRAP("<", tok(oom(seq(npd(lit("=>")),cls("a-zA-Z0-9:/?_=&@.#+")))), ">"), ~WS), furi_action;
      FURI_INLINE <= seq(~WS, WRAP("<", tok(seq(
                                     oom(cho(cls("a-zA-Z:/?_#+"), seq(lit("`.")))),
                                     zom(seq(npd(lit("=>")), cho(seq(lit("`.")), cls("a-zA-Z0-9:/?_=&#+")))))),
                                   ">"), ~WS), furi_action;
      FURI_NO_Q <= seq(~WS, WRAP("<", tok(seq(oom(cls("a-zA-Z:/_.#+")),
                                   zom(seq(npd(lit("=>")), cls("a-zA-Z0-9:/_=&@.#+"))))),
                                 ">"), ~WS), furi_action;
      URI <= cho(lit("<>"), FURI_INLINE, FURI), uri_action;
      REC <= cho(lit("[=>]"), seq(lit("["), ~WS, opt(seq(OBJ, lit("=>"), OBJ)), ~WS,
                                  zom(seq(lit(","), OBJ, lit("=>"), ~WS, OBJ)), ~WS, lit("]"), ~WS)), rec_action;
      LST <= seq(~WS, lit("["), ~WS, opt(OBJ), ~WS, zom(seq(lit(","), ~WS, OBJ)), ~WS, lit("]"), ~WS), lst_action;
      OBJS <= seq(~WS, lit("{"), ~WS, opt(OBJ), ~WS, zom(seq(lit(","), ~WS, OBJ)), ~WS, lit("}"), ~WS), objs_action;
      ARGS <= cho(ARGS_REC, ARGS_LST), args_action;
      ARGS_REC <= cho(lit("(=>)"), seq(lit("("), opt(seq(OBJ, lit("=>"), OBJ)),
                                       zom(seq(lit(","), OBJ, lit("=>"), OBJ)), lit(")"))), rec_action;
      ARGS_LST <= cho(lit("()"), seq(lit("("), opt(seq(OBJ, zom(seq(lit(","), OBJ)), lit(")"))))), lst_action;
      INST <= seq(TYPE_ID, ARGS, opt(VALUE_ID)), inst_action;
      INST_P <= cho(INST_SUGAR, INST, NO_CODE_OBJ);
      CODE <= seq(DOM_RNG, lit("|"), opt(ARGS), lit("["), PROTO, lit("]"), opt(VALUE_ID)), code_action;
      EMPTY_BCODE <= lit("_"), empty_bcode_action;
      BCODE <= cho(EMPTY_BCODE,
                   seq(INST_P, zom(cho(END, seq(opt(lit(".")), INST_SUGAR), seq(lit("."), INST_P))))),
          bcode_action;
      // TODO: stream ring theory
      CARDINALITY <= cho(chr('.'), chr('?'), chr('o'), chr('O')), cardinality_action;
      COEF <= tok(seq(chr('{'), oom(cls("0-9")), opt(seq(chr(','), oom(cls("0-9#")))), chr('}'))), coeff_action;
      SET <= seq(FURI_NO_Q, opt(seq(lit("{"), CARDINALITY, lit("}")))),
          [](const SemanticValues &vs) -> Pair<fURI_p, Cardinality> {
            const auto furi = any_cast<fURI_p>(vs[0]);
            const auto cardinality = 1 == vs.size() ? Cardinality::ONE : any_cast<Cardinality>(vs[1]);
            return {furi, cardinality};
          };
      DOM_RNG <= WRAP("<", seq(~WS,FURI_NO_Q, chr('?'), SET, /*opt(COEF),*/lit("<="), SET, ~WS), ">"), dom_rng_action;
      TYPE_ID <= seq(cho(DOM_RNG, FURI_INLINE)), furi_action;
      VALUE_ID <= seq(chr('@'), FURI);
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      NOOBJ.name = "noobj";
      BOOL.name = "bool";
      INT.name = "int";
      STR.name = "str";
      URI.name = "uri";
      LST.name = "lst";
      REC.name = "rec";
      TYPE_ID.name = "type_id";
      VALUE_ID.name = "value_id";
      DOM_RNG.name = "range<=domain";

      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      INST_ARG_PROTO <= cho(NOOBJ, BOOL, REAL, INT, STR, LST, REC, OBJS, BCODE, URI);
      NO_CODE_PROTO <= cho(NOOBJ, BOOL, REAL, INT, STR, LST, REC, OBJS, URI);
      PROTO <= cho(REAL, BCODE, NOOBJ, BOOL, REAL, INT, STR, LST, REC, OBJS, URI); //NO_CODE_PROTO);
      EMPTY <= lit(""), [](const SemanticValues &vs) {
        return make_shared<Pair<Any, OType>>(Any(), OType::OBJ);
      };
      ////////////////////////////////////////////////////////////
      INST_ARG_OBJ <= OBJ_PEG_GENERATOR(INST_ARG_PROTO), obj_action;
      NO_CODE_OBJ <= OBJ_PEG_GENERATOR(NO_CODE_PROTO), obj_action;
      OBJ <= OBJ_PEG_GENERATOR(PROTO), obj_action;
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      START <= seq(~WS, opt(OBJ), zom(cho(seq(lit("."), OBJ), seq(opt(lit(".")), INST_SUGAR))), ~WS),
          [](SemanticValues &vs) {
            if(vs.size() == 1 && !any_cast<Obj_p>(vs[0])->is_code())
              return any_cast<Obj_p>(vs[0]);
            const auto insts = make_shared<List<Inst_p>>();
            bool first = true;
            for(const auto &i: vs) {
              const Obj_p o = any_cast<Obj_p>(i);
              if(!o->is_code()) {
                insts->push_back(Obj::to_inst({first ? Obj::to_objs({o}) : o}, id_p(first ? "start" : "map")));
              } else if(o->is_inst()) {
                insts->push_back(o);
              } else {
                for(const Inst_p &inst: *o->bcode_value()) {
                  insts->push_back(inst);
                }
              }
              first = false;
            }
            return Obj::to_bcode(insts);
          };
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

      /////////////////////////////////////////////////////////////////
      ///////////////////////  INST SUGARS ////////////////////////////
      /////////////////////////////////////////////////////////////////
#ifndef FOS_SUGARLESS_MMADT
      INST_SUGAR <= cho(AT, PLUS, MULT, WITHIN, FROM, PASS, REF, BLOCK, EACH, END, MERGE, SPLIT/*, REPEAT*/);
      AT <= seq(chr('@'), WRAP("(", cho(URI, BCODE), ")")), at_action;
      // REPEAT <= seq(chr('('), INST_ARG_OBJ, lit(")^*")), repeat_action; // )^*(until,emit)
      FROM <= seq(lit("*"), cho(WRAP("(", cho(URI, BCODE), ")"),
                                seq(lit("("), cho(URI, BCODE), opt(seq(lit(","), INST_ARG_PROTO)),
                                    lit(")")))), from_action;
      REF <= seq(lit("->"), WRAP("(", OBJ, ")")), ref_action;
      BLOCK <= seq(lit("|"), WRAP("(", OBJ, ")")), block_action;
      PASS <= seq(lit("-->"), WRAP("(", INST_ARG_OBJ, ")")), pass_action;
      MERGE <= seq(chr('>'), opt(INST_ARG_OBJ), chr('-')), merge_action;
      SPLIT <= seq(lit("-<"), WRAP("(", INST_ARG_OBJ, ")")), split_action;
      EACH <= seq(lit("=="), WRAP("(", INST_ARG_OBJ, ")")), each_action;
      END <= lit(";"), end_action;
      WITHIN <= seq(lit("_/"), WRAP("(", INST_ARG_OBJ, ")"), lit("\\_")), within_action;
      PLUS <= seq(~WS, chr('+'), chr(' '), ~WS, WRAP("(", INST_ARG_OBJ, ")")), plus_action;
      MULT <= seq(~WS, chr('x'), chr(' '), ~WS, WRAP("(", INST_ARG_OBJ, ")")), mult_action;
      ///////////////////////// DEBUG UTILITIES //////////////////////////////////////////
      REPEAT.enter = enter_y("repeat");
#endif
      ///////////////////////////////// PREDICATES ///////////////////////////////////////
      BCODE.predicate = [](const SemanticValues &vs, const std::any &dt, std::string &msg) -> bool {
        return vs.size() != 1 || any_cast<Obj_p>(vs[0])->is_code();
      };
      FURI.predicate = FURI_NO_Q.predicate = FURI_INLINE.predicate =
                                             [](const SemanticValues &vs, const std::any &, std::string &) {
                                               try {
                                                 if(vs.token_to_string().empty() || vs.token_to_string() == "_")
                                                   return false;
                                                 fURI(vs.token_to_string());
                                               } catch(const std::exception &e) {
                                                 //msg = e.what();
                                                 return false;
                                               }
                                               return true;
                                             };
      ///////////////////////// DEBUG UTILITIES //////////////////////////////////////////
      OBJ.enter = enter_y("obj");
      TYPE.enter = enter_y("type");
      PROTO.enter = enter_y("proto");
      FROM.enter = enter_y("from");
      INT.enter = enter_y("int");
      TYPE_ID.enter = enter_y("type_id");
      VALUE_ID.enter = enter_y("at_id");
      FURI_INLINE.enter = enter_y("furi_inline");
      //////////////////////// WHITESPACE IGNORING ///////////////////////////////////////
      ROOT.whitespaceOpe =
          LST.whitespaceOpe =
          REC.whitespaceOpe =
          OBJS.whitespaceOpe =
          COMMENT.whitespaceOpe =
          OBJ.whitespaceOpe =
          NO_CODE_OBJ.whitespaceOpe =
          INST_ARG_OBJ.whitespaceOpe = make_shared<Whitespace>(Whitespace(zom(cls(" \t"))));
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
