---
id: overview
title: Overview
---

Type checking with Sorbet is composed of two key components:

- `srb`

  This is the command-line interface to Sorbet. It includes the core type
  checker, which analyzes a project **statically** (before the code runs) to
  report potential mistakes in the code. It also contains utilities to set up a
  project to work with Sorbet for the first time.

- `sorbet-runtime`

  This is the gem that enables adding type annotations to normal Ruby code. It
  exposes the top-level `T` namespace and the `sig` method, which we'll see more
  of in [Signatures](sigs.md). It also **dynamically** type checks the code
  while it runs.

These two components are developed in tandem, and in fact compound each others'
guarantees. Sorbet makes predictions about the runtime, and the runtime enforces
those predictions with contracts.

Here's a taste of what Sorbet can do:

```ruby
# typed: true
require 'sorbet-runtime'

class A
  extend T::Sig

  sig {params(x: Integer).returns(String)}
  def bar(x)
    x.to_s
  end
end

def main
  A.new.barr(91)   # error: Typo!
  A.new.bar("91")  # error: Type mismatch!
end
```

<a href="https://sorbet.run/#%23%20typed%3A%20true%0Arequire%20'sorbet-runtime'%0A%0Aclass%20A%0A%20%20extend%20T%3A%3ASig%0A%0A%20%20sig%20%7Bparams%28x%3A%20Integer%29.returns%28String%29%7D%0A%20%20def%20bar%28x%29%0A%20%20%20%20x.to_s%0A%20%20end%0Aend%0A%0Adef%20main%0A%20%20A.new.barr%2891%29%20%20%20%23%20error%3A%20Typo!%0A%20%20A.new.bar%28%2291%22%29%20%20%23%20error%3A%20Type%20mismatch!%0Aend">
  → View on sorbet.run
</a>

## What's next?

- [Adopting Sorbet](adopting.md)

  Learn how to adopt Sorbet in an existing codebase.

- [Gradual Type Checking and Sorbet](gradual.md)

  Learn about how Sorbet works, and how it's different from other type systems
  you might be familiar with.

- [Enabling Static Checks](static.md)

  Learn how to get **more power** out of Sorbet by enabling static checks.

- [Enabling Runtime Checks](runtime.md)

  Learn how to get **more confidence** out of Sorbet by enabling runtime checks.
