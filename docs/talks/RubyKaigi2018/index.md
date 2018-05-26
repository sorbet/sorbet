# Introductions

Note:

pt starts talking here

---

# Background

---

## Ruby at Stripe

- Ruby is the primary programming language
- Most product code is in a monorepo (intentionally!)
- ~10 macroservices with a few microservices
- New code mostly goes into an existing service

---

## Ruby at Stripe: Stats

|Language                        | lines |||Language                        | lines |
|--------------------------------|------:|||--------------------------------|------:|
|Ruby                            |   34% |||Scala                           |    7% |
|Javascript                      |   16% |||HTML                            |    6% |
|YAML                            |   10% |||Go                              |    6% |

Millions of lines of code

Hundreds of engineers

Thousands of commits per day

Note: lines&files taken from https://hackpad.corp.stripe.com/Code-Metrics-R6KrBcukuSG
engineers taken from https://docs.google.com/spreadsheets/d/1m4EeSEQHBVsSjRuSj4SH7vvXbOMWKsCHUpmuIWQupFI/edit#gid=173620479 ,
commits per day from https://redshift.northwest.corp.stripe.com/queries/dmitry/ab68760e/table

---

## Other Ruby Typing

- Diamondback Ruby
- github.com/plum-umd/rdl
- Unreleased GitHub experiment
- Presentation tomorrow by @soutaro

---

## Open Source?

- Yes! Eventually
- Prove it out internally first
- Have questions? Reach us at **sorbet@stripe.com**

Note:

hand off to nelhage after this slide

---

## Type System Design Principles

- Explicit
- Compatible with Ruby
- As simple as possible, but powerful enough
- Scales

Note:

- Explicit

  We're willing to write annotations, and in fact see them as
  beneficial; They make code more readable and predictable. We're here
  to help readers as much as writers.

- Compatible with Ruby

  In particular, we don't want new syntax. Existing Ruby syntax means
  we can leverage most of our existing tooling (editors, etc). Also,
  the whole point here is to improve an existing Ruby codebase, so we
  should be able to adopt it incrementally.

- As simple as possible, but powerful enough

  Overall, we are not strong believers in super-complex type
  systems. They have their place, and we need a fair emount of
  expressive power to model (enough) real Ruby code, but all else
  being equal we want to be simpler. We believe that such a system
  scales better, and -- most importantly -- is easiest for users to
  learn+understand.

- Scales

  On all axes: in speed, team size, codebase size and time (not
  postponing hard decisions). We're already a large codebase, and will
  only get larger.

---

# Demo (usage)

---

## Usage: Calls into stdlib

```ruby
# Integer
([1, 2].count + 3).next
```

<hr/>

```ruby
"str" + :sym
```

```console
Expression passed as an argument `arg0` to method `+`
      does not match expected type
      github.com/stripe/sorbet/wiki/7002
     1 |"str" + :sym
        ^^^^^^^^^^^^
    github.com/stripe/sorbet/tree/master/rbi/core/string.rbi#L18:
    Method `+` has specified type of argument `arg0` as `String`
    18 |      arg0: String,
              ^^^^^
  Got Symbol(:"sym") originating from:
    -e:1:
     1 |"str" + :sym
                ^^^^
```

---

## Usage: truthiness

```ruby
foo = array_of_strings[0]
# foo is a T.nilable(String) now
return true if foo.nil?
# foo is a String now
return foo.empty?
```

<hr/>

```ruby
foo = array_of_strings[0]
return foo.empty?
```

```console
Method `empty?` does not exist on `NilClass`
component of `T.nilable(String)`
     5 |return foo.empty?
               ^^^^^^^^^^
```

---

## Usage: dead code

```ruby
if Random.rand > 0.5
    foo = 1
else
    foo = 2
end
# foo is an Integer
```

<hr/>

```ruby
if Random.rand
    foo = 1
else
    foo = 2
end
```

```console
This code is unreachable
     6 |    foo = 2
                  ^
```

---

## Usage: union types

```ruby
foo = ["1", 2]
hash = foo.map(&:succ) # Shared methods work
```

<hr/>

```ruby
foo = ["1", 2, [3]]
hash = foo.map(&:succ)
```

