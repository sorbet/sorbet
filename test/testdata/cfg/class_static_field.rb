# typed: true

class A
  extend T::Sig

  @@x = T.let(nil, T.nilable(String))

  sig {returns(String)}
  def self.foo
    @@x # error: Expected `String` but found `T.nilable(String)` for method result type
  end
end
