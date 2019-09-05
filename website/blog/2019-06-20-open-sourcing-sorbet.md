---
id: open-sourcing-sorbet
title: Open-sourcing Sorbet: a fast, powerful type checker for Ruby
---

We're excited to announce that Sorbet is now [open source] and you can try it
today. Sorbet is a fast, powerful type checker designed for Ruby. It scales to
codebases with millions of lines of code and can be adopted incrementally.

[open source]: https://github.com/sorbet/sorbet

We designed Sorbet to be used at Stripe, where the vast majority of our code is
written in Ruby. We've spent the last year and a half developing and adopting
Sorbet internally, and we're finally confident that Sorbet is not just an
experimental, internal project—we're ready to share Sorbet with the entire Ruby
community. In fact, we've had more than 30 companies beta test Sorbet and
provide feedback.

<!--truncate-->

Today's release includes:

- The core static type checker
- Tooling to create new Sorbet projects
- Tooling to gradually adopt Sorbet in existing projects
- A runtime DSL for writing type annotations
- A [central repository] for sharing type definitions for Ruby gems

[central repository]: https://github.com/sorbet/sorbet-typed

... and much more. We're excited for you to play around with Sorbet, integrate
it into your codebases, and share your feedback. We've received immeasurable
value from the Ruby community already: this is our way of giving back.

## Getting started

We've put together a number of resources to help you get started with Sorbet:

- [Adopting Sorbet]: In just a few quick steps, get started using the Sorbet gem
  to type check an existing codebase.

- [Gradual Type Checking]: Sorbet is a gradual type checker. What does that
  mean, and how can we use it to our advantage?

- [Docs]: Check out the documentation to learn all about Sorbet's features.

[adopting sorbet]: https://sorbet.org/docs/adopting
[gradual type checking]: https://sorbet.org/docs/gradual
[docs]: https://sorbet.org/docs/overview

For a round-up of recent changes you can read [State of Sorbet for Spring 2019].
And of course, feel free to browse [the source on GitHub] to learn how to build
Sorbet, read the source code, report bugs, and contribute fixes!

[state of sorbet for spring 2019]:
  https://sorbet.org/blog/2019/05/16/state-of-sorbet-spring-2019
[the source on github]: https://github.com/sorbet/sorbet

## Community

We know that releasing an open-source project is the just the first step, and
we're excited to continue building Sorbet with our community. Stripe is
committed to growing the Sorbet project as a robust part of the Ruby ecosystem.
Here's how you can ask questions, report bugs, and share your experience
reports:

- [Ask us questions on Stack Overflow]: We'll be actively monitoring Stack
  Overflow questions with the `sorbet` tag, where you can ask questions you have
  while adopting Sorbet in your codebase.

- [Chat with us on Slack]: We've been using Slack to communicate with
  collaborators and beta testers; you can join our community to chat with other
  Sorbet users.

- [Report issues on GitHub]: Sorbet is still very young—if you find and can
  reproduce bugs in Sorbet, please share them!

[ask us questions on stack overflow]:
  https://stackoverflow.com/questions/tagged/sorbet
[chat with us on slack]: /slack
[report issues on github]: https://github.com/sorbet/sorbet/issues

## Acknowledgements

Sorbet is the product of a [large community of supporters], and we're deeply
appreciative for their work. You can see the complete list of contributors who
helped us in the process of open sourcing Sorbet. And of course, we'd also like
to thank the dozens of beta testers who braved the rough edges of Sorbet in its
early days. 🎉

[large community of supporters]:
  https://github.com/sorbet/sorbet/blob/master/ACKNOWLEDGEMENTS.md
