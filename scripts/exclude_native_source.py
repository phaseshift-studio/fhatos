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

import glob

import colors

Import("env")

print("{G}[{R}platformio{G}] {B}pre:exclude_native_source.py{N}".format(R=colors.RED,
                                                                             B=colors.BLUE,
                                                                             G=colors.GREEN,
                                                                             N=colors.END))
src_filter = []
for filename in glob.iglob('src' + '**/**', recursive=True):
    if "native" not in filename:
        src_filter.append(filename)
    else:
        print("   {M}excluding{N} from {B}build_src_filter{N}: {G}{FILE}{N}".format(R=colors.RED, M=colors.PURPLE,
                                                                              B=colors.BLUE,
                                                                              G=colors.GREEN, LM=colors.LIGHT_PURPLE,
                                                                              N=colors.END,
                                                                              FILE=filename))
print("\n")
env["build_src_filter"] = src_filter
