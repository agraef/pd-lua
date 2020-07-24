
**pd-lua** is Claude Heiland-Allen's [Pd][] loader extension for the [Lua][] scripting language. In a nutshell, it lets you program Pd objects in Lua.

This is a fork of [umlaeute's repository](https://anonscm.debian.org/git/pkg-multimedia/pd-lua.git), from which the official Debian pd-lua packages are built. It adds Lua 5.3+ compatibility, support for [Purr Data][] (a modern flavor of Pd), some compilation fixes to make sure that the source builds on current systems (Linux, macOS, Windows), and, last but not least, a [tutorial][] to help you get started using pd-lua. Released source tarballs and binaries for Mac and Windows can be found [here](https://github.com/agraef/pd-lua/releases).

**Quick links:**

- [**Downloads**](https://github.com/agraef/pd-lua/releases): This is where you find source tarballs and binaries for **Mac and Windows**.

- [**Arch packages**](https://www.archlinux.org/): pd-lua is available in the official [Arch community repositories](https://www.archlinux.org/packages/community/x86_64/pd-lua/), and also from the [AUR](https://aur.archlinux.org/packages/pd-lua-git/) (the latter builds pd-lua straight from the current git sources)

- [**Sources**](https://github.com/agraef/pd-lua): Github repository with the latest source code

- [**Tutorial**](https://github.com/agraef/pd-lua/blob/master/tutorial/pd-lua-intro.md): **Start here** if you're a new pd-lua user

## Why pd-lua?

Pd's facilities for data structures, iteration, and recursion are somewhat limited, thus sooner or later you'll probably run into a problem that can't be easily solved by a Pd abstraction any more, at which point you'll want to program your own add-on Pd objects, also known as *externals*. Pd externals are usually programmed using C, the same programming language that Pd itself is written in. But novices may find C difficult to learn, and the arcana of Pd's C interface may also be hard to master.

Enter pd-lua, the Pd programmer's secret weapon, which lets you develop your externals in the Lua scripting language. [Lua][], from [PUC Rio](http://www.puc-rio.br/), is open-source (under the MIT license), mature, very popular, and supported by a large developer community. It is a small programming language, but very capable, and is generally considered to be relatively easy to learn. For programming Pd externals, you'll also need to learn a few bits and pieces which let you interface your Lua functions to Pd, but programming externals in Lua is still quite easy and a lot of fun (a lot easier and way more fun than C).

Using pd-lua, you can program your own externals ranging from little helper objects to full-blown algorithmic composition tools. pd-lua only allows you to program control objects at this time (for doing dsp, you might consider using [Faust][] instead), but it gives you access to Pd arrays, clocks and receivers, and also has good support for [live-coding](https://en.wikipedia.org/wiki/Live_coding).

## History and credits

pd-lua was originally written by Claude Heiland-Allen, with contributions by Frank Barknecht and Martin Peach, according to the original source.

Claude's repository is still online at <https://code.mathr.co.uk/pdlua>; please also check his website at <https://mathr.co.uk/>, and <https://mathr.co.uk/blog/lua.html> for Lua-related content on his blog (including pd-lua's original announcement).

Martin Peach took over maintenance of pd-lua in 2011, you can find that part of the history in Pd's old [svn repository](https://sourceforge.net/p/pure-data/svn/HEAD/tree/trunk/externals/loaders/pdlua/) at SourceForge, including IOhannes Zmölnig's loader update for Pd 0.47.

The present source is a fork of IOhannes' repository at <https://anonscm.debian.org/git/pkg-multimedia/pd-lua.git>, from which the official Debian packages are built. This in turn is apparently based on the 0.7.3 version of pd-lua in Pd's svn repository.

## Installing from source

Building and installing from source is quite easy, once you have the requisite development environment (gcc/make on Linux, Xcode on Mac, mingw on Windows) and dependencies (Lua, Pd) installed. Instructions can be found in the [README](https://github.com/agraef/pd-lua/blob/master/README).

[Lua]: https://www.lua.org/
[Faust]: https://faust.grame.fr/
[Pd]: http://msp.ucsd.edu/software.html
[Purr Data]: https://agraef.github.io/purr-data/
[tutorial]: https://github.com/agraef/pd-lua/blob/master/tutorial/pd-lua-intro.md
