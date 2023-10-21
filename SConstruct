#!/usr/bin/env python

env = SConscript("external/SConscript")

sources=[]
sources.append(Glob('src/*.cpp'))

library = env.SharedLibrary(
    "project/addons/luaCodeEdit/bin/libluacodeedit{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
    source=sources,
)

env.Default(library)