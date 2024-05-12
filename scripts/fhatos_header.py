import sys

def head(cl_targets, build_targets, env):
    ASCII_ART = """ 
            PhaseShift Studio Presents
 <`--'>____  ______  __  __  ______  ______  ______  ______    
 /. .  `'  \\/\  ___\\/\ \_\ \\/\  __ \\/\__  _\\/\  __ \\/\  ___\   
('')  ,     @ \  __\\\\ \  __ \ \  __ \\/_/\ \\/\ \ \\/\ \ \___  \  
 `-._,     / \ \_\   \ \_\ \_\ \_\ \_\ \ \_\ \ \_____\\/\_____\ 
    )-)_/-(>  \\/_/    \\/_/\\/_/\\/_/\\/_/  \\/_/  \\/_____/\\/_____/ 
                                    A Dogturd Stynx Production
    """
    print()
    print(ASCII_ART, end="\n\n")
    print("Python version:", sys.version, end="\n\n")
    print(" ", "Environment:", env["PIOENV"])
    print(" ", "CLI targets", cl_targets)
    print(" ", "Build targets", build_targets)
    print()