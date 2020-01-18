---
id: tracking_adoption
title: Measuring Type-checking Coverage
sidebar_label: Tracking Coverage
---

There are two primary metrics that we recommend to track the adoption at Stripe:

- File-level coverage: the number of files in each type
  [strictness level](static.md). There are 5 strictness levels
- Input send coverage: the number of method calls called on a typed object. Only
  reported in type-checked files (strictness level true or higher)

Together, they represent how much of a codebase is type-checked by sorbet.

```
$ srb tc --metrics-file test.txt --metrics-prefix example
  {
   "name": "example.types.input.files.sigil.false",
   "value": 1216
  },
  {
   "name": "example.types.input.files.sigil.true",
   "value": 1660
  },
  {
   "name": "example.types.input.files.sigil.strict",
   "value": 334
  },
  ...
  {
   "name": "example.types.input.sends.typed",
   "value": 69921
  },
  {
   "name": "example.types.input.sends.total",
   "value": 120061
  },
}
```

The metrics are important at different phases of adoption. During the ramp-up,
the first metric is more important. During this phase, we should focus on
enabling more files to be type-checked. This usually means resolving some syntax
errors or obvious type errors in `ignored` or `false` files. Usually, this
process will help to identify and fix a lot of issues that make your code hard
to type-check.

Once the majority of files are typed `true` or higher, it becomes important to
drive up the second metric, input sends typed. In `true` or `strict` files,
there can be objects or methods that are not typed. This metric tells us the
amount of untyped code in those files. Usually, it means we need to have more
type signatures to tell Sorbet the type of objects in the codebase. In the
transition phase, it's effective to reduce this number by writing the type
signature for methods in the core abstractions.

Based on our experience, it is important to keep up the adoption progress by
requiring that new code is type-checked. For example, new files added should be
typed `true`, or new methods added should have signatures.
[Rubocop-sorbet](https://github.com/Shopify/rubocop-sorbet) provides a few rules
for that.
