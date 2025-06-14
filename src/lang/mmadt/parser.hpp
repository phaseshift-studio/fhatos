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
#include "../../model/fos/sys/router/memory/memory.hpp"
#include "../obj.hpp"
#include "../processor/processor.hpp"
#include "../util/peglib.h"

#define WRAP(LEFT, DEFINITION, RIGHT) cho(seq(ign(lit((LEFT))), (DEFINITION), ign(lit((RIGHT)))), (DEFINITION))
#define WRAQ(LEFT, DEF_NOWRAP, DEF_WRAP, RIGHT)                                                                        \
  cho(seq(ign(lit((LEFT))), seq((DEF_WRAP), zom(seq(lit(","), (DEF_WRAP)))), ign(lit((RIGHT)))), (DEF_NOWRAP))
#define COMMENT_TOKEN "dUmMy_CoMmEnT"

namespace mmadt {
  using namespace peg;
  using namespace std;

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

    Tracker *track(const string &line) {
      for(const auto &c: line) {
        track(c);
      }
      return this;
    }

    static bool is_wrapper(const char c) {
      return c == '<' || c == '>' || c == '(' || c == ')' || c == '{' || c == '}';
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
      if((quotes && c != '\'') || (multi_comment != 0 && c != '=')) {
        // do nothing
      } else if(c == '\'')
        quotes = !quotes;
      else if(c == '(')
        parens++;
      else if(c == ')')
        parens--;
      else if(c == '[')
        brackets++;
      else if(c == ']')
        brackets--;
      else if(c == '=' && last[0] == '<' && angles > 0) // <=
        angles--;
      else if(c == '=' && last[0] == '>' && angles < 0) // >=
        angles++;
      else if(c == '<') {
        if(last[0] != '-' && last[0] != '=' && last[0] != '<' && last[0] != '?') // -< =< << ?<
          angles++;
        if(last[0] == '<')
          angles--;
      } else if(c == '>') {
        if(last[0] != '-' && last[0] != '=' && last[0] != '>' &&
           last[0] != '?' /*&& (last[0] != '+' && last[1] != '-')*/) // TODO APPEND and + wildcard can clash!
          // -> => >> ?> -+>
          angles--;
        if(last[0] == '>')
          angles++;
      } else if(c == '-' && last[0] != '-') {
        if(last[0] == '<') // <-
          angles--;
        else if(last[0] == '>' && last[1] != '=') // >-
          angles++;
      } else if(c == '{')
        braces++;
      else if(c == '}')
        braces--;
      else if(c == '/' && last[0] == '_') // _/
        within++;
      else if(c == '_' && last[0] == '\\') // \_
        within--;
      else if(c == '=' && last[0] == '=' && last[1] == '=' && last[2] != '=') {
        if(multi_comment == -1)
          multi_comment++;
        else
          multi_comment = -1;
      }
      //////////////////////////
      if(is_wrapper(c) && this->closed())
        this->clear();
      else {
        last[2] = last[1];
        last[1] = last[0];
        last[0] = c;
      }
      return c;
    }
  };

  static QuadConsumer<const size_t, const size_t, const string, const string> PARSER_LOGGER =
      [](const size_t line, const size_t column, const string &message, const string &rule) {
        throw fError("!^r%s^!y^--!r%s!! at line !y%s!!:!y%s!! !g[!r%s!g]!!", column - 1, message.c_str(), line, column,
                     rule.c_str());
      };

  class Parser final : public Rec {
  public:
    static ptr<Parser> singleton(const ID &id = "/io/parser",
                                 const Rec_p &config = rec({{"stack_size", Obj::to_noobj()}})) {
      static auto parser_p = make_shared<Parser>(id, config);
      return parser_p;
    }

    static void register_module() {
      REGISTERED_MODULES->insert_or_assign(
          "/mmadt/util/parser",
          InstBuilder::build(Typer::singleton()->vid->add_component("/mmadt/util/parser"))
              ->inst_f([](const Obj_p &, const InstArgs &) {
                return Obj::to_rec(
                    {{"/mmadt/util/parser",
                      Obj::to_rec({{"config", __().else_(Obj::to_rec({{"stack_size", jnt(32000)}}))}})}});
              })
              ->create());
    }

  private:
    Definition WS, START, ARGS, ARGS_LST, ARGS_REC, COMMENT, SINGLE_COMMENT, MULTI_COMMENT, FURI_VAR, FURI, FURI_INLINE,
        FURI_NO_Q, DOM_RNG, START_OBJ, NO_MATCH, NOOBJ, BOOL, INT, REAL, STR, LST, REC, URI, INST, SIGNATURE, OBJS, OBJ,
        TYPE_ID, COEFFICIENT, VALUE_ID, PROTO, EMPTY, NORMAL_INST, SUGAR_INST;

#ifndef FOS_SUGARLESS_MMADT
    Definition APPEND, IS_A, EMPTY_BCODE, AT, RSHIFT, LSHIFT, RSHIFT_0, LSHIFT_0, REPEAT, END, FROM, REF, PASS, MULT,
        PLUS, BLOCK, WITHIN, BARRIER, MERGE, DROP, EQ, GT, GTE, LT, LTE, NEQ, SPLIT, EACH, CHOOSE, CHAIN, LIFT, REF_OP;
#endif

  public:
    static Obj_p load_boot_config() {
      if(boot_config_obj_copy) {
        const auto proto = make_unique<Parser>();
        return proto->parse((const char *) boot_config_obj_copy);
      }
      return Obj::to_noobj();
    }

    Obj_p parse(const char *source) const {
      if(0 == strlen(source))
        return Obj::to_noobj();
      Obj_p result;

      if(Definition::Result ret = START.parse_and_get_value<Obj_p>(source, result, nullptr, PARSER_LOGGER); ret.ret) {
        LOG_WRITE(DEBUG, this,
                  L("!gsuccessful!! parse of {} !g==>!!\n\t{}\n", str(source)->toString(), result->toString()));
        if(result->is_empty_bcode() && strcmp(source, "_") != 0)
          return Obj::to_noobj(); // if the source is empty, contains only comments, or is _ (empty bcode)
        // if(ret.error_info.error_pos)
        // ret.error_info.output_log(PARSER_LOGGER, source, strlen(source));
        return result->inst_bcode_obj();
      } else {
        ret.error_info.output_log(PARSER_LOGGER, source, strlen(source));
        throw fError("parse failed: %s\n", source);
      }
    }

    explicit Parser() : Obj(make_shared<RecMap<>>(), OType::REC, REC_FURI) { initialize(); }

    explicit Parser(const ID &id, const Rec_p &config) :
        Obj(rmap({{"config", config}}), OType::REC, REC_FURI, id_p(id)) {
      initialize();
    }

  protected:
    void initialize() {
      OBJ_PARSER = [](const string &obj_string) {
        // StringHelper::replace(const_cast<string *>(&obj_string), "\\\'", "\'");
        const Int_p stack_size = Parser::singleton()->obj_get("config/stack_size")->or_else(jnt(0));
        const int int_stack_size =
            stack_size->is_code()  ? BOOTING ? 32384 : mmADT::delift(stack_size)->apply(str(obj_string))->int_value()
             : stack_size->is_int() ? stack_size->int_value()
                                   : 0;

        return Memory::singleton()->use_custom_stack(InstBuilder::build("custom_parse_stack")
                                                         ->inst_f([](const Str_p &source, const InstArgs &) {
                                                           return Parser::singleton()->parse(
                                                               source->str_value().c_str());
                                                         })
                                                         ->create(),
                                                     Obj::to_str(obj_string), int_stack_size);
      };
      auto noobj_action = [](const SemanticValues &) -> Pair<Any, OType> { return {nullptr, OType::NOOBJ}; };
      auto bool_action = [](const SemanticValues &vs) -> Pair<Any, OType> { return {vs.choice() == 0, OType::BOOL}; };
      auto int_action = [](const SemanticValues &vs) -> Pair<Any, OType> {
        return {vs.token_to_number<FOS_INT_TYPE>(), OType::INT};
      };
      auto real_action = [](const SemanticValues &vs) -> Pair<Any, OType> {
        return {vs.token_to_number<FOS_REAL_TYPE>(), OType::REAL};
      };
      auto str_action = [](const SemanticValues &vs) -> Pair<Any, OType> { return {vs.token_to_string(), OType::STR}; };
      auto uri_action = [](const SemanticValues &vs) -> Pair<Any, OType> {
        return {vs.choice() == 0 ? fURI("") : *any_cast<fURI_p>(vs[0]), OType::URI};
      };
      auto lst_action = [](const SemanticValues &vs) -> Pair<Any, OType> {
        return {make_shared<List<Obj_p>>(vs.transform<Obj_p>()), OType::LST};
      };
      auto rec_action = [](const SemanticValues &vs) -> Pair<Any, OType> {
        const auto map = make_shared<Obj::RecMap<>>();
        if(1 == vs.choice()) {
          for(int i = 0; i < vs.size(); i = i + 2) {
            map->insert_or_assign(any_cast<Obj_p>(vs[i]), any_cast<Obj_p>(vs[i + 1]));
          }
        }
        return {map, OType::REC};
      };
      auto objs_action = [](const SemanticValues &vs) -> Pair<Any, OType> {
        return {make_shared<List<Obj_p>>(vs.transform<Obj_p>()), OType::OBJS};
      };
      auto args_action = [](const SemanticValues &vs) -> InstArgs {
        const auto [v, o] = any_cast<Pair<Any, OType>>(vs[0]);
        const Obj_p args_struct = Obj::create(v, o, OTYPE_FURI.at(o));
        if(0 == vs.choice())
          return args_struct;
        int counter = 0;
        const auto args = Obj::to_inst_args();
        for(const auto &kv: *args_struct->lst_value()) {
          args->rec_value()->insert({vri(to_string(counter++)), kv});
        }
        return args;
      };
      auto inst_action = [](const SemanticValues &vs) -> Inst_p {
        const ID_p op = id_p(*any_cast<fURI_p>(vs[0]));
        const InstArgs args = any_cast<InstArgs>(vs[1]);
        return Obj::to_inst(args, op, vs.size() == 3 ? id_p(*any_cast<fURI_p>(vs[2])) : nullptr);
      };
      auto furi_action = [](const SemanticValues &vs) -> fURI_p {
        string s = vs.token_to_string();
        // process uri code blocks {{...}}
        /*s = StringHelper::replace_groups(&s, "{{", "}}", [](const string &code) {
          // LOG(ERROR, "code to process: %s\n", code.c_str());
          const Obj_p result = BCODE_PROCESSOR(OBJ_PARSER(code))->none_one_all();
          // LOG(ERROR, "result of process: %s\n", result->toString().c_str());
          // LOG(ERROR, "type of process: %s\n", result->tid->toString().c_str());
          if(result->is_uri())
            return result->uri_value().toString();
          if(result->is_str())
            return result->str_value();
          if(result->is_noobj())
            return string();
          throw fError("!yfuri code block !rmust !yreturn !buri!!, !bstr!!, or !bnoobj!!: %s",
                       OTypes.to_chars(result->otype).c_str());
        });*/
        // replace escape characters
        while(s.find("`.") != string::npos) {
          StringHelper::replace(&s, "`.", ".");
        }
        return furi_p((s[0] == '<' && s[s.length() - 1] == '>') ? s.substr(1, s.length() - 2) : s);
      };
      auto dom_rng_action = [](const SemanticValues &vs) -> fURI_p {
        if(vs.choice() == 0)
          return furi_p("");
        if(vs.choice() == 1) {
          const fURI_p name = any_cast<fURI_p>(vs[0]);
          const IntCoefficient coeff = any_cast<IntCoefficient>(vs[1]);
          fURI_p dom_rng = furi_p(name->dom_rng(*OBJ_FURI, coeff, name->no_query(), coeff));
          return dom_rng;
        }
        const bool anonymous = vs.size() == 2;
        const fURI_p name = anonymous ? furi_p("") : any_cast<fURI_p>(vs[0]);
        const auto [rf, rc] = any_cast<Pair<fURI_p, IntCoefficient>>(anonymous ? vs[0] : vs[1]);
        const auto [df, dc] = any_cast<Pair<fURI_p, IntCoefficient>>(anonymous ? vs[1] : vs[2]);
        const fURI_p dom_rng =
            furi_p(name->query({{FOS_DOMAIN, df->toString()},
                                {FOS_DOM_COEF, to_string(dc.first).append(",").append(to_string(dc.second))},
                                {FOS_RANGE, rf->toString()},
                                {FOS_RNG_COEF, to_string(rc.first).append(",").append(to_string(rc.second))}}));
        return dom_rng;
      };
      auto empty_bcode_action = [](const SemanticValues &) -> BCode_p { return Obj::to_bcode(); };
      auto coefficient_action = [](const SemanticValues &vs) -> IntCoefficient {
        int min;
        int max;
        // INT_MAX means "or more"
        string coefficient_string = vs.token_to_string();
        StringHelper::trim(coefficient_string);
        if(coefficient_string.empty()) {
          min = 1;
          max = 1;
        } else if(coefficient_string == "*") {
          min = 0;
          max = INT_MAX;
        } else if(coefficient_string == "?") {
          min = 0;
          max = 1;
        } else if(coefficient_string == "+") {
          min = 1;
          max = INT_MAX;
        } else if(coefficient_string == ".") {
          min = 0;
          max = 0;
        } else {
          const List<int> components = StringHelper::tokenize<int>(
              ',', coefficient_string, [](const string &s) { return StringHelper::is_integer(s) ? stoi(s) : INT_MAX; });
          if(components.empty()) { // I believe this first part is an unreachable branch
            min = 1;
            max = 1;
          } else if(1 == components.size()) {
            min = components[0];
            max = min;
          } else {
            min = components[0];
            max = components[1];
          }
        }
        if(min > max)
          throw fError("coefficient min can not be larger than max: %i > %i", min, max);
        return {min, max};
      };
      auto obj_action = [this](const SemanticValues &vs) -> Obj_p {
        switch(vs.choice()) {
          case 0: { // [a][b]@xyz
            const ID_p type_id = id_p(*any_cast<fURI_p>(vs[0]));
            const auto [v, o] = any_cast<Pair<Any, OType>>(vs[1]);
            return Obj::to_type(type_id, Obj::create(v, o, type_id),
                                vs.size() == 3 ? id_p(*std::any_cast<fURI_p>(vs[2])) : nullptr);
          }
          case 1: { // a(c)[b]@xyz
            bool has_type_id = false;
            ID_p type_id = id_p("");
            try { // TODO: figure out how to 'tag' semantic values
              if(vs.size() > 2) {
                type_id = id_p(*std::any_cast<fURI_p>(vs[0]));
                has_type_id = true;
              }
            } catch(const bad_any_cast &) {
              has_type_id = false;
              // do nothing
            }
            const auto args = any_cast<InstArgs>(vs[has_type_id ? 1 : 0]);
            const auto [v, o] = any_cast<Pair<Any, OType>>(vs[has_type_id ? 2 : 1]);
            const Obj_p body = Obj::create(v, o, OTYPE_FURI.at(o));
            const ID_p value_id =
                (vs.size() == (has_type_id ? 4 : 3)) ? id_p(*std::any_cast<fURI_p>(vs[has_type_id ? 3 : 2])) : nullptr;
            return Obj::to_inst(make_tuple(args, InstF(body), nullptr), type_id, value_id);
          }
          case 2: { // a(b)@xyz
            return any_cast<Inst_p>(vs[0]);
          }
          case 3: { // a[b]@xyz
            const ID_p type_id = id_p(*any_cast<fURI_p>(vs[0]));
            const auto [v, o] = any_cast<Pair<Any, OType>>(vs[1]);
            return Obj::create(v, o, type_id, vs.size() == 3 ? id_p(*std::any_cast<fURI_p>(vs[2])) : nullptr);
          }
          case 4: { // b@xyz
            const auto &[v, o] = any_cast<Pair<Any, OType>>(vs[0]);
            return Obj::create(v, o, OTYPE_FURI.at(o), vs.size() == 2 ? id_p(*std::any_cast<fURI_p>(vs[1])) : nullptr);
          }

          default:
            throw fError("unknown obj parse branch");
        }
      };
      //////////////////////////////////////////////////////////////////////////
      auto enter_y = [](const string &rule) {
        return [rule](const Context &c, const char *s, size_t n, any &dt) {
          // LOG_WRITE(TRACE, nullptr, L("entering rule !b{}!! with token !y{}!!\n", rule, s));
        };
      };
      //////////////////////////////////////////////////////////////////////////
#ifndef FOS_SUGARLESS_MMADT

      const auto SUGAR_GENERATOR = [this](Definition &definition, const string &sugar, const string &opcode) {
        definition <= seq(lit(sugar.c_str()), WRAQ("(", OBJ, START, ")")),
            [opcode](const SemanticValues &vs) -> Inst_p {
              return Obj::to_inst(vs.transform<Obj_p>(), id_p(ID(opcode)));
            };
      };
      auto barrier_action = [](const SemanticValues &vs) -> Inst_p {
        return Obj::to_inst(vs.transform<Obj_p>(), id_p(MMADT_PREFIX "barrier"));
      };
      auto within_action = [](const SemanticValues &vs) -> Inst_p {
        return Obj::to_inst(vs.transform<Obj_p>(), id_p(MMADT_PREFIX "within"));
      };
      auto end_action = [](const SemanticValues &) -> Inst_p {
        return Obj::to_inst(Obj::to_inst_args(), id_p(MMADT_PREFIX "end"));
      };
      auto merge_action = [](const SemanticValues &vs) -> Inst_p {
        return Obj::to_inst(vs.empty() ? Obj::to_inst_args() : Obj::to_inst_args({any_cast<Obj_p>(vs[0])}),
                            id_p(MMADT_PREFIX "merge"));
      };
      auto lshift_0_action = [](const SemanticValues &vs) -> Inst_p {
        return Obj::to_inst(rec(), id_p(MMADT_PREFIX "lshift"));
      };
      auto rshift_0_action = [](const SemanticValues &vs) -> Inst_p {
        return Obj::to_inst(rec(), id_p(MMADT_PREFIX "rshift"));
      };
      auto pass_action = [](const SemanticValues &vs) -> Inst_p {
        return Obj::to_inst(Obj::to_inst_args({any_cast<Obj_p>(vs[0]), dool(false)}), id_p(MMADT_PREFIX "ref"));
      };


#endif

      auto start_action = [](const SemanticValues &vs) {
        if(vs.size() == 1 && any_cast<Obj_p>(vs[0])->is_str() &&
           any_cast<Obj_p>(vs[0])->value_equals(*str(COMMENT_TOKEN)))
          return Obj::to_noobj();
        if(vs.size() == 1 && !any_cast<Obj_p>(vs[0])->is_code()) // is_bcode?
          return any_cast<Obj_p>(vs[0]);
        const auto insts = make_shared<List<Inst_p>>();
        Inst_p prev = nullptr;
        for(const auto &obj: vs.transform<Obj_p>()) {
          if(obj->is_str() && obj->str_value() == COMMENT_TOKEN) {
            // do nothing on comments
          } else if(!obj->is_code()) {
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
        // return insts->size() == 1 ? insts->front() : Obj::to_bcode(insts);
      };

      auto start_obj_action = [start_action](const SemanticValues &vs) -> Pair<Any, OType> {
        const auto obj = start_action(vs);
        return {obj->value_, obj->otype};
      };

      auto is_maker = [](const ID_p &predicate, const Obj_p &obj) -> Inst_p {
        return Obj::to_inst(Obj::to_inst_args({Obj::to_inst(Obj::to_inst_args({obj}), predicate)}), id_p("is"));
      };

      WS <= zom(cls(" \t\n"));
      ////////////////////// COMMENTS ///////////////////////////
      COMMENT <= cho(SINGLE_COMMENT, MULTI_COMMENT), [](const SemanticValues &) { return str("comment"); };
      SINGLE_COMMENT <= seq(~WS, lit("---"), zom(seq(npd(lit("\n")), dot())), lit("\n"), ~WS);
      MULTI_COMMENT <= seq(~WS, lit("==="), zom(seq(npd(lit("===")), dot())), lit("==="), ~WS);
      ////////////////////// FURI VARIANTS ///////////////////////////
      //      FURI_VAR <= cho(lit("}/"), lit("{/"), lit("/{"), lit("/}"));
      FURI_VAR <= seq(lit("{{"), START, lit("}}"));
      FURI <= WRAP("<",
                   tok(seq(npd(chr('-')), zom(cho(FURI_VAR, cls("a-zA-Z0-9:/%_@.#+-"))),
                           opt(seq(chr('?'), zom(seq(npd(lit("=>")), cls("a-zA-Z0-9:/%_=&@.#+-,"))))), npd(chr('-')))),
                   ">"),
          furi_action;
      FURI_INLINE <= WRAP("<",
                          tok(seq(zom(cho(FURI_VAR, cls("a-zA-Z:/%_#+"))), zom(cho(FURI_VAR, cls("a-zA-Z0-9:/%_#+"))),
                                  opt(seq(chr('?'), zom(seq(npd(lit("=>")), cls("a-zA-Z0-9:/%_=&@.#+,"))))))),
                          ">"),
          furi_action;
      FURI_NO_Q <=
          WRAP("<", tok(seq(oom(cho(FURI_VAR, cls("a-zA-Z:/%_.#+"))), zom(cho(FURI_VAR, cls("a-zA-Z0-9:/%_@.#+"))))),
               ">"),
          furi_action;
      DOM_RNG <= cho(seq(lit("<"), lit(">")), WRAP("<", seq(FURI_NO_Q, chr('?'), lit("{"), COEFFICIENT, lit("}")), ">"),
                     WRAP("<", seq(opt(FURI_NO_Q), chr('?'), SIGNATURE, lit("<="), SIGNATURE), ">")),
          dom_rng_action;
      TYPE_ID <= seq(ign(npd(lit("<<"))), cho(DOM_RNG, FURI_INLINE));
      VALUE_ID <= seq(chr('@'), FURI_INLINE);
      /////////////////// BASE TYPES ///////////////////////////
      NOOBJ <= lit("noobj"), noobj_action;
      BOOL <= cho(lit("true"), lit("false")), bool_action;
      INT <= tok(seq(opt(chr('-')), oom(cls("0-9")))), int_action;
      REAL <= tok(seq(opt(chr('-')), oom(cls("0-9")), chr('.'), oom(cls("0-9")))), real_action;
      STR <= seq(~WS, chr('\''), tok(zom(cho(lit("\\'"), ncls("\'")))), chr('\''), ~WS), str_action;
      URI <= cho(seq(lit("<"), lit(">")), FURI_INLINE, FURI), uri_action;
      LST <= cho(seq(lit("["), lit("]")), seq(lit("["), START, zom(seq(lit(","), START)), lit("]"))), lst_action;
      // LST.error_message = "typed polys are type wrapped polys: lst[[]] not lst[]";
      REC <= cho(seq(lit("["), lit("=>"), lit("]")),
                 seq(lit("["), START, lit("=>"), START, zom(seq(lit(","), START, lit("=>"), START)), lit("]"))),
          rec_action;
      OBJS <= cho(seq(lit("{"), lit("}")), seq(lit("{"), START, zom(seq(lit(","), START)), lit("}"))), objs_action;

      /////////////////// INST COMPONENTS ///////////////////////////
      INST <= cho(SUGAR_INST, NORMAL_INST);
      NORMAL_INST <= seq(TYPE_ID, ARGS, opt(VALUE_ID)), inst_action;
      ARGS <= cho(ARGS_REC, ARGS_LST), args_action;
      ARGS_REC <= cho(seq(lit("("), lit("=>"), lit(")")),
                      seq(lit("("), START, lit("=>"), START, zom(seq(lit(","), START, lit("=>"), START)), lit(")"))),
          rec_action;
      ARGS_LST <= cho(seq(lit("("), lit(")")), seq(lit("("), START, zom(seq(lit(","), START)), lit(")"))), lst_action;
      //////////////////////////////////////////////////////////////////////////////////////// TODO: stream ring theory
      COEFFICIENT <= tok(cho(cls("?*+."), seq(opt(INT), opt(lit(",")), opt(INT)))), coefficient_action;
      // zoo,zom,oom
      SIGNATURE <= seq(FURI_NO_Q, opt(seq(lit("{"), COEFFICIENT, lit("}")))),
          [](const SemanticValues &vs) -> Pair<fURI_p, IntCoefficient> {
            const auto furi = any_cast<fURI_p>(vs[0]);
            const auto coefficient = 1 == vs.size() ? IntCoefficient(1, 1) : any_cast<IntCoefficient>(vs[1]);
            return {furi, coefficient};
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
      EMPTY <= seq(lit(""), npd(chr(']'))),
          [](const SemanticValues &) -> Pair<Any, OType> { return {Obj::to_bcode(), OType::TYPE}; };
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      OBJ <=
          cho(seq(lit("["), TYPE_ID, lit("]"), lit("["), cho(START_OBJ, PROTO, EMPTY), lit("]"), opt(VALUE_ID)),
              // [a][b]@xyz
              seq(opt(TYPE_ID), ARGS, lit("["), cho(START_OBJ, PROTO, EMPTY), lit("]"), opt(VALUE_ID)), // a(c)[b]@xyz)
              seq(INST), // a(b)@xyz
              seq(TYPE_ID, lit("["), cho(START_OBJ, PROTO, EMPTY), lit("]"), opt(VALUE_ID)), // a[b]@xyz
              seq(PROTO, opt(VALUE_ID)) // b@xyz
              ),
          obj_action;
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////// START //////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      START <= seq(opt(~COMMENT), opt(OBJ), zom(cho(~COMMENT, seq(END, START), seq(lit("."), OBJ), SUGAR_INST))),
          start_action;
      START_OBJ <= START, start_obj_action;
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

      /////////////////////////////////////////////////////////////////
      ///////////////////////  INST SUGARS ////////////////////////////
      /////////////////////////////////////////////////////////////////
#ifndef FOS_SUGARLESS_MMADT
      SUGAR_INST <= cho(APPEND, EQ, GTE, GT, LTE, LT, NEQ, IS_A, AT, RSHIFT, LSHIFT, RSHIFT_0, LSHIFT_0, PLUS, MULT,
                        BARRIER, WITHIN, EMPTY_BCODE, FROM, PASS, LIFT, DROP, REF, CHOOSE, CHAIN, BLOCK, EACH, END,
                        MERGE, SPLIT, REF_OP /*, REPEAT*/);
      EMPTY_BCODE <= lit("_"), empty_bcode_action; // seq(lit("_"), ncls("0-9")), empty_bcode_action;
      SUGAR_GENERATOR(AT, "@", "at");
      SUGAR_GENERATOR(RSHIFT, ">>", MMADT_PREFIX "rshift");
      SUGAR_GENERATOR(LSHIFT, "<<", MMADT_PREFIX "lshift");
      SUGAR_GENERATOR(LIFT, "^", MMADT_PREFIX "lift");
      SUGAR_GENERATOR(DROP, "V", MMADT_PREFIX "drop");
      SUGAR_GENERATOR(FROM, "*", MMADT_PREFIX "from");
      SUGAR_GENERATOR(REF, "->", MMADT_PREFIX "ref");
      SUGAR_GENERATOR(APPEND, "-+>", MMADT_PREFIX "append");
      SUGAR_GENERATOR(BLOCK, "|", MMADT_PREFIX "block");
      SUGAR_GENERATOR(SPLIT, "-<", MMADT_PREFIX "split");
      SUGAR_GENERATOR(CHOOSE, "-|", MMADT_PREFIX "choose");
      SUGAR_GENERATOR(CHAIN, "-:", MMADT_PREFIX "chain");
      SUGAR_GENERATOR(EACH, "==", MMADT_PREFIX "each");
      SUGAR_GENERATOR(PLUS, "+", MMADT_PREFIX "plus");
      SUGAR_GENERATOR(MULT, "x", MMADT_PREFIX "mult");
      SUGAR_GENERATOR(EQ, "?=", MMADT_PREFIX "eq");
      SUGAR_GENERATOR(NEQ, "?!=", MMADT_PREFIX "neq");
      SUGAR_GENERATOR(GT, "?>", MMADT_PREFIX "gt");
      SUGAR_GENERATOR(GTE, "?>=", MMADT_PREFIX "gte");
      SUGAR_GENERATOR(LT, "?<", MMADT_PREFIX "lt");
      SUGAR_GENERATOR(LTE, "?<=", MMADT_PREFIX "lte");
      SUGAR_GENERATOR(IS_A, "?", MMADT_PREFIX "isa");
      SUGAR_GENERATOR(REF_OP, "<-", MMADT_PREFIX "ref_op");
      // SUGAR_GENERATOR(IS_NOT_A, "?!", MMADT_PREFIX "nota");
      EQ <= seq(lit("?="), WRAQ("(", OBJ, START, ")")),
          [](const SemanticValues &vs) -> Inst_p { return __().is(__().eq(any_cast<Obj_p>(vs[0]))); };
      NEQ <= seq(lit("?!="), WRAQ("(", OBJ, START, ")")),
          [](const SemanticValues &vs) -> Inst_p { return __().is(__().neq(any_cast<Obj_p>(vs[0]))); };
      GT <= seq(lit("?>"), WRAQ("(", OBJ, START, ")")),
          [](const SemanticValues &vs) -> Inst_p { return __().is(__().gt(any_cast<Obj_p>(vs[0]))); };
      GTE <= seq(lit("?>="), WRAQ("(", OBJ, START, ")")),
          [](const SemanticValues &vs) -> Inst_p { return __().is(__().gte(any_cast<Obj_p>(vs[0]))); };
      LT <= seq(lit("?<"), WRAQ("(", OBJ, START, ")")),
          [](const SemanticValues &vs) -> Inst_p { return __().is(__().lt(any_cast<Obj_p>(vs[0]))); };
      LTE <= seq(lit("?<="), WRAQ("(", OBJ, START, ")")),
          [](const SemanticValues &vs) -> Inst_p { return __().is(__().lte(any_cast<Obj_p>(vs[0]))); };
      LSHIFT_0 <= lit("<<"), lshift_0_action;
      RSHIFT_0 <= lit(">>"), rshift_0_action;
      PASS <= seq(lit("-->"), WRAQ("(", OBJ, START, ")")), pass_action;
      MERGE <= seq(chr('>'), opt(OBJ), chr('-')), merge_action;
      END <= lit(";"), end_action;
      WITHIN <= seq(lit("_/"), START, lit("\\_")), within_action;
      BARRIER <= seq(lit("_]"), START, lit("[_")), barrier_action;
      // REDUCE <= seq(lit("-{"), START, lit("}-")), reduce_action;
      ///////////////////////// DEBUG UTILITIES //////////////////////////////////////////
      REPEAT.enter = enter_y("repeat");
#endif
      ///////////////////////////////// PREDICATES ///////////////////////////////////////
      FURI.predicate = FURI_NO_Q.predicate =
          FURI_INLINE.predicate = [](const SemanticValues &vs, const std::any &, std::string &) {
            try {
              if(vs.token_to_string().empty() || vs.token_to_string() == "_")
                return false;
              fURI(vs.token_to_string()); // bad: using exception handling for branching
            } catch(const std::exception &e) {
              // msg = e.what();
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
      START.eoi_check = false;
      /////////////////////////////////////////////////////////////////////////////////////
    }
  };
} // namespace mmadt

#endif
