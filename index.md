**pd-lua** is Claude Heiland-Allen's [Pd][] loader extension for the [Lua][] scripting language. In a nutshell, it lets you program Pd objects in Lua.

The present version was forked from [umlaeute's repository](https://anonscm.debian.org/git/pkg-multimedia/pd-lua.git) on Debian's Salsa server. It provides Lua 5.3+ compatibility, support for [Purr Data][] (a modern flavor of Pd) and [plugdata][] (another modern flavor of Pd which can also be run as a plug-in inside a DAW), some compilation fixes to make sure that the source builds on current systems (Linux, macOS, Windows), various bug fixes and improvements (including Timothy Schoen's signal and graphics support), and, last but not least, a [tutorial][] to help you get started using pd-lua. Moreover, it includes the latest help and example patches which have been updated and reorganized by Alexandre Porres.

**Quick links:**

- [**Downloads**](https://github.com/agraef/pd-lua/releases): This is where you find source tarballs and binaries for **Ubuntu**, **Mac and Windows**. These are ready to be used with vanilla [Pd][]; [Purr Data][] already includes pd-lua and has it enabled by default.

- [**Arch packages**](https://www.archlinux.org/): pd-lua is available in the official [Arch extra repositories](https://www.archlinux.org/packages/extra/x86_64/pd-lua/), and also from the [AUR](https://aur.archlinux.org/packages/pd-lua-git/); the latter builds pd-lua straight from the current git sources.

- [**Debian package**](https://salsa.debian.org/multimedia-team/pd/pd-lua): The official Debian package is maintained by IOhannes Zmölnig.

- [**Deken package**](http://deken.puredata.info/library/pdlua): The Deken package is maintained by Alexandre Porres.

- [**Sources**](https://github.com/agraef/pd-lua): Github repository with the latest source code. See [Installation](#installation) below.

- [**Tutorial**](https://agraef.github.io/pd-lua/tutorial/pd-lua-intro.html): **Start here** if you're a new pd-lua user.

## Why pd-lua?

Pd's facilities for data structures, iteration, and recursion are somewhat limited, thus sooner or later you'll want to program your own Pd objects, also known as *externals*. Pd externals are usually programmed using C, the same programming language that Pd itself is written in. But novices may find C difficult to learn, and the arcana of Pd's C interface may also be hard to master.

Enter pd-lua, the Pd programmer's secret weapon, which lets you develop your externals in the Lua scripting language. [Lua][], from [PUC Rio](http://www.puc-rio.br/), is a small programming language, but very capable, and is generally considered to be relatively easy to learn. For programming Pd externals, you'll also need to learn a few bits and pieces which let you interface your Lua functions to Pd, but programming externals in Lua is still quite easy and a lot of fun. pd-lua only allows you to program control objects at this time (for doing dsp, you might consider using [Faust][] instead), but it gives you access to Pd arrays, clocks and receivers, and also has good support for [live-coding](https://en.wikipedia.org/wiki/Live_coding).

## History and credits

pd-lua was originally written by Claude Heiland-Allen, with contributions by Frank Barknecht and Martin Peach, according to the original source. Claude's repository is still online at <https://code.mathr.co.uk/pdlua>; please also check his website at <https://mathr.co.uk/>, and <https://mathr.co.uk/blog/lua.html> for Lua-related content on his blog (including pd-lua's original announcement).

Martin Peach took over maintenance of pd-lua in 2011, you can find that part of the history in Pd's old [svn repository](https://sourceforge.net/p/pure-data/svn/HEAD/tree/trunk/externals/loaders/pdlua/) at SourceForge, including IOhannes Zmölnig's loader update for Pd 0.47. The present source started as a fork of IOhannes' repository at <https://anonscm.debian.org/git/pkg-multimedia/pd-lua.git>, from which the official Debian packages are built. This in turn was based on the 0.7.3 version of pd-lua in Pd's svn repository at the time. But it has since been updated to the present source, which is actively maintained and works with Lua 5.3 and later. You're invited to submit bug reports and pull requests to help with keeping pd-lua current and useful to Pd and Lua programmers.

## Installation

Released source tarballs and binaries for Ubuntu, Mac and Windows can be found on the [Releases page](https://github.com/agraef/pd-lua/releases).

Building and installing from source is quite easy, once you have the requisite development environment (gcc/make on Linux, Xcode on Mac, mingw on Windows) and dependencies (Lua, Pd) installed. Instructions can be found in the [README](https://github.com/agraef/pd-lua/blob/master/README).

[Lua]: https://www.lua.org/
[Faust]: https://faust.grame.fr/
[Pd]: http://msp.ucsd.edu/software.html
[Purr Data]: https://agraef.github.io/purr-data/
[plugdata]: https://github.com/plugdata-team/plugdata
[tutorial]: https://agraef.github.io/pd-lua/tutorial/pd-lua-intro.html
