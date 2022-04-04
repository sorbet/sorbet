---
id: state-of-sorbet-spring-2019
title: State of Sorbet Spring 2019
author: James Iry
authorURL: https://twitter.com/jamesiry
---

Stripe uses Ruby extensively[^languages]. It's the main language we use to build
the business logic behind our APIs, and our Ruby codebase is on the order of
millions of lines of code. At that scale and with our expected rapid growth rate
two things are true: 1) we need all the tooling help we can get to understand
and modify that much code, and 2) a total rewrite in a statically typed language
would be a massive undertaking.

<!-- prettier-ignore-start -->

[^languages]: We also use plenty of of other languages including Go for
infrastructure tasks, Scala for data wrangling, and JavaScript for client-side
work.

<!-- prettier-ignore-end -->

With that in mind, in October 2017 a small team of engineers conceived of
building Sorbet, a gradual static type system for Ruby. Static type systems look
for certain classes of potential errors without running your code. A gradual
static type system allows you to gradually add static typing, leaving some parts
of your code purely dynamically typed. Other examples of gradual static type
systems that have been added onto existing dynamically typed languages are
[Flow] and [TypeScript] for Javascript, and [Hack] for PHP. In this post, we'd
like to give a brief update about what we've been working on, and where we're
going next.

[flow]: https://flow.org/
[typescript]: https://www.typescriptlang.org/
[hack]: https://hacklang.org/

<!--truncate-->

