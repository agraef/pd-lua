**pd-lua** is Claude Heiland-Allen's [Pd][] loader extension for the [Lua][] scripting language. In a nutshell, it lets you program Pd objects in Lua.

The present version (0.8 and later) originated as a fork of the Debian 0.7.3 version maintained by IOhannes Zmölnig. It provides Lua 5.3+ compatibility, support for [Purr Data][] (a modern flavor of Pd) and [plugdata][] (another modern flavor of Pd which can also be run as a plug-in inside a DAW), some compilation fixes to make sure that the source builds on current systems (Linux, macOS, Windows), improved live-coding support, a lot of other bug fixes and improvements, and, last but not least, a [tutorial][] to help you get started using pd-lua.

**Quick links:**

- [**Sources**](https://github.com/agraef/pd-lua): Github repository with the latest source code. See [Installation](#installation) below.

- [**Deken package**](http://deken.puredata.info/library/pdlua): Install the latest version using Pd's package manager.

- [**Tutorial**](https://agraef.github.io/pd-lua/tutorial/pd-lua-intro.html): **Start here** if you're a new pd-lua user.


## Why pd-lua?

Pd's facilities for data structures, iteration, and recursion are somewhat limited, thus sooner or later you'll want to program your own Pd objects, also known as *externals*. Pd externals are usually programmed using C, the same programming language that Pd itself is written in. But novices may find C difficult to learn, and the arcana of Pd's C interface may also be hard to master.

Enter pd-lua which lets you develop your externals in the Lua scripting language. [Lua][], from [PUC Rio](http://www.puc-rio.br/), is a small programming language, but very capable, and is generally considered to be relatively easy to learn. For programming Pd objects, you'll also need to know a few bits and pieces which let you interface your Lua functions to Pd, but these aren't very complicated and can be picked up by just looking at a few basic examples.

pd-lua is also very mature, portable across different platforms and Pd flavors, and covers a lot of areas of the Pd programming interface. Specifically, it gives you access to Pd [arrays](https://agraef.github.io/pd-lua/tutorial/pd-lua-intro.html#using-arrays-and-tables), [clocks](https://agraef.github.io/pd-lua/tutorial/pd-lua-intro.html#using-clocks) and [receivers](https://agraef.github.io/pd-lua/tutorial/pd-lua-intro.html#using-receivers), and also has support for [signals, graphics](https://agraef.github.io/pd-lua/tutorial/pd-lua-intro.html#signals-and-graphics]), and [live-coding](https://agraef.github.io/pd-lua/tutorial/pd-lua-intro.html#live-coding). While C still rules in terms of execution speed, pd-lua is efficient enough for many purposes, gives you access to an abundance of 3rd party [Lua modules](https://luarocks.org/), and generally makes developing Pd objects much easier. It's also a lot of fun, just give it a try!

## History and credits

pd-lua was originally written by Claude Heiland-Allen, with contributions by Frank Barknecht and Martin Peach, according to the original source. Please check Claude's website at <https://mathr.co.uk/>, and <https://mathr.co.uk/blog/lua.html> for Lua-related content on his blog (including pd-lua's original announcement).

Martin Peach took over maintenance of pd-lua in 2011, you can find that part of the history in Pd's old [svn repository](https://sourceforge.net/p/pure-data/svn/HEAD/tree/trunk/externals/loaders/pdlua/) at SourceForge, including IOhannes Zmölnig's loader update for Pd 0.47. The present source started as a fork of the 0.7.3 version in IOhannes' [Debian package repository](https://anonscm.debian.org/git/pkg-multimedia/pd-lua.git). It is actively maintained by Albert Gräf (Lua 5.3 update, Purr Data support, pdx.lua live-coding extension, tutorial), with contributions by Claude Heiland-Allen (Lua 5.4 update, bugfixes), Alexandre Porres (documentation updates), sebshader (relative path loading), Timothy Schoen (plugdata compatibility, signal and graphics support), and Ben Wesch (GitHub workflow, Deken tests and uploads).

## Installation

Released source tarballs and binaries for Ubuntu, Mac and Windows can be found on the [Releases page](https://github.com/agraef/pd-lua/releases).

Building and installing from source is quite easy, once you have the requisite development environment (gcc/make on Linux, Xcode on Mac, mingw on Windows) and dependencies (Lua, Pd) installed. Instructions can be found in the [README](https://github.com/agraef/pd-lua/blob/master/README).

[Purr Data][] and [plugdata][] include pd-lua and have it enabled by default, so no extra software is needed.

For vanilla Pd, we recommend installing the [Deken package](http://deken.puredata.info/library/pdlua). This was originally uploaded by Alexandre Porres, but Ben Wesch recently added Deken tests and uploads to our GitHub workflow, so that the Deken releases are now automatized and always in sync with the GitHub releases. Thanks, Ben!

Other ready-made packages from various Linux distributions I know of:

- pd-lua is available in the official [Arch extra repositories](https://www.archlinux.org/packages/extra/x86_64/pd-lua/) (maintained by David Runge), and also from the [AUR](https://aur.archlinux.org/packages/pd-lua-git/); the latter builds pd-lua straight from the current git sources.
- The official [Debian package](https://salsa.debian.org/multimedia-team/pd/pd-lua) is maintained by IOhannes Zmölnig.

[Lua]: https://www.lua.org/
[Pd]: http://msp.ucsd.edu/software.html
[Purr Data]: https://agraef.github.io/purr-data/
[plugdata]: https://github.com/plugdata-team/plugdata
[tutorial]: https://agraef.github.io/pd-lua/tutorial/pd-lua-intro.html
