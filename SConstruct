#!/usr/bin/env python

env = SConscript("external/SConscript")

sources=[]
sources.append(Glob('src/*.cpp'))
sources.append(Glob('src/platform/*.cpp'))

if env['platform'] == 'windows':
    sources.append(Glob('src/platform/windows/*.cpp'))
    env.Append(CPPDEFINES=['LCE_PLATFORM_WINDOWS'])
elif env['platform'] == 'linux' or env['platform'] == 'macos' or env['platform'] == 'osx' or env['platform'] == 'linuxbsd':
    sources.append(Glob('src/platform/posix/*.cpp'))
    env.Append(CPPDEFINES=['LCE_PLATFORM_POSIX'])
else:
    print("Unsupported platform: " + env['platform'])
    Exit(1)


library = env.SharedLibrary(
    "project/addons/luaCodeEdit/bin/libluacodeedit{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
    source=sources,
)

env.Default(library)