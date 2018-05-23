# Who are we?

---
# Outline
 1. Ruby at Stripe
 1. What did we want from a type system
 1. Examples
 1. Production experience

---

# Ruby at Stripe

- Ruby is the primary programming language at Stripe
- Stripe has a monorepo
- Mostly ~10 macroservices with a few microservices
- Monolith is growing faster than services

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

# Other Ruby typing

- github.com/plum-umd/rdl
- Typed Ruby from github
- Presentation tomorrow by @soutaro

---

# Is this open source?

- Yes! Eventually
- Prove it out internally first
Have questions? reach us at sorbet@stripe.com

---

## What did we want from a type system?
- explicitness
- compatible with ruby
- feels useful, instead of burdensome
- scales: in speed, team size, codebase size and time (not postponing hard decisions)

---

# Demo (usage)

---

## Usage: calls on stdlib(3)
```ruby
[1, 2, 3, 4].max + [1, 2, 3, 4].min
```


---

## Usage: union and intersection types(3)
TODO

---

## Usage: truthiness
TODO

---

## Usage: dead code

---

# Demo (declarations)

---

## Declaration: compatible syntax
this seems like the only mention of runtime type system

---

## Declaration: kinda manifest

---

## Declaration: nominal

---

## Declaration: non-nullable types.(2 min)

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

## When is this open source?

- When we are confident it works well internally
- Will post on stripe.com/blog

---
# Could you collaborate before it's open source?

Yes. Reach out to us via sorbet@stripe.com

---
# Let's collaborate on typing Ruby!

- there are multiple parties working to add types to ruby
- we'd love to chat and share, please email us sorbet@stripe.com

---

<img src="http://thecatapi.com/api/images/get?format=src&type=gif" width = 600>

