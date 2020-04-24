# typed: true

class BadCustomInitialize < T::Struct
  # Intentionally incompatible redefinition (would have been silent in `typed: false`): 
  def initialize(foo) # error: redefined with argument `foo` as a non-keyword argument
    super(foo: foo)
  end

  const :foo, Integer # error: must be declared inside `initialize`
end

class AttemptToFixBadInitialize < T::Struct
  def initialize(foo) # error: redefined with argument `foo` as a non-keyword argument
    super(foo: foo)
    # Even though the instance variable is annotated here, there's still an error below
    # because the rewriter-synthesized initialize has been mangle renamed to `initialize$1`
    @foo = T.let(@foo, Integer)
  end

  const :foo, Integer # error: must be declared inside `initialize`
end
