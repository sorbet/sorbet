# typed: true

class A
  def self.my_singleton_class_method; end

  def example
    my_singleton_class_method # error: does not exist
    self.my_singleton_class_method # error: does not exist
    a = A.new
    a.my_singleton_class_method # error: does not exist
  end
end
