# typed: true

# `Opus::Foo::Baz` is not a package. The only package under it is `Opus::Foo::Baz::Qux`.
module Opus::Foo # error: Package `Opus::Foo::Early` may not open `Opus::Foo`
  module Early
    X = Baz # error: `Opus::Foo` is not imported
    T.reveal_type(X) # error: Revealed type: `T.untyped`
  end
end
