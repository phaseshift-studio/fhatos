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

#ifndef fhatos_test_parser_hpp
#define fhatos_test_parser_hpp

#define FOS_DEPLOY_ROUTER
#define FOS_DEPLOY_SCHEDULER
#define FOS_DEPLOY_PROCESSOR
#define FOS_DEPLOY_TYPE
#define FOS_DEPLOY_PARSER
#define FOS_DEPLOY_COMPILER
#define FOS_DEPLOY_SHARED_MEMORY

#include "../../test_fhatos.hpp"

namespace fhatos {
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////

  void test_no_input_parsing() { TEST_ASSERT_FALSE(Parser::singleton()->try_parse_obj("").has_value()); }

  void test_start_inst_parsing() {
    FOS_CHECK_RESULTS({"fhat"}, "'fh'.plus('at')");
    FOS_CHECK_RESULTS({22}, "1.plus(21)");
    FOS_CHECK_RESULTS({*vri("a/b")}, "a.plus(b)");
  }

  void test_noobj_parsing() {
    const Obj_p n = Parser::singleton()->try_parse_obj(FOS_TYPE_PREFIX "noobj/[]").value();
    TEST_ASSERT_EQUAL(OType::NOOBJ, n->otype);
    TEST_ASSERT_TRUE(n->is_noobj());
    TEST_ASSERT_EQUAL_STRING("!rnoobj!!", n->toString().c_str());
  }

  void test_bool_parsing() {
    Type::singleton()->save_type(id_p(FOS_TYPE_PREFIX "bool/fact"), Obj::to_bcode());
    const auto bools = List<Trip<string, bool, fURI>>(
    {{"true", true, *BOOL_FURI},
     {"false", false, *BOOL_FURI},
     {"fact[true]", true, BOOL_FURI->resolve("fact")}});
    for (const auto &trip: bools) {
      const Bool_p b = Parser::singleton()->try_parse_obj(get<0>(trip)).value();
      TEST_ASSERT_EQUAL(OType::BOOL, b->otype);
      TEST_ASSERT_EQUAL(get<1>(trip), b->bool_value());
      FOS_TEST_FURI_EQUAL(get<2>(trip), *b->tid);
    }
  }

  void test_int_parsing() {
    Type::singleton()->save_type(id_p(FOS_TYPE_PREFIX "int/zero"), Obj::to_bcode());
    Type::singleton()->save_type(id_p(FOS_TYPE_PREFIX "int/nat"), Obj::to_bcode());
    const auto ints =
        List<Trip<string, FOS_INT_TYPE, fURI>>({{"45", 45, *INT_FURI},
                                               {"0", 0, *INT_FURI},
                                               {"-12", -12, *INT_FURI},
                                               {"nat[100]", 100, INT_FURI->resolve("nat")},
                                               {"zero[0]", 0, INT_FURI->resolve("zero")},
                                               {"zero[2]", 2, INT_FURI->resolve("../int/zero")},
                                               {FOS_TYPE_PREFIX "/int/zero[0]", 0, INT_FURI->resolve("zero")},
                                               {FOS_TYPE_PREFIX "/int/zero[98]", 98, INT_FURI->resolve(
                                                    "../int/zero")},
                                               {FOS_TYPE_PREFIX "/int/zero[001]", 1, INT_FURI->extend("zero")}});
    for (auto &trip: ints) {
      const ptr<Int> i = Parser::singleton()->try_parse_obj(get<0>(trip)).value();
      TEST_ASSERT_EQUAL(OType::INT, i->otype);
      TEST_ASSERT_EQUAL_INT(get<1>(trip), i->int_value());
      FOS_TEST_FURI_EQUAL(get<2>(trip), *i->tid);
    }
  }

