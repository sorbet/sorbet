---
id: non-forcing-constants
title: Non-Forcing Constants with T::NonForcingConstants
sidebar_label: T::NonForcingConstants
---

> **Note**: This is an advanced feature of Sorbet. It's more likely that you're
> looking about how Sorbet handles `is_a?` using [Flow-Sensitive
> Typing](flow-sensitive.md). Please read carefully before using this feature.

This is a variant of `is_a?` that doesn't force its class argument to be
loaded.


```ruby
class Outer
  autoload :Nested, './expensive_file_to_load.rb'
end

# ...

def foo(x)
  if T::NonForcingConstants.non_forcing_is_a?(x, '::Outer::Nested')
    # ...
  end

  # The above can be better than this, which will cause
  # `./expensive_file_to_load.rb` to load:
  if x.is_a?(::Outer::Nested)
    # ...
  end
end
```

The idea is basically: if a constant like `::Outer::Nested` hasn't loaded (e.g.,
because it hasn't been referenced yet), then certainly no instances of
`::Outer::Nested` can exist yet, so the instance of check will short circuit and
return `false`. If `::Outer::Nested` **has** loaded, the `is_a?` check will be
carried out like normal.

This method should **only** be used when there's a measurable performance
benefit from using it. This method is rarely used, which means that people
will not be familiar with it when they see it. This makes code harder to read
and reason about. Only use this method **sparingly** and when there's proof that
it speeds a certain code path up.

## Usage

The `klass` given as a string must be an absolute constant reference (starting
with `::`). Even though the argument is a string, the constant that the string
refers to must still exist from Sorbet's perspective of the whole codebase. This
means it will be an error if there's a typo in the string or the constant is
renamed in the future.

```ruby
class MyClass; end

T::NonForcingConstants.non_forcing_is_a?(
  nil,
  '::MyClas' # error: Unable to resolve constant `::MyClas`
)
```

<a href="https://sorbet.run/#%23%20typed%3A%20true%0Aclass%20MyClass%3B%20end%0A%0AT%3A%3ANonForcingConstants.non_forcing_is_a%3F(%0A%20%20nil%2C%0A%20%20'%3A%3AMyClas'%20%23%20error%3A%20Unable%20to%20resolve%20constant%20%60%3A%3AMyClas%60%0A)">
  → View on sorbet.run
</a>

While Sorbet can see whether the constant exists or not statically, it cannot do
so at runtime (since it does not force constants). This means that `is_a?` and
`non_forcing_is_a?` can have different behaviors at runtime when the constant
doesn't exist, which can cause subtle bugs if misused:

```ruby
val.is_a?(::DoesntExist) # NameError (uninitialized constant DoesntExist)

T::NonForcingConstants.non_forcing_is_a?(val, '::DoesntExist') # => false
```

In this case, only Sorbet's static check that the constant in the string
literal must resolve prevents this situation.
