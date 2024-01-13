# typed: true

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

  Other.wrap_instance("hi", "there") # error: does not exist
  o = Other
  o.wrap_instance("hi") # error: does not exist
end
