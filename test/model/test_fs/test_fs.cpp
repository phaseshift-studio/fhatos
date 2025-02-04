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

#ifndef fhatos_test_fs_cpp
#define fhatos_test_fs_cpp

#define FOS_DEPLOY_SCHEDULER
#define FOS_DEPLOY_ROUTER
#define FOS_DEPLOY_PARSER
#define FOS_DEPLOY_TYPE
#define FOS_DEPLOY_SHARED_MEMORY
#define FOS_DEPLOY_FILE_SYSTEM
#include <../test/test_fhatos.hpp>



namespace fhatos {
  static fs::path base_directory;
  static ptr<FileSystem> file_system;

  void stage() {
    fs::current_path(base_directory);
    LOG(INFO, "Original working directory: %s\n", base_directory.c_str());
    const fs::path p = fs::path(base_directory.string().c_str()).concat("/tmp");
    const uintmax_t removed = fs::remove_all(p);
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
    FOS_TEST_FURI_EQUAL(ID(FOS_TYPE_PREFIX "uri/fs:dir"), *file_system->to_dir("/")->tid_);
    TEST_ASSERT_EQUAL_INT(0, file_system->ls(file_system->to_dir("/"))->objs_value()->size());
    for (int i = 0; i < 10; i++) {
      const ID id = file_system->pattern()->extend(("a_" + to_string(i) + ".txt").c_str());
      File_p a = file_system->touch(id);
    }
    for (int i = 0; i < 10; i++) {
      const ID id = file_system->pattern()->extend(("a_" + to_string(i) + ".txt").c_str());
      File_p a = file_system->to_file(id);
      FOS_TEST_FURI_EQUAL(file_system->pattern()->extend(("a_" + to_string(i) + ".txt").c_str()), a->uri_value());
    }
    const Objs_p files = file_system->ls(file_system->to_dir("/"));
    TEST_ASSERT_TRUE(files->is_objs());
    int counter = 0;
    for (const auto &o: *files->objs_value()) {
      TEST_ASSERT_TRUE(o->uri_value().toString()[0] == 'a' && o->uri_value().toString()[1] == '_');
      counter++;
    }
    TEST_ASSERT_EQUAL_INT(10, counter);
    file_system->stop();
    unstage();
  }

  void test_filesystem_patterns() {
  }

  FOS_RUN_TESTS( //
    base_directory = string(fs::current_path().c_str()) + "/build"; //
    FOS_RUN_TEST(test_uris); //
    //FOS_RUN_TEST(test_files); //
  );
} // namespace fhatos

SETUP_AND_LOOP()

#endif
