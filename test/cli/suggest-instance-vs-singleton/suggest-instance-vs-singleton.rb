# typed: true

class A
  def my_instance_method; end
  def self.my_singleton_class_method; end

  def example1
    my_singleton_class_method
  end
  def self.example2
    my_instance_method
  end
end
