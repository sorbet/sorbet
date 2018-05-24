# Introductions

---

# Outline
 1. Ruby at Stripe
 1. Our Type System Design Principles
 1. Examples
 1. Production experience

---

# Ruby at Stripe

- Ruby is the primary programming language
- Most product code is in a monorepo (intentionally!)
- ~10 macroservices with a few microservices
- New code mostly goes into an existing service

---

## Ruby at Stripe: Stats

|Language                        | files | lines |
|--------------------------------|------:|------:|
|Ruby                            | 25k   | 2,300k|
|Javascript                      |  8k   | 1,100k|
|YAML                            |  3k   |   700k|
|Scala                           |  5k   |   500k|
|HTML                            |  1k   |   400k|
|Go                              |  3k   |   400k|

engineers: ~400

commits per day: ~1800

Note: lines&files taken from https://hackpad.corp.stripe.com/Code-Metrics-R6KrBcukuSG
engineers taken from https://docs.google.com/spreadsheets/d/1m4EeSEQHBVsSjRuSj4SH7vvXbOMWKsCHUpmuIWQupFI/edit#gid=173620479 ,
commits per day from https://redshift.northwest.corp.stripe.com/queries/dmitry/ab68760e/table

----

# Other Ruby Typing

- github.com/plum-umd/rdl
- Typed Ruby from github
- Presentation tomorrow by @soutaro

---

# Open Source?

- Yes! Eventually
- Prove it out internally first
- Have questions? Reach us at **sorbet@stripe.com**

---

# Type System Design Principles
- Explicit
- Compatible with Ruby
- Feels useful, instead of burdensome
- Scales: in speed, team size, codebase size and time (not postponing hard decisions)

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
$ sorbet -e '"str" + :sym'
-e:1: Expression passed as an argument `arg0` to method `+` 
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
/tmp/a.rb:6: This code is unreachable
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

# Demo (declarations)

---

## Declaration: compatible syntax

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

## Declaration: runtime typesystem

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

## Declaration: kinda manifest

```ruby
sig.returns(String) # Optional but not inferred
def foo
    a = 5 # Integer
    a = T.let("str", String) # String
end
```

---

## Declaration: nominal

```ruby
class A; end
class B; end
class C; end
class D; end

sig(a: A, b: T::Array[B]).returns(T.any(C, D))
```

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

---
# Practical experience

---

## Organic penetration

---

## Percent of type-able files and sites

---

## Speed of our typer

---

## illustrations of bugs found by it(3 min)

---

# Collaboration

---

## Collaboration: Open Source

- Will open source when it is ready
- Perfect it internally before pushing out
- Will post on stripe.com/blog

---

## Collaboration: Using it

- Please reach out, even before we open source
- Would love beta testers
- sorbet@stripe.com

---

## Collaboration: Building it

- There are multiple parties working to add types to Ruby
- We'd love to chat and share
- sorbet@stripe.com

---

# Thank you!