  void test_real_parsing() {
    Type::singleton()->save_type(id_p(FOS_TYPE_PREFIX "real/zero"), Obj::to_bcode());
    Type::singleton()->save_type(id_p(FOS_TYPE_PREFIX "real/nat"), Obj::to_bcode());
    // REAL
    const auto reals = List<Trip<string, FOS_REAL_TYPE, fURI>>(
    {{"45.1", 45.1f, *REAL_FURI},
     {"0.0000", 0.0f, *REAL_FURI},
     {"-12.1", -12.1f, *REAL_FURI},
     {"nat[10.0]", 10.0f, REAL_FURI->resolve("nat")},
     {"zero[0.21]", 0.21f, REAL_FURI->resolve("zero")},
     {"zero[2.0]", 2.0f, REAL_FURI->resolve("../real/zero")},
     {FOS_TYPE_PREFIX "/real/zero[1.1]", 1.1f, REAL_FURI->resolve("zero")},
     {FOS_TYPE_PREFIX "/real/zero[98.00]", 98.00f, REAL_FURI->resolve("../real/zero")},
     {FOS_TYPE_PREFIX "/real/zero[001.1]", 1.1f, REAL_FURI->extend("zero")}});
    for (auto &trip: reals) {
      const Real_p r = Parser::singleton()->try_parse_obj(get<0>(trip)).value();
      TEST_ASSERT_EQUAL(OType::REAL, r->otype);
      TEST_ASSERT_EQUAL_INT(get<1>(trip), r->real_value());
      FOS_TEST_FURI_EQUAL(get<2>(trip), *r->tid);
    }
  }

  void test_uri_parsing() {
    Type::singleton()->save_type(id_p(FOS_TYPE_PREFIX "uri/x"), Obj::to_bcode());
    Type::singleton()->save_type(id_p(FOS_TYPE_PREFIX "uri/furi:"), Obj::to_bcode());
    const auto uris = List<Trip<string, fURI, fURI>>({{"<blah.com>", fURI("blah.com"), *URI_FURI},
                                                      {"ga", fURI("ga"), *URI_FURI},
                                                      {"x[x]", fURI("x"), URI_FURI->resolve("x")},
                                                      {"furi:[blah_com]", fURI("blah_com"), URI_FURI->resolve(
                                                           "furi:")},
                                                      {"/abc_2467", fURI("/abc_2467"), *URI_FURI}});
    for (auto &trip: uris) {
      const Uri_p u = Parser::singleton()->try_parse_obj(std::get<0>(trip)).value();
      TEST_ASSERT_EQUAL(OType::URI, u->otype);
      FOS_TEST_FURI_EQUAL(get<1>(trip), u->uri_value());
      FOS_TEST_FURI_EQUAL(get<2>(trip), *u->tid);
    }
  }

  void test_str_parsing() {
    Type::singleton()->save_type(id_p(FOS_TYPE_PREFIX "str/name"), Obj::to_bcode());
    Type::singleton()->save_type(id_p(FOS_TYPE_PREFIX "str/origin"), Obj::to_bcode());
    const auto strs = List<Trip<string, string, fURI>>(
    {{"'bob'", "bob", *STR_FURI},
     {"'ga'", "ga", *STR_FURI},
     {"name['fhat']", "fhat", STR_FURI->resolve("name")},
     {"'a long and winding\nroad of\nstuff'", "a long and winding\nroad of\nstuff", *STR_FURI},
     {"origin['abc_2467']", "abc_2467", STR_FURI->resolve(
          "origin")}});
    for (auto &trip: strs) {
      const Str_p s = Parser::singleton()->try_parse_obj(std::get<0>(trip)).value();
      TEST_ASSERT_EQUAL(OType::STR, s->otype);
      TEST_ASSERT_EQUAL_STRING(get<1>(trip).c_str(), s->str_value().c_str());
      FOS_TEST_FURI_EQUAL(get<2>(trip), *s->tid);
    }
  }

