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
  wrap = Interface.wrap_instance(s) # error: does not exist
  T.reveal_type(wrap) # error: `T.untyped`
  wrap.other_method
  wrap.some_method

  Other.wrap_instance("hi", "there")
  o = Other
  o.wrap_instance("hi")
end
