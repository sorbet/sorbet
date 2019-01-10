---
id: overview
title: Overview
---

Type checking with Sorbet is composed of two key components:

- `sorbet`

  This is the command-line interface to Sorbet's type checker. It analyzes your
  code **statically** (before your code runs) to report potential mistakes in
  the code.

- `sorbet-runtime`

  This is the gem that enables adding type annotations to normal Ruby code. It
  exposes the top-level `T` namespace and the `sig` method, which we'll see more
  of in [Signatures](sigs.md). It also **dynamically** type checks the code
  while it runs.

These two components are developed in tandem, and in fact compound each others'
guarantees:

- When we write more runtime annotations, it gives more information to Sorbet.

- When `sorbet` gets more information, it can emit more errors before that code
  breaks in production.

- The runtime backs up assertions that Sorbet makes, so untyped code can't break
  a static claim made by Sorbet.

## Example

Here's a taste of what Sorbet can do:

```ruby
# -- foo.rb --

class A
  extend T::Helpers
  sig {params(foo: Integer).returns(String)}
  def bar(foo)
    foo.to_s
  end
end

def main
  A.new.barr(91)
  A.new.bar("91")
end
```

[→ View on sorbet.run](#TODO-jez)

This outputs:

```text
❯ sorbet foo.rb
foo.rb:12: Method barr does not exist on A
    10 |  A.new.barr(91)
          ^^^^^^^^^^^^^^
    foo.rb:6: Did you mean: A#bar?
     4 |  def bar(foo)
          ^^^^^^^^^^^^ 

foo.rb:13: String("91") doesn't match Integer for argument foo
    11 |  A.new.bar("91")
          ^^^^^^^^^^^^^^^ 
    <ruby>:5: Method A#bar has specified foo as Integer
     3 |  sig {params(foo: Integer).returns(String)}
                      ^^^^   Got String("91") originating from:
    <ruby>:13:
    11 |  A.new.bar("91")
                    ^^^^
```


