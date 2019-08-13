---
id: curry-on-2019
title: Gradual typing of Ruby at Scale
sidebar_label: Curry On! 2019
---

- [→ Talk page](https://www.curry-on.org/2019/sessions/gradual-typing-for-ruby-at-scale-with-sorbet.html)
- [→ Video](https://www.curry-on.org/2019/sessions/gradual-typing-for-ruby-at-scale-with-sorbet.html)
- [→ Slides](https://sorbet.run/talks/CurryOn2019/#/)

## Abstract

Stripe maintains an extremely large and growing Ruby code base in which ~3/4 of
Stripe's engineers do most of their work. Continuing to scale development in
that code base is one of the most critical tasks to maintaining product velocity
and the productivity of Stripe engineering.

Based on a wide range of experiences, we believe that adding static types for a
significant subset of that codebase helps developers understand code better,
write code with more confidence, and detect+prevent significant classes of bugs.

This talk shares experience of Stripe successfully been building a typechecker
for internal use, including core design decisions made in early days of the
project and how they withstood reality of production use:

- The choice of building a gradual type system, allowing different teams to
  adopt it independently on different pace, at cost of reduced guarantees;
- The choice of using manifest types and leaning towards explicitness, to make
  it easier to read and maintain code, at cost of more verbose definitions;
- The choice of using a nominal type system, to encourage development of
  interfaces where every type has to be declare all the interfaces upfront, at
  cost of ease of adoption in a duck-typed language;
- The choice to build a very custom control flow dependent type checking to
  infer truthiness and organically support smart casts and pattern matching.
