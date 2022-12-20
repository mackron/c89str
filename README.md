<h4 align="center">C89 compatible string library.</h4>

<p align="center">
    <a href="https://discord.gg/9vpqbjU"><img src="https://img.shields.io/discord/712952679415939085?label=discord&logo=discord&style=flat-square" alt="discord"></a>
    <a rel="me" href="https://fosstodon.org/@mackron"><img src="https://img.shields.io/mastodon/follow/109293691403797709?color=blue&domain=https%3A%2F%2Ffosstodon.org&label=mastodon&logo=mastodon&style=flat-square" alt="mastodon"></a>
    <a href="https://twitter.com/mackron"><img src="https://img.shields.io/twitter/follow/mackron?label=twitter&color=1da1f2&logo=twitter&style=flat-square" alt="twitter"></a>
</p>

This is a single file library for making working with strings in C a bit easier.

Main features:
  - A suite of UTF-8, UTF-16 and UTF-32 conversion routines.
  - An API for dynamic strings (`c89str`).
  - Alternatives to some standard library functions.
  - A suite of miscellaneous APIs that might be useful.
  - An implementation of sprintf().

The APIs in this library are focused on flexibility and will favor verbosity over simplicity. All functions return
a result code. When a function could possibly need to do a memory allocation, a parameter will exist for a pointer
to allocation callbacks. If you're after a terser API you should consider looking elsewhere or build your own
wrapper.

I do not maintain version numbers for this library, nor do I guarantee API-compatibility. You need to look
elsewhere if this is an issue for you.

To define the implementation, do this in one source file:

    #define C89STR_IMPLEMENTATION
    #include "c89str.h"

See the top of c89str.h for more details about the library.
