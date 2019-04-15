---
id: ruby-kaigi-2018
title: "A practical type system for Ruby at Stripe"
sidebar_label: "RubyKaigi 2018"
---

- [→ Talk page](https://rubykaigi.org/2018/presentations/DarkDimius.html#may31)
- [→ Video](https://www.youtube.com/watch?v=eCnnBS2LXcI)
- [→ Slides](https://sorbet.run/talks/RubyKaigi2018/#/)

## Abstract

At Stripe, we believe that a typesystem provides substantial benefits for a big
codebase. Types:

- are documentation that is always kept up-to-date;
- speed up the development loop via faster feedback from tooling;
- help discover corner cases that are not handled by the happy path;
- allow building tools that expose knowledge obtained through type-checking,
  such as "jump to definition."

We have built a type system that is currently being adopted by our Ruby code at
Stripe. This type system can be adopted gradually with different teams and
projects adopting it at a different pace. We support "and" and "or" types as
well as basic generics. Our type syntax is backwards compatible with untyped
ruby.

In this talk we describe our experience in developing and adopting a type system
for our multi-million line ruby codebase. We will also discuss what future tools
are made possible by having knowledge about types in the code base.
