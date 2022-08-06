# typed: true

class A
  extend T::Sig

  def self.my_dsl
    define_method(:magic_method) do
      T.reveal_type(self) # error: `A`
      x = returns_integer
      T.reveal_type(x) # error: `Integer`
    end
  end

  define_method(:positional) do |x|
    T.reveal_type(x) # error: `T.untyped`
  end

  define_method(:binary) do |x, y|
    T.reveal_type(x) # error: `T.untyped`
    T.reveal_type(y) # error: `T.untyped`
  end

  define_method(:optional) do |x, y=nil|
    T.reveal_type(x) # error: `T.untyped`
    T.reveal_type(y) # error: `T.untyped`
  end

  define_method(:keyword) do |x:|
    T.reveal_type(x) # error: `T.untyped`
  end

  define_method(:rest) do |*x|
    T.reveal_type(x) # error: `T.untyped`
  end

  define_method(:kwrest) do |**x|
    T.reveal_type(x) # error: `T.untyped`
  end

  define_method(:everything) do |*rest, **kwargs, &blk|
    T.reveal_type(rest) # error: `T.untyped`
    T.reveal_type(kwargs) # error: `T.untyped`
    T.reveal_type(blk) # error: `T.untyped`
  end

  sig {returns(Integer)}
  def returns_integer
    0
  end
end