```console
Method `succ` does not exist on `Array` component of
`T.any(String, Integer, T::Array[Integer])`
     4 |hash = foo.map(&:succ)
                        ^^^^^
```

---

# Declaration Syntax

---

## Declaration: Compatible syntax

```ruby
extend T::Helpers
...

sig(
    # Both positional and named parameters are referred to by name.
    # You must declare all parameters (and the return value below).
    foobar: <Type>,
    widget: <Type>
)
.returns(<Type>)
def foo(foobar, widget: nil)
    ...
end
```

---

## Declaration: Runtime Typesystem

```ruby
sig.returns(String)
def foo
    :foo
end
```

```console
Return value: Expected type String, got type Symbol
```

Note: We built this first and it works independently of the static typechecker

---

## Declaration: Local Inference

```ruby
sig.returns(String) # Optional but not inferred
def foo
    a = 5 # Integer
    a = T.let("str", String) # String
end
```

---

## Declaration: Interfaces


```ruby
module Fooable
  sig.abstract; def foo; end
end

class MyFooable
  include Fooable

  def foo; ...; end
end
```

Note:

tbd? explain not structural?

---

## Declaration: non-nullable types


```ruby
T::Array[String].new[0] # This is T.nilable(String)
```

<hr/>

```ruby
sig(id: Integer).returns(T.nilable(MyORM))
def load(id); ... ; end

load(123).name
```

```console
Method `name` does not exist on `NilClass`
component of `T.nilable(MyORM)`
```

Note:

dmitry takes over after this slide

---
## Declaration: Generic classes


```ruby
class Box
  extend T::Generic

  Elem = type_member

  sig.returns(Elem)
  attr_reader :x

  sig(x: Elem).returns(Elem)
  attr_writer :x
end

int_box = Box[Integer].new
```

---
## Declaration: Generic methods

```ruby
class Array
  Elem = type_member

  type_parameters(:U).sig(
      blk: T.proc(arg0: Elem).returns(T.type_parameter(:U)),
  )
  .returns(T::Array[T.type_parameter(:U)])
  sig.returns(T::Array[Elem])
  def map(&blk); end
end
```

---
# Practical experience

---

## Organic penetration

TODO: stats

- number of human-authored signatures: 3k
- number of generated signatures: 240k

---

## Percent of type-able files and sites

---

## Bugs found in production code

- `nil` checks
- errors in dead code
- error-handling
- incorrect declarations
- ???

---
## User response

> “DeveloperHappiness” would be a good name for ruby-typer
> <!-- .element: class="smallquote"  -->

<span> </span>

> nice!! beautiful errors and damn that was fast!!
> <!-- .element: class="smallquote"  -->

<span> </span>

> I'm trying to use it locally, it's been super helpful to catch bugs in my own code
> <!-- .element: class="smallquote"  -->

<span> </span>

> I introduced a typo with sed and ruby-typer caught it in CI within seconds of me pushing the branch.
> It’s really nice to get this kind of notification quickly rather than having to wait for potentially several minutes before the test job fails.
> <!-- .element: class="smallquote"  -->


---

## Speed of our typer

175k lines/second/cpu core

---

## Implementation

- C++
- Don't depend on a Ruby VM
- C++ port of `whitequark/parser` by GitHub
- Extensive test suite
- CI runs against Stripe codebase

---
## Metaprogramming support

- Minimal native support
- Reflection-based signature generation

Note:

Hand off to pt after this slide

---

# Can I use it?

---

## Open Source

- Will open source, timeline TBD
- Focusing on internal deployment for now
- Will post on stripe.com/blog

---

## Using it


- Please reach out, even before we open source <!-- .element: class="fragment" data-fragment-index="1" -->
- Interested in your use cases <!-- .element: class="fragment" data-fragment-index="1" -->
- Will let you know when it's ready for beta <!-- .element: class="fragment" data-fragment-index="1" -->
- sorbet@stripe.com <!-- .element: class="fragment" data-fragment-index="1" -->

---

## Building it

- There are multiple parties working to add types to Ruby <!-- .element: class="fragment" data-fragment-index="1" -->
- We'd love to chat and share <!-- .element: class="fragment" data-fragment-index="1" -->
- sorbet@stripe.com <!-- .element: class="fragment" data-fragment-index="1" -->

---

# Thank you!