Sorbet has come a long way since its original design, thanks in no small part to
some very helpful collaborators. We now run Sorbet's type checking as a part of
every build in Stripe's Ruby codebase, and our developers have come to rely on
its feedback. Having proven out Sorbet's value internally, we're now getting
ready to share it with the rest of the Ruby community as open source. We've also
begun beta testing an editor integration which you can play with at [sorbet.run]
(if you're on a desktop browser[^mobile]). You can read more about how Sorbet
works in our [documentation](/docs/overview).

<!-- prettier-ignore-start -->

[^mobile]: [sorbet.run] also works on mobile devices but does not offer all the
code navigation and exploration features of the desktop version.

<!-- prettier-ignore-end -->

## Where Sorbet is now

It took about eight months to build and test Sorbet by deeply typing a handful
of isolated components. We focused equally on building Sorbet and using it to
type-check real code. After finalizing the basics, we spent seven months
incorporating it across our entire main Ruby repository. All that work has
resulted in real impact for Stripe.

Today, every CI build of the main repository is checked by Sorbet. In 100% of
our production Ruby files, we catch problems with missing constants such as
class names:

```ruby
class Hello
end
def main
  Helo.new
end
```

```plaintext
editor.rb:6: Unable to resolve constant `Helo`
     6 |  Helo.new
          ^^^^
```

In 82% of our production Ruby files, we prevent calls to methods that don't
exist and discover situations where a method is called with too many or too few
parameters:

```ruby
# typed: true
class Hello
  def greeting
    'Hello, world'
  end
end
def main
  Hello.new.greet
end
```

```plaintext
editor.rb:10: Method `greet` does not exist on `Hello`
    10 |  Hello.new.greet
          ^^^^^^^^^^^^^^^
```

And 63% of call sites[^call-site] are calling methods with type signatures
allowing us to find mismatches in the types of parameters or return values:

```ruby
# typed: true
class Hello
  extend T::Sig

  sig {params(name: String).void}
  def greeting(name)
    'Hello, #{name}'
  end
end
def main
  Hello.new.greeting(:foo)
end
```

```plaintext
editor.rb:12: Symbol(:"foo") doesn't match String for argument name
    12 |  Hello.new.greeting(:foo)
          ^^^^^^^^^^^^^^^^^^^^^^^^
```

<!-- prettier-ignore-start -->

[^call-site]: A call site is just a single location in a codebase that will
result in a call to function or method when run. We don't use the term _method
calls_ because that can also mean a call at runtime. For example, `foo.bar`
might exist at only one call site in your code, but if it's in a loop it might
result in 0, 1, or `n` method calls at runtime.

<!-- prettier-ignore-end -->

There's more integration work to do and features to build, but so far work has
already paid off in the number bugs prevented. The feedback from Stripe
developers has already been largely positive:

![Testimonial from Sorbet user](/img/testimonial_once_every_never.png)

![Testimonial from Sorbet user](/img/testimonial_pair_programming.png)

## Where Sorbet is going

### Open sourcing Sorbet

We expect to open source Sorbet this summer. We're working with a small set of
early users and contributors (including teams at Coinbase and Shopify) and
gathering feedback. We've been productionizing Sorbet and building tools to
automate some of the lessons we learned as we rolled it out to Stripe's
codebase. We've also published our documentation and offered a beta program
signup to get early feedback at [sorbet.org](https://sorbet.org).

A natural question might be _why not open source now?_ Given the strong feelings
that people have around static type systems and the effort required to integrate
static types, we don't want rough edges in developer experience to accidentally
drive people away from the idea entirely. Developer experience is as crucial to
a type system's acceptance as its design and we want to make Sorbet's experience
great by investing in tooling and debugging before its first release.

On the tooling front we've packaged our binaries into gems:

```ruby
# Gemfile
gem 'sorbet', :group => :development
gem 'sorbet-runtime'
```

The `sorbet` gem holds the `srb` executable, while the `sorbet-runtime` gem
holds `sig` and other Ruby code.

`srb init` will prepare an existing project for Sorbet by creating a `sorbet`
subdirectory with metadata about your project:

```plaintext
❯ srb init
...
```

```plaintext
sorbet/
│ # Default options to passed to sorbet on every run
├── config
└── rbi/
    │ # Community-written type definition files for your gems
    ├── sorbet-typed/
    │ # Autogenerated type definitions for your gems
    ├── gems/
    │ # Things defined when run, but hidden statically
    ├── hidden-definitions/
    │ # Constants which were still missing
    └── todo.rbi
```

The `srb` command with no arguments runs the type checker over your project. For
example, as part of a build script:

```plaintext
❯ srb
No errors! Great job
```

For more details see the [CLI documentation](https://sorbet.org/docs/cli).

Shopify has contributed the ability to write Ruby plugins that give type
signatures for DSLs based on metaprogramming. For example, with the right plugin
something like:

```ruby
attribute my_id, :integer
```

…can conceptually expand into something that Sorbet sees as a typed interface
like:

```ruby
sig {returns(T.nilable(Integer))}
def my_id; end

sig {returns(T::Boolean)}
def my_id?; end

sig {params(new_value: T.nilable(Integer)).void}
def my_id=(new_value); end
```

### Supporting the existing Ruby ecosystem

Sorbet will support all of the [types that Matz announced] for the Ruby 3
release. In fact, we're part of the working group (with Matz and the Ruby core
team) that is collaborating on the goals and design of Ruby 3 types, and we've
been sharing everything we've learned from Sorbet.

[types that matz announced]:
  https://www.youtube.com/watch?v=cmOt9HhszCI&feature=youtu.be&t=2148

Coinbase has built [sorbet-typed], a central repository for sharing type
definitions for existing Ruby gems. This enables types for the existing
ecosystem of gems we all rely on.

The single most common dependency for Ruby projects is Rails. Although Stripe
doesn't use Rails, we know it's a staple in the Ruby community and users need
type definitions for the framework to adopt Sorbet. To that end, we've created
the [sorbet-typed] gems for Rails.

A new sample Rails project:

```plaintext
❯ rails new blog
...
❯ srb init
...
❯ git grep -h typed: | sort | uniq -c
    2 # typed: false
  120 # typed: true
```

RubyGems.org:

```plaintext
❯ srb init
...
❯ git grep -h typed: | sort | uniq -c
  189 # typed: false
  265 # typed: true
```

GitLab:

```plaintext
❯ srb init
...
❯ git grep -h typed: | sort | uniq -c
    47 # typed: ignore
  6516 # typed: false
  1579 # typed: true
```

It's worth mentioning that even with `# typed: false` Sorbet will still find
invalid constants such as misspelled class names. Only the small number of
`# typed: ignore` files are getting nothing from Sorbet.

### Editor integration

We've built a beta integration with VS Code, which you can use in a desktop
browser[^mobile] at [sorbet.run]. As we stabilize our editor integration we plan
to roll it out to Stripe's engineering team and eventually open source it.
Here's a quick walkthrough of some of its features.

Error squiggles details behind the error on hover:

![Editor integration screenshot](/img/editor_error_squiggles.gif)

Go to definition uses Sorbet's type information and is more accurate than simply
relying on strings:

![Editor integration screenshot](/img/editor_go_to_definition.gif)

Autocomplete and inline documentation:

![Editor integration screenshot](/img/editor_autocomplete.gif)

Sorbet's editor integration uses Microsoft's Language Server Protocol, which
means other editors and tools will also be able to integrate with it. For
example, Sourcegraph has a prototype of a browser plugin that allows developers
to get rich information within GitHub. Here's what it looks like when integrated
with Sorbet:

![Sorbet Sourcegraph integration](/img/sourcegraph_github.gif)

## Collaborators

We're indebted to several people and groups who have helped us get where we are:

- Jeff Foster at Tufts University and his students have been working on RDL. We
  based our initial type annotations for the Ruby standard library on theirs.
- Our initial Ruby parser was from [@haileys] at GitHub.
- Coinbase has contributed code to Sorbet and created [sorbet-typed], a central
  repository for sharing type definitions for Ruby gems.
- Shopify has had several contributions including a mechanism for adding types
  to DSLs that result from metaprogramming.
- Sourcegraph has built a code exploration browser plugin for GitHub and used
  Sorbet as one of their integrations.

In addition to code and tool contributions we've had many fruitful conversations
that helped shape our thinking. Our thanks go out to [@soutaro], author of
Steep; [@mame], who is working on a Type profiler; and of course our favorite
BDFL [@matz].

[@haileys]: https://github.com/haileys
[@soutaro]: https://github.com/soutaro
[@mame]: https://github.com/mame
[@matz]: https://github.com/matz

## Wrapping up

Sorbet has matured a lot since its first conception and a host of fantastic
contributors and collaborators have helped significantly. We've proven Sorbet's
usefulness within Stripe. With just a bit more developer experience polish we'll
be ready to share it with the rest of the Ruby community as open source. With
ongoing work to integrate with editors and other tools we expect the benefits of
Sorbet to become even greater.

Please take a moment to explore our documentation at [sorbet.org] and play with
Sorbet at [sorbet.run].

James Iry [@jamesiry](https://twitter.com/jamesiry) on behalf of the Sorbet team
(current and former):

- Dmitry Petrashko ([@darkdimius](https://twitter.com/darkdimius))
- Paul Tarjan ([@ptarjan](https://twitter.com/ptarjan))
- John Vilk ([@jvilk](https://github.com/jvilk))
- Jake Zimmerman ([@jez](https://github.com/jez))
- Neil Parikh ([@neilparikh](https://github.com/neilparikh))
- Russell Davis ([@\_russelldavis](https://twitter.com/_russelldavis))
- Nelson Elhage ([@nelhage](https://twitter.com/nelhage))

[sorbet-typed]: https://github.com/sorbet/sorbet-typed
[sorbet.org]: https://sorbet.org
[sorbet.run]: https://sorbet.run
