# typed: true

module Opus::Foo
  module Bar
    X = Baz # error: `Opus::Foo::Baz` resolves but is not exported from `Opus::Foo`
  end
end