  void test_lst_parsing() {
    // LST
    Type::singleton()->save_type(id_p(FOS_TYPE_PREFIX "int/nat"), Obj::to_bcode());
    Type::singleton()->save_type(id_p(FOS_TYPE_PREFIX "lst/atype"), Obj::to_bcode());
    Type::singleton()->save_type(id_p(FOS_TYPE_PREFIX "lst/btype"), Obj::to_bcode());
    Type::singleton()->save_type(id_p(FOS_TYPE_PREFIX "lst/ctype"), Obj::to_bcode());
    Type::singleton()->save_type(id_p(FOS_TYPE_PREFIX "bool/abool"), Obj::to_bcode());
    ///// EMPTY LIST
    FOS_CHECK_RESULTS({*Obj::to_lst(share(List<Obj_p>()))}, "[]"); // empty list
    FOS_CHECK_RESULTS({*Obj::to_lst(share(List<Obj_p>()), id_p(FOS_TYPE_PREFIX "lst/atype"))},
                      "atype[[]]"); // empty list
    ///// CONSTRUCTION PERMUTATIONS
    const auto lsts = List<Trip<string, List<Obj_p>, fURI>>(
    {{"['a',13,<actor>,false]",
      {Obj::to_str("a"), Obj::to_int(13), Obj::to_uri("actor"), Obj::to_bool(false)},
      *LST_FURI},
     {"['a' , 13,<actor> , false ]",
      {Obj::to_str("a"), Obj::to_int(13), Obj::to_uri("actor"), Obj::to_bool(false)},
      *LST_FURI},
     {"['a' ,    13 , <actor> ,    false  ]",
      {Obj::to_str("a"), Obj::to_int(13), Obj::to_uri("actor"), Obj::to_bool(false)},
      *LST_FURI},
     {"['a',    13 ,<actor>,   false]",
      {Obj::to_str("a"), Obj::to_int(13), Obj::to_uri("actor"), Obj::to_bool(false)},
      *LST_FURI},
     {"atype[['a',13 ,<actor>,   false]]",
      {Obj::to_str("a"), Obj::to_int(13), Obj::to_uri("actor"), Obj::to_bool(false)},
      LST_FURI->resolve("atype")},
     {"atype  [['a',13 ,<actor>,   false]]",
      {Obj::to_str("a"), Obj::to_int(13), Obj::to_uri("actor"), Obj::to_bool(false)},
      LST_FURI->resolve("atype")},
     {"btype[['a',  nat[13] , actor, abool   [false]]]",
      {Obj::to_str("a"), Obj::to_int(13), Obj::to_uri("actor"), Obj::to_bool(false)},
      LST_FURI->resolve("btype")},
     {"    btype[['a',  nat[13] ,actor,   abool [false]]]",
      {Obj::to_str("a"), Obj::to_int(13), Obj::to_uri("actor"), Obj::to_bool(false)},
      LST_FURI->resolve("btype")},
     {"ctype[['a',    13 , <actor>,  false]]",
      {Obj::to_str("a"), Obj::to_int(13), Obj::to_uri("actor"), Obj::to_bool(false)},
      LST_FURI->resolve("ctype")},
     {"   ctype    [['a',    13 ,<actor>,   false]]",
      {Obj::to_str("a"), Obj::to_int(13), Obj::to_uri("actor"), Obj::to_bool(false)},
      LST_FURI->resolve("ctype")}});
    for (const auto &trip: lsts) {
      FOS_TEST_MESSAGE("!yTesting!! !blst!! form %s", std::get<0>(trip).c_str());
      const Lst_p l = Parser::singleton()->try_parse_obj(std::get<0>(trip)).value();
      TEST_ASSERT_EQUAL(OType::LST, l->otype);
      TEST_ASSERT_EQUAL_STRING("a", l->lst_get(share(Int(0)))->str_value().c_str());
      TEST_ASSERT_EQUAL_INT(13, l->lst_get(share(Int(1)))->int_value());
      FOS_TEST_FURI_EQUAL(ID("actor"), l->lst_get(share(Int(2)))->uri_value());
      TEST_ASSERT_FALSE(l->lst_get(share(Int(3)))->bool_value());
      FOS_TEST_FURI_EQUAL(get<2>(trip), *l->tid);
    }
    ////////// SPLIT
    FOS_SHOULD_RETURN({"1"}, "1");
    FOS_SHOULD_RETURN({"[1,1]"}, "1-<[_,_]");
    FOS_SHOULD_RETURN({"[3,5]"}, "1-<[_,_]=[plus(2),plus(4)]");
    FOS_SHOULD_RETURN({"[3,true]"}, "1.-<[_,_]=[plus(2),plus(4)]=[_,type().eq(" FOS_TYPE_PREFIX "int/)]");
    /////////// GET/SET
    FOS_SHOULD_RETURN({"'fhat'"}, "[1,2,'fhat'].get(2)");
  }

