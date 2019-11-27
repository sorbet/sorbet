# typed: false

class X; end
#     ^ type-def: x-type

class A
  def initialize
    @x = T.let(X.new, X)
    #^ def: x-def
  end

  def foo
    @x
    #^ usage: x-def
    #^ type: x-type
  end
end
