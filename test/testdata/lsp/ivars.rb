# typed: strict

class Foo
  extend T::Sig

  sig {void}
  def initialize
    @boo = T.let(3, Integer)
   # ^ def: boo
   # ^ hover: Integer
  end

  sig {returns(Integer)}
  def boo
    @boo
   # ^ usage: boo
   # ^ hover: Integer
    @boo += 1
   # ^ usage: boo
   # ^ hover: Integer
    @boo + 10
   # ^ usage: boo
   # ^ hover: Integer
  end
end