  void test_rec_parsing() {
    // REC
   Type::singleton()->save_type(id_p("int/nat"),Obj::to_bcode());
    Type::singleton()->save_type(id_p(FOS_TYPE_PREFIX "rec/person"), Obj::to_bcode());
    Type::singleton()->save_type(id_p(FOS_TYPE_PREFIX "rec/atype"), Obj::to_bcode());
  Type::singleton()->save_type(id_p(FOS_TYPE_PREFIX "rec/btype"), Obj::to_bcode());
    Type::singleton()->save_type(id_p(FOS_TYPE_PREFIX "rec/ctype"),Obj::to_bcode());
    Type::singleton()->save_type(id_p(FOS_TYPE_PREFIX "bool/abool"),Obj::to_bcode());
    List<string> recs = {"['a'=>13,actor=>false]",
                         "['a' => 13,actor => false ]",
                         "['a'=> 13 , actor=>false]",
                         "['a' =>    13 , actor =>    false  ]",
                         "['a'=>    13 ,actor=>   false]",
                         "atype[['a'=>13 ,actor=>   false]]",
                         "btype[['a'=>  nat[13] , actor=>   abool[false]]  ]",
                         "ctype[  ['a'=>    13 ,  actor=>   false]]"};
    for (const string &form: recs) {
      FOS_TEST_MESSAGE("!yTesting!! !brec!! form %s", form.c_str());
      const Rec_p rc1 = Parser::singleton()->try_parse_obj(form).value();
      TEST_ASSERT_EQUAL(OType::REC, rc1->otype);
      TEST_ASSERT_EQUAL_INT(13, rc1->rec_get(str("a"))->int_value()); // a's value is 13
      TEST_ASSERT_TRUE(rc1->rec_get(jnt(13))->is_noobj()); // no key is 13
      TEST_ASSERT_TRUE(rc1->rec_get(str("no key"))->is_noobj()); // no key is no key
      TEST_ASSERT_FALSE(rc1->rec_get(vri("actor"))->bool_value());
    }

    Type::singleton()->save_type(id_p(FOS_TYPE_PREFIX "rec/person"),
                                    parse("[:age=>as(/type/int/nat),:name=>as(/type/str/)]"));
    recs = {"person[[:age=>nat[29],:name=>'dogturd']]"};
    for (const string &form: recs) {
      FOS_TEST_MESSAGE("!yTesting!! !brec!! structure %s", form.c_str());
      const Rec_p rc2 = Parser::singleton()->try_parse_obj(form).value();
      TEST_ASSERT_EQUAL(OType::REC, rc2->otype);
      TEST_ASSERT_EQUAL_STRING("person", rc2->tid->name().c_str());
      TEST_ASSERT_EQUAL_INT(29, rc2->rec_get(vri(":age"))->int_value());
      TEST_ASSERT_EQUAL_STRING("dogturd", rc2->rec_get(vri(":name"))->str_value().c_str());
      TEST_ASSERT_EQUAL(OType::NOOBJ, rc2->rec_get(13)->otype); // TODO
      TEST_ASSERT_TRUE(rc2->rec_get("no key")->is_noobj());
    } /*
     ///////////////////////////////////
     const ptr<Rec> rc2 = Parser::singleton()->tryParseObj("['a'=>13,/actor=>['b'=>1,'c'=>3]]").value();
     TEST_ASSERT_EQUAL(OType::REC, rc2->otype);
     TEST_ASSERT_EQUAL_INT(13, rc2->rec_get("a")->int_value());
     //    TEST_ASSERT_EQUAL(OType::NOOBJ, rc2->get<Str>(ptr<Int>(new Int(13)))->otype());
     TEST_ASSERT_TRUE(rc2->rec_get("/actor")->is_noobj()); // it's a string, not a uri
     const ptr<Rec> rc3 = rc2->rec_get(u("/actor"));
     TEST_ASSERT_EQUAL(OType::REC, rc3->otype);
     TEST_ASSERT_EQUAL_INT(1, rc3->rec_get("b")->int_value());
     TEST_ASSERT_EQUAL_INT(3, rc3->rec_get("c")->int_value());
     TEST_ASSERT_EQUAL_STRING("['a'=>13,/actor=>['b'=>1,'c'=>3]]",
                              Options::singleton()->printer<>()->strip(rc2->toString().c_str()).get());*/
  }

