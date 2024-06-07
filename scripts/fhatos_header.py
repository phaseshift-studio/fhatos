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