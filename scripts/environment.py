Import("env")
from fhatos_header import head
head(COMMAND_LINE_TARGETS, BUILD_TARGETS, env)
# BEFORE BUILDING FIRMWARE


def before_build(source, target, env):
    print("SOURCE: ", end="")
    print(*source, sep=' ')
    print("TARGET: ", end="")
    print(*target, sep=' ')
    print("ENV: ")
    for key, value in env.items():
        print("\t", key, ":", value)
    # do some actions

env.AddPreAction("buildprog", before_build)
