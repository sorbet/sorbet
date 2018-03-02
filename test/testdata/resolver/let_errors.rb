class LetErrors
  def initialize
    @@cvar = T.let(0, Integer) # error: Class variables must be declared at class scope.
  end

  def not_initialize
    @a = T.let(0, Integer) # error: Instance variables must be declared inside `initialize`
  end

  @b = T.let("hi", String)
  @b = T.let("foo", String) # error: Illegal variable redeclaration

  @@cv = T.let(:foo, Symbol)
  @@cv = T.let(:bar, Symbol) # error: Illegal variable redeclaration
end
