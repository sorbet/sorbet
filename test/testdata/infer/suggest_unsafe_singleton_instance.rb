# typed: true
# enable-suggest-unsafe: true
extend T::Sig

class A
  def my_instance_method; end
  def self.my_singleton_class_method; end

  def example1
    my_singleton_class_method # error: Method `my_singleton_class_method` does not exist on `A`
  end
  def self.example2
    my_instance_method # error: Method `my_instance_method` does not exist on `T.class_of(A)`
  end
end

sig {params(a: A).void}
def example3(a)
  a.my_singleton_class_method # error: Method `my_singleton_class_method` does not exist on `A`
end
