# typed: true

class Foo
  extend T::Helpers

  interface!
# ^^^^^^^^^^ error: Classes can't be interfaces. Use `abstract!` instead of `interface!`
end
