# typed: strict

module Interface
  def some_method
  end
end


class SomeClass
  include Interface

  def other_method
  end
end

class Other
  def self.wrap_instance(x, y=nil)
  end
end


def testit
  s = SomeClass.new
  wrap = Interface.wrap_instance(s)
  T.assert_type!(wrap, Interface)
  wrap.other_method # error: does not exist
  wrap.some_method

  Other.wrap_instance("hi", "there") # error: Wrong number of arguments
  o = Other
  o.wrap_instance("hi") # error: Unsupported wrap_instance() on a non-constant-literal
end