  void test_define_as_parsing() {
    FOS_CHECK_RESULTS({*parse("is(mod(2).eq(0))")},
                      FOS_TYPE_PREFIX "int/even.->|(is(mod(2).eq(0)))"); // TODO: parse is off for ->
    FOS_CHECK_RESULTS({*vri(FOS_TYPE_PREFIX "int/even")}, "{32}.as(even).type()");
    FOS_CHECK_RESULTS({*vri(FOS_TYPE_PREFIX "int/even")}, "even[32].type()");
    FOS_CHECK_RESULTS({*vri(FOS_TYPE_PREFIX "int/even")}, "{even[32]}.type()");
    FOS_TEST_ERROR("even[1]");
    FOS_TEST_ERROR("even[3]");
    FOS_TEST_ERROR("even[5]");
    ////// INLINE TYPE-SLOT
    FOS_CHECK_RESULTS({*vri(FOS_TYPE_PREFIX "int/even")}, "{32}.even[_].type()");
    FOS_CHECK_RESULTS({*jnt(32, id_p(FOS_TYPE_PREFIX "int/even"))}, "{32}.even[_]");
    FOS_CHECK_RESULTS({*jnt(32, id_p(FOS_TYPE_PREFIX "int/even"))}, "{32}.map(even[_])");
    FOS_CHECK_RESULTS({*jnt(10, id_p(FOS_TYPE_PREFIX "int/even")), *jnt(32, id_p(FOS_TYPE_PREFIX "int/even"))},
                      "{10,32}.map(even[_])");
    FOS_TEST_ERROR("{1}.even[_]");
    FOS_TEST_ERROR("{3}.map(even[_])");
    FOS_TEST_ERROR("{5}.plus(1).plus(1).even[_]");
  }

  void test_to_from() {
    FOS_CHECK_RESULTS({10},
                      List<string>({"y.to(x)", "z.to(y)", "10.to(z)"})); // TODO: fix to() so it doesn't Ø on start
    FOS_CHECK_RESULTS({10}, "from(z)");
    FOS_CHECK_RESULTS({10}, "from(from(y))");
    FOS_CHECK_RESULTS({10}, "from(from(from(x)))");
    FOS_CHECK_RESULTS({10}, "*z");
    FOS_CHECK_RESULTS({10}, "**y");
    FOS_CHECK_RESULTS({10}, "*(**x)", {}, true); // TODO:: parse is off for *
    ///// VARIATIONS OF TYPE DEFINITIONS
    BCODE_PROCESSOR(OBJ_PARSER("/type/inst/testing.->(block(plus(10).mult(2)))"));
    FOS_CHECK_RESULTS({22}, "{1}.testing(6)");
    BCODE_PROCESSOR(OBJ_PARSER("/type/inst/testing.-> block(plus(10).mult(2))"));
    FOS_CHECK_RESULTS({22}, "{1}.testing(6)");
   BCODE_PROCESSOR(OBJ_PARSER("/type/inst/testing -> block(plus(10).mult(2))"));
    FOS_CHECK_RESULTS({22}, "{1}.testing(6)");
    BCODE_PROCESSOR(OBJ_PARSER("/type/inst/testing -> |(plus(10).mult(2))"));
    FOS_CHECK_RESULTS({22}, "{1}.testing(6)");
  }

  void test_process_thread_parsing() {
    const ptr<BCode> bcode = Parser::singleton()
        ->try_parse_obj("thread[[setup => |print('.setup complete.'),"
            "        loop  => |stop(/abc/)]].to(/abc/)")
        .value();
    FOS_PRINT_OBJ(bcode);
    Fluent(bcode).iterate();
    Scheduler::singleton()->barrier((Options::singleton()->router<Router>()->vid->toString() + "_wait").c_str(),
                                    [] { return Scheduler::singleton()->count("/abc/") == 0; });
  }

  void test_group_parsing() {
    FOS_CHECK_RESULTS(List<Obj>({*Obj::to_rec({{false, {1, 3}},
                                               {true, {2, 4}}})}),
                      "{0,1,2,3}.plus(1).group(mod(2).eq(0))");
    FOS_CHECK_RESULTS(List<Obj>({*Obj::to_rec({{false, {2, 4}},
                                               {true, {3, 5}}})}),
                      "{0,1,2,3}.plus(1).group(mod(2).eq(0),plus(1))");
    ////////////////////
    /*FOS_CHECK_RESULTS<>(List<Obj>({*Obj::to_rec({{false, {2, 4}},
                                                 {true,  {3, 5}}})}),
                        "{0,1,2,3}.plus(1).group(mod(2).eq(0)).by(plus(1))");
    FOS_CHECK_RESULTS<>(List<Obj>({*Obj::to_rec({{false, {2, 4}},
                                                 {true,  {3, 5}}})}),
                        "{0,1,2,3}.plus(1).group().by(mod(2).eq(0)).by(plus(1))");
    FOS_TEST_ERROR("{0,1,2,3}.plus(1).group().by(mod(2).eq(0)).by(plus(1)).by(_).by(_)");*/
  }

