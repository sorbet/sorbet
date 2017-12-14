# @typed
module Boolean
  include Kernel
end

class TrueClass
  include Boolean
end

class FalseClass
  include Boolean
end

class TestBoolean
  standard_method({b: Boolean}, returns: Boolean)
  def test_boolean(b)
    "b is: #{b}"
    !b
  end
end
