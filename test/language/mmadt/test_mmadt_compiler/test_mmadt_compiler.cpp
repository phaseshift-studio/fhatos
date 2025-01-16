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


#define FOS_DEPLOY_PRINTER
#define FOS_DEPLOY_ROUTER
#define FOS_DEPLOY_SCHEDULER
#define FOS_DEPLOY_TYPE
#define FOS_DEPLOY_PARSER
#define FOS_DEPLOY_COMPILER
#define FOS_DEPLOY_PROCESSOR
#define FOS_DEPLOY_SHARED_MEMORY /compiler/#
#include "../../../../src/fhatos.hpp"
#include "../../../test_fhatos.hpp"
#include "../../../../src/util/string_helper.hpp"

namespace fhatos {
  using namespace mmadt;

  void test_type_check_base_types() {
    Compiler compiler = Compiler(false,false);
    FOS_TEST_COMPILER_TRUE(dool(true),BOOL_FURI,compiler.type_check);
    FOS_TEST_COMPILER_TRUE(jnt(1),INT_FURI,compiler.type_check);
    FOS_TEST_COMPILER_TRUE(real(1.2),REAL_FURI,compiler.type_check);
    FOS_TEST_COMPILER_TRUE(str("one.two"),STR_FURI,compiler.type_check);
    FOS_TEST_COMPILER_TRUE(vri("http://one/two"),URI_FURI,compiler.type_check);
    FOS_TEST_COMPILER_TRUE(lst({jnt(1),str("point"),real(2.0)}),LST_FURI,compiler.type_check);
    FOS_TEST_COMPILER_TRUE(rec({{jnt(1),str("point")},{real(2.0),dool(true)}}),REC_FURI,compiler.type_check);
    ////
    FOS_TEST_COMPILER_FALSE(dool(true),INT_FURI,compiler.type_check);
    FOS_TEST_COMPILER_FALSE(jnt(1),REAL_FURI,compiler.type_check);
    FOS_TEST_COMPILER_FALSE(real(1.2),BOOL_FURI,compiler.type_check);
    FOS_TEST_COMPILER_FALSE(str("one.two"),URI_FURI,compiler.type_check);
    FOS_TEST_COMPILER_FALSE(vri("http://one/two"),STR_FURI,compiler.type_check);
    FOS_TEST_COMPILER_FALSE(lst({jnt(1),str("point"),real(2.0)}),REC_FURI,compiler.type_check);
    FOS_TEST_COMPILER_FALSE(rec({{jnt(1),str("point")},{real(2.0),dool(true)}}),LST_FURI,compiler.type_check);
  }

  void test_type_check_derived_mono_types() {
    Compiler compiler = Compiler(false,false);
     for(const auto& TYPE_MAKER : List<BiConsumer<string,string>>({
             [](const string id, const string source){  TYPE_SAVER(id_p(id.c_str()),mmadt::Parser::singleton()->parse(source));},
             [](const string id, const string source){  PROCESS(StringHelper::format("%s -> |%s",id.c_str(),source.c_str())); }})) {
    	/////////////////////////////////// BOOL /////////////////////////////////////////////////////
    	TYPE_MAKER("/compiler/truth","true");
    	FOS_TEST_COMPILER_TRUE(dool(true),id_p("/compiler/truth"),compiler.type_check);
    	FOS_TEST_COMPILER_FALSE(dool(false),id_p("/compiler/truth"),compiler.type_check);
    	FOS_TEST_COMPILER_FALSE(jnt(43),id_p("/compiler/truth"),compiler.type_check);
    	FOS_TEST_COMPILER_FALSE(str("true"),id_p("/compiler/truth"),compiler.type_check);
    	/////////////////////////////////// INT /////////////////////////////////////////////////////
    	TYPE_MAKER("/compiler/nat","is(gt(0))");
    	FOS_TEST_COMPILER_TRUE(jnt(1),id_p("/compiler/nat"),compiler.type_check);
    	FOS_TEST_COMPILER_FALSE(jnt(-1),id_p("/compiler/nat"),compiler.type_check);
    	FOS_TEST_COMPILER_FALSE(real(1.46),id_p("/compiler/nat"),compiler.type_check);
    	FOS_TEST_COMPILER_FALSE(real(-1.46),id_p("/compiler/nat"),compiler.type_check);
    	////////////////////////////////////////////////////////////////////////////////////////////////////
    	TYPE_MAKER("/compiler/nat2","/compiler/nat2?int<=int(=>)[is(gt(0))]");
    	FOS_TEST_COMPILER_TRUE(jnt(1),id_p("/compiler/nat2"),compiler.type_check);
    	FOS_TEST_COMPILER_FALSE(jnt(-1),id_p("/compiler/nat2"),compiler.type_check);
    	FOS_TEST_COMPILER_FALSE(real(-1.23),id_p("/compiler/nat2"),compiler.type_check);
    	////////////////////////////////////////////////////////////////////////////////////////////////////
    	TYPE_MAKER("/compiler/nat3","/compiler/nat3?int{?}<=int(=>)[is(gt(0))]");
    	FOS_TEST_COMPILER_TRUE(jnt(1),id_p("/compiler/nat3"),compiler.type_check);
    	FOS_TEST_COMPILER_TRUE(jnt(-1),id_p("/compiler/nat3"),compiler.type_check);
    	FOS_TEST_COMPILER_FALSE(real(1.23),id_p("/compiler/nat3"),compiler.type_check);
    	FOS_TEST_COMPILER_FALSE(real(-1.23),id_p("/compiler/nat3"),compiler.type_check);
    }
  }

