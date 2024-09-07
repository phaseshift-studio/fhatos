#   FhatOS: A Distributed Operating System
#   Copyright (c) 2024 PhaseShift Studio, LLC
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU Affero General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU Affero General Public License for more details.
#
#   You should have received a copy of the GNU Affero General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.

import shutil
from pathlib import Path
Import("env")
source = "esp"
target = "none"
print("source:", source)
print("target:", target)
if 'esp' in source:
    src_dir = env.GetProjectOption("fs_mount_src")
    dst_dir = env.GetProjectOption("fs_mount_dst")
else:
    src_dir = "mmadt/common.mmadt"
    dst_dir = "build/tmp"
print("fs_mount_src: ", src_dir)
print("fs_mount_dst: ", dst_dir)
Path(dst_dir).mkdir(parents=True, exist_ok=True)
shutil.copy(src_dir, dst_dir)
