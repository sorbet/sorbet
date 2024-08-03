# typed: true
class TestCase
  def self.class_method
  end

  def instance_method
    self.class.class_method
    self.class.no_such_method # error: does not exist

    # Test that Class's methods work
    self.class.allocate

    T.reveal_type(self.class.new) # error: `TestCase`
    T.reveal_type(self.class) # error: `T.class_of(TestCase)`
    T.reveal_type(self.class.class) # error: `T::Class[T.class_of(TestCase)]`
  end
end

T.reveal_type(0.class) # error: `T.class_of(Integer)[Integer(0)]`
T.reveal_type([0, 1].class) # error: `T.class_of(Array)[[Integer(0), Integer(1)]]`