  void test_type_check_derived_poly_types() {
    Compiler compiler = Compiler(false,false);
      for(const auto& TYPE_MAKER : List<BiConsumer<string,string>>({
             [](const string id, const string source){  TYPE_SAVER(id_p(id.c_str()),mmadt::Parser::singleton()->parse(source));},
             [](const string id, const string source){  PROCESS(StringHelper::format("%s -> |%s",id.c_str(),source.c_str())); }})) {
		// lst
   		TYPE_MAKER("/compiler/lst_alias","[lst][]");
		FOS_TEST_COMPILER_TRUE(lst(),id_p("/compiler/lst_alias"),compiler.type_check);
		FOS_TEST_COMPILER_TRUE(lst({jnt(1),jnt(2)}),id_p("/compiler/lst_alias"),compiler.type_check);
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        TYPE_MAKER("/compiler/lst_swap_name","/compiler/lst_swap_name?lst<=lst()[-<[<1>.as(str),<0>.as(str)]]");
		//FOS_TEST_COMPILER_TRUE(lst({str("fhat"),str("os")}),id_p("/compiler/lst_swap_name"),compiler.type_check); // TODO: start vs. map mid-monoid
		FOS_TEST_COMPILER_FALSE(lst({jnt(1),str("two")}),id_p("/compiler/lst_swap_name"),compiler.type_check);
		//FOS_TEST_COMPILER_FALSE(lst({str("one")}),id_p("/compiler/lst_swap_name"),compiler.type_check);
        ////////////////////////////////////////////////////////////////////////////////////////////////////
		TYPE_MAKER("/compiler/lst_swap_name2", "/compiler/lst_swap_name2?lst<=lst()[==[as(str),as(str)]]");
		FOS_TEST_COMPILER_TRUE(lst({str("fhat"),str("os")}),id_p("/compiler/lst_swap_name2"),compiler.type_check);
		FOS_TEST_COMPILER_FALSE(lst({jnt(1),str("two")}),id_p("/compiler/lst_swap_name2"),compiler.type_check);
		//FOS_TEST_COMPILER_FALSE(lst({str("one")}),id_p("/compiler/lst_swap_name2"),compiler.type_check);
        ////////////////////////////////////////////////////////////////////////////////////////////////////
	}
  }

  void test_inst_resolution() {
    Compiler compiler = Compiler(true,true);
    FOS_TEST_ASSERT_EQUAL_FURI(INT_FURI->add_component(MMADT_SCHEME "/plus"), compiler.resolve_inst(jnt(1),Obj::to_inst({jnt(10)}, id_p("plus")))->tid_->no_query());
    string ds;
    compiler.print_derivation_tree(&ds);
    LOG_OBJ(INFO, jnt(10), "%s\n", ds.c_str());
  }

   void test_derived_type_inst_resolution() {
  	FOS_TEST_ASSERT_EQUAL_FURI(ID("/compiler/nat?dom=/mmadt/int&dc=1,1&rng=/mmadt/int&rc=1,1"), *PROCESS("/compiler/nat -> |/compiler/nat?int<=int()[is(gt(0))]")->tid());
    FOS_TEST_OBJ_EQUAL(Obj::to_int(5,id_p("/compiler/nat")), PROCESS("/compiler/nat[5]"));
    FOS_TEST_ERROR("/compiler/nat[-5]");
    FOS_TEST_OBJ_EQUAL(Obj::to_int(15,id_p("/compiler/nat")), PROCESS("/compiler/nat[5].plus(10)"));
    FOS_TEST_ERROR("/compiler/nat[5] + -10");
   }

  FOS_RUN_TESTS( //
    FOS_RUN_TEST(test_type_check_base_types); //
    FOS_RUN_TEST(test_type_check_derived_mono_types); //
    FOS_RUN_TEST(test_type_check_derived_poly_types); //
    FOS_RUN_TEST(test_inst_resolution); //
    FOS_RUN_TEST(test_derived_type_inst_resolution); //
  )
} // namespace fhatos

SETUP_AND_LOOP();