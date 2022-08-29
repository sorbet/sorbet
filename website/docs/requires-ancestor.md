---
id: requires-ancestor
title: Requiring Ancestors
sidebar_label: Requiring Ancestors
---

> This feature is experimental and might be changed or removed without notice.
> To enable it pass the `--enable-experimental-requires-ancestor` option to
> Sorbet or add it to your `sorbet/config`.

It's not uncommon in Ruby to define helper modules that depends on other
modules. For example, let's take the following helper which provides `say_error`
method:

```ruby
# typed: true

module MyHelper
  def say_error(message)
    raise "InternalError: #{message}" # error: Method `raise` does not exist on `MyHelper`
  end
end

class MyClass
  include MyHelper

  def do_something(x)
    say_error("some error") unless x
    # ...
  end
end
```

If we run Sorbet on this example, we will get a type-checking error saying that
the method `raise` does not exist on `MyHelper` since this method is defined on
`Kernel`. Thanks to this error, Sorbet is protecting us against some edge-cases
where we would try to include the `MyHelper` module in a class that does not
include `Kernel`:

```ruby
class MyBaseClass < BasicObject
  include MyHelper

  def do_something(x)
    say_error("some error") unless x
    # ...
  end
end

MyBaseClass.new.do_something(false) # runtime-error: in `say_error': undefined method `raise' for #<MyBaseClass> (NoMethodError)
```

This example would raise an error at runtime because the method `raise` is
undefined for instances of `MyBaseClass` as it doesn't include `Kernel`.

## Requiring Ancestors

Sorbet provides the `requires_ancestor` method as a way to ensure that classes
or modules including `MyHelper` will also include `Kernel`.

Let's change our base example to use `requires_ancestor`:

```ruby
module MyHelper
  extend T::Helpers

  requires_ancestor { Kernel }

  def say_error(message)
    raise "InternalError: #{message}"
  end
end
```

This way we specify that any module including `MyHelper` must also include
`Kernel` and Sorbet will display an error if it's not the case:

```ruby
class MyBaseClass < BasicObject # error: `MyBaseClass` must include `Kernel` (required by `MyHelper`)
  include MyHelper
end
```

`requires_ancestor` also works to require that a specific class must be
inherited:

```ruby
module MyHelper
  extend T::Helpers

  requires_ancestor { Object }

  def class_name
    self.class.name
  end
end

class MyBaseClass < BasicObject # error: `MyBaseClass` must inherit `Object` (required by `MyHelper`)
  include MyHelper
end
```

Note that requirements are transitive:

```ruby
class MyBaseClass2 < MyBaseClass # error: `MyBaseClass2` must inherit `Object` (required by `MyHelper`)
  include MyHelper
end
```

`requires_ancestor` can be used to require more than one ancestor:

```ruby
module Test
  module TestAssertions
    def assert_equal(x, y)
      x == y
    end
  end

  class TestBase
    def test; end
  end

  class TestCase < TestBase
    include TestAssertions
  end
end

module MyLogger
  def log_test_failed; end
end

module MyTestHelper
  extend T::Helpers

  requires_ancestor { Test::TestAssertions }
  requires_ancestor { MyLogger }

  def assert_not_equal(x, y)
    if assert_equal(x, y)
      true
    else
      log_test_failed
      false
    end
  end
end

class MyValidTest < Test::TestCase
  include MyTestHelper
  include MyLogger
end

class MyBrokenTest < Test::TestBase # error: `MyBrokenTest` must include `Test::TestAssertions` (required by `MyTestHelper`)
                                    # error: `MyBrokenTest` must include `MyLogger` (required by `MyTestHelper`)
  include MyTestHelper
end
```

`requires_ancestor` can also be used to require a singleton class as an
ancestor:

```
module MyHelper
  extend T::Helpers

  requires_ancestor { T.class_of(MyBaseClass) }

  def helper
    my_singleton_method
  end
end

class MyBaseClass
  class << self
    include MyHelper

    def my_singleton_method; end
  end
end
```
