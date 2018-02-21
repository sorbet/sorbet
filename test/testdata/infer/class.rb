# @typed
class TestCase
  def self.class_method
  end

  def instance_method
    self.class.class_method
    self.class.no_such_method # error: does not exist

    # Test that Class's methods work
    self.class.allocate

    T.assert_type!(self.class.new, TestCase)
    T.assert_type!(self.class, TestCase.singleton_class)
    T.assert_type!(self.class, Class)
    T.assert_type!(self.class.class, Class)
  end
end
