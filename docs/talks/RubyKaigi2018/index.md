# Introductions

Note:
- dmitry: PhD Compiler architecture & a bit of type theory @ next major version of Scala Compiler(3.0)
- nelhage: MIT grad, One of the longest tenured engineers at Stripe
- pt: Stanford grad, Previously at Facebook on HHVM and Hack

pt starts talking here

---

# Background

---

## Stripe

- Payments processor++
- 23 countries, 100,000 business worldwide
- 60% of people in US have used it in the last year
- If you'd like to take money, look at us
- Have a Tokyo office
  - Many in the audience
  - Come chat!
  - stripe.com/jobs

---

## Developer productivity

- Large dedicated team
- Testing
- Code
- Dev Env
- Abstractions
- Language Tools

---

## Ruby at Stripe

- Ruby is the primary programming language
    - No Rails
    - Strict subset of Ruby (thanks @bbatsov for Rubocop)
- Most product code is in a monorepo (intentionally!)
- ~10 macroservices with a few microservices
- New code mostly goes into an existing service

---

## Stats

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

## Other ruby typing

- Diamondback Ruby
- github.com/plum-umd/rdl
- Unreleased GitHub experiment
- Presentation tomorrow by @soutaro

Note:
- RDL from Jeff Foster and his group at the Maryland goes by many names.

---

## Open source?

- Yes! Eventually
- Prove it out internally first
- Have questions? Reach us at **sorbet@stripe.com**

Note:

- Yes, we very much would like to open source this. We're announcing today to
gather feedback on the approach and find folks who are interested in
collaborating. After we deploy it at Stripe and work out the kinks we will open
source it for everyone else to try.

- But wait, this isn't just a vaporware announcement :) Thanks to the fabulous
emscripten, we have a browser demo.

And with that brief teaser, let me hand off to Dmitry to dive into our type
system.

https://sorbet.run/#%221%22%20%2B%202

---

## Try it &#8681;

http://sorbet.run

<img src="img/QR_sorbet_run.png"></img>

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
    amount: Integer,
    currency: String,
)
.returns(Stripe::Charge)
def create_charge(amount, currency)
    ...
end
```

---

## Declaration: Runtime Typesystem

```ruby
sig(amount: Integer, currency: String)
.returns(Stripe::Charge)
def create_charge(amount, currency)
    ...
end

create_charge(10_000, :jpy)
```

```console
#<TypeError: Parameter currency:
  Expected type String, got type Symbol>
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

Note:

 Our typesystem also has minimal number of features to model Ruby code.
 One of such features is generics.
 In this example, we model a box that can store an element of a specific type.
 Box declaration does not know what it will store.
 I will be specified by use site.

 The most common use of this is to model various containers: arrays, sets, hashes

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

Note:

  We can also model methods with complex signatures such as Array map
  that require generic methods.

---
# Practical experience

---

## Organic penetration

- number of human-authored signatures: 3k over last 2 months internal alpha mode
- number of generated signatures: 240k

---

# Bugs found in production code

---

## `nil` checks

```ruby
-  endpoint = getEndpoint(user_provided_endpoint_id)
+  maybe_endpoint = getEndpoint(user_provided_endpoint_id)
+
+  if maybe_endpoint.nil?
+    raise UserError.new(:webhook_endpoint_not_found)
+  end
+  endpoint = maybe_endpoint
```

---

## Incorrect syntax use(simplified)
```
    case var
        ...
 -      when Case1 || Case2
 +      when Case1 , Case2
        ...
    end
```

---

## Incorrect declarations(simplified)
```
    class Super
      # @return [String]
      def id
        raise NotImplementedError.new("id wasn't implemented");
      end
    end

    class Child
      def id
 -      :my_id
 +      "my_id"
      end
    end
```

---

## Errors in error handling

 ```ruby
     begin
       data = JSON.parse(File.read(path))
 -   rescue JSON::ParseError => e
 +   rescue JSON::ParserError => e
       raise "#{PACKAGE_REL_PATH} contains invalid JSON: #{e}"
     end
 ```

---

## Errors in error handling

```ruby
   if look_ahead_days < 1 || look_ahead_days > 30
-    raise ArgumentError('look_ahead_days must be between 1 and 30, inclusive')
+    raise ArgumentError.new('look_ahead_days must be between 1 and 30, inclusive')
   end
```

---

## Errors in dead code(simplified)
```ruby
-   soft_assert_failed(
+   soft_assertion_failed(
      **args
    )
```

---

## Typos(simplified)

```
simplified/pattern.rb:41: Method to_I does not exist on Integer http://go/e/5002
    31 |        case "1245".to_I
                            ^^^^
ruby-typer/stdlib/integer.rbi:1: Did you mean `to_i`?
    562|        def to_i
                ^^^^^^^^
```

```
simplified/model/data.rb:41: Unable to resolve constant `Foat` http://go/e/5002
    41 |        orm_entry :data, Foat
                                 ^^^^
ruby-typer/stdlib/float.rbi:1: Did you mean: `::Float`?
     1 |class Float
        ^^^^^^^^^^^
```

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

100k lines/second/cpu core

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

## Open source

- Sneak peak: http://sorbet.run
- Will open source, timeline TBD
- Focusing on internal deployment for now
- Will post on stripe.com/blog

Note:

Try out the browser demo. The core typechecker is done, most of the work for us
is around how to roll it out to a big codebase. How do you deal with
metaprogramming, or unannotated gems, or editor integrations. Please play with
it and give us feedback.

Keep an eye on the Stripe blog, we'll announce it there.

---

## Using it

- Interested in your use cases
- Will let you know when it's ready for beta
- sorbet@stripe.com

Note:



---

## Building it

- There are multiple parties working to add types to Ruby
- We'd love to chat and share
- sorbet@stripe.com

Note:

Let me implore you, if you are working on typechecking Ruby or scaling Ruby we
would love to chat. Please email us or find us somehow. We've put a lot of
effort into this space. I've been leading the dev productivity team for 2.5
years now and we've focussed largely on how to scale our Ruby for the needs of
Stripe, so we have some tools to share. Check out Andrews great talk about our
autoloader we built last year. Having said that, we also really want to use your
tools and systems too!

---

# Thank you!

sorbet@stripe.com

Note: 

With that, thank you so much for listening to us, thank you to the conference
organizers for letting us speak here today. I hope we can share our typechecker
with you soon and would love to hear your feedback.
