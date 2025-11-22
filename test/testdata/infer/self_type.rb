# typed: true
class Parent
  extend T::Sig

  sig {returns(T.self_type)}
  def returns_self
    self
  end
end
class Normal < Parent
end

class Generic < Parent
  extend T::Generic
  TM = type_member()

  sig {returns(Generic[T.self_type])}
  def good
    if [true, false].sample
      return Generic[T.self_type].new
    else
      return Generic[Parent].new
    # ^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Expected `Generic[T.self_type (of Generic[Generic::TM])]` but found `Generic[Parent]` for method result type
    end
  end
end

T.assert_type!(Normal.new.returns_self, Normal)
T.assert_type!(Generic[String].new.returns_self, Generic[String])

module B
end

a = Generic[String].new
if a.is_a?(B)
  T.assert_type!(a.returns_self, T.all(Generic[String], B))
end

class Array
    extend T::Sig

    sig {returns(T.self_type)}
    def returns_self
      self
    end
end

T.assert_type!([1, 2].returns_self, [Integer, Integer])

class A
end

module B
  extend T::Sig

  sig {returns(T.self_type)}
  def returns_self
     self
  end

  sig { params(x: T.self_type).void }
  #            ^ error: `T.self_type` may only be used in an `:out` context, like `returns`
  def takes_self_public(x)
    x
  end

  sig { params(x: T.self_type).void }
  private def takes_self_private(x)
    T.reveal_type(x) # error: `T.self_type (of B)`
    x
  end

  sig { params(blk: T.proc.params(arg0: T.self_type).void).void }
  def yield_self_void(&blk)
    yield self
  end

  sig { params(x: T.any(T.self_type, Integer)).void }
  private def takes_self_or_integer(x)
    T.reveal_type(x) # error: `T.any(Integer, T.self_type (of B))`
    x
  end

  sig { params(other: T.self_type).void }
  #            ^^^^^ error: `T.self_type` may only be used in an `:out` context, like `returns`
  def ==(other)
    return self.class == other.class
    #           ^^^^^ error: Method `class` does not exist on `B`
    #                          ^^^^^ error: Method `class` does not exist on `B`
  end
end

s = A.new

def rnd
end

while (rnd)
  if s.is_a?(B)
    s = s.returns_self
  end
end

puts s
