# typed: true
class LetErrors
  def initialize
    @@cvar = T.let(0, Integer) # error: Class variables must be declared at class scope
    @ivar = T.cast(nil, Integer) # error: Use T.let() to specify the type of constants
  end

  def not_initialize
    @a = T.let(0, Integer) # error: Instance variables must be declared inside `initialize`
  end

  @b = T.let("hi", String)
  @b = T.let("foo", String) # explicitly not an error: the type is the same

  @@cv = T.let(:foo, Symbol)
  @@cv = T.let(:bar, Symbol) # explicitly not an error: the type is the same

  @d = T.let("hi", String)
  @d = T.let(0, Integer) # error: Redeclaring variable `@d` with mismatching type

  @@dv = T.let(:foo, Symbol)
  @@dv = T.let(0.0, Float) # error: Redeclaring variable `@@dv` with mismatching type
end
