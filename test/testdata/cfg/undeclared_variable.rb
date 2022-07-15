# typed: strict

class A
  extend T::Sig

  @declared = T.let(0, Integer)
  @undeclared = 0 # error: The instance variable `@undeclared` must be declared using `T.let` when specifying `# typed: strict`

  @undefined # error: Use of undeclared variable `@undefined`

  sig {void}
  def self.foo
    T.reveal_type(@declared) # error: Revealed type: `Integer`
    @undeclared # error: Use of undeclared variable `@undeclared`
  end
end
