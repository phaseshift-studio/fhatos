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
#define WRAQ(LEFT,DEF_NOWRAP,DEF_WRAP,RIGHT) cho(seq(ign(lit((LEFT))), seq((DEF_WRAP), zom(seq(lit(","),(DEF_WRAP)))), ign(lit((RIGHT)))),(DEF_NOWRAP))

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
        WS, START, ARGS, ARGS_LST, ARGS_REC, COMMENT, SINGLE_COMMENT, MULTI_COMMENT,
        FURI, FURI_INLINE, FURI_NO_Q, DOM_RNG, START_OBJ, NO_MATCH,
        NOOBJ, BOOL, INT, REAL, STR, LST, REC, URI, INST, SET, OBJS, OBJ, TYPE_ID,
        COEFFICIENT, VALUE_ID, PROTO, EMPTY, NORMAL_INST, SUGAR_INST;

#ifndef FOS_SUGARLESS_MMADT
    Definition
        EMPTY_BCODE, AT, REPEAT, END, FROM, REF, PASS,
        MULT, PLUS, BLOCK, WITHIN, MERGE, DROP,
        SPLIT, EACH;
#endif
    QuadConsumer<const size_t, const size_t, const string, const string> PARSER_LOGGER =
        [](const size_t line, const size_t column, const string &message, const string &rule) {
      throw fError("!^r%s^!y^--!r%s!! at line !y%s!!:!y%s!! !g[!r%s!g]!!",
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
      static auto noobj_action = [](const SemanticValues &) -> Pair<Any, OType> {
        return {nullptr, OType::NOOBJ};
      };
      static auto bool_action = [](const SemanticValues &vs) -> Pair<Any, OType> {
        return {vs.choice() == 0, OType::BOOL};
      };
      static auto int_action = [](const SemanticValues &vs) -> Pair<Any, OType> {
        return {vs.token_to_number<FOS_INT_TYPE>(), OType::INT};
      };
      static auto real_action = [](const SemanticValues &vs) -> Pair<Any, OType> {
        return {vs.token_to_number<FOS_REAL_TYPE>(), OType::REAL};
      };
      static auto str_action = [](const SemanticValues &vs) -> Pair<Any, OType> {
        return {vs.token_to_string(), OType::STR};
      };
      static auto uri_action = [](const SemanticValues &vs) -> Pair<Any, OType> {
        return {vs.choice() == 0 ? fURI("") : *any_cast<fURI_p>(vs[0]), OType::URI};
      };
      static auto lst_action = [](const SemanticValues &vs) -> Pair<Any, OType> {
        return {make_shared<List<Obj_p>>(vs.transform<Obj_p>()), OType::LST};
      };

      static auto rec_action = [](const SemanticValues &vs) -> Pair<Any, OType> {
        const auto map = make_shared<Obj::RecMap<>>();
        if(1 == vs.choice()) {
          for(int i = 0; i < vs.size(); i = i + 2) {
            map->insert(make_pair<Obj_p, Obj_p>(any_cast<Obj_p>(vs[i]), any_cast<Obj_p>(vs[i + 1])));
          }
        }
        return {map, OType::REC};
      };

      static auto objs_action = [](const SemanticValues &vs) -> Pair<Any, OType> {
        return {make_shared<List<Obj_p>>(vs.transform<Obj_p>()), OType::OBJS};
      };

      static auto args_action = [](const SemanticValues &vs) -> InstArgs {
        const auto [v, o] = any_cast<Pair<Any, OType>>(vs[0]);
        const Obj_p args_struct = Obj::create(v, o, OTYPE_FURI.at(o));
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
        while(s.find("`.") != string::npos) {
          StringHelper::replace(&s, "`.", ".");
        }
        return furi_p((s[0] == '<' && s[s.length() - 1] == '>') ? s.substr(1, s.length() - 2) : s);
      };

      static auto dom_rng_action = [](const SemanticValues &vs) -> fURI_p {
        const auto [rf, rc] = any_cast<Pair<fURI_p, Cardinality>>(vs[1]);
        const auto [df, dc] = any_cast<Pair<fURI_p, Cardinality>>(vs[2]);
        const string card_str = Cardinalities.to_chars(rc).append(",").append(Cardinalities.to_chars(dc));
        const fURI_p dom_rng = furi_p(any_cast<fURI_p>(vs[0])->query({
          {FOS_DOMAIN, ROUTER_RESOLVE(*df)->toString()},
          {FOS_F,
            Cardinalities.to_chars(dc)
            .append(",")
            .append(Cardinalities.to_chars(rc))},
          {FOS_RANGE, ROUTER_RESOLVE(*rf)->toString()}}));
        return dom_rng;
      };

      static auto empty_bcode_action = [](const SemanticValues &) -> BCode_p {
        return Obj::to_bcode();
      };

      static auto coefficient_action = [](const SemanticValues &vs) -> Cardinality {
        int left;
        int right;
        // -1 means "or more"
        string card_chr = vs.token_to_string();
        StringHelper::trim(card_chr);
        if(card_chr.empty()) {
          left = 1;
          right = 1;
        } else if(card_chr == "*") {
          left = -1;
          right = -1;
        } else if(card_chr == "?") {
          left = 0;
          right = 1;
        } else if(card_chr == "+") {
          left = 1;
          right = -1;
        } else {
          const List<int> components = StringHelper::tokenize<int>(',', card_chr, [](const string &s) {
            return StringHelper::is_integer(s) ? stoi(s) : -1;
          });
          if(0 == components.size()) { // I believe this first part is an unreachable branch
            left = 1;
            right = 1;
          } else if(1 == components.size()) {
            left = components[0];
            right = left;
          } else {
            left = components[0];
            right = components[1];
          }
        }
        if(0 == left && 0 == right)
          return Cardinality::ZERO;
        if(1 == left && 1 == right)
          return Cardinality::ONE;
        if(0 == left && right != 0)
          return Cardinality::MAYBE;
        if(left > 1 || right > 1 || left == -1 || right == -1)
          return Cardinality::MANY;
        throw fError("unknown cardinality %i %s", left, card_chr.c_str());
      };


      static auto obj_action = [this](const SemanticValues &vs) -> Obj_p {
        LOG_OBJ(TRACE, Parser::singleton(), "obj_action: %i\n", vs.choice());
        switch(vs.choice()) {
          case 0: { // [a][b]@xyz
            const ID_p type_id = id_p(*ROUTER_RESOLVE(*any_cast<fURI_p>(vs[0])));
            const auto [v,o] = any_cast<Pair<Any, OType>>(vs[1]);
            return Obj::to_type(type_id,
                                Obj::create(v, o, type_id),
                                vs.size() == 3 ? id_p(*std::any_cast<fURI_p>(vs[2])) : nullptr);
          }
          case 1: { // a(c)[b]@xyz
            const ID_p type_id = id_p(*ROUTER_RESOLVE(*any_cast<fURI_p>(vs[0])));
            const auto args = any_cast<InstArgs>(vs[1]);
            const auto [v,o] = any_cast<Pair<Any, OType>>(vs[2]);
            const Obj_p body = Obj::create(v, o, type_id);
            const ID_p value_id = vs.size() == 4 ? id_p(*std::any_cast<fURI_p>(vs[3])) : nullptr;

            IType itype;
            if(type_id->has_query(FOS_F)) {
              const List<string> f = type_id->query_values(FOS_F);
              const string itype_str = string(f.at(0)).append("->").append(f.at(1));
              itype = ITypeSignatures.to_enum(itype_str);
            } else {
              itype = IType::ONE_TO_ONE;
            }
            const Obj_p seed = is_gather(itype) ? Objs::to_objs() : Obj::to_noobj();
            return Obj::to_inst(InstValue(args, make_shared<InstF>(body), itype, seed),
                                type_id, value_id);
            // TODO: deduce itype from type_id
          }
          case 2: { // a(b)@xyz
            return any_cast<Inst_p>(vs[0]);
          }
          case 3: { // a[b]@xyz
            const ID_p type_id = id_p(*ROUTER_RESOLVE(*any_cast<fURI_p>(vs[0])));
            const auto [v,o] = any_cast<Pair<Any, OType>>(vs[1]);
            return Obj::create(v, o, type_id,
                               vs.size() == 3 ? id_p(*std::any_cast<fURI_p>(vs[2])) : nullptr);
          }
          case 4: { // b@xyz
            const auto &[v,o] = any_cast<Pair<Any, OType>>(vs[0]);
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

      static const auto SUGAR_GENERATOR = [this](Definition &definition, const string &sugar, const string &opcode) {
        definition <= seq(lit(sugar.c_str()), WRAQ("(", OBJ, START, ")")),
            [opcode](const SemanticValues &vs) -> Inst_p const {
              return Obj::to_inst(vs.transform<Obj_p>(), id_p(*ROUTER_RESOLVE(opcode)));
            };
      };
      static auto within_action = [](const SemanticValues &vs) -> Inst_p {
        return Obj::to_inst(vs.transform<Obj_p>(), id_p(*ROUTER_RESOLVE("within")));
      };
      static auto end_action = [](const SemanticValues &) -> Inst_p {
        return Obj::to_inst(Obj::to_inst_args(), id_p(*ROUTER_RESOLVE("end")));
      };
      static auto merge_action = [](const SemanticValues &vs) -> Inst_p {
        return Obj::to_inst(vs.empty() ? Obj::to_inst_args() : Obj::to_inst_args({any_cast<Obj_p>(vs[0])}),
                            id_p(*ROUTER_RESOLVE("merge")));
      };
      static auto pass_action = [](const SemanticValues &vs) -> Inst_p {
        return Obj::to_inst(Obj::to_inst_args({any_cast<Obj_p>(vs[0]), dool(false)}),
                            id_p(*ROUTER_RESOLVE("to_inv")));
      };


#endif

      static auto start_action = [](const SemanticValues &vs) {
        if(vs.size() == 1 && !any_cast<Obj_p>(vs[0])->is_code()) // is_bcode?
          return any_cast<Obj_p>(vs[0]);
        const auto insts = make_shared<List<Inst_p>>();
        Inst_p prev = nullptr;
        for(const auto &obj: vs.transform<Obj_p>()) {
          if(!obj->is_code()) {
            const bool as_start = nullptr == prev || /*is_terminal(prev->itype()) ||*/ prev->inst_op() == "end";
            prev = Obj::to_inst({as_start ? Obj::to_objs({obj}) : obj}, id_p(as_start ? "start" : "map"));
            insts->push_back(prev);
          } else if(obj->is_inst()) {
            insts->push_back(obj);
            prev = obj;
          } else {
            for(const Inst_p &inst: *obj->bcode_value()) {
              insts->push_back(inst);
              prev = inst;
            }
          }
        }
        return Obj::to_bcode(insts);
      };

      static auto start_obj_action = [](const SemanticValues &vs) -> Pair<Any, OType> {
        const auto obj = start_action(vs);
        return {obj->value_, obj->o_type()};
      };

      WS <= zom(cls(" \t\n"));
      ////////////////////// COMMENTS ///////////////////////////
      COMMENT <= cho(SINGLE_COMMENT, MULTI_COMMENT);
      SINGLE_COMMENT <= seq(~WS, lit("_oO"), zom(seq(npd(lit("Oo_")), dot())), lit("Oo_"), ~WS);
      MULTI_COMMENT <= seq(~WS, lit("###"), zom(seq(ncls("#"), dot())), lit("###"), ~WS);
      ////////////////////// FURI VARIANTS ///////////////////////////
      FURI <= WRAP("<", tok(oom(seq(npd(lit("=>")),cls("a-zA-Z0-9:/%?_=&@.#+")))), ">"), furi_action;
      FURI_INLINE <= WRAP("<", tok(seq(
                            oom(cho(cls("a-zA-Z:/%?_#+"), seq(lit("`.")))),
                            zom(seq(npd(lit("=>")), cho(seq(lit("`.")), cls("a-zA-Z0-9:/%?_=&#+")))))),
                          ">"), furi_action;
      FURI_NO_Q <= WRAP("<", tok(seq(oom(cls("a-zA-Z:/%_.#+")),
                          zom(seq(npd(lit("=>")), cls("a-zA-Z0-9:/%_=&@.#+"))))),
                        ">"), furi_action;
      DOM_RNG <= WRAP("<", seq(FURI_NO_Q, chr('?'), SET, /*opt(COEF),*/ lit("<="), SET), ">"), dom_rng_action;
      TYPE_ID <= seq(cho(DOM_RNG, FURI_INLINE));
      VALUE_ID <= seq(chr('@'), FURI_INLINE);
      /////////////////// BASE TYPES ///////////////////////////
      NOOBJ <= lit("noobj"), noobj_action;
      BOOL <= cho(lit("true"), lit("false")), bool_action;
      INT <= tok(seq(opt(chr('-')), oom(cls("0-9")))), int_action;
      REAL <= tok(seq(opt(chr('-')), oom(cls("0-9")), chr('.'), oom(cls("0-9")))), real_action;
      STR <= seq(~WS, chr('\''), tok(zom(cho(lit("\\'"), ncls("\'")))), chr('\''), ~WS), str_action;
      URI <= cho(lit("<>"), FURI_INLINE, FURI), uri_action;
      LST <= cho(lit("[]"), seq(lit("["), START,
                                zom(seq(lit(","), START)), lit("]"))), lst_action;
      LST.error_message = "typed polys are type wrapped polys: lst[[]] not lst[]";
      REC <= cho(lit("[=>]"), seq(lit("["), START, lit("=>"), START,
                                  zom(seq(lit(","), START, lit("=>"), START)), lit("]"))), rec_action;
      OBJS <= seq(lit("{"), START, zom(seq(lit(","), START)), lit("}")), objs_action;

      /////////////////// INST COMPONENTS ///////////////////////////
      INST <= cho(SUGAR_INST, NORMAL_INST);
      NORMAL_INST <= seq(TYPE_ID, ARGS, opt(VALUE_ID)), inst_action;
      ARGS <= cho(ARGS_REC, ARGS_LST), args_action;
      ARGS_REC <= cho(lit("(=>)"), seq(lit("("), START, lit("=>"), START,
                                       zom(seq(lit(","), START, lit("=>"), START)), lit(")"))), rec_action;
      ARGS_LST <= cho(lit("()"), seq(lit("("), START, zom(seq(lit(","), START)), lit(")"))), lst_action;
      //////////////////////////////////////////////////////////////////////////////////////// TODO: stream ring theory
      COEFFICIENT <= tok(cho(cls("?*+"), seq(opt(INT), opt(lit(",")), opt(INT)))), coefficient_action;
      //zoo,zom,oom
      SET <= seq(FURI_NO_Q, opt(seq(lit("{"), COEFFICIENT, lit("}")))),
          [](const SemanticValues &vs) -> Pair<fURI_p, Cardinality> {
            const auto furi = any_cast<fURI_p>(vs[0]);
            const auto cardinality = 1 == vs.size() ? Cardinality::ONE : any_cast<Cardinality>(vs[1]);
            return {furi, cardinality};
          };
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      START.name = "start";
      NOOBJ.name = "noobj";
      BOOL.name = "bool";
      INT.name = "int";
      REAL.name = "real";
      STR.name = "str";
      URI.name = "uri";
      LST.name = "lst";
      REC.name = "rec";
      OBJS.name = "objs";
      INST.name = "inst";
      TYPE_ID.name = "type_id";
      VALUE_ID.name = "value_id";
      DOM_RNG.name = "range<=domain";
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      PROTO <= cho(NOOBJ, BOOL, REAL, INT, STR, LST, REC, OBJS, URI);
      EMPTY <= seq(lit(""), npd(chr(']'))), [](const SemanticValues &) -> Pair<Any, OType> {
        return {Obj::to_bcode(), OType::TYPE};
      };
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      OBJ <= cho(
        seq(lit("["), TYPE_ID, lit("]"), lit("["), cho(START_OBJ, PROTO, EMPTY), lit("]"), opt(VALUE_ID)), // [a][b]@xyz
        seq(TYPE_ID, ARGS, lit("["), cho(START_OBJ, PROTO, EMPTY), lit("]"), opt(VALUE_ID)), // a(c)[b]@xyz)
        seq(INST), // a(b)@xyz
        seq(TYPE_ID, lit("["), cho(START_OBJ, PROTO, EMPTY), lit("]"), opt(VALUE_ID)), // a[b]@xyz
        seq(PROTO, opt(VALUE_ID)) // b@xyz
      ), obj_action;
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////// START //////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      START <= seq(opt(OBJ), zom(cho(seq(END, START), seq(lit("."), OBJ), seq(SUGAR_INST)))), start_action;
      // (END,OBJ) => ; next...
      START_OBJ <= START, start_obj_action;
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

      /////////////////////////////////////////////////////////////////
      ///////////////////////  INST SUGARS ////////////////////////////
      /////////////////////////////////////////////////////////////////
#ifndef FOS_SUGARLESS_MMADT
      SUGAR_INST <= cho(AT, PLUS, MULT, WITHIN, EMPTY_BCODE, FROM, PASS, REF,
                        BLOCK, EACH, END, MERGE, SPLIT/*, REPEAT*/);
      EMPTY_BCODE <= lit("_"), empty_bcode_action; //seq(lit("_"), ncls("0-9")), empty_bcode_action;
      SUGAR_GENERATOR(AT, "@", "at");
      SUGAR_GENERATOR(DROP, "v", "drop");
      SUGAR_GENERATOR(FROM, "*", "from");
      SUGAR_GENERATOR(REF, "->", "to_inv");
      SUGAR_GENERATOR(BLOCK, "|", "block");
      SUGAR_GENERATOR(SPLIT, "-<", "split");
      SUGAR_GENERATOR(EACH, "==", "each");
      SUGAR_GENERATOR(PLUS, "+", "plus");
      SUGAR_GENERATOR(MULT, "x", "mult");
      PASS <= seq(lit("-->"), WRAQ("(", OBJ, START, ")")), pass_action;
      MERGE <= seq(chr('>'), opt(OBJ), chr('-')), merge_action;
      END <= lit(";"), end_action;
      WITHIN <= seq(lit("_/"), START, lit("\\_")), within_action;
      ///////////////////////// DEBUG UTILITIES //////////////////////////////////////////
      REPEAT.enter = enter_y("repeat");
#endif
      ///////////////////////////////// PREDICATES ///////////////////////////////////////
      FURI.predicate =
          FURI_NO_Q.predicate =
          FURI_INLINE.predicate =
          [](const SemanticValues &vs, const std::any &, std::string &) {
            try {
              if(vs.token_to_string().empty() || vs.token_to_string() == "_")
                return false;
              fURI(vs.token_to_string()); // bad: using exception handling for branching
            } catch(const std::exception &e) {
              //msg = e.what();
              return false;
            }
            return true;
          };
      ///////////////////////// DEBUG UTILITIES //////////////////////////////////////////
      OBJ.enter = enter_y("obj");
      FROM.enter = enter_y("from");
      INT.enter = enter_y("int");
      INST.enter = enter_y("inst");
      TYPE_ID.enter = enter_y("type_id");
      VALUE_ID.enter = enter_y("at_id");
      DOM_RNG.enter = enter_y("range<=domain");
      FURI_INLINE.enter = enter_y("furi_inline");
      //////////////////////// WHITESPACE IGNORING ///////////////////////////////////////
      START.whitespaceOpe = WS.get_core_operator();
      START.enablePackratParsing = true;
      START.eoi_check = true;
      /////////////////////////////////////////////////////////////////////////////////////
      OBJ_PARSER = [](const string &obj_string) {
        return Parser::singleton()->parse(obj_string);
      };
      Options::singleton()->parser<const Obj>(OBJ_PARSER);
    }
  };
}

#endif
