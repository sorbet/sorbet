---
id: strange-loop-2018
title: "Gradual typing of Ruby at Scale"
sidebar_label: "Strange Loop 2018"
---

- [→ Talk page](https://www.thestrangeloop.com/2018/gradual-typing-of-ruby-at-scale.html)
- [→ Video](https://www.youtube.com/watch?v=uFFJyp8vXQI)
- [→ Slides](https://sorbet.run/talks/StrangeLoop2018/#/)

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

