---
id: procs
title: Proc Types
---

> TODO: This page is still a fragment. Contributions welcome!

```
T.proc.params(arg0: Arg0Type, arg1: Arg2Type, ...).returns(ReturnType)
```

This is the type of a `Proc` (such as a block passed to a method as a `&blk`
parameter) that accepts arguments of types `Arg0Type`, `Arg1Type`, etc., and
returns `ReturnType`.

At present, all parameters are assumed to be required positional parameters. We
may add support for optional or keyword parameters in the future, if there is
demand.

Types of procs are not checked at all at runtime (the same way methods are), and
serve only as hints to `sorbet` statically (and for documentation).

Question: what do I put in my `params` if I'm using `yield` instead of
`blk.call`?

Answer: just add a new block parameter to your method definition. Ruby will
automatically `yield` to the thing named by the `blk` parameter.
