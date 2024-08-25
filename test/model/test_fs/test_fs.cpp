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

#include <test_fhatos.hpp>

#ifndef fhatos_test_fs_cpp
#define fhatos_test_fs_cpp

#undef FOS_TEST_ON_BOOT

#include <language/insts.hpp>
#include <test_fhatos.hpp>
#include FOS_FILE_SYSTEM(fs.hpp)
#include <structure/router.hpp>
#include <language/parser.hpp>
#include <language/types.hpp>
#include FOS_PROCESS(scheduler.hpp)
#include <model/model.hpp>
#include <util/options.hpp>


namespace fhatos {
  static fs::path base_directory;

  void stage() {
    fs::current_path(base_directory);
    LOG(INFO, "Original working directory: %s\n", base_directory.c_str());
    const fs::path p = fs::path(base_directory.string().c_str()).concat("/tmp");
    uintmax_t removed = fs::remove_all(p);
    LOG(INFO, "Deleted existing working directory %s with %i items\n", p.c_str(), removed);
    TEST_ASSERT_TRUE(fs::create_directory(p));
    fs::current_path(p); //
    LOG(INFO, "Test working directory: %s\n", fs::current_path().c_str());
  }

  void unstage() {
    fs::current_path(base_directory);
    LOG(INFO, "Test complete, reverting to base directory: %s\n", fs::current_path().c_str());
  }

  void test_uris() {
    stage();
    FOS_TEST_OBJ_EQUAL(process("|as(/type/uri/)")->objs_value(0), process("*/type/uri/fs:dir")->objs_value(0));
    FOS_TEST_OBJ_EQUAL(process("|as(/type/uri/)")->objs_value(0), process("*/type/uri/fs:file")->objs_value(0));
    //      FOS_CHECK_RESULTS<Uri>({*bcode({Insts::as(uri(URI_FURI))})}, "*/type/uri/fs:dir");
    //FOS_CHECK_RESULTS<Uri>({*FileSystem::singleton()->root()}, "*/type/inst/fs:root");
    unstage();
  }

  void test_files() {
    stage();
    auto fs = FileSystem::singleton();
    FOS_TEST_ASSERT_EQUAL_FURI(ID(FOS_TYPE_PREFIX "uri/fs:dir"), *fs->to_dir("/")->id());
    TEST_ASSERT_EQUAL_INT(0, fs->ls(fs->to_dir("/"))->objs_value()->size());
    for (int i = 0; i < 10; i++) {
      const ID id = fs->id()->extend(("a_" + to_string(i) + ".txt").c_str());
      File_p a = fs->touch(id);
    }
    for (int i = 0; i < 10; i++) {
      const ID id = fs->id()->extend(("a_" + to_string(i) + ".txt").c_str());
      File_p a = fs->to_file(id);
      FOS_TEST_ASSERT_EQUAL_FURI(fs->id()->extend(("a_" + to_string(i) + ".txt").c_str()), a->uri_value());
    }
    const Objs_p files = fs->ls(fs->to_dir("/"));
    TEST_ASSERT_TRUE(files->is_objs());
    int counter = 0;
    for (const auto &o: *files->objs_value()) {
      TEST_ASSERT_TRUE(o->uri_value().toString()[0] == 'a' && o->uri_value().toString()[1] == '_');
      counter++;
    }
    TEST_ASSERT_EQUAL_INT(10, counter);
    fs->stop();
    unstage();
  }

  void test_filesystem_patterns() {
  }


  FOS_RUN_TESTS( //
          Model::deploy(Types::singleton());
          Model::deploy(Parser::singleton());
          base_directory = fs::current_path(); //
          Model::deploy(FileSystem::singleton("/fs", string(base_directory.c_str()) + "/tmp"));
          FOS_RUN_TEST(test_uris); //
          FOS_RUN_TEST(test_files); //
  );
} // namespace fhatos

SETUP_AND_LOOP()

#endif
