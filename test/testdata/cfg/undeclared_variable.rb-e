# typed: strict

class A
  extend T::Sig

  @declared = T.let(0, Integer)
  @undeclared = 0 # error: Use of undeclared variable `@undeclared`

  @undefined # error: Use of undeclared variable `@undefined`

  sig {void}
  def self.foo
    T.reveal_type(@declared) # error: Revealed type: `Integer`
    @undeclared # error: Use of undeclared variable `@undeclared`
  end
end
