# typed: true
class Foo; extend T::Sig
    include Baz
    sig {returns(Integer)}
    def bar
        1
    end
end

module Baz; extend T::Sig
  sig {returns(Integer)}
  def quux
    1
  end
end

foo = Foo.new
foo.something # error: Method `something` does not exist on `Foo`
      # ^ hover: (nothing)
