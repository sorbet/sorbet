# typed: true
class LetErrors
  @@topvar = T.cast(nil, Integer) # error: Use `T.let` to specify the type of class variables

  def initialize
    @@cvar = T.let(0, Integer) # error: The class variable `@@cvar` must be declared at class scope
    @ivar = T.cast(nil, Integer) # error: Use `T.let` to specify the type of instance variables
  end

  def not_initialize
    @a = T.let(0, Integer) # error: The instance variable `@a` must be declared inside `initialize`
  end

  @b = T.let("hi", String)
  @b = T.let("foo", String) # explicitly not an error: the type is the same

  @@cv = T.let(:foo, Symbol)
  @@cv = T.let(:bar, Symbol) # explicitly not an error: the type is the same

  @d = T.let("hi", String) # error: Redeclaring variable `@d` with mismatching type
  @d = T.let(0, Integer)

  @@dv = T.let(:foo, Symbol) # error: Redeclaring variable `@@dv` with mismatching type
  @@dv = T.let(0.0, Float)
end
