---
id: sealed
title: Sealed Classes and Modules
sidebar_label: Sealed Classes
---

Sorbet supports sealed classes. Sealed classes restrict who's allowed to
subclass a class. This is a considerably advanced feature, but it's also
considerably powerful. That being said, if you're already familiar with the
concept, here's what the syntax looks like in Sorbet:

```ruby
# -- foo.rb --
module Parent
  extend T::Helpers
  # (1) Mark module as sealed
  sealed!
end

# (2) CAN be included/extended/inherited in `foo.rb`
class Child1; include Parent; end
class Child2; include Parent; end

sig {params(x: Parent).void}
def foo(x)
  case x
  when Child1 then # ...
  when Child2 then # ...
  # (3) Sealed classes support exhaustiveness
  else T.absurd(x)
  end
end


# -- bar.rb --
# (4) CANNOT be included/extended/inherited in other files
class BadChild; include Parent; end # error!
```

## What's the point of sealed classes?

Sealed classes (and modules) let us declare "there are only these subclasses of
this class." Consider code that looks like this:

```ruby
class NotNeeded; ...; end
class Paid;      ...; end
class Failed;    ...; end

ChargeAttemptResult = T.type_alias {T.any(NotNeeded, Paid, Failed)}
```

We might be annoyed that the definitions for `NotNeeded`, `Paid`, and `Failed`
are not intrinsically connected to each other... It's only after defining them
that we've linked them with an ad-hoc type alias, declaring a
`ChargeAttemptResult` as the union of those three classes.

We could use a parent class or module (or even an [interface](abstract.md)) to
relate these classes to each other:

```ruby
module ChargeAttemptResult; end

class NotNeeded; include ChargeAttemptResult; end
class Paid;      include ChargeAttemptResult; end
class Failed;    include ChargeAttemptResult; end
```

But this would mean we lose out on [exhaustiveness](exhaustiveness.md): there's
no way for Sorbet to know that `ChargeAttemptResult` is only included in these
classes.

But that's precisely what sealed classes let us declare to Sorbet:

```ruby
module ChargeAttemptResult
  extend T::Helpers
  sealed!     # ← Adding `sealed!` here
end

# ... same classes from before ...
```

Now Sorbet will do two things:

- It will only allow `ChargeAttemptResult` to be included into classes defined
  in this file.
- It will will feed information into Sorbet's
  [control flow-sensitive typing](flow-sensitive.md) to check that all
  subclasses are handled exhaustively.

Specifically, here's what it would look like to `case` over this sealed class:

```ruby
T.reveal_type(x)                     # type:  ChargeAttemptResult
case x
when NotNeeded then T.reveal_type(x) # type:  NotNeeded
when Paid      then T.reveal_type(x) # type:  Paid
else T.absurd(x)                     # error: Missing case for `Failed`
end
```

As an extra convenience, a sealed class or module will also have a method called
`sealed_subclasses` that returns the complete set of sealed subclasses. This can
be used to dynamically iterate over all the subclasses:

```ruby
ChargeAttemptResult.sealed_subclasses.each do |klass|
  puts "Charge attempt can be #{klass.name}"
end
```

Sealed classes are a powerful feature. They let us declare intentional,
definition-side interfaces, without sacrificing flow-sensitive typing or
exhaustiveness.

## Sealed classes vs sealed modules

Throughout this doc we've been talking about sealed classes and sealed modules
as if they were interchangeable. But they're actually different in one key way:

- Sealed modules only require being declared `sealed!`
- Sealed classes require being declared both `sealed!` and `abstract!`

Modules can't be instantiated, but classes can. So in order to ensure that
something exhaustively covers all the subclasses and can ignore handling the
parent class, it must be marked abstract so that Sorbet knows a value of that
type could never need to be created in the first place.

To re-iterate:

```ruby
module SealedModule
  extend T::Helpers
  sealed!
end

class SealedClass
  extend T::Helpers
  abstract!
  sealed!
end
```

## Approximating algebraic data types

Given sealed classes, Sorbet is powerful enough to model algebraic data types
the same way as other object-oriented languages model them, because it also has
record types (`T::Struct`). An "algebraic data type" is basically a way to say
that a type is composed of a fixed set of cases, and in each case, a fixed
number of fields are available.

For example, here's an example that combines `T::Struct` with `sealed!` modules
to create an algebraic data type representing an invoice, which could be in a
number of different states:

```ruby
module InvoiceState
  extend T::Helpers
  sealed!

  class Drafted < T::Struct
    include InvoiceState
  end

  class Opened < T::Struct
    include InvoiceState
    prop :due_at, Time
  end

  class Paid < T::Struct
    include InvoiceState
    prop :charge, Charge
    prop :paid_at, Time
  end

  class Voided < T::Struct
    include InvoiceState
    prop :voided_at, Time
  end
end

sig {params(invoice_state: InvoiceState).void}
def process_invoice(invoice_state)
  case invoice_state
  when InvoiceState::Drafted
    # ... no special props available!
  when InvoiceState::Opened
    puts invoice_state.due_at
  when InvoiceState::Paid
    puts invoice_state.charge
    puts invoice_state.paid_at
  when InvoiceState::Voided
    puts invoice_state.voided_at
  else
    T.absurd(invoice_state)
  end
end
```

## What's next?

- [Abstract Classes and Interfaces](abstract.md)

  Sealed classes are often combined with some sort of parent interface which has
  common methods that all child classes implement. This can be either an
  abstract parent class or an interface that's included.

- [Final Methods, Classes, and Modules](final.md)

  Final classes and methods are similar to sealed, in that they're both ways to
  limit the extent and effects of inheritance. And in fact, they're often
  combined.
