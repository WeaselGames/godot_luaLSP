# LuaCodeEdit

WIP, come back later

The idea is for this extension to comunicate with the [lua language server](https://github.com/LuaLS/lua-language-server) over STDIN and STDOUT. I would of chosen to comunicate over a socket instead, but that currently does not work on unix systems with luaLS.

I do not plan on adding in editor support for the lua lanuage. But a editor plugin might be able to use this addon or parts of it to achieve that.

## TODO
- Basic auto completion
- Basic error checking
