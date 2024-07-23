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

#include <test_fhatos.hpp>
//
#include <structure/furi.hpp>
#include FOS_FILE_SYSTEM(filesystem.hpp)

namespace fhatos {
  void stage() {
    LOG(INFO, "Original working directory: %s\n", fs::current_path().c_str());
    const fs::path p = fs::current_path().concat("/tmp");
    int removed = fs::remove_all(p);
    LOG(INFO, "Deleted existing working directory with %i items\n", removed);
    TEST_ASSERT_TRUE(fs::create_directory(p));
    fs::current_path(p); //
    LOG(INFO, "Test working directory: %s\n", fs::current_path().c_str());
  }

  void test_files() {
    stage();
    FileSystem *fs = FileSystem::singleton(ID("/io/fs"), ID(fs::current_path().string()));
    Scheduler::singleton()->spawn(fs);
    FOS_TEST_ASSERT_EQUAL_FURI(ID("/uri/dir"), *fs->to_dir("/")->id());
    TEST_ASSERT_EQUAL_INT(0, fs->ls(fs->to_dir("/"), "#")->objs_value()->size());
    for (int i = 0; i < 10; i++) {
      string filename = "a_" + to_string(i) + ".txt";
      File_p a = fs->touch(ID(filename));
      FOS_TEST_ASSERT_EQUAL_FURI(ID(filename), a->uri_value());
    }
    const Objs_p files = fs->ls(fs->to_dir("/"), "#");
    TEST_ASSERT_EQUAL(files->o_type(), OType::OBJS);
    int counter = 0;
    for (const auto &o: *files->objs_value()) {
      TEST_ASSERT_TRUE(o->uri_value().toString()[0] == 'a' && o->uri_value().toString()[1] == '_');
      counter++;
    }
    TEST_ASSERT_EQUAL_INT(10, counter);
  }


  FOS_RUN_TESTS( //
      FOS_RUN_TEST(test_files); //
  );
} // namespace fhatos

SETUP_AND_LOOP()

#endif
