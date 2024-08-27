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

import sys
import colors

def head(cl_targets, build_targets, env):
    ASCII_ART = (""" 
{R}            PhaseShift Studio Presents
{M} <`--'>____  {B}______  __  __  ______  ______  {G}______  ______    
{M} /. .  `'  \\{B}/\\  ___\\/\\ \\_\\ \\/\\  __ \\/\\__  _\\/{G}\\  __ \\/\\  ___\\   
{M}(`')  ,     {LM}@{B} \\  __\\\\ \\  __ \\ \\  __ \\/_/\\ \\/{G}\\ \\ \\/\\ \\ \\___  \\  
{M} `-._,     / {B}\\ \\_\\   \\ \\_\\ \\_\\ \\_\\ \\_\\ \\ \\_\\{G} \\ \\_____\\/\\_____\\ 
{M}    )-)_/-(> {B} \\/_\\/   \\/_/\\/_/\\/_/\\/_/  \\/_/ {G} \\/_____/\\/_____/ 
{R}                                    A Dogturd Stynx Production{N}
    """).format(R=colors.RED, M=colors.PURPLE,B=colors.BLUE,G=colors.GREEN,LM=colors.LIGHT_PURPLE,N=colors.END)
    print()
    print(ASCII_ART, end="\n\n")
    print("Python version:", sys.version, end="\n\n")
    print(" ", "Environment:", env["PIOENV"])
    print(" ", "CLI targets", cl_targets)
    print(" ", "Build targets", build_targets)
    print()