  void test_window_parsing() {
    FOS_CHECK_RESULTS(List<Obj>{{1, 2},
                                {2, 3},
                                {3, 4}}, "[1,2,3,4].window([_,_])");
    FOS_CHECK_RESULTS(List<Obj>{"12", "23", "34"}, "'1234'.window([_,_])");
  }

  void test_split_within_merge_parsing() {
    FOS_CHECK_RESULTS(List<Obj>{{1, 3, 1}}, "1-<[_,plus(2),_]");
    FOS_CHECK_RESULTS(List<Obj>{{2, 30, 1}}, "1-<[_,plus(2),_]=[plus(1),mult(10),_]");
    FOS_CHECK_RESULTS(List<Obj>{2, 30, 1}, "1-<[_,plus(2),_]=[plus(1),mult(10),_]>-");
    FOS_CHECK_RESULTS(List<Obj>{{33}}, "1-<[_,plus(2),_]=[plus(1),mult(10),_]._/sum()\\_");
    FOS_CHECK_RESULTS(List<Obj>{33}, "1-<[_,plus(2),_]=[plus(1),mult(10),_]._/sum()\\_>-");
    FOS_CHECK_RESULTS(List<Obj>{33}, "1-<[_,plus(2),_]=[plus(1),mult(10),_]_/sum()\\_>-");
    FOS_CHECK_RESULTS(List<Obj>{*vri("/abc/"), *vri("/abc/d"), *vri("/d")}, "/abc/-<[_,./d,/d]>-");
  }

  void test_one_to_many() {
    // without
    FOS_CHECK_RESULTS({5}, "{1,2,3,4,5}.count()");
    FOS_CHECK_RESULTS({15}, "{1,2,3,4,5}.sum()");
    FOS_CHECK_RESULTS({120}, "{1,2,3,4,5}.prod()");
    // within
    FOS_CHECK_RESULTS({5}, "[1,2,3,4,5]_/count()\\_>-");
    FOS_CHECK_RESULTS({15}, "[1,2,3,4,5]_/sum()\\_>-");
    FOS_CHECK_RESULTS({120}, "[1,2,3,4,5]_/prod()\\_>-");
  }

  void test_pubsub_writeread() {
    process("z2 -> noobj");
    TEST_ASSERT_TRUE(process("*z2")->objs_value()->empty());
    process("<z?sub> -> |(z2 -> 'abc')");
    //LOG(INFO, "HERE %s\n", process("*z2")->toString().c_str());
    //TEST_ASSERT_TRUE(process("*z2")->objs_value()->empty());
    process("z -> 55");
    TEST_ASSERT_EQUAL_STRING("abc", process("*z2")->objs_value()->front()->str_value().c_str());
    process("z2 -> noobj");
    TEST_ASSERT_TRUE(process("*z2")->objs_value()->empty());
  }

  FOS_RUN_TESTS( //
      //Options::singleton()->log_level(TRACE); //
      FOS_RUN_TEST(test_no_input_parsing); //
      FOS_RUN_TEST(test_start_inst_parsing); //
      FOS_RUN_TEST(test_noobj_parsing); //
      FOS_RUN_TEST(test_bool_parsing); //
      FOS_RUN_TEST(test_int_parsing); //
      FOS_RUN_TEST(test_real_parsing); //
      FOS_RUN_TEST(test_uri_parsing); //
      FOS_RUN_TEST(test_str_parsing); //
      FOS_RUN_TEST(test_lst_parsing); //
      FOS_RUN_TEST(test_rec_parsing); //
      ////////////// PARTICULAR MONOID IDIOMS
      FOS_RUN_TEST(test_define_as_parsing); //
      FOS_RUN_TEST(test_to_from); //
      //FOS_RUN_TEST(test_process_thread_parsing); //
      FOS_RUN_TEST(test_group_parsing); //
      FOS_RUN_TEST(test_window_parsing); //
      FOS_RUN_TEST(test_split_within_merge_parsing); //
      FOS_RUN_TEST(test_one_to_many); //
      FOS_RUN_TEST(test_pubsub_writeread);
      )
}; // namespace fhatos

SETUP_AND_LOOP();


#endif